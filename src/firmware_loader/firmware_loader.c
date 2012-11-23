/*
 *  Copyright (c) 2011, Texas Instruments Incorporated
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

/*
 *  Media Controller Firmware Loader
 *
 *  TI81xx Media Controller Firmware Loader program.
 */

/* Standard Headers */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

/* Syslink Headers */
#include <ti/syslink/Std.h>
#include <ti/syslink/SysLink.h>
#include <ti/syslink/ProcMgr.h>
#include <ti/syslink/IpcHost.h>

/* SDK memory map Headers */
#include <mem_setup.h>
#include <ldr_memseg.h>
#include <ldr_memcfg.h>

#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>

#define NUM_ARGS 1
#define FIRMWARE_TMPFILE "/tmp/firmware."

/*
 * Debugging sub-system
 */
#define TRACE_MUST         (0)
#define TRACE_ERROR        (1)
#define TRACE_WARNING      (2)
#define TRACE_INFO         (3)
#define TRACE_DEBUG        (4)
#define TRACE_LOG          (5)
#define TRACE_CURR_LEVEL   (TRACE_WARNING)
#define UTL_TRACE(level, fmt...) {             \
            if (level <= currDebugLevel) {   \
              printf (fmt);                    \
            }                                  \
          }                                    \

uint8_t currDebugLevel = TRACE_CURR_LEVEL;

/*--------------------- External references ----------------------------------*/
extern LDR_MemSeg sdk_memseg_default[];

String filePath;

ProcMgr_Handle handle;
ProcMgr_AttachParams attachParams;
ProcMgr_StartParams startParams;

UInt32 fileId = 0;

/* Macros */
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

/* Memory configuration timeout period in ms */
#define MEMCFG_TIMEOUT (2 * 1000)

#define MEMCFG_DEFAULT_MM_FILE "mm_dm81xxbm.bin"

#define MEMCFG_FILE_LENGTH  (100)

#ifdef PLATFORM_TI811X
#define FIRMWARE_STOP_FLAG  (0x9FFFFFFC)
#else
#define FIRMWARE_STOP_FLAG  (0xBFFFEFFC)
#endif
/* Optional arguments to Firmware Loader */
typedef struct Args
{
  char mm_fileName[MEMCFG_FILE_LENGTH];
  uint8_t mmFileParam;
  uint8_t isI2cInitRequiredOnM3;
} FL_ARGS;

void usage (FL_ARGS *argsp)
{
  printf ("Usage : firmware_loader <Processor Id> <Location of Firmware> <start|stop> " \
          "[-mmap <memory_map_file>] [-i2c <0|1>]\n");
  printf ("===Mandatory arguments=== \n"
          "<Processor Id>         0: DSP, 1: Video-M3, 2: Vpss-M3 \n"
          "<Location of Firmware> firmware binary file \n"
          "<start|stop>           to start/stop the firmware \n"
          "===Optional arguments=== \n"
          "-mmap                  input memory map bin file name \n"
          "-i2c                   0: i2c init not done by M3, 1(default): i2c init done by M3 \n");
  return;
}


uint8_t isFileExists(uint8_t *pFileName)
{
  uint8_t ret_val = TRUE;
  FILE *fp = NULL;

  fp = fopen ((char *)pFileName, "rb");
  if (NULL == fp)
  {
    ret_val = FALSE;
  }
  else
  {
    fclose(fp);
    ret_val = TRUE;
  }
  
  return ret_val;
}

int findOpt(int argc, char *argv[], char *opt)
{
  int index = 0;
  int i = 0;

  for (i=0; i<argc-1; i++)
   {
     if(0 == strcmp(opt,argv[i]))
     {
       index = i;
       break;
     }
   }

  return index;
}

/* ========================================================================== */
/**
* parse_args() : This function parses the optional arguments provided to
*                firmware loader
*
* @param argc             : number of args 
* @param argv             : args passed by app
* @param argsp            : parsed data pointer
*
*  @return      
*
*
*/
/* ========================================================================== */

void parse_args (int argc, char *argv[], FL_ARGS *argsp)
{
  int infile = 0, i2c = 0;
  int index = 0;
  
  index = findOpt(argc, argv, "-mmap");
  
  if(index)
   {
     strncpy (argsp->mm_fileName, argv[index+1], MEMCFG_FILE_LENGTH);
     infile = 1;
     argsp->mmFileParam = 1;
   }

  index = findOpt(argc, argv, "-i2c");

  if(index)
   {
     argsp->isI2cInitRequiredOnM3 = atoi (argv[index+1]);
     i2c = 1;
   }

  if (!infile)
  {
    argsp->mmFileParam = 0;
  }

  if (!i2c)
  {
    argsp->isI2cInitRequiredOnM3 = 1;
    printf ("FIRMWARE: I2cInit will be done by M3\n");
  }

  if(argsp->mmFileParam) 
  {
   if(FALSE == isFileExists ((uint8_t *)argsp->mm_fileName))
     {
      printf ("FIRMWARE: Memory map bin file <%s> - not exists\n", argv[4]);
      argsp->mmFileParam = 0;
     }
   if(argsp->mmFileParam)
     {
      printf ("FIRMWARE: memory map bin file: %s\n", argsp->mm_fileName);
     }
  }
  else
  {
      printf ("FIRMWARE: Memory map bin file not passed\n");
      usage (argsp);
  }
  printf ("FIRMWARE: isI2cInitRequiredOnM3: %d\n", argsp->isI2cInitRequiredOnM3);
}


/* Save the Proc ID */
Int save_fileId (UInt16 procId)
{
  FILE *fp;
  char tmpfile[20];

  sprintf (tmpfile, "%s%d", FIRMWARE_TMPFILE, procId);
  fp = fopen (tmpfile, "w");

  if (NULL == fp)
  {
    UTL_TRACE (TRACE_ERROR, "FIRMWARE: Could not open tmp file: %d", -1);

    return -1;
  }

  fwrite (&fileId, sizeof (fileId), 1, fp);

  fclose (fp);

  return 0;
}


/* Retrieve a previously saved Proc ID */
Int read_fileId (UInt16 procId)
{
  FILE *fp;
  char tmpfile[20];

  sprintf (tmpfile, "%s%d", FIRMWARE_TMPFILE, procId);
  fp = fopen (tmpfile, "r");

  if (NULL == fp)
  {
    UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not open tmp file: %d", -1);
    return -1;
  }

  fread (&fileId, sizeof (fileId), 1, fp);

  fclose (fp);

  return 0;
}

/**
 *  @brief      Configure the debugging system, This functio queries the 
 *              environment varible FL_DEBUG (Firmware Loader debug), &
 *              if it is present then set the debug trace level as per that
 *              otherwise defauls teh debug trace to warning. This function
 *              also displays the help for FL_DEBUG.
 *
 *  @param[OUT]  pCurrLevel - Updated with the debug trace level
 *
 *  @retval     void
 *
 *  @pre        None 
 *
 *  @sa         None
 */
static void configTraceLevel(uint8_t *pCurrLevel)
{
  uint8_t *dbgLevelStr = NULL;
  uint8_t status = TRUE;

  dbgLevelStr = (uint8_t *) getenv("FL_DEBUG");

  if (NULL != dbgLevelStr) {
    if(0 == strcmp((char *) dbgLevelStr, "error")) {
      *pCurrLevel = TRACE_ERROR;
    }
    else if(0 == strcmp((char *) dbgLevelStr, "warning")) {
      *pCurrLevel = TRACE_WARNING;
    }
    else if(0 == strcmp((char *) dbgLevelStr, "info")) {
      *pCurrLevel = TRACE_INFO;
    }
    else if(0 == strcmp((char *) dbgLevelStr, "debug")) {
      *pCurrLevel = TRACE_DEBUG;
    }
    else if(0 == strcmp((char *) dbgLevelStr, "log")) {
      *pCurrLevel = TRACE_LOG;
    }
    else {
      status = FALSE;
      UTL_TRACE (TRACE_MUST, "Invalid Firmware Loader debugging : %s\n", 
                 dbgLevelStr);
    }
  }
  else {
      status = FALSE;
      UTL_TRACE (TRACE_MUST, "Firmware Loader debugging not configured\n");
  }
  
  if (FALSE == status) {
      UTL_TRACE (TRACE_MUST, "Default FL_DEBUG: warning\n");
      *pCurrLevel = TRACE_WARNING;
  }
  else {
      UTL_TRACE (TRACE_MUST, "Current FL_DEBUG = %s\n", dbgLevelStr);
  }

  UTL_TRACE (TRACE_MUST, "Allowed FL_DEBUG levels: error, warning, info, debug, log\n");

}

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

  if ((pSrEntry->base == (Ptr) pMemSegEntry->slave_virtual_addr) &&
      (pSrEntry->len == pMemSegEntry->size) &&
      (pSrEntry->ownerProcId == pMemSegEntry->master_core_id) &&
      (1 == pSrEntry->createHeap))
  {
    retVal = 1;
  }

  return retVal;
}

/**
 * @name memstp_mapPhyAddr2UsrVirtual()
 *
 * @brief Function to map physical address to user virtual
 *
 * @param[in] phyAddr  : Physical address to be translated
 *
 * @param[in] len      : Length of memory block to be translated
 *
 * @return none
 */
static uint8_t *memstp_mapPhyAddr2UsrVirtual (uint32_t phyAddr, uint32_t len)
{
  int32_t          status = 0;
  ProcMgr_AddrInfo addrInfo;
  uint8_t *pUsrVirtAddr = NULL;

  UTL_TRACE(TRACE_DEBUG, "Entered memstp_mapPhyAddr2UsrVirtual(0x%x, %d)\n",
              phyAddr, len);

  /* Map the kernel space address to user space */
  addrInfo.addr[ProcMgr_AddrType_MasterPhys] = phyAddr;
  addrInfo.addr[ProcMgr_AddrType_SlaveVirt] = phyAddr;
  addrInfo.size = len;
  addrInfo.isCached = FALSE;

  status = ProcMgr_translateAddr (handle,
                                    (Ptr) & pUsrVirtAddr,
                                    ProcMgr_AddrType_MasterUsrVirt,
                                    (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);

  if (0 > status)
  {
    status = ProcMgr_map (handle,
                          (ProcMgr_MASTERKNLVIRT |
                           ProcMgr_MASTERUSRVIRT),
                          &addrInfo, ProcMgr_AddrType_MasterPhys);
    if (status < 0)
    {
      UTL_TRACE (TRACE_ERROR, "ProcMgr_map Failed status: 0x%x", status);
    }
    else
    {
      UTL_TRACE(TRACE_DEBUG, "ProcMgr_map Success VirtAddr: 0x%x",
                  addrInfo.addr[ProcMgr_AddrType_MasterUsrVirt]);

      status = ProcMgr_translateAddr (handle,
                                      (Ptr) & pUsrVirtAddr,
                                      ProcMgr_AddrType_MasterUsrVirt,
                                      (Ptr) phyAddr, ProcMgr_AddrType_MasterPhys);

    }

    if (status < 0)
    {
      UTL_TRACE(TRACE_DEBUG, "Error in ProcMgr_translateAddr [0x%x]\n", status);
      pUsrVirtAddr = NULL;
    }
    else
    {
      UTL_TRACE (TRACE_DEBUG, "\n\nProcMgr_translateAddr Status [0x%x]"
                  " User Virtual Address [0x%x]\n",
                  status,  (uint32_t) pUsrVirtAddr);
    }
    
  }
  return (pUsrVirtAddr);
}

MemCfg_Error frmldr_createSr (LDR_MemSeg *pMemSegEntry, memcfg_SrInfo *pSrInfo)
{
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  int32_t srErrCode = SharedRegion_S_SUCCESS;
  uint16_t myMultiProcId = 0;
  uint8_t reTry = 1;
  SharedRegion_Entry srCurrEntry;
  uint32_t srBaseVirtual = 0x0;

  UTL_TRACE(TRACE_DEBUG, "Entering memstp_createSr(0x%x, 0x%x)\n",
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
        UTL_TRACE(TRACE_ERROR, "Invalid Memmap : memCfgErrCode=%d \n",
                (uint32_t) memCfgErrCode );
      }
    }
    else
    {
      /* Fill the SR fields */
      SharedRegion_entryInit (&pSrInfo->entry);

      /* Convert the physical address to virtual address. */
      srBaseVirtual = (uint32_t) 
            memstp_mapPhyAddr2UsrVirtual (pMemSegEntry->system_addr,
                                              pMemSegEntry->size);

      pSrInfo->entry.base = (Ptr) srBaseVirtual;
      pSrInfo->entry.len = pMemSegEntry->size;
      pSrInfo->entry.createHeap = 1;
      pSrInfo->entry.isValid = TRUE;

      pSrInfo->entry.ownerProcId = pMemSegEntry->master_core_id;

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

  UTL_TRACE(TRACE_DEBUG, "Leaving memstp_createSr, retVal: %d\n",
              memCfgErrCode);

  return memCfgErrCode;
}

void start_dspServer ()
{
  Int stop_dsp_server = FIRMWARE_STOP_FLAG;
  uint8_t *virt_addr;

  virt_addr = memstp_mapPhyAddr2UsrVirtual (stop_dsp_server, 4);

  *virt_addr = 0;
}

/* Load the firmware and start the processor */
Int start_firmware (UInt16 procId, uint8_t* pMemCfgSpace, uint8_t ipcInit_memCfg)
{
  Int status;
  String args[NUM_ARGS];
  uint32_t idx2HeapHdl = 0;
  uint32_t idx2MemSeg = 0;
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
  LDR_MemSeg *pMemSeg = NULL;
  LDR_MemSeg *pMemSegEntry = NULL;
  memcfg_SrInfo SrInfo = {{0}};
  uint16_t myMultiProcId = MultiProc_self ();
  
  UTL_TRACE(TRACE_DEBUG, "Entered start_firmware\n");

  ProcMgr_getAttachParams (NULL, &attachParams);
  status = ProcMgr_attach (handle, &attachParams);

  if (status < 0)
  {
    UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not attach: ProcMgr status 0x%x\n", status);
    return -1;
  }

  UTL_TRACE(TRACE_DEBUG, "ProcMgr_attach: Success\n");

  args[0] = filePath;
  status = ProcMgr_load (handle, filePath, NUM_ARGS, args, NULL, &fileId);

  if (status < 0)
  {
    UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not load: ProcMgr status 0x%x\n", status);
    return -2;
  }

  UTL_TRACE(TRACE_DEBUG, "ProcMgr_load: Success\n");


  save_fileId (procId);

  if (procId == 0)
      start_dspServer ();
  
  if (TRUE == ipcInit_memCfg) {
    status = Ipc_control (procId, Ipc_CONTROLCMD_LOADCALLBACK, NULL);

    if (status < 0)
    {
      UTL_TRACE (TRACE_ERROR, "FIRMWARE: Ipc_CONTROLCMD_LOADCALLBACK Error: " \
             "ProcMgr status 0x%x\n", status);
      return -3;
    }
    UTL_TRACE(TRACE_DEBUG, "Ipc_control: Success\n");
  }

  ProcMgr_getStartParams (handle, &startParams);
  status = ProcMgr_start (handle, &startParams);

  if (status < 0)
  {
    UTL_TRACE (TRACE_ERROR, "FIRMWARE: Could not start: ProcMgr status 0x%x\n", status);
    return -4;
  }

  if (TRUE == ipcInit_memCfg) {
    status = Ipc_control (procId, Ipc_CONTROLCMD_STARTCALLBACK, NULL);

    if (status < 0)
    {
      UTL_TRACE(TRACE_ERROR, "FIRMWARE: Ipc_CONTROLCMD_STARTCALLBACK Error: " \
             "ProcMgr status 0x%x\n", status);
      return -5;
    }
  }

  if (TRUE == ipcInit_memCfg) {
    idx2HeapHdl = 0;
    pMemSeg = (LDR_MemSeg *) (pMemCfgSpace + sizeof (LDR_Memseg_Version_Hdr));

    /* Loop for all segments */
  
    while ((MemCfg_ErrorNone == memCfgErrCode) &&
           (pMemSeg[idx2MemSeg].valid == 1) &&
           (idx2MemSeg < LDR_MAX_MEMSEG))
    {
      pMemSegEntry = &pMemSeg[idx2MemSeg];

      if ((pMemSegEntry->size > 0) &&
               (pMemSegEntry->core_id_mask & (1 << myMultiProcId)) &&
               (pMemSegEntry->seg_type == LDR_SEGMENT_TYPE_DYNAMIC_SHARED_HEAP))
      {
        memCfgErrCode = frmldr_createSr (pMemSegEntry, &SrInfo);
        if (MemCfg_ErrorNone != memCfgErrCode)
        {
          UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not create SR: ProcMgr status 0x%x\n", status);
        }
      } /* End of else if */

    
      idx2MemSeg++;
    } /* End Of while */
  } /* End Of if check for (TRUE == ipcInit_memCfg) */

  return 0;
}

void stop_dspServer ()
{
  Int stop_dsp_server = FIRMWARE_STOP_FLAG;
  uint8_t *virt_addr;

  virt_addr = memstp_mapPhyAddr2UsrVirtual (stop_dsp_server, 4);

  *virt_addr = 1;
}


/* Stop the processor */
Int stop_firmware (UInt16 procId, uint8_t ipcInit_memCfg)
{

  if (procId == 0)
    stop_dspServer();
   
  if (TRUE == ipcInit_memCfg) {
    Ipc_control (procId, Ipc_CONTROLCMD_STOPCALLBACK, NULL);
  }

  ProcMgr_stop (handle);

  read_fileId (procId);
  ProcMgr_unload (handle, fileId);

  ProcMgr_detach (handle);

  return 0;
}

extern uint32_t ldrmemcfg_ddrSize;

int main (int argc, char *argv[])
{
  Int status;
  UInt16 procId;

  uint8_t *mem_cfg_ptr = NULL;
  MemCfg_Error memCfgErrCode = MemCfg_ErrorNone;
#ifdef CHECK_MEMCFG_STATUS
  struct timeval currTime = {0, 0};
  struct timeval startTime = {0, 0};
  struct timezone tz = {0, 0};
  struct timeval resultTime = {0, 0};
#endif
  uint32_t secondsElapsed = 0;
  uint8_t mm_fileName[MEMCFG_FILE_LENGTH] = {0};
  FL_ARGS args = {{0}};
  LDR_MemSeg *pMegSegCfg = NULL;
  LDR_CtrlCfg sCtrlCfg = {0};
  uint8_t * heapHandle = NULL;
  uint8_t ipcInit_memCfg = TRUE;

  /* Usage: firmware_loader <Processor Id> <Location of Firmware> <start|stop> */
  if (argc < 4)
  {
    usage(&args);
    return -1;
  }
  else 
  {
    if (FALSE == isFileExists ((uint8_t *)argv[2]))
    {
      printf ("FIRMWARE: firmware file <%s> - not exists\n", argv[2]);
      return -1;
    }
  }

  /* Parse optional arguments of firmware loader */
  parse_args (argc, argv, &args);

  if (!args.mmFileParam)
  {
    printf ("FIRMWARE: Default memory configuration is used\n");
    pMegSegCfg = &sdk_memseg_default[0];
  }
  else {
    strcpy((char *) mm_fileName, args.mm_fileName);
  }

  if (args.isI2cInitRequiredOnM3)
  {
    sCtrlCfg.isI2cInitRequiredOnM3 = TRUE;
  }
  else {
    sCtrlCfg.isI2cInitRequiredOnM3 = FALSE;
  }

  procId = strtol (argv[1], NULL, 16);
  filePath = argv[2];

  /* Check if the debug level is set */
  configTraceLevel (&currDebugLevel);

  SysLink_setup ();

#ifdef PLATFORM_TI811X

  /* Display All Multi-proc Ids */
  UTL_TRACE (TRACE_LOG, "DSP      : %d\n", MultiProc_getId("DSP"));
  UTL_TRACE (TRACE_LOG, "VIDEO-M3 : %d\n", MultiProc_getId("VIDEO-M3"));
  UTL_TRACE (TRACE_LOG, "VPSS-M3  : %d\n", MultiProc_getId("VPSS-M3"));
  UTL_TRACE (TRACE_LOG, "HOST     : %d\n", MultiProc_getId("HOST"));
  /*
   * Do not perform IPC control and memory configuration 
   * for HDVPSS core for TI811x platfrom 
   */
  if (MultiProc_getId("VPSS-M3") == procId) {

    UTL_TRACE (TRACE_INFO, "This is Platfrom: TI811x, slave core: DSS-M3\n");
    UTL_TRACE (TRACE_INFO, "IPC control and Mem cfg not required for this\n");

    ipcInit_memCfg = FALSE;
  }
  
#endif

  if (TRUE == ipcInit_memCfg) {
    /* Map memory segment configuration space in DDR */
    mem_cfg_ptr = memcfg_mapMemCfgSpace ();

    /* Load memory map information to DDR, slaves/applications reads the memory
       map configuration from DDR */
    memcfg_loadMemSegInfo (mem_cfg_ptr, pMegSegCfg, mm_fileName, procId);

    /* Load control configuration information to DDR, slaves/applications reads 
       the configuration from DDR */
    memcfg_loadCtrlCfgInfo (mem_cfg_ptr, &sCtrlCfg, procId);
  }

  status = ProcMgr_open (&handle, procId);
  if (status < 0)
  {
    UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not open ProcMgr: ProcMgr status 0x%x\n", status);
    return -1;
  }

  if (0 == strcmp ("start", argv[3]))
  {
    if (start_firmware (procId, mem_cfg_ptr, ipcInit_memCfg) < 0)
    {
      UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not start: %d\n", -1);
      return -1;
    }
    if(procId == 0) 
    {
      heapHandle = SharedRegion_getHeap (0);
      MessageQ_registerHeap (heapHandle, 2);
    }
  }
  else if (0 == strcmp ("stop", argv[3]))
  {
    if (stop_firmware (procId, ipcInit_memCfg) < 0)
    {
      UTL_TRACE(TRACE_ERROR, "FIRMWARE: Could not stop: %d\n", -1);
      return -1;
    }
    if(procId == 0) 
    {
      MessageQ_unregisterHeap (2);
    }
  }
  else
  {
    UTL_TRACE(TRACE_ERROR, "FIRMWARE: Invalid usage %d\n", -1);
    return -1;
  }


  if (TRUE == ipcInit_memCfg) {
    memCfgErrCode = MemCfg_ErrorMax;
    secondsElapsed = 0;

  #ifdef CHECK_MEMCFG_STATUS

    /* Wait till memory configuration done by slave/app or timeout */
    gettimeofday (&startTime, &tz);
    do
    {

      memCfgErrCode = ldr_getMemCfgStatus (mem_cfg_ptr, procId);

      /* Check if timeout has occured */
      gettimeofday (&currTime, &tz);
      timersub (&currTime, &startTime, &resultTime);

      if (MEMCFG_TIMEOUT <=
          (resultTime.tv_usec / 1000 + resultTime.tv_sec * 1000))
      {

        /* Timeout occured */
        memCfgErrCode = MemCfg_ErrorTimeout;
      }
    
      /* Wait for some time before next poll for status */
      if (MemCfg_ErrorMax == memCfgErrCode)
      {

        /* Display status message once in a second */
        if ((resultTime.tv_usec / 1000 + resultTime.tv_sec * 1000) >=
            (secondsElapsed * 1000))
        {

          secondsElapsed++;
          UTL_TRACE(TRACE_ERROR, "FIRMWARE: No response from slave %d on memory configuration\n", 
                 procId);
        }
        usleep (1000 * 5);
      }
    }
    while (MemCfg_ErrorMax == memCfgErrCode);

  #endif /* CHECK_MEMCFG_STATUS */

    /* Display the memory configuration status */
    if (MemCfg_ErrorMax == memCfgErrCode)
    {
      UTL_TRACE(TRACE_DEBUG, "FIRMWARE: Memory Configuration status : %s\n", "In Progress");
    }
    else 
    {
      UTL_TRACE(TRACE_DEBUG, "FIRMWARE: Memory Configuration status : %s\n",
             (char *) ldr_getErrStr (memCfgErrCode));
    }
  }

  ProcMgr_close (&handle);

  SysLink_destroy ();

  if (TRUE == ipcInit_memCfg) {
    /* Map memory segment configuration space in DDR */
    memcfg_unMapMemCfgSpace (mem_cfg_ptr);
  }

  UTL_TRACE (TRACE_MUST, "FIRMWARE: %d %s Successful\n", procId, argv[3]);

  return 0;
}

/* Nothing beyond this point */
