TARGET = boot_logo.ko

obj-m += boot_logo.o

ifeq ($(PLATFORM),ti814x)
	EXTRA_CFLAGS +=	-DTI814X_BUILD
else ifeq ($(PLATFORM),ti816x)
	EXTRA_CFLAGS +=	-DTI816X_BUILD
endif

all:
	make -C $(LINUXKERNEL_INSTALL_DIR) M=`pwd` $(MAKE_ENV) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

install:
	install -d $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/kernel/boot_logo
	install $(TARGET) $(DESTDIR)/lib/modules/$(KERNEL_VERSION)/kernel/boot_logo

clean:
	-rm -rf *.o *mod* *Mod* *.ko .tmp*

