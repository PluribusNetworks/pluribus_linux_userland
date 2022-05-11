#
# COPYRIGHT 2011 Pluribus Networks Inc.
# 
# All rights reserved. This copyright notice is Copyright Management
# Information under 17 USC 1202 and is included to protect this work and
# deter copyright infringement.  Removal or alteration of this Copyright
# Management Information without the express written permission from
# Pluribus Networks Inc is prohibited, and any such unauthorized removal
# or alteration will be a violation of federal law.
#
# Pluribus libedit Makefile.
#
TOP = ../..
include $(TOP)/Makedefs.os

NAME=libnvedit
VERSION=20110802-3.0
SRCDIR=$(NAME)-$(VERSION)
BUILDDIR=$(NAME)-$(VERSION)-$(TARGET_BUILD)-out
BUILD64DIR=$(NAME)-$(VERSION)-out64
PACKAGE = pn-nvos

HOST_FLAG=
ifeq ($(TARGET_BUILD),linux_client)
HOST_FLAG=--host x86_64-linux
endif

PACKAGE_FILES = $(INSTALL_LIBDIR)/libnvedit.so.0 \
	$(INSTALL_LIBDIR)/libnvedit.so.0.0.38 $(INSTALL_LIBDIR)/libnvedit.so

ifeq ($(TARGET_BUILD),sunos)
PACKAGE_FILES += \
	$(INSTALL_LIB64DIR)/libnvedit.so.0 \
	$(INSTALL_LIB64DIR)/libnvedit.so.0.0.38 $(INSTALL_LIB64DIR)/libnvedit.so
endif

DEFS = "-DHAVE_WCSDUP -DHAVE_CONFIG_H"
DIRT += $(BUILDDIR) $(BUILD64DIR)

GMAKE=gmake

include $(TOP)/Makedefs
CFGPREFIX=/usr

CFLAGS = $(OPTIMIZER) 

build: compile

$(BUILDDIR)/config.log:
	mkdir -p $(BUILDDIR)
	(cd $(BUILDDIR); env CC='$(CC)' \
	    MAKE=$(GMAKE) ../$(SRCDIR)/configure $(HOST_FLAG) \
	    --prefix=$(CFGPREFIX) --libdir=$(HOST_LIBDIR))

compile: $(BUILDDIR)/config.log
	(cd $(BUILDDIR); $(GMAKE) DEFS=$(DEFS) all)
	(cd $(BUILDDIR); $(GMAKE) DESTDIR=$(INSTALL_DIR) install)

ifeq ($(TARGET_BUILD),sunos)
$(BUILD64DIR)/config.log:
	mkdir -p $(BUILD64DIR)
	(cd $(BUILD64DIR); env CC='$(CC)'\
	    MAKE=$(GMAKE) CFLAGS=-m64 LDFLAGS=-m64 \
	    ../$(SRCDIR)/configure \
	    --prefix=$(CFGPREFIX) --libdir=$(HOST_LIB64DIR))

compile64: $(BUILD64DIR)/config.log
	(cd $(BUILD64DIR); $(GMAKE) DEFS=$(DEFS) all)
	(cd $(BUILD64DIR); $(GMAKE) DESTDIR=$(INSTALL_DIR)  install)

build: compile64
$(PACKAGE_FILES): compile64
endif

test:

$(PACKAGE_FILES): compile

.PHONY: compile compile32 compile64
