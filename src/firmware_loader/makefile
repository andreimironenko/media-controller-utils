# Makefile for firmware loader app

APP_NAME = firmware_loader

SRCDIR = .
INCDIR = . ../ldrmemcfg

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXERNAL_INTERFACES = ipc syslink

# List of all external libraries that need to be linked
EXTLIBS_a8host = $(ROOTDIR)/lib/lib/a8/debug/ldrmemcfg.av5T \
                 $(syslink_PATH)/packages/ti/syslink/lib/syslink.a_debug

# List all the components required by the application
COMP_LIST_a8host = ldrmemcfg

# Common source files and CFLAGS across all platforms and cores
# The memory map for TI811x is different from TI814x and TI816x
# because we do not have OMX in TI811x. 
ifeq ($(PLATFORM), ti811x-evm)
  SRCS_COMMON = firmware_loader.c memsegdef_default_ti811x.c
  CFLAGS_LOCAL_COMMON = -DPLATFORM_TI811X
else
  SRCS_COMMON = firmware_loader.c memsegdef_default.c
endif

CFLAGS_LOCAL_COMMON += -D_LOCAL_CORE_a8host_

# Include common make files
include $(ROOTDIR)/makerules/common.mk

# Nothing beyond this point
