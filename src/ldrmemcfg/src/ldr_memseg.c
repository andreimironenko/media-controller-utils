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
 *  @file   ldr_memseg.c
 *
 *  @brief  Memory map configuration 
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
/*******************************************************************************
 *                             Compilation Control Switches
 ******************************************************************************/
/* None */

/*******************************************************************************
 *                             INCLUDE FILES
 ******************************************************************************/

/* -------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/*----------------------------- Memory Segment configuration -----------------*/
#include <ldr_memseg.h>

/*******************************************************************************
 * Build time checks 
 ******************************************************************************/
/* #warning Ensure that loader memcfg version changed if common \
          data structures (between loader & slaves) are changed */

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/
/*---------------------data declarations -------------------------------------*/

/*! @var
 *  @brief Points to memory configuration written by Slave loader
 */
#ifdef LDRMEMCFG_DYNAMIC_SETTING
 #ifdef __ti__
  #pragma DATA_SECTION (memConfigSpaceBuf, ".bss:memCfgSpace");
  uint8_t memConfigSpaceBuf[LDR_MEMCFG_SPACE_SIZE];
  uint8_t *pMemSegCfg = &memConfigSpaceBuff;
 #else
  /* Linux side queries base address of MemCfgSpace from memseg table */
  uint8_t *pMemSegCfg = NULL;
 #endif
#else
 uint8_t *pMemSegCfg = (uint8_t *) LDR_CONFIG_ADDR_MEMCFG_BASE;
#endif

/**
 *  @brief      Check if Memory segments configured by Loader
 *
 *  Check if atleast one segment is present 
 *
 *  @param[in]  pMemSeg        Points to memseg storage location
 *
 *  @retval     1              if atleast one valid memseg is present
 *              0              if no valid mem seg is present
 *
 *  @pre        @c pMemSeg points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped.
 *
 *  @sa         ldr_getValidMemSegCnt
 */
int8_t ldr_isMemSegConfigured (LDR_MemSeg *pMemSeg)
{
  uint8_t retVal = 0;

  if (ldr_getValidMemSegCnt (pMemSeg) > 0)
  {
    retVal = 1;
  }

  return retVal;
}

/**
 *  @brief      Get the number of valid mem segment informations.
 *
 *  Loader program writes the memory configuration to DDR. This API
 *  finds out number of valid memory segments in DDR.
 *
 *
 *  @param[in]  pMemSeg        Points to memseg storage location
 *
 *  @retval     Number of segments
 *
 *  @pre        @c pMemSeg points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped.
 *
 *  @sa         ldr_dumpMemSegInfo
 */
int32_t ldr_getValidMemSegCnt (LDR_MemSeg *pMemSeg)
{
  uint32_t idx2MemSeg = 0;

  /* Loop till all the valid memseg or max memseg */
  idx2MemSeg = 0;
  while ((pMemSeg[idx2MemSeg].valid == 1) && (idx2MemSeg < LDR_MAX_MEMSEG))
  {
    idx2MemSeg++;
  }

  return idx2MemSeg;
}

/**
 *  @brief      Chech if the give memory map is a valid map
 *
 *  This API checks if the given memory configuration is valid.
 * 
 *  @param[in]  pMemSeg        Points to memseg storage location
 *
 *  @retval     1 Valid memory configuration  
 *              0 In valid memory configuration
 *
 *  @pre        @c pMemSeg points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped.
 *
 *  @sa         ldr_dumpMemSegInfo
 *  @todo       Currently this Checks for segmetn overlap only 
 *  @todo       Check for duplicate segments
 *  @todo       Check for multiple occurance of SRs
 *  @todo       Check for 0 size segments
 *  @todo       Check for invalid base addresses
 */
int8_t ldr_isValidMemMap (LDR_MemSeg *pMemSeg)
{
  uint32_t idx2MemSeg = 0;
  uint32_t i = 0;
  uint8_t mapValid = 0;
  uint32_t r1b = 0;
  uint32_t r1e = 0;
  uint32_t r2b = 0;
  uint32_t r2e = 0;

  if (ldr_getValidMemSegCnt (pMemSeg) > 0)
  {
    /* Loop till all the valid memseg & not exceeding max memseg & valid map */
    idx2MemSeg = 0;
    mapValid = 1;
    while ((mapValid) &&
           (pMemSeg[idx2MemSeg].valid == 1) && (idx2MemSeg < LDR_MAX_MEMSEG))
    {

      /* Check current memseg does not overlap with previous segments */

      for (i = 0; i < idx2MemSeg; i++)
      {
        r1b = pMemSeg[i].system_addr;
        r1e = pMemSeg[i].system_addr + pMemSeg[i].size;
        r2b = pMemSeg[idx2MemSeg].system_addr;
        r2e = pMemSeg[idx2MemSeg].system_addr + pMemSeg[idx2MemSeg].size;
        if (!(((r2b <= r1b) && (r2e <= r1b)) || ((r2b >= r1e) && (r2e >= r1e))))
        {
          printf ("Memory Seg %d overlaps with segmet %d\n", idx2MemSeg, i);
          mapValid = 0;
          break;
        }
      }

      idx2MemSeg++;
    }
  }

  return mapValid;
}

/**
 *  @brief      Chack if the configured memory segments meets the application
 *              requirement
 *
 *  This API checks if the given memory configuration meets the application
 *  memory requirements.
 * 
 *  @param[in]  pMemSeg        Points to memory configuration
 *
 *  @param[in]  pAppReq        Points to application requirements
 *
 *  @retval     1 Meets the application requirement
 *              0 Does not meet application requirement
 *
 *  @pre        @c pMemSeg is a valid memory map configuration
 *
 *  @pre        @c pAppMemSeg application's minimal configuration
 *
 *  @pre        @c pAppMemSeg's last memory section is marked by valid flag set to 1
 *
 *  @sa         ldr_isValidMemMap
 *
 *  @todo       Currently this checks only the section name & the size
 *  @todo       Check for the SRs Ids
 *  @todo       Check for Cache & cache operation
 *  @todo       Check for segmetn type
 *  @todo       Check for CoreId
*/
int8_t ldr_isMeetsAppReq (LDR_MemSeg *pMemSeg, LDR_MemSeg *pAppMemSeg)
{
  uint32_t idx2MemSeg = 0;
  uint32_t idx2AppReqMemSeg = 0;
  uint8_t meetsAppReq = 0;

  meetsAppReq = 1;
  idx2AppReqMemSeg = 0;

  if (pAppMemSeg != NULL)
  {
    /* Loop for all the app requirement segments and the application req is
       met */
    while (meetsAppReq && pAppMemSeg[idx2AppReqMemSeg].valid)
    {
      idx2MemSeg = 0;
      meetsAppReq = 0;

      /* Loop till all the valid memseg & not exceeding max memseg & valid map */
      while ((pMemSeg[idx2MemSeg].valid == 1) && (idx2MemSeg < LDR_MAX_MEMSEG))
      {
        /* 
         * Check if the application's segment is present in the mem cfg 
         * Currently only section name & size is been validated
         * It can be enhanced to validate all parameters 
         */
        if (((strcmp ((const char *) pMemSeg[idx2MemSeg].name,
                      (const char *) pAppMemSeg[idx2AppReqMemSeg].name) == 0) &&
             (pMemSeg[idx2MemSeg].size >= pAppMemSeg[idx2AppReqMemSeg].size)))
        {
          meetsAppReq = 1;
          break;
        } /* End of if */

        idx2MemSeg++;
      } /* End of while */

      idx2AppReqMemSeg++;
    } /* End of while */
  } /* End of if */

  return meetsAppReq;
}

/**
 *  @brief      Get Error String of the error code
 *
 *  @param[in]  errCode        Error code
 *
 *  @retval     Error string associated with the error code
 *
 *  @pre        @c errCode is a valid Error Code
 *
 *  @sa         MemCfg_Error
 */
uint8_t *ldr_getErrStr (MemCfg_Error errCode)
{
  uint8_t *pErrStr = NULL;

  switch (errCode)
  {
    case MemCfg_ErrorNone:
      pErrStr = (uint8_t *) "ErrorNone";
      break;
    case MemCfg_ErrorInvalidParam:
      pErrStr = (uint8_t *) "ErrorInvalidParam";
      break;
    case MemCfg_ErrorInternal:
      pErrStr = (uint8_t *) "ErrorInternal";
      break;
    case MemCfg_ErrorInvalidMemMap:
      pErrStr = (uint8_t *) "ErrorInvalidMemMap";
      break;
    case MemCfg_ErrorLessMem:
      pErrStr = (uint8_t *) "ErrorLessMem";
      break;
    case MemCfg_ErrorTimeout:
      pErrStr = (uint8_t *) "ErrorTimeout";
      break;
    case MemCfg_ErrorIncompatibleVersion:
      pErrStr = (uint8_t *) "ErrorIncompatibleVersion";
      break;
    case MemCfg_ErrorVersionMismatch:
      pErrStr = (uint8_t *) "ErrorVersionMismatch";
      break;
    default:
      pErrStr = (uint8_t *) "UnKnown";
      break;
  }

  return pErrStr;
}

/**
 *  @brief      Get the status of memory configuration from given 
 *              MultiProc core id
 *
 *  @param[in]  pMemCfgSatusBase        Points to memseg configuration space
 *
 *  @param[in]  multiProcId             Multiproc Id for which status to be 
 *                                      retrieved
 *
 *  @retval     Memory configuration status for given core
 *
 *  @pre        @c pMemCfgSatusBase points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped & pMemCfgSatusBase 
 *                 points to user virtual address of the same
 *  @pre        @c Memory configuration is initialized to MemCfg_ErrorMax 
 *                 before memory configuration started by slave/app
 *  @pre        @c Memory is configuration is done
 *
 *  @sa         None
 */
MemCfg_Error ldr_getMemCfgStatus (uint8_t *pMemCfgSatusBase,
                                  uint32_t multiProcId)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  LDR_MemSeg *pMemSeg = NULL;

  pMemSeg = (LDR_MemSeg *) (pMemCfgSatusBase + sizeof (LDR_Memseg_Version_Hdr));

  if (!ldr_isValidMemMap (pMemSeg))
  {
    printf ("Memory is not configurated\n");
    memCfgErrCode = MemCfg_ErrorMax;
  }
  else
  {

    memCfgErrCode = (MemCfg_Error) *((uint32_t *) (pMemCfgSatusBase +
                                     (LDR_CONFIG_ADDR_MEMCFG_SIZE) +
                                     multiProcId * 4));
  }

  return memCfgErrCode;
}

/**
 *  @brief      Get memseg info by name
 *
 *  @param[in]  pMemSegList        Points to memseg list
 *
 *  @param[in]  pMemSegName        Memseg name to be found in the memseg list
 *
 *  @retval     NULL   Segment not found
 *              
 *              ! NULL Memseg info associated with the given seg name
 *
 *  @pre        @c pMemCfgSatusBase points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped & pMemCfgSatusBase 
 *                 points to user virtual address of the same
 *
 *  @sa         None
 */
LDR_MemSeg *ldr_getSegInfoByName (LDR_MemSeg *pMemSegList, uint8_t *pMemSegName)
{
  //MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  LDR_MemSeg  *pMemSeg = NULL;
  int          i = 0;

  /* Search memseg name in the list */
  i = 0;
  while (pMemSegList[i].valid)
  {
    if (strcmp((const char *) pMemSegList[i].name, (const char *) pMemSegName) == 0)
    {
      pMemSeg = &pMemSegList[i];
      break;
    }
    i++;
  }

  return pMemSeg;
}

/**
 *  @brief      Display informations of the memory segmetns
 *
 *  @param[in]  pMemSeg        Points to memseg storage location
 *
 *  @retval     Number of segments
 *
 *  @pre        @c pMemSeg points to Memory Config space
 *
 *  @pre        @c Memory Config space is mapped.
 *
 *  @sa         ldr_dumpMemSegInfo
 */
int8_t ldr_dumpMemSegInfo (LDR_MemSeg *pMemSeg)
{
  uint32_t idx2MemSeg = 0;
  LDR_MemSeg *pMemSegEntry;
  uint8_t    *pMemCfgSpace = NULL;

  /* If application passes the pointer use that otherwise use the
   * default MEMCFG_SPACE system address 
   * DSP, Media controller passes NULL
   * Linux passes the user virtual address
   */
  if (NULL == pMemSeg)
  {
    pMemCfgSpace = (uint8_t *) LDR_CONFIG_ADDR_MEMCFG_BASE;

    /* Beginning of memcfg space has the version header
     */
    pMemSeg = (LDR_MemSeg *) (pMemCfgSpace + 
                              sizeof (LDR_Memseg_Version_Hdr));
  }

  printf ("Total Memory segments: %d\n", ldr_getValidMemSegCnt (pMemSeg));

  /* Loop till all the valid memseg or max memseg */
  idx2MemSeg = 0;
  while ((pMemSeg[idx2MemSeg].valid == 1) && (idx2MemSeg < LDR_MAX_MEMSEG))
  {
    pMemSegEntry = &pMemSeg[idx2MemSeg];

    printf ("\n");
    printf ("Memory Segment : %d\n", idx2MemSeg);
    printf ("===================\n");
    printf ("name                 : %s\n", pMemSegEntry->name);
    printf ("size                 : %d\n", pMemSegEntry->size);
    printf ("seg_type             : %d\n", pMemSegEntry->seg_type);
    printf ("flags                : 0x%x\n", pMemSegEntry->flags);
    printf ("system_addr          : 0x%x\n", pMemSegEntry->system_addr);
    printf ("slave_virtual_addr   : 0x%x\n", pMemSegEntry->slave_virtual_addr);
    printf ("master_core_id       : %d\n", pMemSegEntry->master_core_id);
    printf ("core_id_mask         : 0x%x\n", pMemSegEntry->core_id_mask);
    printf ("cache_enable_mask    : 0x%x\n", pMemSegEntry->cache_enable_mask);
    printf ("cache_operation_mask : 0x%x\n",
            pMemSegEntry->cache_operation_mask);
    printf ("shared_region_id     : %d\n", pMemSegEntry->shared_region_id);

    idx2MemSeg++;
  }

  return idx2MemSeg;
}

/* End Of File */
