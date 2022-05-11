#
# COPYRIGHT 2015 Pluribus Networks Inc.
# 
# All rights reserved. This copyright notice is Copyright Management
# Information under 17 USC 1202 and is included to protect this work and
# deter copyright infringement.  Removal or alteration of this Copyright
# Management Information without the express written permission from
# Pluribus Networks Inc is prohibited, and any such unauthorized removal
# or alteration will be a violation of federal law.

TOP		= ../..
PACKAGE		= pn-nvos
TARGET_BUILDS	= linux linux_client
INCS		= -I . -I $(TOP)/lib/libnvutil
C_SRCS		= avl.c
SOLIB		= libnvavl.so
PROTO_HEADERDIR	= $(INSTALL_INCDIR)/sys
PROTO_HEADERS	= $(PROTO_HEADERDIR)/avl.h $(PROTO_HEADERDIR)/avl_impl.h
DIRT		= $(PROTO_HEADERS)

include $(TOP)/Makedefs

$(TARGETS): $(PROTO_HEADERS)

$(PROTO_HEADERS): $(PROTO_HEADERDIR)/%: sys/% | $(PROTO_HEADERDIR)
	$(INSTALL_FILE) $(PROTO_HEADERDIR) $<

$(PROTO_HEADERDIR):
	$(INSTALL) -d $@
