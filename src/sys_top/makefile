# Makefile for sys_top app

APP_NAME = sys_top

SRCDIR = .
INCDIR = .  ../ldrmemcfg

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXERNAL_INTERFACES = syslink

# List of all external libraries that need to be linked
EXTLIBS_a8host = $(syslink_PATH)/packages/ti/syslink/lib/syslink.a_debug \
				 -lpthread

# List all the components required by the application
COMP_LIST_a8host = 

# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON = sys_top_svr_main.c
CFLAGS_LOCAL_COMMON =  -I$(LDRMEMSEGCFG_PATH)

# Include common make files
include $(ROOTDIR)/makerules/common.mk

# Nothing beyond this point
