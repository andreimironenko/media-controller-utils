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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _LOCAL_CORE_a8host_
#include <unistd.h>
#endif /* _LOCAL_CORE_a8host_ */

#include <xdc/std.h>

#include <ti/syslink/SysLink.h>
#include <ti/syslink/IpcHost.h>
#include <ti/syslink/ProcMgr.h>
/*-------------------------program files --------------------------------------*/
#include <mem_setup.h>


#include <xdc/runtime/IHeap.h>

/*------------------------ Log Related Files ----------------------------------*/
#ifndef _LOCAL_CORE_a8host_
  #include <xdc/runtime/Log.h>
  #include <xdc/runtime/Assert.h>
  #include <xdc/runtime/Diags.h>
#else
  /* Use Slog at A8 side */
  #if (USE_SLOG_PRINT == 1)
    #include <Log.h>
    #include <LoggerSys.h>

    /* Every module needs to define unique module id */
    #ifndef SLOG_LDRMEMCFG_Module__MID
    #define SLOG_LDRMEMCFG_Module__MID (0x8000)
    #endif
    #undef Module__MID
    #define Module__MID SLOG_LDRMEMCFG_Module__MID
  #else
    #include <xdc/runtime/Log.h>
    #include <xdc/runtime/Assert.h>
    #include <xdc/runtime/Diags.h>
  #endif /* End Of #if (USE_SLOG_PRINT == 1) */
#endif /* End of #else */

#ifndef _LOCAL_CORE_a8host_
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#endif

/*-------------------------- Multi Proc ---------------------------------------*/
#include <ti/ipc/MultiProc.h>
/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/*-----------------------data declarations -----------------------------------*/
/* None */

/*--------------------- function prototypes ----------------------------------*/
/* None */

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*---------------------data declarations -------------------------------------*/
/* None */

/*---------------------function prototypes -----------------------------------*/
#ifdef _LOCAL_CORE_a8host_
static MemCfg_Error memstp_unmapPhyAddr (UInt32 phyAddr, UInt32 len);
#else
MemCfg_Error memstp_unmapPhyAddr (UInt32 phyAddr, UInt32 len);
#endif

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/
/*---------------------data declarations -------------------------------------*/
/*! @var
 *  @brief Pool of Heap handles
 */
static memcfg_HeapInfo g_memcfg_heapInfo[LDR_MAX_MEMSEG];

/*! @var
 *  @brief Pool of SR Entries for the application
 */
static memcfg_SrInfo g_memcfg_srInfo[LDR_MAX_MEMSEG];


#ifdef _LOCAL_CORE_a8host_
static ProcMgr_Handle pSlaveProcHandle = NULL;
#endif

/*--------------------------- macros  ----------------------------------------*/

/*---------------------- function prototypes ---------------------------------*/

static MemCfg_Error memstp_createSRsHeaps (uint8_t *pMemCfg,
                                LDR_MemSeg *pAppReq,
                                memcfg_HeapInfo pHeapInfo[],
                                memcfg_SrInfo pSrInfo[]);
static MemCfg_Error memstp_deleteSRsHeaps (uint8_t *pMemCfg,
                                memcfg_HeapInfo pHeapInfo[],
                                memcfg_SrInfo pSrInfo[]);

/*---------------------- function definitions --------------------------------*/
/**
 *  @brief      Setup Memory configurations by creating Shared regions and
 *              Heaps
 *
 *              This is the function to be called by application
 *
 *  @param[in]  pMemCfgSpace   Points to MEMCFG_SPACE
 *
 *  @param[in]  pAppMegSeg     Application Memory seg requirement
 * 
 *  @retval     MemCfg_ErrorNone if success
 *
 *              !MemCfg_ErrorNone    if fails
 *
 *  @pre        @c pMemCfgSpace must be populated with right memory 
 *                 configuration if non NULL passed
 *
 *  @sa         ldrmemcfg_deleteSRsHeaps
 */
MemCfg_Error ldrmemcfg_createSRsHeaps(uint8_t *pMemCfgSpace, 
                                      LDR_MemSeg *pAppMegSeg)
{
  MemCfg_Error ret_val = MemCfg_ErrorNone;

  /* If application passes the pointer use that otherwise use the
   * default MEMCFG_SPACE system address 
   * DSP, Media controller passes NULL
   * Linux passes the user virtual address
   */
  if (NULL == pMemCfgSpace)
  {
    pMemCfgSpace = (uint8_t *) LDR_CONFIG_ADDR_MEMCFG_BASE;
  }

  ret_val = memstp_createSRsHeaps(pMemCfgSpace, pAppMegSeg, 
                                  g_memcfg_heapInfo, g_memcfg_srInfo);

  return ret_val;
}

/**
 *  @brief      delete Memory configurations by deleting Shared regions and
 *              Heaps
 *
 *              This is the function to be called by application
 *
 *  @param[in]  pMemCfgSpace   Points to MEMCFG_SPACE
 *
 *  @param[in]  pAppMegSeg     Application Memory seg requirement
 * 
 *  @retval     MemCfg_ErrorNone if success
 *
 *              !MemCfg_ErrorNone    if fails
 *
 *  @pre        @c pMemCfgSpace must be populated with right memory 
 *                 configuration if non NULL passed
 *
 *  @sa         ldrmemcfg_createSRsHeaps
 */
MemCfg_Error ldrmemcfg_deleteSRsHeaps(uint8_t *pMemCfgSpace)
{
  MemCfg_Error ret_val = MemCfg_ErrorNone;

  /* If application passes the pointer use that otherwise use the
   * default MEMCFG_SPACE system address 
   * DSP, Media controller passes NULL
   * Linux passes the user virtual address
   */
  if (NULL == pMemCfgSpace)
  {
    pMemCfgSpace = (uint8_t *) LDR_CONFIG_ADDR_MEMCFG_BASE;
  }

  ret_val = memstp_deleteSRsHeaps(pMemCfgSpace, 
                                  g_memcfg_heapInfo, g_memcfg_srInfo);

  return ret_val;
}

#ifndef _LOCAL_CORE_a8host_
/**
 *  @brief      Retrieve the HeapHandle by name
 *
 *  @param[in]  pMemSeg          Points to list of memory segments, Currently unused
 *
 *  @param[in & out] pHeapInfo   List of Heap Info
 * 
 *  @param[in & out] pSrInfo     List of SR Info
 *
 *  @retval     non NULL heap handle if success
 *
 *              NULL    if fails
 *
 *  @pre        @c memstp_createSRsHeaps is been invoked and returns success
 *
 *  @sa         memstp_createSRsHeaps
 *  @sa         memstp_createHeap
 *  @sa         memstp_createSr
 */
HeapMem_Handle memstp_getHeapHdlByName (uint8_t *pHeapName)
{
  uint32_t idx = 0;
  HeapMem_Handle heapHdl = NULL;

  Log_print1 (Diags_ENTRY, 
              "Entering memstp_getHeapHdlByName(%s)\n", (xdc_IArg) pHeapName);

  /* Loop through global pool of Heaps */
  for (idx = 0; idx < LDR_MAX_MEMSEG; idx++)
  {
    if (strcmp ((const char *) pHeapName, (const char *) g_memcfg_heapInfo[idx].name) == 0)
    {
      heapHdl = g_memcfg_heapInfo[idx].hdl;
    } /* End of if block if (strcmp(pHeapName,
         g_memcfg_heapInfo[idx].name) == 0) */
  } /* End of for (idx = 0; idx < LDR_MAX_MEMSEG; idx++) */

  Log_print1 (Diags_EXIT, "Leaving memstp_getHeapHdlByName > ret_val: 0x%x\n", 
              (uint32_t) heapHdl);

  return heapHdl;
}

/**
 *  @brief      Retrieve the system addr by name
 *
 *  @param[in]  pMemSeg          Points to memory segments
 *  @param[in]  pHeapName        Points to the heap name
 *
 *  @retval     System address
 *
 *              NULL    if fails
 *
 *  @pre        @c memstp_createHeap is been invoked and returns success
 *
 *  @sa         memstp_createSrsHeaps
 *  @sa         memstp_createHeap
 *  @sa         memstp_createSr
 */
uint32_t memstp_getSystemAddrByName (void *ldr_memseg,
                                      uint8_t *pHeapName)
{
  uint32_t idx = 0;
  uint32_t system_addr = NULL;
  LDR_MemSeg *pMemSegEntry = NULL;

  /* If application passes the pointer use that otherwise use the
   * default MEMCFG_SPACE system address 
   * DSP, Media controller passes NULL
   * Linux passes the user virtual address
   */
  if (NULL == ldr_memseg)
  {
    ldr_memseg = (uint8_t *) (LDR_CONFIG_ADDR_MEMCFG_BASE + 
                                            sizeof (LDR_Memseg_Version_Hdr));
  }
  
  pMemSegEntry = (LDR_MemSeg *) ldr_memseg;

  /* Loop through global pool of Heaps */
  for (idx = 0; idx < LDR_MAX_MEMSEG; idx++)
  {
    if (strcmp ((const char *) pHeapName, (const char *) pMemSegEntry[idx].name) == 0)
    {
      system_addr = pMemSegEntry[idx].system_addr;
    } /* End of if block if (strcmp(pHeapName,
         g_memcfg_heapInfo[idx].name) == 0) */
  } /* End of for (idx = 0; idx < LDR_MAX_MEMSEG; idx++) */

  return system_addr;
}


/**
 *  @brief      Retrieve the Vitual addr by name
 *
 *  @param[in]  pMemSeg          Points to memory segments
 *  @param[in]  pHeapName        Points to the heap name
 *
 *  @retval     Virtual address used by slave
 *
 *              NULL    if fails
 *
 *  @pre        @c memstp_createHeap is been invoked and returns success
 *
 *  @sa         memstp_createSrsHeaps
 *  @sa         memstp_createHeap
 *  @sa         memstp_createSr
 */
uint32_t memstp_getVirtualAddrByName (void *ldr_memseg,
                                      uint8_t *pHeapName)
{
  uint32_t idx = 0;
  uint32_t virtualAddr = NULL;
  LDR_MemSeg *pMemSegEntry = NULL;

  /* If application passes the pointer use that otherwise use the
   * default MEMCFG_SPACE system address 
   * DSP, Media controller passes NULL
   * Linux passes the user virtual address
   */
  if (NULL == ldr_memseg)
  {
    ldr_memseg = (uint8_t *) (LDR_CONFIG_ADDR_MEMCFG_BASE + 
                                            sizeof (LDR_Memseg_Version_Hdr));
  }
  
  pMemSegEntry = (LDR_MemSeg *) ldr_memseg;

  /* Loop through global pool of Heaps */
  for (idx = 0; idx < LDR_MAX_MEMSEG; idx++)
  {
    if (strcmp ((const char *) pHeapName, (const char *) pMemSegEntry[idx].name) == 0)
    {
      virtualAddr = pMemSegEntry[idx].slave_virtual_addr;
    } /* End of if block if (strcmp(pHeapName,
         g_memcfg_heapInfo[idx].name) == 0) */
  } /* End of for (idx = 0; idx < LDR_MAX_MEMSEG; idx++) */

  return virtualAddr;
}
#endif

#ifndef _LOCAL_CORE_a8host_
/**
 *  @brief      Creating Heap from memory segment information
 *
 *  @param[in]  pMemSeg        Points to memory segments
 *
 *  @param[out] pHeapHdl       Heap Handle
 * 
 *
 *  @retval     MemCfg_ErrorNone if success
 *
 *              !MemCfg_ErrorNone    if fails
 *
 *  @pre        @c pMemSeg must be populated with right memory segment
 *
 *  @sa         memstp_deleteSRsHeaps
 *  @sa         memstp_createSr
 */
static MemCfg_Error memstp_createHeap (LDR_MemSeg *pMemSegEntry,
                                memcfg_HeapInfo *pHeapInfo)
{
  HeapMem_Params heapMemPrm;
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;

  Log_print2 (Diags_ENTRY, "Entering memstp_createHeap(0x%x, 0x%x)\n",
              (uint32_t) pMemSegEntry, (uint32_t) pHeapInfo);

  /* If it is system Heap then check the existing parameters */
  /* TBD */

  /* Setup HeapMem parameteres */
  HeapMem_Params_init (&heapMemPrm);
  heapMemPrm.buf = (xdc_Ptr) pMemSegEntry->slave_virtual_addr;
  heapMemPrm.size = pMemSegEntry->size;

  strcpy ((char *) pHeapInfo->name, pMemSegEntry->name);
  pHeapInfo->hdl = HeapMem_create (&heapMemPrm, NULL);

  if (NULL == pHeapInfo->hdl)
  {
    memCfgErrCode = MemCfg_ErrorInternal;
  }

  Log_print1 (Diags_EXIT, "Leaving memstp_createHeap, retVal: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}
#endif
/**
 *  @brief      Compares the SR entry with the configured memory segment.
 *              This is mostly used in comparing SR0 entry with the memory
 *              configuration
 *
 *  @param[in]  pMemSegEntry   Points to memory segments
 *
 *  @param[in] pSrEntry       SR entries
 * 
 *  @retval     1 if equal
 *
 *              0 not same
 *
 *  @pre        @c pSrEntry retrieved using SharedRegion_getEntry
 *
 *  @sa         None
 */
static uint32_t cmpMemSegSrEntry (LDR_MemSeg *pMemSegEntry,
                                  SharedRegion_Entry *pSrEntry)
{
  uint32_t retVal = 0;

  Log_print2 (Diags_ENTRY, "Entering cmpMemSegSrEntry(0x%x, 0x%x)\n",
              (uint32_t) pMemSegEntry, (uint32_t) pSrEntry);

  if ((pSrEntry->base == (Ptr) pMemSegEntry->slave_virtual_addr) &&
      (pSrEntry->len == pMemSegEntry->size) &&
      (pSrEntry->ownerProcId == pMemSegEntry->master_core_id) &&
      (1 == pSrEntry->createHeap))
  {
    retVal = 1;
  }

  Log_print1 (Diags_EXIT, "Leaving cmpMemSegSrEntry, retVal: %d\n", 
              retVal);

  return retVal;
}

#ifdef _LOCAL_CORE_a8host_
/**
 * @name DomxCore_mapPhyAddr2UsrVirtual()
 * @brief DomxCore function to map physical address to user virtual
 * @param phyAddr  : Physical address to be translated
 * @param len      : Length of memory block to be translated
 * @return none
 */
static uint8_t *memstp_mapPhyAddr2UsrVirtual (uint32_t phyAddr, uint32_t len)
{
  int32_t          status = 0;
  ProcMgr_AddrInfo addrInfo;
  uint8_t *pUsrVirtAddr = NULL;

  Log_print2 (Diags_ENTRY, "Entered memstp_mapPhyAddr2UsrVirtual(0x%x, %d)\n", 
              phyAddr, len);

  /* Map the kernel space address to user space */
  addrInfo.addr[ProcMgr_AddrType_MasterPhys] = phyAddr;
  addrInfo.addr[ProcMgr_AddrType_SlaveVirt] = phyAddr;
  addrInfo.size = len;
  addrInfo.isCached = FALSE;

  status = ProcMgr_translateAddr (pSlaveProcHandle,
                                    (Ptr) & pUsrVirtAddr,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);

  if (0 > status)
  {
    status = ProcMgr_map (pSlaveProcHandle,
                          (ProcMgr_MASTERKNLVIRT |
                           ProcMgr_MASTERUSRVIRT),
                          &addrInfo, ProcMgr_AddrType_MasterPhys);
    if (status < 0)
    {
      Log_print1 (Diags_USER1, "ProcMgr_map Failed status: 0x%x", status);
    }
    else
    {
      Log_print1 (Diags_USER1, "ProcMgr_map Success VirtAddr: %p",
                  addrInfo.addr[ProcMgr_AddrType_MasterUsrVirt]);

      status = ProcMgr_translateAddr (pSlaveProcHandle,
                                      (Ptr) & pUsrVirtAddr,
                                      ProcMgr_AddrType_MasterUsrVirt,
                                      (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);

    }

    if (status < 0)
    {
      Log_print1 (Diags_USER1,
                  "Error in ProcMgr_translateAddr [0x%x]\n", status);
      pUsrVirtAddr = NULL;
    }
    else
    {
      Log_print2 (Diags_USER1, "\n\nProcMgr_translateAddr Status [0x%x]"
                  " User Virtual Address [0x%x]\n",
                  status, (IArg) pUsrVirtAddr);
    }
    
  }
  return (pUsrVirtAddr);
}

/**
 * @name DomxCore_unmapPhyAddr2UsrVirtual()
 * @brief DomxCore_mapPhyAddr2UsrVirtual unmaps previous mapped  physical address
 * @param phyAddr  : Physical address to be unmapped
 * @param len      : Length of memory block to be unmapped
 * @return none
 */
static MemCfg_Error memstp_unmapPhyAddr (UInt32 phyAddr, UInt32 len)
{
  Int32 status = 0;
  ProcMgr_AddrInfo addrInfo;
  Ptr pUsrVirtAddr = NULL;
  Ptr pKnlVirtAddr = NULL;

  /* Map the kernel space address to user space */
  addrInfo.addr[ProcMgr_AddrType_MasterPhys] = phyAddr;
  addrInfo.addr[ProcMgr_AddrType_SlaveVirt] = phyAddr;
  addrInfo.size = len;
  addrInfo.isCached = FALSE;

  status = ProcMgr_translateAddr (pSlaveProcHandle,
                                  (Ptr) & pUsrVirtAddr,
                                  ProcMgr_AddrType_MasterUsrVirt,
                                  (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);
  if (status < 0)
  {
    Log_print1 (Diags_USER1, "Error in ProcMgr_translateAddr [0x%x]\n", status);

  }
  else
  {
    Log_print2 (Diags_USER1, "ProcMgr_translateAddr Status [0x%x]"
                " User Virtual Address [0x%x]\n", status, (IArg) pUsrVirtAddr);
  }

  addrInfo.addr[ProcMgr_AddrType_MasterUsrVirt] = (UInt32) pUsrVirtAddr;

  status = ProcMgr_translateAddr (pSlaveProcHandle,
                                  (Ptr) & pKnlVirtAddr,
                                  ProcMgr_AddrType_MasterKnlVirt,
                                  (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);
  if (status < 0)
  {
    Log_print1 (Diags_USER1, "Error in ProcMgr_translateAddr [0x%x]\n", status);

  }
  else
  {
    Log_print2 (Diags_USER1,
                "ProcMgr_translateAddr Status [0x%x]"
                " KNl Virtual Address [0x%x]\n", status, (IArg) pKnlVirtAddr);
  }

  addrInfo.addr[ProcMgr_AddrType_MasterKnlVirt] = (UInt32) pKnlVirtAddr;

  status = ProcMgr_unmap (pSlaveProcHandle,
                          (ProcMgr_MASTERKNLVIRT |
                           ProcMgr_MASTERUSRVIRT),
                          &addrInfo, ProcMgr_AddrType_MasterPhys);
  if (status < 0)
  {
    Log_print1 (Diags_USER1, "ProcMgr_unmap Failed status: 0x%x", status);
  }
  else
  {
    Log_print0 (Diags_USER1, "ProcMgr_unmap Success ");
  }
  Log_print3 (Diags_USER1, "Unmap done:%p:%d:%d\n", phyAddr, len, status);

  return (status);
}
#endif /* _LOCAL_CORE_a8host_ */

/**
 *  @brief      Creating Shared regions from memory segment information
 *
 *  @param[in]  pMemSeg        Points to memory segments
 *
 *  @param[out] pSrEntry       SR entries, Just for Book Keeping
 * 
 *  @param[out] pSrId          SR Id, used during the deletion process
 *
 *  @retval     MemCfg_ErrorNone if success
 *
 *              !MemCfg_ErrorNone    if fails
 *
 *  @pre        @c pMemSeg must be populated with right memory segment
 *
 *  @sa         memstp_deleteSRsHeaps
 *  @sa         memstp_createHeap
 */
static MemCfg_Error memstp_createSr (LDR_MemSeg *pMemSegEntry, memcfg_SrInfo *pSrInfo)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  int32_t srErrCode = SharedRegion_S_SUCCESS;
  uint16_t myMultiProcId = 0;
  uint8_t reTry = 1;
  SharedRegion_Entry srCurrEntry;
  uint32_t srBaseVirtual = 0x0;

  Log_print2 (Diags_ENTRY, "Entering memstp_createSr(0x%x, 0x%x)\n",
              (uint32_t) pMemSegEntry, (uint32_t) pSrInfo);

  myMultiProcId = MultiProc_self ();
  /* Query the SR entry first */
  srErrCode = SharedRegion_getEntry (pMemSegEntry->shared_region_id,
                                     &srCurrEntry);

  if (SharedRegion_S_SUCCESS != srErrCode)
  {
    memCfgErrCode = MemCfg_ErrorInternal;
  }

  if (MemCfg_ErrorNone == memCfgErrCode)
  {
    /* SR0 is statically created Create, ensure that existing setting matches
       the configuration */
    if (pMemSegEntry->shared_region_id == 0)
    {
      if (!((TRUE == srCurrEntry.isValid) &&
            (cmpMemSegSrEntry (pMemSegEntry, &srCurrEntry) == 1)))
      {
        memCfgErrCode = MemCfg_ErrorInvalidMemMap;
      }
    }
    else if (TRUE == srCurrEntry.isValid)
    {
      /* It is already set */
      memCfgErrCode = MemCfg_ErrorInternal;
    }
    else
    {
      /* Fill the SR fields */
      SharedRegion_entryInit (&pSrInfo->entry);

      
      #ifdef _LOCAL_CORE_a8host_
        /* Convert the physical address to virtual address. */
        srBaseVirtual = (uint32_t) 
              memstp_mapPhyAddr2UsrVirtual (pMemSegEntry->system_addr,
                                              pMemSegEntry->size);
      #else
        srBaseVirtual = (uint32_t) pMemSegEntry->slave_virtual_addr;
      #endif /* _LOCAL_CORE_a8host_ */

      pSrInfo->entry.base = (Ptr) srBaseVirtual;
      pSrInfo->entry.len = pMemSegEntry->size;
      pSrInfo->entry.createHeap = 1;
      pSrInfo->entry.isValid = TRUE;

      if (pMemSegEntry->master_core_id == myMultiProcId)
      {
        pSrInfo->entry.ownerProcId = myMultiProcId;
      }
      else
      {
        pSrInfo->entry.ownerProcId = SharedRegion_INVALIDREGIONID;
      }

      if (pMemSegEntry->cache_operation_mask & (1 << myMultiProcId))
      {
        pSrInfo->entry.cacheEnable = 1;
      }
      else
      {
        pSrInfo->entry.cacheEnable = 0;
      }
      strcpy ((char *) pSrInfo->name, pMemSegEntry->name);
      do
      {
        srErrCode = SharedRegion_setEntry (pMemSegEntry->shared_region_id,
                                           &pSrInfo->entry);

        /* Retry if i am the slave & setEntry fails as master core is yet to
           create this SR */
        reTry = 0;
        if ((pMemSegEntry->master_core_id != myMultiProcId) &&
            (SharedRegion_S_SUCCESS != srErrCode))
        {
          reTry = 1;
          /* Retry after 10 ms, Clock_tickPeriod gives period of each tick in
             micro seconds */
          #ifdef _LOCAL_CORE_a8host_
            usleep ( 1000 * 1000 * 1);
          #else
          Log_print1(Diags_USER1, "srerrCode", srErrCode);
          Task_sleep ((10000 * (1000 / Clock_tickPeriod)));
          #endif
        }
      }
      while (reTry);

      if (SharedRegion_S_SUCCESS == srErrCode)
      {
        pSrInfo->id = pMemSegEntry->shared_region_id;
      }
      else
      {
        memCfgErrCode = MemCfg_ErrorInternal;
      }
    }
  }

  Log_print1 (Diags_EXIT, "Leaving memstp_createSr, retVal: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}

/**
 *  @brief      Setup Memory configurations by creating Shared regions and
 *              Heaps
 *
 *  @param[in]  pMemSeg        Points to list of memory segments
 *
 *  @param[out] pHeapInfo      List of Heap Info
 * 
 *  @param[out] pSrInfo        List of SR Info
 *
 *  @retval     MemCfg_ErrorNone if success
 *
 *              !MemCfg_ErrorNone    if fails
 *
 *  @pre        @c pMemSeg must be populated with right memory segments
 *
 *  @sa         memstp_deleteSRsHeaps
 *  @sa         memstp_createHeap
 *  @sa         memstp_createSr
 */
static MemCfg_Error memstp_createSRsHeaps (uint8_t *pMemCfg, LDR_MemSeg *pAppReq,
                                memcfg_HeapInfo pHeapInfo[],
                                memcfg_SrInfo pSrInfo[])
{
  LDR_MemSeg *pMemSegEntry = NULL;
  uint16_t myMultiProcId = 0;
  uint32_t idx2HeapHdl = 0;
  uint32_t idx2SrEntry = 0;
  uint32_t idx2MemSeg = 0;
  uint32_t i = 0;
#ifdef _LOCAL_CORE_a8host_
  int32_t procMgrErrCode = 0;
#endif
  MemCfg_Error *pMemCfgStatus = NULL;
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  LDR_Memseg_Version_Hdr *pVerHdr = NULL;
  LDR_MemSeg *pMemSeg = NULL;
  uint8_t     mrsVersionSame = TRUE;

  Log_print4 (Diags_ENTRY, 
              "Entering memstp_createSRsHeaps(0x%x, 0x%x, 0x%x, 0x%x)\n",
              (uint32_t) pMemSegEntry, (uint32_t) pSrInfo, 
              (uint32_t) pHeapInfo, (uint32_t) pSrInfo);

  myMultiProcId = MultiProc_self ();

  /* Check if the version number is compatible, Beginning of memcfg space has
     the version header */
  pVerHdr = (LDR_Memseg_Version_Hdr *) pMemCfg;

  if (!(pVerHdr->major == LDR_MEMSEG_VERSION_NUM_MAJOR))
  {

    memCfgErrCode = MemCfg_ErrorIncompatibleVersion;
  }
  else if (!((pVerHdr->minor == LDR_MEMSEG_VERSION_NUM_MINOR) &&
            (pVerHdr->revision == LDR_MEMSEG_VERSION_NUM_REVISION) &&
            (pVerHdr->step == LDR_MEMSEG_VERSION_NUM_STEP)))
  {

    /* To inform this to firmware loader that mismatch is present 
     * even if system can work properly */
    mrsVersionSame = FALSE;
  }

  if ((MemCfg_ErrorNone == memCfgErrCode))
  {
    /* Check if it is valid memmap, memseg info follows version header */
    pMemSeg = (LDR_MemSeg *) (pMemCfg + sizeof (LDR_Memseg_Version_Hdr));
    if (!(ldr_isValidMemMap (pMemSeg) && ldr_isMeetsAppReq (pMemSeg, pAppReq)))
    {
      memCfgErrCode = MemCfg_ErrorInvalidMemMap;
    }
  }

  /* inform slave loader of early failure status
   * Final memory status can not be indicated to firmware loader
   * as memory cfg by slave depends on other slaves loaded or not
   * so firmware loader can not wait to know
   * final status
   */
  pMemCfgStatus = (MemCfg_Error *) (pMemCfg + LDR_CONFIG_ADDR_MEMCFG_SIZE +
                                    (myMultiProcId * 4));

  if (FALSE == mrsVersionSame)
  {
    *pMemCfgStatus = MemCfg_ErrorVersionMismatch;
  }
  else 
  {
    *pMemCfgStatus = memCfgErrCode;
  }
   
  if (MemCfg_ErrorNone == memCfgErrCode)
  {
    /* Initialize all Heap Handle to NULL */
    for (i = 0; i < LDR_MAX_MEMSEG; i++)
    {
      strcpy ((char *) pHeapInfo[i].name, "");
      #ifndef _LOCAL_CORE_a8host_
      pHeapInfo[i].hdl = NULL;
      #endif
    }

    /* Initialize all SR entries to invalid */
    for (i = 0; i < LDR_MAX_MEMSEG; i++)
    {
      pSrInfo[i].entry.isValid = 0;
      strcpy ((char *) pSrInfo[i].name, "");
      pSrInfo[i].id = 0;
    }

    #ifdef _LOCAL_CORE_a8host_
      /* Open the ProcMgr Module */
      procMgrErrCode = ProcMgr_open(&pSlaveProcHandle, 1);
      printf ("pSlaveProcHandle: 0x%x procId: %d\n", (unsigned int) pSlaveProcHandle, 1);
      if (0 > procMgrErrCode)
      {
        Log_print1 (Diags_ERROR, "Error in ProcMgr_open, status: %d", 
                    procMgrErrCode);
      } 
    #endif

    idx2HeapHdl = 0;
    idx2SrEntry = 0;
    /* Loop for all segments */
    while ((MemCfg_ErrorNone == memCfgErrCode) &&
           (pMemSeg[idx2MemSeg].valid == 1) && (idx2MemSeg < LDR_MAX_MEMSEG))
    {
      pMemSegEntry = &pMemSeg[idx2MemSeg];

      if ((pMemSegEntry->core_id_mask & (1 << myMultiProcId)) &&
          (pMemSegEntry->seg_type == LDR_SEGMENT_TYPE_DYNAMIC_LOCAL_HEAP))
      {
        #ifndef _LOCAL_CORE_a8host_
        /* Create Heap */
        memCfgErrCode =
          memstp_createHeap (pMemSegEntry, &pHeapInfo[idx2HeapHdl]);

        if (MemCfg_ErrorNone == memCfgErrCode)
        {
          idx2HeapHdl++;
        }
        #endif /* _LOCAL_CORE_a8host_ */
      }                         /* End Of if */
      else if ((pMemSegEntry->size > 0) &&
               (pMemSegEntry->core_id_mask & (1 << myMultiProcId)) &&
               (pMemSegEntry->seg_type == LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP))
      {
        memCfgErrCode = memstp_createSr (pMemSegEntry, &pSrInfo[idx2SrEntry]);
        if (MemCfg_ErrorNone == memCfgErrCode)
        {
          idx2SrEntry++;
        }
      } /* End of else if */

      idx2MemSeg++;
    } /* End Of while */
  } /* End of if */

  /* Inform slave Loader of the status of memory configuration */
  pMemCfgStatus = (MemCfg_Error *) (pMemCfg + LDR_CONFIG_ADDR_MEMCFG_SIZE +
                                    (myMultiProcId * 4));

  if (FALSE == mrsVersionSame)
  {
    *pMemCfgStatus = MemCfg_ErrorVersionMismatch;
  }
  else 
  {
    *pMemCfgStatus = memCfgErrCode;
  }

  Log_print1 (Diags_EXIT, "Leaving memstp_createSRsHeaps > ret_val: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}

/**
 *  @brief      Cleanup Memory configurations by deleting Shared regions and
 *              Heaps
 *
 *  @param[in]  pMemSeg          Points to list of memory segments, Currently unused
 *
 *  @param[in & out] pHeapInfo   List of Heap Info
 * 
 *  @param[in & out] pSrInfo     List of SR Info
 *
 *  @retval     MEMSETUP_SUCCESS if success
 *
 *              MEMSETUP_FAIL    if fails
 *
 *  @pre        @c memstp_createSRsHeaps is been invoked and returns success
 *
 *  @sa         memstp_createSRsHeaps
 *  @sa         memstp_createHeap
 *  @sa         memstp_createSr
 */
static MemCfg_Error memstp_deleteSRsHeaps (uint8_t *pMemCfg, 
                                    memcfg_HeapInfo pHeapInfo[],
                                    memcfg_SrInfo pSrInfo[])
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  int32_t     srErrCode   = SharedRegion_S_SUCCESS;
  uint32_t    idx         = 0;
  uint32_t    idx2MemSeg  = 0;
  uint32_t    srPhyAddr   = 0;
  uint32_t    srSize      = 0;
#ifdef _LOCAL_CORE_a8host_ 
  int32_t procMgrErrCode  = 0;
#endif
  LDR_MemSeg *pMemSegBase = NULL;

  Log_print3 (Diags_ENTRY, 
              "Entering memstp_deleteSRsHeaps(0x%x, 0x%x, 0x%x)\n",
              (uint32_t) pMemCfg, (uint32_t) pHeapInfo, 
              (uint32_t) pSrInfo);

  pMemSegBase = (LDR_MemSeg *) (pMemCfg + sizeof (LDR_Memseg_Version_Hdr));

  /* Loop through pool of Heap Handles */
  for (idx = 0; idx < LDR_MAX_MEMSEG; idx++)
  {

    #ifndef _LOCAL_CORE_a8host_ 
    if (pHeapInfo[idx].hdl != NULL)
    {

      HeapMem_delete (&pHeapInfo[idx].hdl);

      pHeapInfo[idx].hdl = NULL;
    } /* End of if (pHeapHdl[idx] != NULL) */
    #endif /* _LOCAL_CORE_a8host_ */
  } /* End of for (idx = 0; idx < LDR_MAX_MEMSEG; idx++) */

  /* Loop through pool of SR entries */
  for (idx = 0; idx < LDR_MAX_MEMSEG; idx++)
  {
    if (pSrInfo[idx].entry.isValid)
    {
      srErrCode = SharedRegion_clearEntry (pSrInfo[idx].id);

      if (SharedRegion_S_SUCCESS != srErrCode)
      {

        printf ("Sr id %d could not be cleared\n", idx);
        memCfgErrCode = MemCfg_ErrorInternal;
      }                         /* End of if (SharedRegion_S_SUCCESS !=
                                   srErrCode) */
      
      /* Get physical address for this Shared region */
      idx2MemSeg = 0;
      srPhyAddr = 0;
      srSize = 0;
      while (1 == pMemSegBase[idx2MemSeg].valid)
      {

        if (pMemSegBase[idx2MemSeg].shared_region_id == pSrInfo[idx].id)
        {

          srPhyAddr = pMemSegBase[idx2MemSeg].system_addr;
          srSize = pMemSegBase[idx2MemSeg].size;
          break;
        } /* End Of if block
             if (pMemSegBase[idx2MemSeg].shared_region_id == 
                 pSrInfo[idx].id) */

        idx2MemSeg++;
      } /* End Of while block
           while (1 == pMemSegBase[idx2MemSeg].valid) */

      if (0 == srPhyAddr) 
      {
        Log_print1 (Diags_ASSERT, 
                    "Physical address of SR %d could not be found\n", 
                    pSrInfo[idx].id);
      } /* End of if block  if (0 == srPhyAddr) */
      else 
      {
        memstp_unmapPhyAddr(srPhyAddr, srSize);
      } /* End of else */

      pSrInfo[idx].entry.isValid = 0;
      pSrInfo[idx].id = 0;
    } /* End Of if (pSrEntry[idx].isValid) */
  } /* End Of for (idx = 0; idx < LDR_MAX_MEMSEG; idx++) */

  #ifdef _LOCAL_CORE_a8host_ 
    if (NULL != pSlaveProcHandle)
    {
      procMgrErrCode = ProcMgr_close(&pSlaveProcHandle);
      if (0 > procMgrErrCode)
      {
        Log_print1 (Diags_ERROR,
                 "Error in ProcMgr_close(), errCode: %d", procMgrErrCode);
      }
    }
  #endif
  
  if (SharedRegion_S_SUCCESS != srErrCode)
  {
    memCfgErrCode = MemCfg_ErrorInternal;
  }
  Log_print1 (Diags_EXIT, "Leaving memstp_deleteSRsHeaps > ret_val: %d\n", 
              memCfgErrCode);
  
  return memCfgErrCode;
}

/* End Of File */
