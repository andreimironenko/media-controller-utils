PLATFORM = "ti816x-evm"
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

install:
	install -d $(DESTDIR)$(bindir) $(DESTDIR)$(datadir)/$(PKG_NAME)
	install $(PLATFORM)/*.sh $(DESTDIR)$(datadir)/$(PKG_NAME)
