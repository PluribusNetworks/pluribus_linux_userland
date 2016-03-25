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
# Copyright 2011, 2013, EveryCity Ltd. All rights reserved.
#

include ../../make-rules/shared-macros.mk

COMPONENT_NAME=		helloworld
COMPONENT_VERSION=	1.0
COMPONENT_LICENSE=	BSD
COMPONENT_PROJECT_URL=	http://www.pluribusnetworks.com
COMPONENT_SRC=		$(COMPONENT_NAME)
COMPONENT_ARCHIVE=	

include $(WS_MAKE_RULES)/prep.mk
include $(WS_MAKE_RULES)/configure.mk
include $(WS_MAKE_RULES)/deb.mk

build:		$(DEFAULT_BUILD)

install:	$(DEFAULT_INSTALL)

include $(WS_MAKE_RULES)/depend.mk
