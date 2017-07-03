==============
Pyfixbuf
==============

Introduction to Pyfixbuf
"""""""""""""""""""""""""

pyfixbuf is a Python API for `libfixbuf <http://tools.netsa.cert.org/fixbuf/index.html>`_, an implementation of the IPFIX protocol used for building collecting and exporting processes.  pyfixbuf can be used to write applications, often
called mediators, that collect and export IPFIX.  Mediators are useful in 
modifying, filtering, or adding to the content of the message before forwarding
to another IPFIX collection point, or in converting IPFIX to another format 
(text, database, JSON, etc.).

libfixbuf is a compliant implementation of the IPFIX Protocol, as defined in the `Specification of the IPFIX Protocol for Export of IP Flow Information` :rfc:`5101`.  It supports the Information Model defined in `Information Model for IP Flow Information Export` :rfc:`5102` extended as proposed by `Bidirectional Flow Export using IPFIX` :rfc:`5103` to support information elements for representing biflows.  It also supports `Exporting Type Information for IPFIX Information Elements` :rfc:`5610` and structured data elements
as described in `Export of Structured Data in IPFIX` :rfc:`6313`.

libfixbuf, as well as pyfixbuf, supports UDP and TCP as transport protocols.  It also supports operation as an IPFIX File Writer or IPFIX File Reader. Spread support for pyfixbuf is forthcoming.

     

Installation
"""""""""""""""

pyfixbuf is compatible with Python versions 2.6 and greater.

netsa-python should be built and installed before builiding and installing
pyfixbuf.  netsa-python can be downloaded from <http://tools.netsa.cert.org/netsa-python/index.html>.

libfixbuf should be built and installed before building and installing
pyfixbuf.  libfixbuf can be downloaded from <http://tools.netsa.cert.org/fixbuf/index.html>. It may
be necessary to set the PKG_CONFIG_PATH environment variable to the location of libfixbuf.pc
before building pyfixbuf.

Building and installing pyfixbuf is done using the standard setup.py mechanism.
The following commands should suffice in most cases:

.. sourcecode:: python

    python setup.py build

    python setup.py install             #as root




API Documentation
""""""""""""""""""

.. toctree::
   :maxdepth: 1

   pyfixbuf.rst
   intro.rst

