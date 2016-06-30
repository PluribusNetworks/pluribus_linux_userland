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

COMPONENT_NAME=		openssh
COMPONENT_VERSION=	6.6p1
COMPONENT_ORIG_DIR=	$(COMPONENT_NAME)-$(COMPONENT_VERSION)
POOL_DIR=		http://archive.ubuntu.com/ubuntu/pool/main/o/openssh
ORIG_SRC=		openssh_6.6p1.orig.tar.gz
DEBIAN_SRC=		openssh_6.6p1-2ubuntu2.7.debian.tar.gz
DEBIAN_DSC=		openssh_6.6p1-2ubuntu2.7.dsc

build:
	[ ! -f $(ORIG_SRC) ] && wget $(POOL_DIR)/$(ORIG_SRC); \
	[ ! -f $(DEBIAN_SRC) ] && wget $(POOL_DIR)/$(DEBIAN_SRC); \
	[ ! -f $(DEBIAN_DSC) ] && wget $(POOL_DIR)/$(DEBIAN_DSC); \
	if [ -d $(COMPONENT_ORIG_DIR) ]; then echo "Directory $(COMPONENT_ORIG_DIR) already present. Please run make clean"; exit 1; fi; \
	tar xzvf $(ORIG_SRC); \
	(cd $(COMPONENT_ORIG_DIR); tar xzvf ../$(DEBIAN_SRC)); \
	cp $(COMPONENT_ORIG_DIR)/debian/changelog $(COMPONENT_ORIG_DIR)/debian/changelog.orig; \
	cp debian/changelog $(COMPONENT_ORIG_DIR)/debian/changelog; \
	cat $(COMPONENT_ORIG_DIR)/debian/changelog.orig >> $(COMPONENT_ORIG_DIR)/debian/changelog; \
	rm $(COMPONENT_ORIG_DIR)/debian/changelog.orig; \
	cat debian/patches/series >> $(COMPONENT_ORIG_DIR)/debian/patches/series; \
	for pf in `cat debian/patches/series`; do cp debian/patches/$${pf} $(COMPONENT_ORIG_DIR)/debian/patches; done; \
	cd $(COMPONENT_ORIG_DIR); dpkg-buildpackage -nc -us -uc

install:
	echo "Done"


clean:
	rm -rf $(COMPONENT_ORIG_DIR); \
	rm -f *.deb *.changes *.udeb

distclean: clean
	for f in $(ORIG_SRC) $(DEBIAN_SRC) $(DEBIAN_DSC); do rm -f $$f; done
