#
# This file and its contents are supplied under the terms of the
# Common Development and Distribution License ("CDDL"). You may
# only use this file in accordance with the terms of the CDDL.
#
# A full copy of the text of the CDDL should have accompanied this
# source. A copy of the CDDL is also available via the Internet at
# http://www.illumos.org/license/CDDL.
#

#
# Copyright 2016, PLuribus Networks. All rights reserved.
#

COMPONENT_NAME=		dpkg
COMPONENT_VERSION=	1.17.5ubuntu5.6
COMPONENT_ORIG_DIR=	$(COMPONENT_NAME)-$(COMPONENT_VERSION)
POOL_DIR=		http://archive.ubuntu.com/ubuntu/pool/main/d/dpkg
ORIG_SRC=		$(COMPONENT_NAME)_$(COMPONENT_VERSION).tar.xz

build:
	[ ! -f $(ORIG_SRC) ] && wget $(POOL_DIR)/$(ORIG_SRC); \
	if [ -d $(COMPONENT_ORIG_DIR) ]; then echo "Directory $(COMPONENT_ORIG_DIR) already present. Please run make clean"; exit 1; fi; \
	xzcat $(ORIG_SRC) | tar xvf -; \
	for pf in `cat debian/patches/series`; do cat debian/patches/$${pf} | patch -p0; [ $$? -ne 0 ] && exit 1; done; \
	cd $(COMPONENT_ORIG_DIR); dpkg-buildpackage -us -uc

install:
	echo "Done"


clean:
	rm -rf $(COMPONENT_ORIG_DIR); \
	rm -f *.deb *.changes *.udeb *.dsc

distclean: clean
	for f in $(ORIG_SRC); do rm -f $$f; done; rm -f *.xz *.gz *.bz2
