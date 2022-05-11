#
# COPYRIGHT 2012 Pluribus Networks Inc.
# 
# All rights reserved. This copyright notice is Copyright Management
# Information under 17 USC 1202 and is included to protect this work and
# deter copyright infringement.  Removal or alteration of this Copyright
# Management Information without the express written permission from
# Pluribus Networks Inc is prohibited, and any such unauthorized removal
# or alteration will be a violation of federal law.

TOP		= ../..
PACKAGE		= pn-nvos
sunos_C_SRCS	= nss_solaris_nvos.c
linux_C_SRCS	= nss_linux_nvos.c
C_SRCS		= nss_nvos.c
INCS		= -I../libnvutil -I../../uts/common/sys
LIBS		= -lnvutil -lnvOS
NSSWITCH_FILE	= nsswitch.nvos

include $(TOP)/Makedefs.os
ifeq ($(TARGET_OS),sunos)
	SOLIB		= nss_nvos.so.1
	SRC_NSSWITCH	= nsswitch_solaris.nvos
endif

ifeq ($(TARGET_OS),linux)
	SOLIB		= libnss_nvos.so.2
	INSTALL_SODIR	= $(INSTALL_DIR)/lib/x86_64-linux-gnu
	SRC_NSSWITCH	= nsswitch_linux.nvos
endif

TARGETS = $(INSTALL_ETCDIR)/$(NSSWITCH_FILE)
DIRT += $(NSSWITCH_FILE)

include $(TOP)/Makedefs

$(INSTALL_ETCDIR)/$(NSSWITCH_FILE): $(SRC_NSSWITCH) | $(INSTALL_ETCDIR)
	cp $(SRC_NSSWITCH) $(NSSWITCH_FILE)
	$(INSTALL_FILE) $(INSTALL_ETCDIR) -m 0644 $(NSSWITCH_FILE)
