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
 *  @file   mm_host_util_main.c
 *
 *  @brief  Implementation for host side memory map binary file generation 
 *          utility.
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
/*
 * Command to compile & create bin file: make all
 * 
 */

/*******************************************************************************
 *                             Compilation Control Switches
 ******************************************************************************/


/*-------------------------------- Build time checks -------------------------*/
/* None */

/*******************************************************************************
 *                             INCLUDE FILES
 ******************************************************************************/
/*------------------ System & platform files ---------------------------------*/
#include <stdio.h>

/*------------------ Memory map ----------------------------------------------*/
#include <ldr_memseg.h>

/*******************************************************************************
 * LOCAL DECLARATIONS Defined here, used here
 ******************************************************************************/

/*------------------ Macros --------------------------------------------------*/


/*----------------- Data declarations ----------------------------------------*/

/*******************************************************************************
 * EXTERNAL DECLARATIONS Defined elsewhere, used here
 ******************************************************************************/
extern LDR_MemSeg sdk_memseg[];

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
static uint32_t getValidMemSegCnt (LDR_MemSeg *pMemSeg)
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
 *  @brief      Entry point for mm_host_util tool application used in EZSDK
 *
 *  @param[in]  argc       
 *  @param[in]  argv
 *
 *  @retval     
 *
 *  @pre        @c linked with memsegdef_<usecase>.c file which defined 
 *                 sdk_memseg
 *
 *  @post       binary file with the name passed via command line is generated
 *
 *  @sa         .bin files, definition of LDR_MemSeg, firmware loader
 */
int
main (int argc, char *argv[])
{
  FILE *fp = NULL;
  uint32_t memseg_size = 0;
  uint32_t memseg_cnt = 0;
  uint32_t written_size = 0;

  if (argc != 2)
  {
    printf ("Usage: %s <mem map bin file name>\n", argv[0]);
    goto EXIT1;
  }

  memseg_cnt = getValidMemSegCnt(sdk_memseg);

  /* +1 to indicate Last segment */
  memseg_size = (memseg_cnt + 1) * sizeof(LDR_MemSeg);
  printf ("Size of memory map structure: %d\n",  memseg_size);
  printf ("Number of memory segments: %d\n", memseg_cnt);

  fp = fopen (argv[1], "wb");

  if (NULL == fp) 
  {
    printf ("Memory Map binary file %s can not be opened\n", argv[1]);

    goto EXIT2;
  }

  written_size = fwrite (sdk_memseg, 1, memseg_size, fp);

  if (written_size != memseg_size)
  {
    printf ("Could not write size : %d to memory map bin file\n", memseg_size);
  }

EXIT2:
  fclose (fp);

EXIT1:

  return (0);
}

/* End Of File */
