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
 *  @file  main.c
 *  @brief This file contains platform (A8) specific initializations and 
 *         the main () of the test application.
 *
 *  @rev 1.0
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdio.h>
#include <xdc/runtime/knl/Thread.h>

/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "ilclient.h"
#include "ilclient_utils.h"
#include "domx_util.h"
#include "msgq.h"
#include "ldr_memseg.h"
#include "sys_top.h"
#include <ti/syslink/SysLink.h>
#include <ti/syslink/IpcHost.h>
#include <ti/syslink/ProcMgr.h>
#include <ti/ipc/SharedRegion.h>
/*-------------------------program files --------------------------------------*/
/* None */

/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/*-----------------------data declarations -----------------------------------*/

/*--------------------- function prototypes ----------------------------------*/
extern void platform_init ();

extern void platform_deinit ();

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

/**
 *  @brief      system information region file descriptor
 *
 *  @sysTop_regFd
 */
int sysTop_regFd = 0;

/**
 *  @brief      Point to system information region 
 *
 *              This is the user virtual address
 *
 *  @sysTop_regUserVirtAddr
 */
char *sysTop_regUserVirtAddr = NULL;

/**
 *  @brief      sy_top module task id
 *
 *  @sysTop_thId
 */
pthread_t sysTop_thId;

/**
 *  @brief      sy_top module task attribute
 *
 *  @sysTop_thId
 */
pthread_attr_t sysTop_thAttr;

/**
 *  @brief      sy_top execution status notification flag
 *
 *              when the API sysTop_delete called, this API 
 *              indicate the stop to sys_top task through this flag
 *
 *  @sysTop_run
 */
uint8_t  sysTop_run = FALSE; 

/**
 *  @brief      sy_top initialzied flag
 *
 *              Used to protect multiple create delete calls
 *
 *  @sysTop_run
 */
uint8_t  sysTop_initialized = FALSE; 


/**
 *  @brief      Map the system info region to user virtual space
 *
 *  @param[in]  baseAddr        base address of  the status shared memory
 *  @param[in]  size            Size of the status address
  *
 *  @retval     NULL            An error has occurred.
 *  @retval     non-NULL        User virtual address of mapped region
 *
 *  @pre        @c baseAddr is a valid system address & not overlapping with 
 *                 other modules
 *
 *  @pre        @c size is a non zero value
 *
 *  @sa         sysTop_unmapSysInfoRegion()
 */
static char *sysTop_mapSysInfoRegion (unsigned int baseAddr,
                                     unsigned int size)
{
  int dstErrCode = DST_SUCCESS;

  char *mapBase = NULL;

  sysTop_regFd = open ("/dev/mem", O_RDWR | O_SYNC);
  if (sysTop_regFd == -1)
  {
    printf ("Could not open the mem file \n");
    dstErrCode = DST_FAIL;
  }

  mapBase =
    mmap (0, size, PROT_READ | PROT_WRITE, MAP_SHARED, sysTop_regFd, baseAddr);

  if (mapBase == (void *) -1)
  {
    printf ("Could not map System Status Region \n");
    sysTop_regUserVirtAddr = NULL;
  }
  else
  {
    sysTop_regUserVirtAddr = (char *) (mapBase);
  }

  return sysTop_regUserVirtAddr;
}

/**
 *  @brief      UnMap system info space in DDR
 *
 *  @param      pMemCfg   User space pointer system info space 
 *
 *  @retval     TRUE      On Success
 *              FALSE     On Failure
 *
 *  @pre        @c already Mapped using sysTop_mapSysInfoRegion
 *
 *  @sa         sysTop_mapSysInfoRegion
 */
uint8_t
sysTop_unmapSysInfoRegion (uint8_t * pMemCfg, uint32_t size)
{
  uint8_t retVal = TRUE;

  printf ("Entering memcfg_unMapMemCfgSpace(0x%x)\n", (uint32_t) pMemCfg);

  if ((NULL == pMemCfg) || (sysTop_regFd == (int32_t) NULL))
  {
    retVal = FALSE;
  }

  if (TRUE == retVal)
  {

    munmap (pMemCfg, size);

    close (sysTop_regFd);
  }

  Log_print1 (Diags_EXIT, "Leaving memcfg_unMapMemCfgSpace, retVal: %d\n", 
              retVal);

  return retVal;
}

/**
 *  @brief      Update the Header information
 *
 *  @param      pHdrInfo   Points to Header info 
 *
 *  @retval     void
 */
void
sysTop_updateHdr (dst_HeaderInfo * pHdrInfo)
{
  pHdrInfo->running = 1;
  pHdrInfo->verNumMajor = SYSTOP_VERSION_NUM_MAJOR;
  pHdrInfo->verNumMinor = SYSTOP_VERSION_NUM_MINOR;
  pHdrInfo->verNumRevision = SYSTOP_VERSION_NUM_REVISION;
  pHdrInfo->verNumStep = SYSTOP_VERSION_NUM_STEP;
}

/**
 *  @brief      Update the Header information
 *
 *  @param      pHdrInfo   Points to Header info 
 *
 *  @retval     void
 *
 *  @sa         sysTop_mapSysInfoRegion
 */
void sysTop_taskFunc (void *threadsArg)
{
  uint8_t     *mem_cfg_ptr = NULL;
  dst_a8Info  *pA8Info = NULL;
  SharedRegion_Entry ipcSrEntry;
  dst_SREntry *pDstSrEntry = NULL;
  dstInfo *pSysInfo = NULL;
  uint32_t numSRs = 0;
  uint32_t j = 0;
  uint32_t i = 0;

  /* Map memory segment configuration space in DDR */
  mem_cfg_ptr = sysTop_mapSysInfoRegion(LDR_CONFIG_ADDR_SYSINFO_BASE,
                                       LDR_CONFIG_ADDR_SYSINFO_SIZE);

  if (NULL == mem_cfg_ptr)
  {
    printf ("Failed to map System Info Region \n");
  }

  pSysInfo = (dstInfo *) (mem_cfg_ptr);
  pA8Info = &pSysInfo->a8Info;
  while (TRUE == sysTop_run)
  {

    sysTop_updateHdr (&pA8Info->hdrInfo);

        /* Retrieve SR0 info */
    numSRs = SharedRegion_getNumRegions ();

    pA8Info->srInfo.numSRs = numSRs;

    j = 0;
    for (i = 0; i < numSRs; i++)
    {

      SharedRegion_getEntry (i, &ipcSrEntry);

      if (TRUE == ipcSrEntry.isValid)
      {

        pDstSrEntry = &pA8Info->srInfo.srEntries[j];
        pDstSrEntry->isValid = 1;
        pDstSrEntry->srIndex = i;
        pDstSrEntry->phyBaseAddr = 0x0;
        pDstSrEntry->virtBaseAddr = (unsigned int) ipcSrEntry.base;
        
        pDstSrEntry->size = ipcSrEntry.len;

        if (TRUE == ipcSrEntry.createHeap)
        {
          printf ("Heap info can not be retrieved\n");
        }
        j++;
      }
    }
    
    /* Refresh after a sec */
    usleep ( 1000 * 1000 * 1);
  }
    
  if (FALSE == sysTop_unmapSysInfoRegion(mem_cfg_ptr, 
               LDR_CONFIG_ADDR_SYSINFO_SIZE))
  {
     printf ("Unable to unmap system info region\n");
  }

}

/**
 *  @brief      Create sys_top module
 * 
 *              This is the first API needs to be called by 
 *              Application, this creates the sys_top task &
 *              runs the client side of sys_top
 *
 *  @param      void
 *
 *  @retval     void
 *
 *  @sa         sysTop_delete
 */
void sysTop_create()
{
  if (FALSE == sysTop_initialized) 
  {
    pthread_attr_init (&sysTop_thAttr);  

    sysTop_run = TRUE;
    if (0 != pthread_create (&sysTop_thId,
                             &sysTop_thAttr,
                             sysTop_taskFunc, NULL))
    {
      printf ("sys_top task create failed");
    }

    sysTop_initialized = TRUE;
    printf ("sys_top task created \n "); 
  }
}

/**
 *  @brief      Deletes sys_top module
 * 
 *              This is the last API needs to be called by 
 *              Application, this deletes sys_top task
 *
 *  @param      void
 *
 *  @retval     void
 *
 *  @sa         sysTop_create
 */
void sysTop_delete()
{
  int32_t pthreadErrCode = 0;

  if (TRUE == sysTop_initialized)
  {  
    /* Notify sys_top thread to stop */
    sysTop_run = FALSE;

    /* Wait for sys_top thread to exit */
    pthread_join(sysTop_thId, &pthreadErrCode);

    sysTop_initialized = FALSE;
  }
}

/** 
********************************************************************************
 *  @fn     main
 *  @brief  This function does the platform specific initialization and just 
 *          sleeps. The test application that runs in the vpss-m3 instantiates
 *          components such as VSNK (proxy the video encoder) and VSRC (proxy 
 *          for decoder and tunnels the data to and from those components 
 *          respectively.
 * 
 *  @param[in ]  arg1  : FrameWidth
 *  @param[in ]  arg2  : nFrameHeight
 *  @param[in]   arg3  : Input File Name
 * 
 *  @returns none 
********************************************************************************
*/
int main (int argc, char *argv[])
{
  IL_ARGS args;
  ConfigureUIA uiaCfg;

  parse_args (argc, argv, &args);

  printf (" Decoder-Display example \n");
  printf ("===============================\n");

  /* do omx_init for sytem level initialization */
  /* Initializing OMX core , functions related to platform specific
     initialization could be placed inside this */

  /* run sys_top module */
  sysTop_create();

  OMX_Init ();

  printf (" OMX_Init completed \n ");

  /* UIA is an utility to get the logs from firmware on Linux terminal, and also
     to use System Analyzer, Following UIA configuration is required to change 
     the default debug logging configuration, and is optional  */

  /* Configuring logging options on slave cores */
  /* can be 0 or 1 */
  uiaCfg.enableAnalysisEvents = 0;
  /* can be 0 or 1 */
  uiaCfg.enableStatusLogger = 1;
  /* can be OMX_DEBUG_LEVEL1|2|3|4|5 */
  uiaCfg.debugLevel = OMX_DEBUG_LEVEL1;
  /* configureUiaLoggerClient( COREID, &Cfg); */
  configureUiaLoggerClient(2, &uiaCfg);
  configureUiaLoggerClient(1, &uiaCfg);

  /* OMX IL client for decoder component */
  Decode_Display_Example (&args);

  /* de initialze the DOMX / memory configuration */
  
  OMX_Deinit();

  /* stop sys_top module */
  sysTop_delete();
  
  exit (0);
}                               /* main */

/* main.c - EOF */
