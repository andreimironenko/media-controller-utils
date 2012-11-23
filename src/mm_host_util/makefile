#FIRMWARE_LOADER_INSTALL_DIR = _your_firmware_loader_install_
#DESTDIR = _your_output_file_install_
#HOST_CODEGEN_INSTALL_DIR = _your_host_code_gen_install_

FIRMWARE_LOADER_INSTALL_DIR = ../
DESTDIR = ./
HOST_CODEGEN_INSTALL_DIR = 

# Include EZSDK Rules.make if it exists
#-include $(EZSDK_ROOT)/Rules.make

ALL_CFLAGS = -I${FIRMWARE_LOADER_INSTALL_DIR}/ldrmemcfg -g
ALL_LDFLAGS = 


all: mm_dm81xxbm.bin mm_dm81xxbm_512M.bin

mm_dm81xxbm.bin: memsegdef_dm81xxbm
	./memsegdef_dm81xxbm mm_dm81xxbm.bin

memsegdef_dm81xxbm: memsegdef_dm81xxbm.o mm_host_util_main.o
	$(HOST_CODEGEN_INSTALL_DIR)gcc -o $(DESTDIR)/memsegdef_dm81xxbm memsegdef_dm81xxbm.o mm_host_util_main.o
  
memsegdef_dm81xxbm.o: memsegdef_dm81xxbm.c
	$(HOST_CODEGEN_INSTALL_DIR)gcc $(ALL_CFLAGS) -c memsegdef_dm81xxbm.c

mm_host_util_main.o: mm_host_util_main.c
	$(HOST_CODEGEN_INSTALL_DIR)gcc $(ALL_CFLAGS) -c mm_host_util_main.c

mm_dm81xxbm_512M.bin: memsegdef_dm81xxbm_512M
	./memsegdef_dm81xxbm_512M mm_dm81xxbm_512M.bin

memsegdef_dm81xxbm_512M: memsegdef_dm81xxbm_512M.o mm_host_util_main.o
	$(HOST_CODEGEN_INSTALL_DIR)gcc -o $(DESTDIR)/memsegdef_dm81xxbm_512M memsegdef_dm81xxbm_512M.o mm_host_util_main.o
  
memsegdef_dm81xxbm_512M.o: memsegdef_dm81xxbm_512M.c
	$(HOST_CODEGEN_INSTALL_DIR)gcc $(ALL_CFLAGS) -c memsegdef_dm81xxbm_512M.c

clean: mm_dm81xxbm_clean mm_dm81xxbm_512M_clean
	rm -f *.bin *.o

mm_dm81xxbm_clean:
	rm -f memsegdef_dm81xxbm

mm_dm81xxbm_512M_clean:
	rm -f memsegdef_dm81xxbm_512M
