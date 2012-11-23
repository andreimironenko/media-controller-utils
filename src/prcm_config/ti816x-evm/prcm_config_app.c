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
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/
 *        product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

/*
 * Trace prints
 */
#define DISPLAY_STR(str)  printf (str);

#define TRACE_PRINT(mstr) printf (mstr);

#define TRACE_ERROR(str)  printf (str);
#define TRACE_ERROR1(str, x)  printf (str, x);

#define DISP_ADDRPHY_ADDRVIRT_DATA(x, y, z)    \
         printf ("\t\t\tPhy Addr : 0x%0.8x Data : 0x%0.8x\n", x,  z);

#define DISP_ADDRPHY_ADDRVIRT_DATA_BW(x, y, z) \
         printf ("\t\t\tBW Phy Addr : 0x%0.8x Data : 0x%0.8x\n", x,  z);

#define DISP_ADDRPHY_ADDRVIRT_DATA_AW(x, y, z) \
         printf ("\t\t\tAW Phy Addr : 0x%0.8x Data : 0x%0.8x\n", x,  z);

#define MMAP_SIZE (1024*1024)
#define MMAP_MASK (MMAP_SIZE-1)

#define PRCM_APP_VERSION_NUM_MAJOR     (2)
#define PRCM_APP_VERSION_NUM_MINOR     (0)
#define PRCM_APP_VERSION_NUM_REVISION  (0)
#define PRCM_APP_VERSION_NUM_STEP      (1)

/*
 * PRCM base address
 */
#define PRCM_BASE_ADDR                  (0x48180000)

/* IVAHD0 PRCM memory register base addresses */
#define CM_IVAHD0_CLKSTCTRL             (PRCM_BASE_ADDR + 0x0600) 
#define CM_IVAHD0_IVAHD_CLKCTRL         (PRCM_BASE_ADDR + 0x0620) 
#define CM_IVAHD0_SL2_CLKCTRL           (PRCM_BASE_ADDR + 0x0624)  
#define PM_IVAHD0_PWRSTST               (PRCM_BASE_ADDR + 0x0C04)  
#define RM_IVAHD0_RSTCTRL               (PRCM_BASE_ADDR + 0x0C10)  
#define RM_IVAHD0_RSTST                 (PRCM_BASE_ADDR + 0x0C14)  

/* IVAHD1 PRCM memory register base addresses */
#define CM_IVAHD1_CLKSTCTRL             (PRCM_BASE_ADDR + 0x0700) 
#define CM_IVAHD1_IVAHD_CLKCTRL         (PRCM_BASE_ADDR + 0x0720) 
#define CM_IVAHD1_SL2_CLKCTRL           (PRCM_BASE_ADDR + 0x0724)  
#define PM_IVAHD1_PWRSTST               (PRCM_BASE_ADDR + 0x0D04)  
#define RM_IVAHD1_RSTCTRL               (PRCM_BASE_ADDR + 0x0D10)  
#define RM_IVAHD1_RSTST                 (PRCM_BASE_ADDR + 0x0D14)  

/* IVAHD2 PRCM memory register base addresses */
#define CM_IVAHD2_CLKSTCTRL             (PRCM_BASE_ADDR + 0x0800) 
#define CM_IVAHD2_IVAHD_CLKCTRL         (PRCM_BASE_ADDR + 0x0820) 
#define CM_IVAHD2_SL2_CLKCTRL           (PRCM_BASE_ADDR + 0x0824)  
#define PM_IVAHD2_PWRSTST               (PRCM_BASE_ADDR + 0x0E04)  
#define RM_IVAHD2_RSTCTRL               (PRCM_BASE_ADDR + 0x0E10)  
#define RM_IVAHD2_RSTST                 (PRCM_BASE_ADDR + 0x0E14)  

/* Device Id Base address register */
#define DM816X_DEVICE_ID_BASE  (0x48140040)

/* Write to 32 bit register */
static void WR_MEM_32(unsigned int addr, unsigned int data);

/* Read from 32 bit register */
static unsigned int RD_MEM_32(unsigned int addr);

/*
 * File descriptor for physically mapped region
 */
int mmap_fd;

/*
 * User Virtual address for physically mapped region
 */
volatile unsigned int *virt_addr;

/* Write to 32bit register */
static void WR_MEM_32(unsigned int addr, unsigned int data)
{
  void *map_base;
  unsigned int data_bs;
  unsigned int data_as;
  unsigned int size;
  off_t target;

  target = addr;
  size = 4;

  /* Map one page */
  map_base = mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                  mmap_fd, target & ~MMAP_MASK);
  if(map_base == (void *) -1){
    TRACE_ERROR1 ("Could not map the register at addr: 0x%x \n", addr);
  }

  virt_addr = (unsigned int *)(map_base + (target & MMAP_MASK));
  data_bs = *virt_addr;
  *virt_addr = data;
  data_as = *virt_addr;

  DISP_ADDRPHY_ADDRVIRT_DATA_BW(target, virt_addr, data_bs);
  DISP_ADDRPHY_ADDRVIRT_DATA_AW(target, virt_addr, data_as);

  munmap(map_base, MMAP_SIZE);
}

/* Read from 32bit register */
static unsigned int RD_MEM_32(unsigned int addr)
{
  void *map_base;
  unsigned int data;
  unsigned int data_as;
  unsigned int size;
  off_t target;

  target = addr;
  size = 4;
  
  /* Map one page */
  map_base = mmap(0, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 
                  mmap_fd, target & ~MMAP_MASK);
  if(map_base == (void *) -1){
    TRACE_ERROR1 ("Could not map the register at addr: 0x%x \n", addr);
  }

  virt_addr = (unsigned int*)(map_base + (target & MMAP_MASK));
  data = *virt_addr;
  DISP_ADDRPHY_ADDRVIRT_DATA(target, virt_addr, data);

  munmap(map_base, MMAP_SIZE);

  return data;
}

/* Display device type */
static void disp_deviceType()
{
  unsigned int device_id;
  device_id = RD_MEM_32(DM816X_DEVICE_ID_BASE);
  DISPLAY_STR ("Device Type : ");
  if (((device_id & 0x700)>>8) == 3) {
    DISPLAY_STR ("GP\n");
  }
  else if (((device_id & 0x700)>>8) == 0) {
    DISPLAY_STR ("TEST\n");
  }
  else {
    DISPLAY_STR ("Unkown\n");
  }
}

/* Display All Device Data */
static void disp_add()
{
  disp_deviceType();
}

/* Do PRCM for IVAHD0 */
static void IVAHD0ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD0 is in Progress, Please wait.....  \n");
  WR_MEM_32(CM_IVAHD0_CLKSTCTRL, 		2); /*Enable Power Domain Transition*/
  while(RD_MEM_32(PM_IVAHD0_PWRSTST)!=0x37);	/*Check Power is ON*/
  WR_MEM_32(CM_IVAHD0_IVAHD_CLKCTRL, 	2); /*Enable IVHD0 Clocks*/
  WR_MEM_32(CM_IVAHD0_SL2_CLKCTRL, 	2); /*Enable IVHD0 SL2 Clocks*/

  /*IVAHD0_GCLK is Active*/
  while(((RD_MEM_32(CM_IVAHD0_CLKSTCTRL)&0x100))!=0x100); 

  WR_MEM_32(RM_IVAHD0_RSTCTRL, 	3); /*Enable IVHD0 logic & SL2 */

  while((RD_MEM_32(RM_IVAHD0_RSTST) & 0x4) !=4);

  WR_MEM_32(0x58088000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT1 ITCM 0 Location*/

  WR_MEM_32(0x58098000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT2 ITCM 0 Location*/

  WR_MEM_32(RM_IVAHD0_RSTCTRL, 	0); /* Bring ICONT1 & ICONT2 out of Reset*/

  while(RD_MEM_32(RM_IVAHD0_RSTST)!=7); /*ICONT1 & ICONT2 are out of Reset*/

  TRACE_PRINT("\tPRCM for IVHD0 is Done Successfully  \n");
}

/* Do PRCM for IVAHD1 */
static void IVAHD1ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD1 is in Progress, Please wait.....  \n");
  WR_MEM_32(CM_IVAHD1_CLKSTCTRL, 		2); /*Enable Power Domain Transition*/
  while(RD_MEM_32(PM_IVAHD1_PWRSTST)!=0x37);	/*Check Power is ON*/
  WR_MEM_32(CM_IVAHD1_IVAHD_CLKCTRL, 	2); /*Enable IVHD1 Clocks*/
  WR_MEM_32(CM_IVAHD1_SL2_CLKCTRL, 	2); /*Enable IVHD1 SL2 Clocks*/

  /*IVAHD1_GCLK is Active*/
  while(((RD_MEM_32(CM_IVAHD1_CLKSTCTRL)&0x100))!=0x100); 

  WR_MEM_32(RM_IVAHD1_RSTCTRL, 	3); /*Enable IVHD1 logic & SL2 */

  while((RD_MEM_32(RM_IVAHD1_RSTST) & 0x4) !=4);

  WR_MEM_32(0x5A088000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT1 ITCM 0 Location*/

  WR_MEM_32(0x5A098000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT2 ITCM 0 Location*/

  WR_MEM_32(RM_IVAHD1_RSTCTRL, 	0); /* Bring ICONT1 & ICONT2 out of Reset*/
	
  while(RD_MEM_32(RM_IVAHD1_RSTST)!=7); /*ICONT1 & ICONT2 are out of Reset*/

  TRACE_PRINT("\tPRCM for IVHD1 is Done Successfully  \n");
}

/* Do PRCM for IVAHD2 */
static void IVAHD2ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD2 is in Progress, Please wait.....  \n");
  WR_MEM_32(CM_IVAHD2_CLKSTCTRL, 		2); /*Enable Power Domain Transition*/
  while(RD_MEM_32(PM_IVAHD2_PWRSTST)!=0x37);	/*Check Power is ON*/
  WR_MEM_32(CM_IVAHD2_IVAHD_CLKCTRL, 	2); /*Enable IVHD2 Clocks*/
  WR_MEM_32(CM_IVAHD2_SL2_CLKCTRL, 	2); /*Enable IVHD2 SL2 Clocks*/

  /*IVAHD2_GCLK is Active*/
  while(((RD_MEM_32(CM_IVAHD2_CLKSTCTRL)&0x100))!=0x100); 

  WR_MEM_32(RM_IVAHD2_RSTCTRL, 	3); /*Enable IVHD1 logic & SL2 */

  while((RD_MEM_32(RM_IVAHD2_RSTST) & 0x4) !=4);

  WR_MEM_32(0x53088000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT1 ITCM 0 Location*/

  WR_MEM_32(0x53098000, 0xEAFFFFFE); /* Write Self Branch Instruction 
                                      * in ICONT2 ITCM 0 Location*/

  WR_MEM_32(RM_IVAHD2_RSTCTRL, 	0); /* Bring ICONT1 & ICONT2 out of Reset*/
	
  while(RD_MEM_32(RM_IVAHD2_RSTST)!=7); /*ICONT1 & ICONT2 are out of Reset*/

  TRACE_PRINT("\tPRCM for IVHD2 is Done Successfully  \n");	
}

/* Do PRCM init */
static void prcmInit()
{

  /* Only IVA-HDs is being powered on by this PRCM utility */

  /* CPU_Bringup->IVAHD0 */
  IVAHD0ClkEnable();

  /* CPU_Bringup->IVAHD1 */
  IVAHD1ClkEnable();

  /* CPU_Bringup->IVAHD2 */
  IVAHD2ClkEnable();
  
  TRACE_PRINT("PRCM Initialization completed \n");
}

/* Display PRCM details for IVAHD0 */
static void print_IVAHD0ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD0 is in Progress, Please wait.....  \n");
  RD_MEM_32(CM_IVAHD0_CLKSTCTRL); /*Enable Power Domain Transition*/
  /* while(RD_MEM_32(PM_IVAHD0_PWRSTST)!=0x37); */ /*Check Power is ON*/
  RD_MEM_32(CM_IVAHD0_IVAHD_CLKCTRL); /*Enable IVHD0 Clocks*/
  RD_MEM_32(CM_IVAHD0_SL2_CLKCTRL); /*Enable IVHD0 SL2 Clocks*/

  /*IVAHD0_GCLK is Active*/
  /* while(((RD_MEM_32(CM_IVAHD0_CLKSTCTRL)&0x100))!=0x100); */

  RD_MEM_32(RM_IVAHD0_RSTCTRL); /*Enable IVHD0 logic & SL2 */

  /* while(RD_MEM_32(RM_IVAHD0_RSTST)!=4); */

  RD_MEM_32(0x58088000); /* Write Self Branch Instruction 
                          * in ICONT1 ITCM 0 Location*/

  RD_MEM_32(0x58098000); /* Write Self Branch Instruction 
                          * in ICONT2 ITCM 0 Location*/

  RD_MEM_32(RM_IVAHD0_RSTCTRL); /*Bring ICONT1 & ICONT2 out of Reset*/
	
  /*ICONT1 & ICONT2 are out of Reset*/
  /* while(RD_MEM_32(RM_IVAHD0_RSTST)!=7); */ 

  TRACE_PRINT("\tPRCM for IVHD0 is Done Successfully  \n");
}

/* Display PRCM details for IVAHD1 */
static void print_IVAHD1ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD1 is in Progress, Please wait.....  \n");
  RD_MEM_32(CM_IVAHD1_CLKSTCTRL); /*Enable Power Domain Transition*/
  /* while(RD_MEM_32(PM_IVAHD1_PWRSTST)!=0x37); */ /*Check Power is ON*/
  RD_MEM_32(CM_IVAHD1_IVAHD_CLKCTRL); /*Enable IVHD1 Clocks*/
  RD_MEM_32(CM_IVAHD1_SL2_CLKCTRL); /*Enable IVHD1 SL2 Clocks*/

  /*IVAHD1_GCLK is Active*/
  /* while(((RD_MEM_32(CM_IVAHD1_CLKSTCTRL)&0x100))!=0x100); */

  RD_MEM_32(RM_IVAHD1_RSTCTRL); /*Enable IVHD1 logic & SL2 */

  /*while(RD_MEM_32(RM_IVAHD1_RSTST)!=4);*/

  RD_MEM_32(0x5A088000); /* Write Self Branch Instruction 
                          * in ICONT1 ITCM 0 Location*/

  RD_MEM_32(0x5A098000); /* Write Self Branch Instruction 
                          * in ICONT2 ITCM 0 Location*/

  RD_MEM_32(RM_IVAHD1_RSTCTRL); /*Bring ICONT1 & ICONT2 out of Reset*/
	
  /*while(RD_MEM_32(RM_IVAHD1_RSTST)!=7); *//*ICONT1 & ICONT2 are out of Reset*/

  TRACE_PRINT("\tPRCM for IVHD1 is Done Successfully  \n");	
}

/* Display PRCM details for IVAHD2 */
static void print_IVAHD2ClkEnable()
{
  TRACE_PRINT("\tPRCM for IVHD2 is in Progress, Please wait.....  \n");								
  RD_MEM_32(CM_IVAHD2_CLKSTCTRL); /*Enable Power Domain Transition*/
  /* while(RD_MEM_32(PM_IVAHD2_PWRSTST)!=0x37); *//*Check Power is ON*/
  RD_MEM_32(CM_IVAHD2_IVAHD_CLKCTRL); /*Enable IVHD2 Clocks*/
  RD_MEM_32(CM_IVAHD2_SL2_CLKCTRL); /*Enable IVHD2 SL2 Clocks*/

  /*IVAHD2_GCLK is Active*/
  /* while(((RD_MEM_32(CM_IVAHD2_CLKSTCTRL)&0x100))!=0x100); */

  RD_MEM_32(RM_IVAHD2_RSTCTRL); /*Enable IVHD1 logic & SL2 */

  /* while(RD_MEM_32(RM_IVAHD2_RSTST)!=4); */

  RD_MEM_32(0x53088000); /* Write Self Branch Instruction 
                          * in ICONT1 ITCM 0 Location*/

  RD_MEM_32(0x53098000); /* Write Self Branch Instruction 
                          * in ICONT2 ITCM 0 Location*/

  RD_MEM_32(RM_IVAHD2_RSTCTRL); /*Bring ICONT1 & ICONT2 out of Reset*/
	
  /* while(RD_MEM_32(RM_IVAHD2_RSTST)!=7); */ /*ICONT1 & ICONT2 are out of Reset*/

  TRACE_PRINT("\tPRCM for IVHD2 is Done Successfully  \n");								
}

/* Display PRCM register contents */
static void print_prcmInit()
{
  /* Only IVA-HDs is being powered on by this PRCM utility */

  /* Bringup->IVAHD0 */
  print_IVAHD0ClkEnable();

  /* CPU_Bringup->IVAHD1 */
  print_IVAHD1ClkEnable();

  /* CPU_Bringup->IVAHD2 */
  print_IVAHD2ClkEnable();

  DISPLAY_STR ("\tPRCM Print completed \n");
}

/* Debug API: Test the register by writing and reading back */
static void test_register (unsigned int addr, unsigned int data) 
{
  DISPLAY_STR ("Before write\n");
  RD_MEM_32(addr);

  DISPLAY_STR ("Writing data\n");
  WR_MEM_32(addr, data);

  DISPLAY_STR ("After writing\n");
  RD_MEM_32(addr);
}

/*
 * Main entry point for PRCM config app
 */
int main(int argc, char **argv) 
{
  int           offset      = 0;	
  void         *map_base    = NULL; 
  unsigned long read_result = 0;
  unsigned long writeval    = 0;
  unsigned int  addr        = 0;
  unsigned int  data        = 0;
  
  printf ("DM816X %s version: %d.%d.%d.%d\n", argv[0], PRCM_APP_VERSION_NUM_MAJOR, 
          PRCM_APP_VERSION_NUM_MINOR, PRCM_APP_VERSION_NUM_REVISION, 
          PRCM_APP_VERSION_NUM_STEP);

  if(argc < 2) {

    printf("Usage: %s [a|p|s|t]\n",  argv[0]);
    /* a - All device details 
     * p - print PRCM register contents
     * s - set PRCM
     * t - Test register by writing & read back
     */

    exit(1);
  }

  if((mmap_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
    TRACE_ERROR ("Could not open the mem file \n");
  }

  switch (argv[1][0]) {

    case 'a':
      printf ("DM816X Device Details\n");
      disp_add();
      break;

    case 's':
      printf ("Doing PRCM settings...\n");
      prcmInit();
      break;

    case 'p':
      printf ("PRCM settings\n");
      print_prcmInit();
      break;

    case 't':

      if (argc <4) {
        printf ("Please enter test address & size\n");
        exit(0);
      }

      addr = strtoul(argv[2], 0, 0);
      data = strtoul(argv[3], 0, 0);

      printf ("Trying to test Addr: 0x%x data :0x%x\n", addr, data);
      test_register(addr, data);

      break;

    default:
      printf ("Choose right option\n");
      break;
  }

  close(mmap_fd);

  return 0;
}

/* End Of File */

