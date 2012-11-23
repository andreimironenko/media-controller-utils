/*
 *  Copyright (c) 2010-2011, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 *
 */


/**
 *******************************************************************************
 *  @file  ducati_mmsw_main_vidm3.c
 *  @brief This file contains supporting implementation for Davinci System Top
 *
 *  @rev 0.1
 *******************************************************************************
 */

/*******************************************************************************
*                             Compilation Control Switches
*******************************************************************************/
/* None */

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/* -------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <xdc/std.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/BIOS.h>
#include <ti/ipc/MultiProc.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/ipc/SharedRegion.h>
#include <sys_top.h>
/*-------------------------program files --------------------------------------*/
/* None */

/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/*-----------------------data declarations -----------------------------------*/
/* None */

/*--------------------- function prototypes ----------------------------------*/

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*---------------------data declarations -------------------------------------*/
/* None */

/*---------------------function prototypes -----------------------------------*/

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/

/*---------------------- function prototypes ---------------------------------*/
/* None */
void
update_M3DstHdrInfo (dst_HeaderInfo * pHdrInfo)
{
  pHdrInfo->running = 1;
  pHdrInfo->verNumMajor = SYSTOP_VERSION_NUM_MAJOR;
  pHdrInfo->verNumMinor = SYSTOP_VERSION_NUM_MINOR;
  pHdrInfo->verNumRevision = SYSTOP_VERSION_NUM_REVISION;
  pHdrInfo->verNumStep = SYSTOP_VERSION_NUM_STEP;
}

void dst_initSysInfo(dstInfo *pSysInfo)
{
  uint32_t           multiProcId    = 0;
  dst_CoresHeapInfo *pCoresHeapInfo = NULL;
  dst_SRInfo        *pSrInfo        = NULL;
  uint32_t           i              = 0;

  multiProcId = MultiProc_self ();
  if (0 == multiProcId)
  {
    pCoresHeapInfo  = &pSysInfo->dspInfo.heapInfo;
    pSrInfo = &pSysInfo->dspInfo.srInfo;
  }
  if ( 1 == multiProcId)
  {
    pCoresHeapInfo  = &pSysInfo->vm3Info.heapInfo;
    pSrInfo  = &pSysInfo->vm3Info.srInfo;
  }
  else if (2 == multiProcId)
  {
    pCoresHeapInfo  = &pSysInfo->dm3Info.heapInfo;
    pSrInfo  = &pSysInfo->dm3Info.srInfo;
  }
  else if (3 == multiProcId)
  {
    // TBD
  }

  /* Init Cores Heap Info */
  pCoresHeapInfo->numHeaps = 0;
  for (i = 0; i <DAVINCI_STATUS_MAX_HEAP; i++) {
    pCoresHeapInfo->heapInfo[i].maxUsed = 0;
  }

  /* Init SR heap Info */
  pSrInfo->numSRs = 0;
  for (i = 0; i <DAVINCI_STATUS_MAX_HEAP; i++) {
    pSrInfo->srEntries[i].heapInfo.maxUsed = 0;
  }

}

dst_cpyMemStat2StHeap(dst_HeapInfo *pStHeapInfo, Memory_Stats *pMemStat)
{
  pStHeapInfo->totalSize = pMemStat->totalSize;
  pStHeapInfo->totalFreeSize = pMemStat->totalFreeSize;
  pStHeapInfo->largestFreeSize = pMemStat->largestFreeSize;

  if (pStHeapInfo->maxUsed < (pStHeapInfo->totalSize - pStHeapInfo->totalFreeSize)) 

  {
    pStHeapInfo->maxUsed = (pStHeapInfo->totalSize - pStHeapInfo->totalFreeSize);
  }
}

void
sysTop_process ()
{
  uint8_t status = TRUE;
  dstInfo *pDavinciSysInfo;
  int numSRs = 0;
  int i = 0;
  int j = 0;
  uint16_t myMultiProcId = 0;
  SharedRegion_Entry ipcSrEntry;
  IHeap_Handle srHeap;
  dst_SREntry *pDstSrEntry = NULL;
  dst_HeapInfo *pDstHeapInfo;
  dst_M3Info *pM3Info = NULL;
  dst_dspInfo *pDspInfo = NULL;
  dst_HeaderInfo *pHdrInfo = NULL;
  
  Memory_Stats srHeapStats;

  pDavinciSysInfo = (dstInfo *) (LDR_CONFIG_ADDR_MEMCFG_BASE + 
                                 LDR_CONFIG_ADDR_MEMCFG_SIZE + 
                                 LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE);

  myMultiProcId = MultiProc_self ();

  if (LDR_CORE_ID_DSP == myMultiProcId)
  {
    pDspInfo = &pDavinciSysInfo->dspInfo;    
  }
  else if (LDR_CORE_ID_VM3 == myMultiProcId)
  {
    pM3Info = &pDavinciSysInfo->vm3Info;
  }
  else if (LDR_CORE_ID_DM3 == myMultiProcId)
  {
    pM3Info = &pDavinciSysInfo->dm3Info;
  }
  else {
    status = FALSE;
  }

  /* Initialize the dst information */
  while (TRUE == status)
  {
    Task_sleep (1000);
    update_M3DstHdrInfo (&pDspInfo->hdrInfo);
    /* Retrieve default Heap info */
    Memory_getStats (NULL, &srHeapStats);
    pDstHeapInfo = &pDspInfo->heapInfo.heapInfo[0];
    pDstHeapInfo->valid = 1;

    dst_cpyMemStat2StHeap(pDstHeapInfo, &srHeapStats);


    pDspInfo->heapInfo.heapInfo[1].valid = 0;

    /* Retrieve SR0 info */
    numSRs = SharedRegion_getNumRegions ();

    pDspInfo->srInfo.numSRs = numSRs;

    j = 0;
    for (i = 0; i < numSRs; i++)
    {

      SharedRegion_getEntry (i, &ipcSrEntry);

      if (TRUE == ipcSrEntry.isValid)
      {

        pDstSrEntry = &pDspInfo->srInfo.srEntries[j];
        pDstSrEntry->isValid = 1;
        pDstSrEntry->srIndex = i;
        pDstSrEntry->phyBaseAddr = (unsigned int) ipcSrEntry.base;
        pDstSrEntry->size = ipcSrEntry.len;

        if (TRUE == ipcSrEntry.createHeap)
        {
          pDstSrEntry->isHeap = 1;
          pDstHeapInfo = &pDstSrEntry->heapInfo;

          srHeap = SharedRegion_getHeap (i);

          Memory_getStats (srHeap, &srHeapStats);
          dst_cpyMemStat2StHeap(pDstHeapInfo, &srHeapStats);
        }
        j++;
      }
    }
  }
}

/* End Of File */

