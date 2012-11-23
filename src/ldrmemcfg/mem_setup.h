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
 *  @file   mem_setup.h
 *
 *  @brief  interface file for Memory map configuration
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
#ifndef MEM_SETUP_H
#define MEM_SETUP_H
#include <ldr_memseg.h>
#include <ti/ipc/SharedRegion.h>
// #include <ti/sdo/ipc/SharedRegion.h>

/*******************************************************************************
 *                             Compilation Control Switches
 ******************************************************************************/
/*------------------------------------ None-----------------------------------*/

/*******************************************************************************
 *                             INCLUDE FILES
 ******************************************************************************/

#ifndef _LOCAL_CORE_a8host_
#include <xdc/std.h>
#include <xdc/runtime/IHeap.h>
#include <ti/sysbios/heaps/HeapMem.h>
#endif /* _LOCAL_CORE_a8host_ */
/*------------------------------------ None-----------------------------------*/

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/

/*--------------------------Data declarations --------------------------------*/
/*! @var
 *  @brief Heap informations shared between memory configuration and the
 *         application.

 *
 *         This is filled in by memory configuration module based on the user 
 *         configuration passed to slave by loader, which and retrieved 
 *         and used by application
 */
typedef struct memcfg_HeapInfo
{
  uint8_t name[32];             /* !< Heap Name */
  
  #ifndef _LOCAL_CORE_a8host_
  HeapMem_Handle hdl;           /* !< Heap Handle */
  #endif /* _LOCAL_CORE_a8host_ */
} memcfg_HeapInfo;

/*! @var
 *  @brief Shared region informations shared between memory configuration and the
 *         application.

 *
 *         This is filled in by memory configuration module and retrieved 
 *         and used by application
 */
typedef struct memcfg_SrInfo
{
  uint8_t name[32];             /* !< Shared region name */
  SharedRegion_Entry entry;     /* !< Shared region entry, used for debug
                                   purpose */
  uint16_t id;                  /* !< Shared region Id */
} memcfg_SrInfo;

/*--------------------------Enumerated Types  --------------------------------*/


/*----------------------function prototypes ---------------------------------*/

/* Setup Memory configurations by creating Shared regions and Heaps */
MemCfg_Error ldrmemcfg_createSRsHeaps(uint8_t *pMemCfgSpace, 
                                      LDR_MemSeg *pAppMegSeg);

/* delete Memory configurations by deleting Shared regions and Heaps */
MemCfg_Error ldrmemcfg_deleteSRsHeaps(uint8_t *pMemCfgSpace);

#ifndef _LOCAL_CORE_a8host_
HeapMem_Handle memstp_getHeapHdlByName (uint8_t *pHeapName);

uint32_t memstp_getVirtualAddrByName (void *ldr_memseg,
                                      uint8_t *pHeapName);
#endif /* _LOCAL_CORE_a8host_ */

#endif /* MEM_SETUP_H */

/* End Of File */
