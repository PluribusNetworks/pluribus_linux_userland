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
# COPYRIGHT 2012-2015 Pluribus Networks Inc.
#
include ../../make-rules/shared-macros.mk

COMPONENT_NAME=		libfixbuf
COMPONENT_VERSION=	1.7.1
COMPONENT_SRC=		$(COMPONENT_NAME)-$(COMPONENT_VERSION)
COMPONENT_ARCHIVE=	$(COMPONENT_SRC).tar.gz
COMPONENT_ARCHIVE_HASH=	sha256:961296ed000d4fdb4ae8690613bba2af2e4d634c044f947795abc24e9caf644a
COMPONENT_ARCHIVE_URL=	https://tools.netsa.cert.org/releases/$(COMPONENT_ARCHIVE)
COMPONENT_PROJECT_URL=	https://tools.netsa.cert.org/fixbuf/

include ../../make-rules/prep.mk
include ../../make-rules/configure.mk
include ../../make-rules/deb.mk

COMPONENT_BUILD_TARGETS = all

build:		$(DEFAULT_BUILD)

install:	$(DEFAULT_INSTALL)

test:		$(DEFAULT_TEST)

BUILD_PKG_DEPENDENCIES = 	$(BUILD_TOOLS)

include ../../make-rules/depend.mk
