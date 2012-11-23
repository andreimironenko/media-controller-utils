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
 *  @file   dstop.c
 *
 *  @brief  Implementation for Davinci System Top similar to Linux top
 *
 *
 *
 *  @ver    0.1
 *  
 *  ============================================================================
 */
/*
 * Command to compile: (CROSS_COMPILE)gcc -o dstop -I(DST_INC_PATH) 
 *                                        -I(LDR_INC_PATH) dstop.c
 *
 *                     Ex: arm-none-linux-gnueabi-gcc -o dstop -I./ 
 *                                     -I$(EZSDK_ROOT)/slave_loader/inc dstop.c
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
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/*------------------ Memory map ----------------------------------------------*/
#include <ldr_memseg.h>

/*------------------ Profiling -----------------------------------------------*/
#include <sys/time.h>

#include <sys_top.h>

/*******************************************************************************
 * LOCAL DECLARATIONS Defined here, used here
 ******************************************************************************/

/*------------------ Macros --------------------------------------------------*/


/*----------------- Data declarations ----------------------------------------*/
/**
 *  @brief      System Status region mapped File descriptior
 *
 *  @ssrSmfd
 */
int ssrSmfd = 0;

/**
 *  @brief      System Shared region user Virtual address
 *
 *  @ssrUserVirtAddr
 */
char *ssrUserVirtAddr = NULL;

#define NS_CORE_DSP (0x00000001)
#define NS_CORE_VM3 (0x00000002)
#define NS_CORE_DM3 (0x00000004)
#define NS_CORE_A8 (0x00000008)

/* Initially all the cores are enabled */
unsigned int netraSysCoreMasks =
  (NS_CORE_DSP | NS_CORE_VM3 | NS_CORE_DM3 | NS_CORE_A8);

/* Enable/Disable display of Heap module, default it is enabled */
unsigned int dst_modHeap = TRUE;

/* Enable/Disabledisplay of Shared regon, defailt it is enabled */
unsigned int dst_sharedRegion = TRUE;

dstInfo sysInfo;
/*------------------ Functions           -------------------------------------*/

/* Map the status shared region space */

static char *dst_mapDavinciStatusRegion (unsigned int baseAddr,
                                         unsigned int size);

/*
 *  ======== Engine_open ========
 */
/**
 *  @brief      Pasrse the command line arguments
 *
 *  This parses the command line inputs for this utility, few of the
 *  parameters accepted are
 *    -c [core id]
 *    -m [module name]
 *    etc...
 *
 *  @param[in]  args            command line argument list
 *
 *  @retval     void
 *
 *  @sa         None
 */
static void parse_args (char **args)
{
  static const char usage[] =
    "-c coreid [,coreid ...] -m module [h - Heap, s - Shared region ...]";
  const char *cp = NULL;
  char *p;
  int coreId = 0;

  while (*args)
  {
    cp = *(args++);

    while (*cp)
    {
      switch (*cp)
      {
        case '\0':
        case '-':
          break;
        case 'c':
          /* Initialize core masks to 0 */
          netraSysCoreMasks = 0;
          do
          {
            if (cp[1])
            {
              cp++;
            }
            else if (*args)
            {
              cp = *args++;
            }
            else
            {
              printf ("-p argument missing");
            }

            if (sscanf (cp, "%d", &coreId) != 1 || coreId > 3)
            {
              printf ("Bad core id (0-DSP, 1-VM3, 2-DM3, 3-A8)\n");
            }

            netraSysCoreMasks = (netraSysCoreMasks | (1 << coreId));

            if (!(p = strchr (cp, ',')))
            {
              break;
            }

            cp = p;
          } while (*cp);
          break;
        case 'm':
          /* Display by modules */
          /* Initialize all the modules to 0 */
          dst_modHeap = FALSE;
          dst_sharedRegion = FALSE;
          do
          {
            if (cp[1])
            {
              cp++;
            }
            else if (*args)
            {
              cp = *args++;
            }
            else
            {
              printf ("-m argument missing");
            }
  
            if (cp[0] == 'h')
            {
              dst_modHeap = TRUE;
            }
            else if (cp[0] == 's')
            {
              dst_sharedRegion = TRUE;
            }
  
            if (!(p = strchr (cp, ',')))
            {
              break;
            }

            cp = p;
          } while (*cp);
          break;
        default:
          printf ("unknown argument '%c'\nusage:\t%s%s", *cp, args[0], usage);
          break;
      } /* end: switch (*cp) */

      /* advance cp and jump over any numerical args used above */
      if (*cp)
      {
        cp += strspn (&cp[1], "- ,.1234567890") + 1;
      }
    } /* end: while (*cp) */
  }                             /* end: while (*args) */
}

/*
 *  ======== dst_mapDavinciStatusRegion ========
 */
/**
 *  @brief      Map the status shared memory space
 *
 *  The shared memory where the request & response from slave core is stored
 *  is mapped to the user virtual address
 *
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
 *  @post       Status region is mapped
 *
 *
 *  @sa         dst_unmapDavinciStatusRegion()
 */
static char *dst_mapDavinciStatusRegion (unsigned int baseAddr,
                                         unsigned int size)
{
  int dstErrCode = DST_SUCCESS;

  char *mapBase = NULL;

  ssrSmfd = open ("/dev/mem", O_RDWR | O_SYNC);
  if (ssrSmfd == -1)
  {
    printf ("Could not open the mem file \n");
    dstErrCode = DST_FAIL;
  }

  mapBase =
    mmap (0, size, PROT_READ | PROT_WRITE, MAP_SHARED, ssrSmfd, baseAddr);

  if (mapBase == (void *) -1)
  {
    printf ("Could not map System Status Region \n");
    ssrUserVirtAddr = NULL;
  }
  else
  {
    ssrUserVirtAddr = (char *) (mapBase);
  }

  return ssrUserVirtAddr;
}

/**
 *  @brief      Display Summary information for sys_top 
 *
 *  @param      None
 *
 *  @retval     void
 *  
 *  @pre        None
 *
 *  @post       None
 *
 *  @sa         dst_refreshData
 *              dst_displayTerms
 */
static void dst_displaySummary ()
{
  uint32_t coresRunning = 0;
  system ("clear");
  printf ("sys_top Ver : %d.%d.%d.%d\n", SYSTOP_VERSION_NUM_MAJOR,
          SYSTOP_VERSION_NUM_MINOR, SYSTOP_VERSION_NUM_REVISION,
          SYSTOP_VERSION_NUM_STEP);

  if (sysInfo.dspInfo.hdrInfo.running)
  {
    coresRunning++;
  }
  if (sysInfo.vm3Info.hdrInfo.running)
  {
    coresRunning++;
  }
  if (sysInfo.dm3Info.hdrInfo.running)
  {
    coresRunning++;
  }

  /* A8 is always running, only app which integrates sys_top might not run */
  coresRunning++;

  printf ("Number Of Running Cores: %d\n", coresRunning);
          
  /* printf ("DSP Load: UD\n"); */
}

/**
 *  @brief      Retrieve the updated information from Shared memory
 *
 *  @param[IN]  ssrPtr - Points to shared memory
 *
 *  @param[OUT]  pSysInfo - System Informations
 *
 *  @retval     Always return DST_SUCCESS
 *  
 *  @pre        @c MEMCFG_SPACE is mapped, & ssrPtr points to this
 *
 *  @pre        @c Invoked at DST_REFRESH_DURATION interval
 *
 *  @post       None
 *
 *  @sa         dst_refreshData
 */
static int dst_getUptdData (char *ssrPtr, dstInfo *pSysInfo)
{

  memcpy ((void *) pSysInfo, (void *) ssrPtr, sizeof (dstInfo));

  return DST_SUCCESS;
}

/**
 *  @brief      Display the Heap informations
 *
 *  @param      pCoresHeapInfo - Heap information
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayVM3CoreInfo
 *              dst_displayDspCoreInfo
 *              dst_displaySharedRegionInfo
 *              dst_displayCoresHeapInfo
 */
static int dst_displayHeapInfo (char *pHeapName, dst_HeapInfo *pHeapInfo)
{
  int dstErrCode = DST_SUCCESS;

  printf ("%-3.3s%-4.4s:", pHeapName, "Heap");
  printf ("%-4.4s %-11d", "Size", pHeapInfo->totalSize);
  printf ("%-4.4s %-11d", "Used", pHeapInfo->totalSize -
          pHeapInfo->totalFreeSize);
  printf ("%-4.4s %-11d", "MaxU", pHeapInfo->maxUsed);
  printf ("%-4.4s %-11d", "Free", pHeapInfo->totalFreeSize);
  printf ("%-4.4s %-11d", "LarF", pHeapInfo->largestFreeSize);
  printf ("\n");

  return dstErrCode;
}

/**
 *  @brief      Display the Heap List informations
 *
 *  @param      pCoresHeapInfo - Heap information
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayVM3CoreInfo
 *              dst_displayDspCoreInfo
 *              dst_displaySharedRegionInfo
 */
static int dst_displayCoresHeapInfo (dst_CoresHeapInfo *pCoresHeapInfo)
{
  int dstErrCode = DST_SUCCESS;
  uint32_t idx = 0;
  char tempBuff[80];

  /* Loop through all the valid Heap */
  idx = 0;
  while (pCoresHeapInfo->heapInfo[idx].valid)
  {
    sprintf (tempBuff, " %d", idx);
    dst_displayHeapInfo (tempBuff, &pCoresHeapInfo->heapInfo[idx]);
    idx++;
  }

  return dstErrCode;
}

/**
 *  @brief      Display the Shared Region informations
 *
 *  @param      pSrInfo - SR information
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayVM3CoreInfo
 *              dst_displayDspCoreInfo
 */
static int dst_displaySharedRegionInfo (dst_SRInfo *pSrInfo)
{
  int dstErrCode = DST_SUCCESS;
  int i = 0;
  dst_SREntry *pSrEntry = NULL;
  char tempBuff[80];

  printf ("%-7.7s:%-10d", "Num SR", pSrInfo->numSRs);

  printf ("\n");

  for (i = 0; i < pSrInfo->numSRs; i++)
  {
    pSrEntry = &pSrInfo->srEntries[i];
    if (pSrEntry->isValid)
    {
      printf ("%-4.4s %-2d:", "SRIndex", pSrEntry->srIndex);
      printf ("%-4.4s 0x%-9x", "PhyAddr", pSrEntry->phyBaseAddr);
      printf ("%-4.4s 0x%-9x", "VirtAddr", pSrEntry->virtBaseAddr);
      printf ("%-4.4s 0x%-9x", "Size", pSrEntry->size);
      printf ("\n");
      if (pSrEntry->isHeap)
      {
        sprintf (tempBuff, " SR%d", pSrEntry->srIndex);
        dst_displayHeapInfo (tempBuff, &pSrEntry->heapInfo);
      }
    }
  }

  return dstErrCode;
}

/**
 *  @brief      Display the DSP slave informations
 *
 *  @param      pVm3Info - VM3 slave information
 *
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayVM3CoreInfo
 */
static int dst_displayDspCoreInfo (dst_dspInfo *pDspInfo)
{
  int dstErrCode = DST_SUCCESS;

  if (TRUE == dst_modHeap)
  {
    dstErrCode = dst_displayCoresHeapInfo(&pDspInfo->heapInfo);
  }

  if (TRUE == dst_sharedRegion)
  {
    dstErrCode = dst_displaySharedRegionInfo (&pDspInfo->srInfo);
  }
}

/**
 *  @brief      Display the VM3 Core informations
 *
 *  @param      pVm3Info - VM3 core information
 *
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayDspCoreInfo
 */
static int dst_displayVM3CoreInfo (dst_VM3Info *pVm3Info)
{
  int dstErrCode = DST_SUCCESS;

  printf ("\nFirmware Version: %s\n", pVm3Info->fw_version_string);
  if (TRUE == dst_modHeap)
  {
    dstErrCode = dst_displayCoresHeapInfo (&pVm3Info->heapInfo);
  }

  if (TRUE == dst_sharedRegion)
  {
    dstErrCode = dst_displaySharedRegionInfo (&pVm3Info->srInfo);
  }
}

/**
 *  @brief      Display the A8 Core informations
 *
 *  @param       - VM3 core information
 *
 *
 *  @retval     DST_SUCCESS on success
 *              DST_FAIL on any failure
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *              dst_refreshData
 *              dst_displayDspCoreInfo
 */
static int dst_displayA8CoreInfo (dst_a8Info *pa8Info)
{
  int dstErrCode = DST_SUCCESS;

  if (TRUE == dst_sharedRegion)
  {
    dstErrCode = dst_displaySharedRegionInfo (&pa8Info->srInfo);
  }
}

/**
 *  @brief      Check for Slaves sys_top version number
 *
 *  Check the sys_top modules version number, This function also checks if the
 *  slave is running
 *  Note: If slave does not integrates sys_top then slave can not communicate  
 *        to sys_top utility that it is running
 *
 *  @param      pSysInfo - System Information
 *
 *  @param      pHdrInfo - Slaves Header information
 *
 *  @retval     DST_SUCCESS if Core is running & version number matches
 *              DST_FAIL on any failure
 *
 *  @pre        @c This function invoked for every slave
 *  
 *  @pre        @c System information retrieved from shared memory
 *
 *  @post       None
 *
 *  @sa         dst_getUptdData
 *
 */
static int isCoreRunVerMatch (dstInfo *pSysInfo, dst_HeaderInfo *pHdrInfo, 
                              uint8_t multiProcCoreId)
{
  int dstErrCode = DST_SUCCESS;
  uint8_t   verMisMatch = 0;

  if (!pHdrInfo->running)
  {

    if (NS_CORE_A8 != multiProcCoreId) 
    {
      /* A8 is always running, sys_top itself is running on A8 */
      printf ("Not Running or ");
    }
    printf ("Do not integrate sys_top functionality\n");
    dstErrCode = DST_FAIL;
  }
  else
  {
    if ((pHdrInfo->verNumMajor == SYSTOP_VERSION_NUM_MAJOR))
    {

      if (!((pHdrInfo->verNumMinor == SYSTOP_VERSION_NUM_MINOR) &&
            (pHdrInfo->verNumRevision == SYSTOP_VERSION_NUM_REVISION) &&
            (pHdrInfo->verNumStep == SYSTOP_VERSION_NUM_STEP)))
      {

        printf ("Compatibility version maintained, " \
                 "But different version of sys_top is being used\n");
        verMisMatch = 1;
      }

      dstErrCode = DST_SUCCESS;
    }
    else
    {

      printf ("Compatibility break in sys_top version number\n");
      printf ("Information on other cores might be incorrect\n");
      verMisMatch = 1;
      dstErrCode = DST_FAIL;
    }
  }

  if (verMisMatch)
  {
      printf ("    Current: %d.%d.%d.%d", pHdrInfo->verNumMajor, 
              pHdrInfo->verNumMinor, pHdrInfo->verNumRevision, 
              pHdrInfo->verNumStep);
      printf ("    Expected %d.%d.%d.%d\n", SYSTOP_VERSION_NUM_MAJOR, 
              SYSTOP_VERSION_NUM_MINOR, SYSTOP_VERSION_NUM_REVISION, 
              SYSTOP_VERSION_NUM_STEP);
  }

  return dstErrCode;
}

/**
 *  @brief      Refresh the screen with new data
 *
 *  Refreshes the screen with new data
 * 
 *  @param      pSysInfo - System Information
 *
 *  @retval     dstErrCode
 *
 *  @pre        @c New data retrieved from shared memory
 *  
 *  @pre        @c Screen is cleared
 *
 *  @pre        @c This funciton invoked once in the interval 
 *                 DST_REFRESH_DURATION ms
 *
 *  @post       None
 *
 *  @sa         dst_displaySummary
 *              dst_getUptdData
 *              dst_refreshData
 *              dst_displayTerms
 *
 */
static int dst_refreshData (dstInfo *pSysInfo)
{
  int dstErrCode = DST_SUCCESS;


  if ((netraSysCoreMasks & NS_CORE_DSP))
  {
    printf ("\n%-7.7s:\n", "DSP");
    if (DST_SUCCESS == isCoreRunVerMatch (pSysInfo, &pSysInfo->dspInfo.hdrInfo,
                                          NS_CORE_DSP))
    {
      dst_displayDspCoreInfo (&pSysInfo->dspInfo);
    }
  }


  if (netraSysCoreMasks & NS_CORE_VM3)
  {
    printf ("\n%-10.10s:\n", "MC.HDVICP2");
    if (DST_SUCCESS == isCoreRunVerMatch (pSysInfo, &pSysInfo->vm3Info.hdrInfo,
                                          NS_CORE_VM3))
    {
      dst_displayVM3CoreInfo (&pSysInfo->vm3Info);
    }
  }

  if (netraSysCoreMasks & NS_CORE_DM3)
  {
    printf ("\n%-9.9s:\n", "MC.HDVPSS");
    if (DST_SUCCESS == isCoreRunVerMatch (pSysInfo, &pSysInfo->dm3Info.hdrInfo,
                                          NS_CORE_DM3))
    {
      dst_displayVM3CoreInfo (&pSysInfo->dm3Info);
    }
  }

  #ifdef ENABLE_SYS_TOP_FOR_A8
  if (netraSysCoreMasks & NS_CORE_A8)
  {
    printf ("\n%-7.7s:\n", "A8");
    if (DST_SUCCESS == isCoreRunVerMatch (pSysInfo, &pSysInfo->a8Info.hdrInfo,
                                          NS_CORE_A8))
    {
      dst_displayA8CoreInfo (&pSysInfo->a8Info);
    }
  }
  #endif

  return dstErrCode;
}

/**
 *  @brief      Display Terms
 *
 *  Shows the meanings for short key words used in the display
 * 
 *  @param      void
 *
 *  @retval     void
 *
 *  @pre        @c None
 *
 *  @post       None
 *
 */
static void dst_displayTerms ()
{
  printf ("\n");
  printf ("Legend:\n");
  printf ("%-4.4s - %-20.20s", "LarF", "Largest Free size");
  printf ("%-4.4s - %-20.20s\n", "SRIn", "Shared Region Index");
  printf ("%-4.4s - %-20.20s", "PhyA", "Physical Address");
  printf ("%-4.4s - %-20.20s\n", "Virt", "Virtual Address");
  printf ("%-4.4s - %-20.20s", "MaxU", "Maximum Used Size");
  printf ("%-4.4s - %-20.20s\n", "SR", "Shared Region");
  printf ("%-4.4s - %-20.20s", "MC", "Media Controller");
  /* printf ("%-4.4s - %-20.20s", "UD", "Under Development"); */
}

/**
 *  @brief      Entry point for sys_top application used in EZSDK
 *
 *  Trying to keep the same behaviour as Linux top 
 * 
 *  @param[in]  argc       
 *  @param[in]  argv
 *
 *  @retval     Never returns, Ctrl-C to stop the application
 *
 *  @pre        @c All the slave applications integrates sys_top functionality 
 *                 in them
 *
 *  @post       Status region is mapped
 *
 *  @sa         systop_update utility function in slaves
 */
int
main (int argc, char *argv[])
{
  int dstErrCode = DST_SUCCESS;
  char *ssrPtr = NULL;
  dst_HeaderInfo *pHdrInfo = NULL;

  parse_args (&argv[1]);

  ssrPtr = dst_mapDavinciStatusRegion (LDR_CONFIG_ADDR_SYSINFO_BASE,
                                       LDR_CONFIG_ADDR_SYSINFO_SIZE);

  if (NULL == ssrPtr)
  {
    printf ("Unable to map System Status Region\n");
    dstErrCode = DST_FAIL;
  }

  if (DST_SUCCESS == dstErrCode)
  {
    /* Clear the status region */
    memset (ssrPtr, 0, LDR_CONFIG_ADDR_SYSINFO_SIZE);
  }

  /* Write version number to Config space for all cores to check */
  pHdrInfo = (dst_HeaderInfo *) ssrPtr;
  pHdrInfo->running = 1;
  pHdrInfo->verNumMajor = SYSTOP_VERSION_NUM_MAJOR;
  pHdrInfo->verNumMinor = SYSTOP_VERSION_NUM_MINOR;
  pHdrInfo->verNumRevision = SYSTOP_VERSION_NUM_REVISION;
  pHdrInfo->verNumStep = SYSTOP_VERSION_NUM_STEP;

  /* Refresh the screen every one sec */
  for (; (DST_SUCCESS == dstErrCode);)
  {
    if (DST_SUCCESS == dstErrCode)
    {
      dst_displaySummary (ssrPtr);

      dst_getUptdData (ssrPtr, &sysInfo);

      dst_refreshData (&sysInfo);
      
      dst_displayTerms();
      fflush (stdout);
      usleep (DST_REFRESH_DURATION * 1000);
    }
  }

  /* Shutdown the App */
  /* dst_unmapDavinciStatusRegion(ssrPtr); */

  return (0);
}

/* End Of File */
