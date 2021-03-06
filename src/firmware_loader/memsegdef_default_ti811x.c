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
 *  @file   memsegdef.c
 *
 *  @brief  Memory map segment definitions, This is auto generated file
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
/*----------------------------- Memory Segment configuration -----------------*/
#include <ldr_memseg.h>

uint32_t ldrmemcfg_ddrSize = LDR_DDR_SIZE_1G ;

LDR_MemSeg sdk_memseg_default[] =
{
  /* Segment 0 */
  {
   1,                           /* valid */
   "IPC_SR_DATA_BUFFERS",       /* name */
   0x03900000,                  /* size */
   LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP,  /* LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP,
                                   seg_type */
   0,                           /* flags */
   0x9C000000,                  /* system_addr */
   0x9C000000,                  /* slave_virtual_addr */
   LDR_CORE_ID_A8,             /* master_core_id */
   (1 << LDR_CORE_ID_VM3) | (1 << LDR_CORE_ID_DM3) | (1 << LDR_CORE_ID_A8) | (1 << LDR_CORE_ID_DSP),
   /* core_id_mask */
   (1 << LDR_CORE_ID_VM3) | (1 << LDR_CORE_ID_DM3) | (1 << LDR_CORE_ID_DSP), /* cache_enable_mask */
   /* cache_enable_mask */
   (1 << LDR_CORE_ID_VM3) | (1 << LDR_CORE_ID_DM3) | (1 << LDR_CORE_ID_DSP), /* cache_operation_mask */
   1                            /* shared_region_id */
  },

  /* Segment 2 */
  {
   1,                           /* valid */
   "DSP_ALG_HEAP",              /* name */
   0x01400000,                  /* size */
   LDR_SEGMENT_TYPE_DYNAMIC_LOCAL_HEAP,        /* seg_type */
   0,                           /* flags */
   0x98C00000,                  /* system_addr */
   0x98C00000,                  /* slave_virtual_addr */
   LDR_CORE_ID_DSP,             /* master_core_id */
   (1 << LDR_CORE_ID_DSP),      /* core_id_mask */
   (1 << LDR_CORE_ID_DSP),      /* cache_enable_mask */
   (1 << LDR_CORE_ID_DSP),      /* cache_operation_mask */
   -1                           /* shared_region_id */
  },

  {
   1,                           /* valid */
   "A8_DSP_CMEM",               /* name */
   0xA00000,                   /* size */
   LDR_SEGMENT_TYPE_CMEM,       /* seg_type */
   0,                           /* flags */
   0x96200000,                  /* system_addr */
   0x96200000,                  /* slave_virtual_addr */
   LDR_CORE_ID_A8,             /* master_core_id */
   (1 << LDR_CORE_ID_A8) | (1 << LDR_CORE_ID_DSP),      /* core_id_mask */
   (1 << LDR_CORE_ID_A8) | (1 << LDR_CORE_ID_DSP),      /* cache_enable_mask */
   (1 << LDR_CORE_ID_A8) | (1 << LDR_CORE_ID_DSP),      /* cache_operation_mask */
   -1                           /* shared_region_id */
  },

  /* Last Segment, Marked by valid flag to 0 */
  {
   0,
  },
};

/* End Of File */
