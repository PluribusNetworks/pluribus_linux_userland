#
# COPYRIGHT 2020  Pluribus Networks Inc.
# 
# All rights reserved. This copyright notice is Copyright Management
# Information under 17 USC 1202 and is included to protect this work and
# deter copyright infringement.  Removal or alteration of this Copyright
# Management Information without the express written permission from
# Pluribus Networks Inc is prohibited, and any such unauthorizedzed removal
# or alteration will be a violation of federal law.
#
TOP			= ../..
TARGET_BUILDS   	= linux
PACKAGE			= pn-nvos
SYSTEMD_SCRIPTS         = svc-wpa_supplicant

SRC_DIR			= wpa_supplicant

WPA_CLI_BIN             = $(SRC_DIR)/wpa_cli
WPA_SUPPLICANT_BIN      = $(SRC_DIR)/wpa_supplicant
PACKAGE_FILES           += $(INSTALL_BINDIR)/wpa_supplicant $(INSTALL_BINDIR)/wpa_cli
SYSTEMD_SERVICES        = wpa_supplicant-wired.service

include $(TOP)/Makedefs
patched:
	patch -p0 < nvos.patch
	touch $@

#build:	patched
build:
	(cd $(SRC_DIR); CC='$(CC)' $(MAKE))
	$(INSTALL) $(WPA_CLI_BIN) $(INSTALL_BINDIR)
	$(INSTALL) $(WPA_SUPPLICANT_BIN) $(INSTALL_BINDIR)

$(INSTALL_BINDIR)/wpa_supplicant:
	(cd $(SRC_DIR); CC='$(CC)' $(MAKE))
	$(INSTALL) $(WPA_CLI_BIN) $(INSTALL_BINDIR)
	$(INSTALL) $(WPA_SUPPLICANT_BIN) $(INSTALL_BINDIR)

clean: cleanbuilddir

cleanbuilddir:
	(cd $(SRC_DIR); $(MAKE) clean)

.PHONY:	build clean cleanbuilddir
