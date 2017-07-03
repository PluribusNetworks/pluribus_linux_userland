pyfixbuf
========

pyfixbuf is a Python API for libfixbuf, an implementation of the IPFIX protocol
used for building collecting and exporting processes.  pyfixbuf can be used
to write applications, often called mediators, that collect and/or export 
IPFIX. Mediators are useful in modifying, filtering, or adding to the content 
of the message before forwarding to another IPFIX collection point, or in 
converting IPFIX to another format (text, database, JSON, etc.).

libfixbuf is a compliant implementation of the IPFIX Protocol, as defined 
in the following RFCs:

RFC 5101: Specification of the IPFIX Protocol for Export of IP Flow Information
RFC 5102: Information Model for IP Flow Information Export
RFC 5103: Bidirectional Flow Export using IPFIX
RFC 5610: Exporting Type Information for IPFIX Information Elements
RFC 6313: Export of Structured Data in IPFIX

libfixbuf, as well as pyfixbuf, supports UDP and TCP as transport protocols.
It also supports operation as an IPFIX file writer or file reader.  Spread 
support for pyfixbuf is forthcoming.

Installation
============

pyfixbuf requires netsa-python version 1.4 or later.  Please build and
install netsa-python before building and installing pyfixbuf.  You should
be able to find netsa-python on the same site that you downloaded pyfixbuf.

pyfixbuf requires libfixbuf version 1.4 or later.  Please build and install
libfixbuf before building and installing pyfixbuf.

pyfixbuf supports Python 2.6 or later. Python 3 support has not been added.

Building and installing pyfixbuf is done using the standard
setup.py mechanism.  The following commands should suffice in most
cases:

    python setup.py build
    python setup.py install    # as root

You may be required to set the PKG_CONFIG_PATH to the location of the
libfixbuf.pc.  Typically this file is located at /usr/local/lib/pkgconfig if
libfixbuf was installed in the default location.
You may need to use the -E option to sudo to preserve the environment variable
when installing.
