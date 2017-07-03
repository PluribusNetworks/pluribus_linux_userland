include ../../../make-rules/shared-macros.mk

COMPONENT_NAME=		pyfixbuf
COMPONENT_VERSION=	0.2.2
COMPONENT_PROJECT_URL=	https://tools.netsa.cert.org/
COMPONENT_SRC=		$(COMPONENT_NAME)-$(COMPONENT_VERSION)
COMPONENT_ARCHIVE=  
COMPONENT_BUGDB=	pyfixbuf

include $(WS_TOP)/make-rules/prep.mk
include $(WS_TOP)/make-rules/setup.py.mk
include $(WS_TOP)/make-rules/deb.mk

# to make sure that the docs get installed in the right place.
PYTHON_DATA = /usr

# common targets
build:		$(BUILD_64)

install:	$(INSTALL_64)

test:		$(NO_TESTS)

BUILD_PKG_DEPENDENCIES =	$(BUILD_TOOLS)

include $(WS_TOP)/make-rules/depend.mk
