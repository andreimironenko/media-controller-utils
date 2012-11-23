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
 
#ifndef _LDR_MEMSEG_H
#define _LDR_MEMSEG_H

/*!
*  @file    ldr_memseg.h
*
*  @brief   Defines types and interfaces used by firmaware loader.
*
*/

#include <stdint.h>

/**
 * @brief       Memory Segment loader version 
 */
#define LDR_MEMSEG_VERSION_NUM_MAJOR    (2)
#define LDR_MEMSEG_VERSION_NUM_MINOR    (1)
#define LDR_MEMSEG_VERSION_NUM_REVISION (2)
#define LDR_MEMSEG_VERSION_NUM_STEP     (1)

/**
 * @brief       Various DDR sizes supported
 */
#define LDR_DDR_SIZE_128M            (0)
#define LDR_DDR_SIZE_256M            (LDR_DDR_SIZE_128M + 1)
#define LDR_DDR_SIZE_512M            (LDR_DDR_SIZE_256M + 1)
#define LDR_DDR_SIZE_768M            (LDR_DDR_SIZE_512M + 1)
#define LDR_DDR_SIZE_1G              (LDR_DDR_SIZE_768M + 1)
#define LDR_DDR_SIZE_1_5G            (LDR_DDR_SIZE_1G + 1)
#define LDR_DDR_SIZE_2G              (LDR_DDR_SIZE_1_5G + 1)

/*! @var
 *  @brief Stores information about one memory segment.
 *  
 *         This is always stored i nthe beginning of 
 *         LDR_CONFIG_ADDR_MEMCFG_BASE
 */
typedef struct _LDR_Memseg_Version_Hdr_tag
{
  uint32_t major;
  uint32_t minor;
  uint32_t revision;
  uint32_t step;
} LDR_Memseg_Version_Hdr;

/**
 *  @brief      Configuration/status storage bae address in DDR
 */
#define LDR_CONFIG_ADDR_BASE           (0xBFF00000)
#define LDR_MEMCFG_SPACE_SIZE          (1*1024*1024)

/**
 *  @brief      memory map configuration 
 */
#define LDR_CONFIG_ADDR_MEMCFG_BASE    (LDR_CONFIG_ADDR_BASE)
#define LDR_CONFIG_ADDR_MEMCFG_SIZE    ((512 * 1024) - (4 * 1024))
#define LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE    (4 * 1024)

/**
 *  @brief      Region to store the status of memory configuration by each 
 *              core, loader polls this region to know the mem config 
 *              status by cores and display the results
 */
#define LDR_MEMCFG_STATUS_BASE_DSP     (LDR_CONFIG_ADDR_MEMCFG_BASE + \
                                        LDR_CONFIG_ADDR_MEMCFG_SIZE)

#define LDR_MEMCFG_STATUS_BASE_VM3     (LDR_MEMCFG_STATUS_BASE_DSP + 4)

#define LDR_MEMCFG_STATUS_BASE_DM3     (LDR_MEMCFG_STATUS_BASE_VM3 + 4)

#define LDR_MEMCFG_STATUS_BASE_A8      (LDR_MEMCFG_STATUS_BASE_DM3 + 4)


/**
 *  @brief      System information space, mainly used for 
 *              system status/Statistics maintaned by all the cores.
 */
#define LDR_CONFIG_ADDR_SYSINFO_BASE   (LDR_CONFIG_ADDR_MEMCFG_BASE + \
                                        LDR_CONFIG_ADDR_MEMCFG_SIZE + \
                                        LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE)
#define LDR_CONFIG_ADDR_SYSINFO_SIZE   ((512 * 1024)  - (8 * 1024))
#define LDR_CONFIG_ADDR_CTRLCFG_SIZE    (4 * 1024)
#define LDR_CONFIG_ADDR_LOADTABLE_SIZE  (4 * 1024)

#define LDR_CONFIG_ADDR_CTRLCFG_BASE   (LDR_CONFIG_ADDR_SYSINFO_BASE + \
                                        LDR_CONFIG_ADDR_SYSINFO_SIZE)
#define LDR_CONFIG_ADDR_LOADTABLE_BASE (LDR_CONFIG_ADDR_CTRLCFG_BASE + \
                                        LDR_CONFIG_ADDR_CTRLCFG_SIZE)

#define LDR_VIRTUAL_ADR_FLAG            0x0001

#define LDR_MAX_MEMSEG                  (32)

/**
 * @brief Core Ids, this needs to be same as the core Id assigned by
 *        multi proc module
 */
#define LDR_CORE_ID_DSP    (0)
#define LDR_CORE_ID_VM3    (1)
#define LDR_CORE_ID_DM3    (2)
#define LDR_CORE_ID_A8     (3)
#define LDR_CORES_MAX      (4)


/**
 * @brief status of memory configuration by each core
 */
typedef enum MemCfg_Error
{
  MemCfg_ErrorNone = 0,
  MemCfg_ErrorInvalidParam,
  MemCfg_ErrorInternal,
  MemCfg_ErrorInvalidMemMap,
  MemCfg_ErrorLessMem,
  MemCfg_ErrorTimeout,
  MemCfg_ErrorIncompatibleVersion,
  MemCfg_ErrorVersionMismatch,
  MemCfg_ErrorMax = 0x7fffffff
} MemCfg_Error;

#define LDR_MEMCFG_STATUS_NONE    (0)   /* !< Initial status */
#define LDR_MEMCFG_STATUS_SUCCESS (1)
#define LDR_MEMCFG_STATUS_FAIL    (2)

/*! @var
 *  @brief Defines types of a memory segment.
 */

typedef enum LDR_SegmentType_tag
{
  LDR_SEGMENT_TYPE_UNDEFINED,   /* !< Undefined segment type */
  LDR_SEGMENT_TYPE_STATIC_IMAGE,        /* !< Segment used a slave executable
                                           image */
  LDR_SEGMENT_TYPE_DYNAMIC_LOCAL_HEAP,  /* !< Slave local heap created
                                           dynamically during loading */
  LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP, /* !< Shared region heap created
                                           dynamically during loading */
  LDR_SEGMENT_TYPE_CONFIG_SPACE,        /* !< Segment used for accessing
                                           Config registers on the device */
  LDR_SEGMENT_TYPE_CMEM,         /* !< Used for CMEM */
  LDR_SEGMENT_TYPE_OTHERS       /* !< Used for all other memory sections cmem, 
                                   linux, etc... */
} LDR_SegmentType;

/*! @var
 *  @brief Stores information about one memory segment.
 */

typedef struct _LDR_Memseg_tag
{
  uint8_t valid;                /* !< Indicates if this memory section is
                                   valid, valid flag to 0 indicates the end of 
                                   the memory segments in the configuration */
  char name[32];                /* !< Name of the memory segment */
  uint32_t size;                /* !< Size of the segment with optional
                                   KB/MB/GB */
  LDR_SegmentType seg_type;     /* !< Type of the memory segment */
  uint32_t flags;               /* !< A set of applicale flags -
                                   [LDR_VIRTUAL_ADR_FLAG] */
  uint32_t system_addr;         /* !< System address of the segment */
  uint32_t slave_virtual_addr;  /* !< Virtual address used by slave */
  uint8_t master_core_id;       /* !< Specifies Master core id */
  uint8_t core_id_mask;         /* !< Specifies set of core id masks for which
                                   the segment is valid */
  uint8_t cache_enable_mask;    /* !< Specifies set of core ids for which
                                   cache is enabled */
  uint8_t cache_operation_mask; /* !< Specifies set of core ids for which cache 
                                   operation is anabled in SysLink/IPC */
  uint8_t shared_region_id;     /* !< Shared Region Id, when used across
                                   multiple coresSpecifies set of core id
                                   masks for which the segment is valid */
} LDR_MemSeg;

/*! @var
 *  @brief Stores information about one memory segment.
 */

typedef struct _LDR_CtrlCfg_tag
{
  uint8_t isI2cInitRequiredOnM3;/* !< Specifies if isI2cInit is required to
                                   be done on Vpss-M3 */
} LDR_CtrlCfg;

/* */
/*----------------------function prototypes ---------------------------------*/
/*  Checks if atleast one segment is present in the memseg configuartion*/
int8_t ldr_isMemSegConfigured (LDR_MemSeg *pMemSeg);

/* Gets number of valid mem segments */
int32_t ldr_getValidMemSegCnt (LDR_MemSeg *pMemSeg);

/* Checks if the given memory configuration is valid by checking 
 * overlaps, etc...
 */
int8_t ldr_isValidMemMap (LDR_MemSeg *pMemSeg);

/* Check if the passed memseg meets the application requirement */
int8_t ldr_isMeetsAppReq (LDR_MemSeg *pMemSeg, LDR_MemSeg *pAppMemSeg);

/* Dump the memory configuration to console, used for debug purpose */
int8_t ldr_dumpMemSegInfo (LDR_MemSeg *pMemSeg);

/* Get the memory configuration status */
MemCfg_Error ldr_getMemCfgStatus (uint8_t *pMemCfgSatusBase,
                                  uint32_t multiProcId);
#endif /* _LDR_MEMSEG_H */
