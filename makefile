MAJOR := 3
MINOR := 00
PATCH := 00
BUILD := 05

PLATFORM ?= ti816x-evm
PKG_NAME = ti-media-controller-utils

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib
sbindir = $(exec_prefix)/sbin
libexecdir = $(exec_prefix)/libexec
datarootdir = $(prefix)/share/ti
datadir = $(datarootdir)
sysconfdir = $(prefix)/etc
includedir = $(prefix)/include
docdir = $(prefix)/doc

DESTDIR = 

ifdef VERBOSE
Q      :=
SILENT :=
else
Q      := @
SILENT := -s
endif
export Q

ROOTDIR ?= $(PWD)
export ROOTDIR

define makesubdir
	@echo 
	@echo MAKE $1 CORE=$2 PLATFORM=$3
	$(Q)$(MAKE) $(SILENT) -C $1 CORE=$2 PLATFORM=$3
endef

all: linux dsp

linux:
	$(call makesubdir,src/ldrmemcfg,a8host,$(PLATFORM))
	$(call makesubdir,src/firmware_loader,a8host,$(PLATFORM))
	$(call makesubdir,src/sys_top,a8host,$(PLATFORM))
ifneq ($(PLATFORM), ti811x-evm)
	$(call makesubdir,src/prcm_config,a8host,$(PLATFORM))
endif

dsp:
	$(call makesubdir,src/ldrmemcfg,c6xdsp,$(PLATFORM))

clean:
	$(Q)$(RM) -fr lib bin

install:
	install -d $(DESTDIR)$(bindir) $(DESTDIR)$(datadir)/$(PKG_NAME)
	install lib/firmware_loader/bin/$(PLATFORM)/firmware_loader_a8host_debug.xv5T $(DESTDIR)$(bindir)/firmware_loader
ifneq ($(PLATFORM), ti811x-evm)
	install lib/prcm_config_app/bin/$(PLATFORM)/prcm_config_app_a8host_debug.xv5T $(DESTDIR)$(bindir)/prcm_config_app
endif
	install lib/sys_top/bin/$(PLATFORM)/sys_top_a8host_debug.xv5T $(DESTDIR)$(bindir)/sys_top
	install src/linux/$(PLATFORM)/*.sh $(DESTDIR)$(datadir)/$(PKG_NAME)

help:
	@echo
	@echo "Available build targets are  :"
	@echo
	@echo "    all                            : Build Media Controller Utils"
	@echo "    linux                          : Build Linux libraries and utilities"
	@echo "    dsp                            : Build DSP libraries"
	@echo "    clean                          : Remove files generated"
	@echo "    install                        : Install libraries, header files and DSP binary"
	@echo
	@echo "To build you need to override variables set in env.mk"

-include makerules/env.mk

.show-products:
	@echo "bios         - $(bios_PATH)"
	@echo "xdc          - $(xdc_PATH)"
	@echo "ipc          - $(ipc_PATH)"
	@echo "syslink      - $(syslink_PATH)"
	@echo "CodeSourcery - $(CODESOURCERY_PATH)"
	@echo "C6x CGTOOLS  - $(CODEGEN_PATH_DSPELF)"

# Nothing beyond this point
