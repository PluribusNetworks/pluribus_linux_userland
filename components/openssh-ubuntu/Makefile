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
COMPONENT_VERSION=	7.2p2
COMPONENT_ORIG_DIR=	$(COMPONENT_NAME)-$(COMPONENT_VERSION)
POOL_DIR=		http://archive.ubuntu.com/ubuntu/pool/main/o/openssh
ORIG_SRC=		openssh_7.2p2.orig.tar.gz
DEBIAN_SRC=		openssh_7.2p2-4ubuntu2.6.debian.tar.xz
DEBIAN_DSC=		openssh_7.2p2-4ubuntu2.6.dsc

build:
	[ -f $(ORIG_SRC) ] || wget $(POOL_DIR)/$(ORIG_SRC)
	[ -f $(DEBIAN_SRC) ] || wget $(POOL_DIR)/$(DEBIAN_SRC)
	[ -f $(DEBIAN_DSC) ] || wget $(POOL_DIR)/$(DEBIAN_DSC)
	if [ -d $(COMPONENT_ORIG_DIR) ]; then echo "Directory $(COMPONENT_ORIG_DIR) already present. Please run make clean"; exit 1; fi
	tar xvf $(ORIG_SRC)
	(cd $(COMPONENT_ORIG_DIR); tar xvf ../$(DEBIAN_SRC))
	cp $(COMPONENT_ORIG_DIR)/debian/changelog $(COMPONENT_ORIG_DIR)/debian/changelog.orig
	cp debian/changelog $(COMPONENT_ORIG_DIR)/debian/changelog
	cat $(COMPONENT_ORIG_DIR)/debian/changelog.orig >> $(COMPONENT_ORIG_DIR)/debian/changelog
	rm $(COMPONENT_ORIG_DIR)/debian/changelog.orig
	cat debian/patches/series >> $(COMPONENT_ORIG_DIR)/debian/patches/series
	for pf in `cat debian/patches/series`; do cp debian/patches/$${pf} $(COMPONENT_ORIG_DIR)/debian/patches; done
	cd $(COMPONENT_ORIG_DIR); dpkg-buildpackage -nc -us -uc

install:
	echo "Done"


clean:
	rm -rf $(COMPONENT_ORIG_DIR)
	rm -f *.deb *.changes *.udeb

distclean: clean
	for f in $(ORIG_SRC) $(DEBIAN_SRC) $(DEBIAN_DSC); do rm -f $$f; done
