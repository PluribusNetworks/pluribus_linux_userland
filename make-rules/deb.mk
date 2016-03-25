#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
#

#
# Rules and Macros for generating an IPS package manifest and publishing an
# IPS package to a pkg depot.
#
# To use these rules, include ../make-rules/deb.mk in your Makefile
# and define an "install" target appropriate to building your component.
# Ex:
#
#	install:	$(BUILD_DIR)/build/$(MACH32)/.installed \
#	 		$(BUILD_DIR)/build/$(MACH64)/.installed
#
# This set of rules makes the "publish" target the default target for make(1)
#

BUILD_DEB =	$(WS_TOOLS)/build-deb

.DEFAULT:		publish

.SECONDARY:

publish:		build install
	cd $(COMPONENT_DIR)
	/bin/bash $(BUILD_DEB) $(WS_REPO) $(BITS) url=$(COMPONENT_ARCHIVE_URL) $(COMPONENT_VERSION) $(COMPONENT_REVISION)


pre-prep:	required-pkgs.mk


CLEAN_PATHS +=	required-pkgs.mk
