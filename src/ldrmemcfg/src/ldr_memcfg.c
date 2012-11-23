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
 *  @file   ldr_memcfg.c
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/*--------------------- Log files --------------------------------------------*/
#ifndef _LOCAL_CORE_a8host_
  /* Bios side use RTSC log or UIA log */
  #include <xdc/runtime/Log.h>
  #include <xdc/runtime/Diags.h>
#endif

/*----------------------------- Memory Segment configuration -----------------*/
#include <ldr_memseg.h>

/*----------------------------- Macros ---------------------------------------*/
/* Log print related macros, this are require to avoid dependency with 
 * external tool
 */
#ifdef _LOCAL_CORE_a8host_
  #if (ENABLE_TRACE == 1)
    #define Log_print0(x,y) printf(y);printf("\n")
    #define Log_print1(x,y,z1) printf(y,z1);printf("\n")
    #define Log_print2(x,y,z1,z2) printf(y,z1,z2);printf("\n")
    #define Log_print3(x,y,z1,z2,z3) printf(y,z1,z2,z3);printf("\n")
    #define Log_print4(x,y,z1,z2,z3,z4) printf(y,z1,z2,z3,z4);printf("\n")
    #define Log_print5(x,y,z1,z2,z3,z4,z5) printf(y,z1,z2,z3,z4,z5);printf("\n")
    #define Log_print6(x,y,z1,z2,z3,z4,z5,z6) printf(y,z1,z2,z3,z4,z5,z6);printf("\n")
  #else
    #define Log_print0(x,y) 
    #define Log_print1(x,y,z1) 
    #define Log_print2(x,y,z1,z2) 
    #define Log_print3(x,y,z1,z2,z3) 
    #define Log_print4(x,y,z1,z2,z3,z4) 
    #define Log_print5(x,y,z1,z2,z3,z4,z5) 
    #define Log_print6(x,y,z1,z2,z3,z4,z5,z6) 
  #endif
#endif
/*******************************************************************************
 * PUBLIC DECLARATIONS Defined here, used elsewhere
 ******************************************************************************/

/*---------------------data declarations -------------------------------------*/
/* None */

/*******************************************************************************
 * PRIVATE DECLARATIONS Defined here, used only here
 ******************************************************************************/
/*---------------------data declarations -------------------------------------*/
int32_t memSegFd = (int32_t) NULL;
int32_t ammuSpaceFd = (int32_t) NULL;

/**
 *  @brief      Map Memory segments configuration space in DDR
 *
 *  @param      None
 *
 *  @retval     NULL      on any failure
 *              Non NULL  User space pointer for Memory configuration space
 *
 *  @pre        None
 *
 *  @sa         memcfg_unMapMemCfgSpace
 */
uint8_t *
memcfg_mapMemCfgSpace ()
{
  uint8_t *pMemCfg = NULL;

  Log_print0 (Diags_ENTRY, "Entering memcfg_mapMemCfgSpace\n");

  memSegFd = open ("/dev/mem", O_RDWR | O_SYNC);
  if (-1 == memSegFd)
  {
    printf ("Unable to open /dev/mem\n");
  }

  /* Get Base address and size from the memseg table */

  pMemCfg = mmap (0, (LDR_CONFIG_ADDR_MEMCFG_SIZE +
                      LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE +
                      LDR_CONFIG_ADDR_SYSINFO_SIZE +
                      LDR_CONFIG_ADDR_CTRLCFG_SIZE),
                  PROT_READ | PROT_WRITE, MAP_SHARED, memSegFd,
                  LDR_CONFIG_ADDR_BASE);
  if ((void *) -1 == pMemCfg)
  {
    printf ("Could not open the mem file \n");  // FATAL; FATAL;
    pMemCfg = NULL;
  }

  Log_print1 (Diags_EXIT, "Leaving memcfg_mapMemCfgSpace, retVal: 0x%x\n", 
              (uint32_t) pMemCfg);

  return pMemCfg;
}

/**
 *  @brief      UnMap Memory segments configuration space in DDR
 *
 *  @param      pMemCfg   User space pointer for Memory configuration space
 *
 *  @retval     MemCfg_ErrorNone      On Success
 *              Other                 On Failure
 *
 *  @pre        @c already Mapped using memcfg_mapMemCfgSpace
 *
 *  @sa         memcfg_mapMemCfgSpace
 */
MemCfg_Error
memcfg_unMapMemCfgSpace (uint8_t * pMemCfg)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;

  Log_print1 (Diags_ENTRY, "Entering memcfg_unMapMemCfgSpace(0x%x)\n", 
              (uint32_t) pMemCfg);

  if ((NULL == pMemCfg) || (memSegFd == (int32_t) NULL))
  {
    memCfgErrCode = MemCfg_ErrorInvalidParam;
  }

  if (MemCfg_ErrorInvalidParam == memCfgErrCode)
  {

    close (memSegFd);

    munmap (pMemCfg, (LDR_CONFIG_ADDR_MEMCFG_SIZE +
                      LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE));
  }

  Log_print1 (Diags_EXIT, "Leaving memcfg_unMapMemCfgSpace, retVal: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}

/**
 *  @brief      Read memory configuration from .bin file
 *
 *  @param      pBuff   buffer to where memory cfg from file needs to be 
 *                      copied.
 *  @param      size    Buffer size
 *
 *  @param      pFileName    points to File name
 *
 *  @retval     MemCfg_ErrorNone      On Success
 *              Other                 On Failure
 *
 *  @pre        @c already Mapped using memcfg_mapMemCfgSpace
 *
 *  @sa         memcfg_mapMemCfgSpace
 */
MemCfg_Error getMemSegFromFile(uint8_t *pBuff, uint32_t size, 
                               uint8_t *pFileName)
{
  MemCfg_Error mcErrCode = MemCfg_ErrorNone;
  FILE *fp;
  int length_read = 0;
  int file_size = 0;

  Log_print3 (Diags_ENTRY, "getMemSegFromFile (0x%x, %d, %s)\n",
              pBuff, size, pFileName);

  if ((pBuff == NULL) || 
      (pFileName == NULL) || 
      (size <= 0)) {
    printf ("Invalid argument\n");
  }

  fp = fopen ((const char *) pFileName, "rb");

  if (fp == NULL) {
    printf ("File can not be opened\n");
  }

  fseek (fp, 0, SEEK_END);
  file_size = ftell(fp);
  fseek (fp, 0, SEEK_SET);
  if (file_size > size)
  {
    printf ("File size is larger than Mem config space\n");
    mcErrCode = MemCfg_ErrorLessMem;
  }
  else {

    Log_print2 (Diags_INFO, "Doing File %s length: %d\n", pFileName, file_size);

    length_read = fread(pBuff, 1, file_size, fp);

    if (length_read != file_size) {
      Log_print2 (Diags_STATUS, "Size read: %d required: %d\n", length_read, 
                  file_size);
    }
    else {
      Log_print2  (Diags_INFO, "Completed reading %d bytes from file : %s\n", 
                 length_read, pFileName);
    }
  }

  fclose(fp);

  Log_print1 (Diags_EXIT, "mv_getRGBDataFromFile > Returning: %d\n", mcErrCode);

  return mcErrCode;
}

/**
 *  @brief      Load the memory segment information to memory
 *
 *  @param      pMemCfgSpace   User space pointer for Memory configuration space
 *
 *  @param      pMegSegCfg  Points to memory configuration which needs to be 
 *                          trasferred to MEMCFG_SPACE pointed by pMemCfgSpace
 *
 *  @param      pFileName memory configuration binary file generated by 
 *              parser utility (host side utility)
 *
 *  @param      multiProcId - Multi proc id
 *
 *  @retval     MemCfg_ErrorNone      On Success
 *              Other                 On Failure
 *
 *  @pre        @cpMemCfg already Mapped using memcfg_mapMemCfgSpace
 *
 *  !pre        @c File sizeof is less or equal to LDR_CONFIG_ADDR_MEMCFG_SIZE
 *  @sa         memcfg_mapMemCfgSpace
 */
MemCfg_Error memcfg_loadMemSegInfo (uint8_t *pMemCfgSpace, LDR_MemSeg *pMemSegCfg,
                                    uint8_t *pFileName, uint32_t multiProcId)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  uint32_t *pMemCfgStatus = NULL;
  LDR_Memseg_Version_Hdr *pVerHdr = NULL;
  LDR_MemSeg *pMemSeg = NULL;

  Log_print4 (Diags_ENTRY, "Entering memcfg_loadMemSegInfo(0x%x, 0x%x, %s, %d)\n", 
              (uint32_t) pMemCfgSpace, pMemSegCfg, pFileName, multiProcId);

  if (NULL == pMemCfgSpace)
  {
    memCfgErrCode = MemCfg_ErrorInvalidParam;
  }

  /* Store version information for check by slaves */
  pVerHdr = (LDR_Memseg_Version_Hdr *) pMemCfgSpace;
  pVerHdr->major = LDR_MEMSEG_VERSION_NUM_MAJOR;
  pVerHdr->minor = LDR_MEMSEG_VERSION_NUM_MINOR;
  pVerHdr->revision = LDR_MEMSEG_VERSION_NUM_REVISION;
  pVerHdr->step = LDR_MEMSEG_VERSION_NUM_STEP;

  printf ("MemCfg: DCMM (Dynamically Configurable Memory Map) Version : " \
          " %d.%d.%d.%d\n", pVerHdr->major, pVerHdr->minor, pVerHdr->revision, 
          pVerHdr->step);

  /* Copy Memory segment information to DDR */
  pMemSeg = (LDR_MemSeg *) (pMemCfgSpace + sizeof (LDR_Memseg_Version_Hdr));

  if (NULL != pMemSegCfg)
  {
    /* Transfer the memseg from buffer */
    /* Copy Memory segment information to DDR */
    memcpy (pMemSeg, pMemSegCfg,
            sizeof (LDR_MemSeg) * ldr_getValidMemSegCnt (pMemSegCfg) + 1);
  }
  else 
  {
    /* Transfer the memsef from file */
    memCfgErrCode = getMemSegFromFile((uint8_t *) pMemSeg,
                                      LDR_CONFIG_ADDR_MEMCFG_SIZE, pFileName);
  }

  /* Check the validity of the memory maps */
  if (ldr_isValidMemMap (pMemSeg) == 0)
  {
    printf ("\n Invalid memory map used\n");
    memCfgErrCode = MemCfg_ErrorInvalidMemMap;
  }

  if (MemCfg_ErrorNone == memCfgErrCode)
  {
    #ifdef LDR_DUMPMEMSEGINFO
    ldr_dumpMemSegInfo (pMemSeg);
    #endif

    /* Initialize the memory configuration status of all cores to predefined
       value */
    pMemCfgStatus = (uint32_t *) (pMemCfgSpace +
                                  LDR_CONFIG_ADDR_MEMCFG_SIZE +
                                  multiProcId * 4);
    *pMemCfgStatus = MemCfg_ErrorMax;
  }

  Log_print1 (Diags_EXIT, "Leaving memcfg_loadMemSegInfo, retVal: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}

/**
 *  @brief      Load the control configuration information to memory
 *
 *  @param      pMemCfg   User space pointer for Memory configuration space
 *
 *  @param      pCtrlCfg  Points to control configuration which needs to be 
 *                          trasferred to MEMCFG_SPACE pointed by pMemCfg
 *
 *  @param      multiProcId - Multi proc id
 *
 *  @retval     MemCfg_ErrorNone      On Success
 *              Other                 On Failure
 *
 *  @pre        @cpMemCfg already Mapped using memcfg_mapMemCfgSpace
 *
 *  !pre        @c File sizeof is less or equal to LDR_CONFIG_ADDR_MEMCFG_SIZE
 *  @sa         memcfg_mapMemCfgSpace
 */
MemCfg_Error memcfg_loadCtrlCfgInfo (uint8_t *pMemCfgSpace, LDR_CtrlCfg *pCtrlCfg,
                                     uint32_t multiProcId)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  uint8_t *pMemCfg = NULL;

  Log_print3 (Diags_ENTRY, "Entering memcfg_loadCtrlCfgInfo(0x%x, 0x%x, %d)\n", 
              (uint32_t) pMemCfg, pCtrlCfg, multiProcId);

  if (NULL == pMemCfgSpace)
  {
    memCfgErrCode = MemCfg_ErrorInvalidParam;
  }

  if (NULL == pCtrlCfg)
  {
    memCfgErrCode = MemCfg_ErrorInvalidParam;
  }

  /* Copy Memory segment information to DDR */
  pMemCfg = (pMemCfgSpace + LDR_CONFIG_ADDR_MEMCFG_SIZE + 
                            LDR_CONFIG_ADDR_MEMCFG_STATUS_SIZE + 
                            LDR_CONFIG_ADDR_SYSINFO_SIZE);

  /* Copy control configuration information to DDR */
  if ((NULL != pMemCfgSpace) && (NULL != pCtrlCfg))
  {
    memcpy (pMemCfg, pCtrlCfg, sizeof (LDR_CtrlCfg));
  }

  Log_print1 (Diags_EXIT, "Leaving memcfg_loadCtrlCfgInfo, retVal: %d\n", 
              memCfgErrCode);

  return memCfgErrCode;
}

/* End Of File */
