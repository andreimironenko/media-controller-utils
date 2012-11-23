This is the host side utilti used in the EZSDK memoy map configuration, This utility generates the memory map bin file which is fed as input to firmware loader

Usage:
=====
  make all - Created memory map bin file for all the usecases
  make mm_dm81xxbm.bin - Create memory map bin file for dm81xxbm usecase
  mm_dm81xxbm_512M.bin - Create memory map bin file for dm81xxbm 512M usecase

  Example:
  ========
     make all

Build:
======
1. Update the below path in makefile
     FIRMWARE_LOADER_INSTALL_DIR
     DESTDIR
     HOST_CODEGEN_INSTALL_DIR
2. execute below command
     make
3. The executables and .bin files present in the DESTDIR directory, take the appropriate .bin file & given this file as command line argument to firmware loader
