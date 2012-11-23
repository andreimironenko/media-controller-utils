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
 *  @file   sys_top.h
 *
 *  @brief  interface file for Davinci System Top similar to Linux top
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
#ifndef DAVINCI_SYSTEM_TOP_H
#define DAVINCI_SYSTEM_TOP_H
#include <ldr_memseg.h>
/*******************************************************************************
 *                             Compilation Control Switches
 ******************************************************************************/
/*------------------------------------ None-----------------------------------*/

/*******************************************************************************
 *                             INCLUDE FILES
 ******************************************************************************/
/*------------------------------------ None-----------------------------------*/

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/

#define SYSTOP_VERSION_NUM_MAJOR     (0)
#define SYSTOP_VERSION_NUM_MINOR     (2)
#define SYSTOP_VERSION_NUM_REVISION  (0)
#define SYSTOP_VERSION_NUM_STEP      (0)

/**
 *  @brief      Davinci System Top error code
 */
#define DST_SUCCESS  (1)  /**< Success */
#define DST_FAIL     (0)  /**< Failure */

/**
 *  @brief      System status refresh interval in ms
 */
#define DST_REFRESH_DURATION       (1000)

/**
 *  @brief      Maximum number of shared region in each core
 */
#define DAVINCI_STATUS_MAX_SR  (10)

/**
 *  @brief      Maximum number of Heaps in each core
 */
#define DAVINCI_STATUS_MAX_HEAP  (10)

/**
 *  @brief      Maximum number charaters for Firmware string
 */
#define DST_FW_VERSION_LENGTH  (32)

/*--------------------------Data declarations --------------------------------*/

/* Pasted these lines from rpc/types.h */
#ifndef FALSE
#      define  FALSE   (0)
#endif

#ifndef TRUE
#      define  TRUE    (1)
#endif

#ifndef NULL
#      define  NULL 0
#endif


/**
 *  @brief      Header information to be populated by all the cores
 *
 *  @sa        dst_dspInfo, dst_VM3Info, dst_DM3Info, dst_A8Info  
 */
typedef struct dst_HeaderInfo
{
  int running;   /**< Core is running, in case of dstop, this applicaiton is
                   *  running
                   */
  int verNumMajor; /**< Version number major */
  int verNumMinor;     /**< Version number Minor  */
  int verNumRevision;  /**< Version number Revision  */
  int verNumStep;      /**< Version number Step  */
} dst_HeaderInfo;

/**
 *  @brief      Heap related informations
 *
 *  @sa         None
 */
typedef struct dst_HeapInfo
{
  uint8_t valid;               /**< valid */
  unsigned int totalSize;       /**< Heap size */
  unsigned int totalFreeSize;   /**< Free size */
  unsigned int largestFreeSize; /**< Largest Free size */
  uint32_t     maxUsed;         /**< Max used size */
} dst_HeapInfo;

typedef struct dst_SREntry
{
  unsigned int isValid;           /**< Is this region Valid */
  unsigned int srIndex;           /**< SR index */
  unsigned int phyBaseAddr;       /**< Physical Base address */
  unsigned int virtBaseAddr;      /**< Virtual Base address  */
  unsigned int size;              /**< Size */
  unsigned int isHeap;            /**< Heap created */
  dst_HeapInfo heapInfo;          /**< Heap info */
} dst_SREntry;

typedef struct dst_SRInfo
{
  int numSRs;               /**< Number of Shared regions */
  dst_SREntry srEntries[DAVINCI_STATUS_MAX_SR];
} dst_SRInfo;

typedef struct dst_CoresHeapInfo
{
  int numHeaps;               /**< Number of Shared regions */
  dst_HeapInfo heapInfo[DAVINCI_STATUS_MAX_HEAP];
} dst_CoresHeapInfo;

/**
 *  @brief      DSP core related informations
 *
 *  @sa         dst_dspInfo
 *  @sa         dst_VM3Info
 *  @sa         dst_DM3Info
 */
typedef struct dst_CoreInfo
{
  dst_HeaderInfo hdrInfo;     /**< Header info */
  char fw_version_string[DST_FW_VERSION_LENGTH]; /**< Firmware version */
  dst_CoresHeapInfo heapInfo; /**< Heap Info */
  dst_SRInfo srInfo;          /**< SR info */
} dst_CoreInfo;

typedef dst_CoreInfo dst_dspInfo;
typedef dst_CoreInfo dst_VM3Info;
typedef dst_CoreInfo dst_DM3Info ;
typedef dst_CoreInfo dst_a8Info ;

/**
 *  @brief      Davinci System Info
 *
 *  @sa         dst_dspInfo
 *  @sa         dst_vm3Info
 *  @sa         dst_dm3Info
 *  @sa         dst_a8Info
 */
typedef struct dstInfo
{
  dst_HeaderInfo hdrInfo; /**< Header Info */
  // LDR_Memseg memseg[LDR_MAX_MEMSEG];
  dst_dspInfo dspInfo;    /**< DSP Core related info */
  dst_VM3Info vm3Info;    /**< VM3 Core related info */
  dst_DM3Info dm3Info;    /**< DM3 Core related info */
  dst_a8Info a8Info;      /**< A8 Core related info */
} dstInfo;

/*--------------------------Enumerated Types  --------------------------------*/
/* None */

/*----------------------function prototypes ---------------------------------*/

/* Application specific information is updated through this application
 * implemented call back function
 */
typedef void SYSTOP_UPD_HEAP_CB_FUNC(dst_HeapInfo *pHeapInfoList);

/* Create sys_top module */
void sysTop_create();

/* Deletes sys_top module */
void sysTop_delete();

/* sys_top processing functions */
void sysTop_process (SYSTOP_UPD_HEAP_CB_FUNC *pAppCb);


#endif /* DAVINCI_SYSTEM_TOP_H */

/* End Of File */
