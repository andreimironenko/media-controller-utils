# Makefile for ldrmemcfg

MODULE_NAME = ldrmemcfg

SRCDIR = . src
INCDIR = .

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXERNAL_INTERFACES = xdc ipc bios slog syslink

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON = ldr_memseg.c mem_setup.c

CFLAGS_LOCAL_COMMON = 

SRCS_m3vpss = 
SRCS_m3video = 
SRCS_dsp = 
SRCS_a8host = ldr_memcfg.c

# Core/SoC/platform specific source files and CFLAGS
# Example: 
#   SRCS_<core/SoC/platform-name> = 
#   CFLAGS_LOCAL_<core/SoC/platform-name> =
ifeq ($(CORE),a8host)
  CFLAGS_LOCAL_a8host += -D_LOCAL_CORE_a8host_ -DUSE_SLOG_PRINT=0 -DSLOG_LDRMEMCFG_Module__MID=0x8005
endif

# Include common make files
include $(ROOTDIR)/makerules/common.mk

# OBJs and libraries are built by using rule defined in rules_<target>.mk 
#     and need not be explicitly specified here

# Nothing beyond this point
