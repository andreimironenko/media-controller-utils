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
 *  @file   ldr_memcfg.h
 *
 *  @brief  Loader Memory configuration utility functions.
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
 
#ifndef LDR_MEMCFG_H
#define LDR_MEMCFG_H


/*******************************************************************************
 *                             Compilation Control Switches
 ******************************************************************************/

/*******************************************************************************
 *                             INCLUDE FILES
 ******************************************************************************/
#include <ldr_memseg.h>
#include <stdint.h>

/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*--------------------------- macros  ----------------------------------------*/

/*--------------------------Data declarations --------------------------------*/

/*--------------------------Enumerated Types  --------------------------------*/


/*----------------------function prototypes ---------------------------------*/

/* Maps the Memory configuration space in to linux user side */
uint8_t *memcfg_mapMemCfgSpace ();

/* UnMaps the Memory configuration space from linux user side */
MemCfg_Error memcfg_unMapMemCfgSpace (uint8_t *pMemCfg);

/* Loads the Memory configuration from auto generated file to memory 
   configuration space in DDR */
MemCfg_Error memcfg_loadMemSegInfo (uint8_t *pMemCfg, LDR_MemSeg *pMegSegCfg,
                                    uint8_t *pFileName, uint32_t multiProcId);

/* Loads the Control Configuration memory configuration space in DDR */
MemCfg_Error memcfg_loadCtrlCfgInfo (uint8_t *pMemCfg, LDR_CtrlCfg *pCtrlCfg,
                                     uint32_t multiProcId);

#endif /* DAVINCI_SYSTEM_TOP_H */

/* End Of File */
