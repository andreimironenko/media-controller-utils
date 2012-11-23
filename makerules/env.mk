# File: env.mk. This file contains all the paths and other ENV variables

#
# Module paths
#

# Directory where all internal software packages are located; typically 
#  those that are checked into version controlled repository. In this case all
#  the OMX components and SDK/OMX demo.
INTERNAL_SW_ROOT = $(ROOTDIR)/src

# Directory where all external (imported) software packages are located; typically 
#  those that are NOT checked into version controlled repository. In this case,
#  compiler tool chains, BIOS, XDC, Syslink, IPC, FC, CE, drivers, codecs, etc.
EXTERNAL_SW_ROOT = "EXTERNAL_SW_ROOT UNDEFINED"

# Destination root directory.
#   - specify the directory where you want to place the object, archive/library,
#     binary and other generated files in a different location than source tree
#   - or leave it blank to place then in the same tree as the source
DEST_ROOT = $(INTERNAL_SW_ROOT)/../lib

# Directory where example-apps/demos are located. By default, it resides along
#  with other source code. This can be over-ridden by specifying it in the 
#  command line.
EXAMPLES_ROOT = $(ROOTDIR)/examples

# Utilities directory. This is required only if the build machine is Windows.
#   - specify the installation directory of utility which supports POSIX commands
#     (eg: Cygwin installation or MSYS installation).
UTILS_INSTALL_DIR = c:/cygwin

# Set path separator, etc based on the OS
ifeq ($(OS),Windows_NT)
  PATH_SEPARATOR = ;
  UTILSPATH = $(UTILS_INSTALL_DIR)/bin/
else 
  # else, assume it is linux
  PATH_SEPARATOR = :
endif

SYSBIOS_INSTALL_DIR = 
IPC_INSTALL_DIR = 
SYSLINK_INSTALL_DIR = 
XDC_INSTALL_DIR = 
CODESOURCERY_PATH = "CODESOURCERY_PATH UNDEFINED"
CODEGEN_PATH_DSPELF = "CODEGEN_PATH_DSPELF UNDEFINED"

# BIOS
bios_PATH = "bios_PATH UNDEFINED"
bios_INCLUDE = $(bios_PATH)/packages

# IPC
ipc_PATH = "ipc_PATH UNDEFINED"
ipc_INCLUDE = $(ipc_PATH)/packages

# SYSLINK 
syslink_PATH = "syslink_PATH UNDEFINED"
syslink_INCLUDE = $(syslink_PATH)/packages

# XDC
xdc_PATH = "xdc_PATH UNDEFINED"
xdc_INCLUDE = $(xdc_PATH)/packages

#
# Tools paths
#
# Cortex-M3

CODEGEN_PATH_M3 = $(TMS470_CODEGEN_INSTALL_DIR)

# Cortex-A8
CODESOURCERY_PATH = "CODESOURCERY_PATH UNDEFINED"
CODEGEN_PATH_A8 = $(CODESOURCERY_PATH)

# DSP - Since same toolchain does not support COFF and ELF, there are two entries
#        This would go away when one version supports both formats
CODEGEN_PATH_DSP = /home/a0131957/ti/TI_CGT_C6000_7.3.4
CODEGEN_PATH_DSPELF = "CODEGEN_PATH_DSPELF UNDEFINED"

# Commands commonly used within the make files

RM = $(UTILSPATH)rm
RMDIR = $(UTILSPATH)rm -rf
MKDIR = $(UTILSPATH)mkdir
ECHO = @echo
# MAKE = $(UTILSPATH)make
EGREP = $(UTILSPATH)egrep
CP = $(UTILSPATH)cp
CHMOD = $(UTILSPATH)chmod

#
# XDC specific ENV variables
#

# XDC Config.bld file (required for configuro) ; Derives from top-level pe_PATH
CONFIG_BLD_XDC_c674 = $(ROOTDIR)/examples/dm81xx/config.bld
CONFIG_BLD_XDC_m3 = $(pe_PATH)/build/config.bld
CONFIG_BLD_XDC_a8 = $(pe_PATH)/build/config_ca8.bld

XDCPATH = $(bios_PATH)/packages;$(fc_PATH)/packages;$(hdvpss_PATH)/packages;$(ipc_PATH)/packages;$(syslink_PATH)/packages;$(xdc_PATH)/packages;$(ce_PATH)/packages;$(pe_PATH);.;$(h264enc_PATH)/packages;$(h264dec_PATH)/packages;$(mpeg2dec_PATH)/packages;$(mp3dec_PATH)/packages;$(aaclcdec_PATH)/packages;$(mpeg2enc_PATH)/packages;$(hdvicp20api_PATH)/packages;$(osal_PATH)/packages;$(xdais_PATH)/packages;$(linuxutils_PATH)/packages;$(uia_PATH)/packages;$(mpeg4enc_PATH)/packages;$(mpeg4dec_PATH)/packages;$(vc1dec_PATH)/packages;$(aaclcenc_PATH)/packages;$(mjpegdec_PATH)/packages;
export XDCPATH

XDCROOT = $(xdc_PATH)
XDCTOOLS = $(xdc_PATH)
export XDCROOT
export XDCTOOLS

TMS470CGTOOLPATH = $(CODEGEN_PATH_M3)
CGTOOLS = $(CODEGEN_PATH_DSP)
CGTOOLS_ELF = $(CODEGEN_PATH_DSPELF)
C674CODEGENTOOL = $(CODEGEN_PATH_DSPELF) 

export TMS470CGTOOLPATH
export CGTOOLS
export CGTOOLS_ELF
export C674CODEGENTOOL

CODESOURCERYCGTOOLS = $(CODEGEN_PATH_A8)
export CODESOURCERYCGTOOLS

PATH += $(PATH_SEPARATOR)$(xdc_PATH)$(PATH_SEPARATOR)$(CODEGEN_PATH_DSPELF)/bin$(PATH_SEPARATOR)$(CODEGEN_PATH_M3)/bin
export PATH

# Nothing beyond this point
