#!/usr/bin/env python
# Copyright 2013-2017 by Carnegie Mellon University
# See license information in LICENSE-OPENSOURCE.txt 

import os.path, sys
import os

try:
    from netsa import dist
    from netsa.util.shell import *
except:
    import sys
    print >>sys.stderr, "setup.py: unable to find required netsa-python\n"
    sys.exit(-1)

FIXBUF_VERSION_REQ = "1.4"
FIXBUF_MAJOR_VERSION = "1"
FIXBUF_MINOR_VERSION = 4

pyversion = (0x020600f0 <= sys.hexversion < 0x03000000)

if (not pyversion):
    print >>sys.stderr, "Unsupported version of Python installed. 2.6 or greater required." 
    sys.exit(1)

dist.set_name("pyfixbuf")
dist.set_version("0.2.2")

dist.set_title("Python fixbuf")
dist.set_description(""" 
A set of Python bindings for the fixbuf IPFIX protocol library.
""")

dist.set_maintainer("NetSA Group <netsa-help@cert.org>")
dist.set_url("http://tools.netsa.cert.org/pyfixbuf/index.html")
dist.set_license("LGPL")
dist.set_copyright("2017 Carnegie Mellon University")

# Helper function to use pkg-config to get extra switches, etc.
def pkgconfig(*pkgs):
    from netsa.util.shell import PipelineException
    import shlex
    try:
        (output, error) = run_collect(command("pkg-config", "--cflags", *pkgs))
        compile_args = shlex.split(output)
        (output, error) = run_collect(command("pkg-config", "--libs", *pkgs))
        link_args = shlex.split(output)
        (output, error) = run_collect(command("pkg-config", "--modversion", *pkgs))
        version_args = shlex.split(output)
        version_list = list(version_args[0])
        if ((version_list[0] != FIXBUF_MAJOR_VERSION) or
            (int(version_list[2]) < FIXBUF_MINOR_VERSION)):
            import sys
            print >>sys.stderr, "FATAL: Found libfixbuf version: " + version_args[0]
            print >>sys.stderr, "pyfixbuf requires version: " + FIXBUF_VERSION_REQ
            sys.exit(-1)
        return dict(extra_compile_args=compile_args,
                    extra_link_args=link_args)
    except PipelineException, ex:
        import sys
        print >>sys.stderr, "setup.py: unable to find required libraries:\n"
        print >>sys.stderr, "\n".join(ex.get_message().split("\n")[2:])
        sys.exit(-1)

dist.add_package("pyfixbuf")
dist.add_package("pyfixbuf.test")

dist.add_module_ext("pyfixbuf/_pyfixbuf", ["pyfixbuf/_pyfixbuf.c"], 
                    **pkgconfig('libfixbuf'))

dist.add_package_data("pyfixbuf.test", "sampleipfix.ipfix")
dist.add_extra_files("samples/*.py")
dist.add_extra_files("LGPL.txt")
dist.add_unit_test_module("pyfixbuf.test")

dist.execute()
