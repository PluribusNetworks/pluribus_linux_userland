#!/usr/bin/env python
# ------------------------------------------------------------------------
## sample_mpls_collector.py
## sample IPFIX collector using pyfixbuf. Collects MPLS Information Elements
## Reads an IPFIX file and writes to a text file.
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt
## ------------------------------------------------------------------------

import sys
import pyfixbuf as p
import binascii
from netsa.data.times import *

# Test that the argument number is correct

if (len(sys.argv) != 3):
    print "Must supply an IPFIX file to read and a text file to write."
    sys.exit()

# Create an InfoModel
infomodel = p.InfoModel()

# Create a Template
tmpl = p.Template(infomodel)

# Create a Stats Template
stats_tmpl = p.Template(infomodel)

# Add some elements to the data template

data_list = [
    p.InfoElementSpec("flowStartMilliseconds"),
    p.InfoElementSpec("flowEndMilliseconds"),
    p.InfoElementSpec("sourceIPv6Address"),
    p.InfoElementSpec("destinationIPv6Address"),
    p.InfoElementSpec("sourceIPv4Address"),
    p.InfoElementSpec("destinationIPv4Address"),
    p.InfoElementSpec("sourceTransportPort"),
    p.InfoElementSpec("destinationTransportPort"),
    p.InfoElementSpec("ipNextHopIPv4Address"),
    p.InfoElementSpec("ipNextHopIPv6Address"),
    p.InfoElementSpec("ingressInterface"),
    p.InfoElementSpec("egressInterface"),
    p.InfoElementSpec("packetDeltaCount"),
    p.InfoElementSpec("octetDeltaCount"),
    p.InfoElementSpec("protocolIdentifier"),
    p.InfoElementSpec("tcpControlBits"),
    p.InfoElementSpec("sourceMacAddress"),
    p.InfoElementSpec("destinationMacAddress"),
    p.InfoElementSpec("vlanId"),
    p.InfoElementSpec("mplsTopLabelStackSection", 3),
    p.InfoElementSpec("mplsLabelStackSection2", 3),
    p.InfoElementSpec("mplsLabelStackSection3", 3),
    p.InfoElementSpec("mplsLabelStackSection4", 3),
    p.InfoElementSpec("mplsLabelStackSection5", 3),
    p.InfoElementSpec("mplsLabelStackSection6", 3),
    p.InfoElementSpec("mplsTopLabelPrefixLength"),
    p.InfoElementSpec("mplsTopLabelType"),
    p.InfoElementSpec("mplsTopLabelIPv4Address"),
    p.InfoElementSpec("bgpSourceAsNumber"),
    p.InfoElementSpec("bgpDestinationAsNumber"),
    p.InfoElementSpec("postVlanId"),
    p.InfoElementSpec("ipClassOfService")]

# Add the lists to their respective template

tmpl.add_spec_list(data_list)

# Create a collector

collector = p.Collector()

# Give the collector your input file to read from

collector.init_file(sys.argv[1])

# Create a session

session = p.Session(infomodel)

# Add your template to the session

session.add_internal_template(tmpl, 999)

# create a Record for each template or subtemplate to give the buffer

rec = p.Record(infomodel, tmpl)

# Create a buffer to read from

buf = p.Buffer(rec)

# Add the session and collector the buffer

buf.init_collection(session, collector)

# Set your internal template to the data template

buf.set_internal_template(999)

# open our output file

outFile = open(sys.argv[2], "w")

flowcount = 0

# Start Reading
for data in buf:
    flowcount += 1
    data = data.as_dict()
    for key,value in data.items():
        type = infomodel.get_element_type(key)
        if (type == p.DataType.MILLISECONDS):
            outFile.write(key + ": " + str(make_datetime(value/1000)) + "\n")
        elif (type == p.DataType.OCTET_ARRAY):
            hexdata = ''.join(format(byte, '02x') for byte in value)
            outFile.write(key + ": 0x" + str(hexdata) + "\n")
        else:
            outFile.write(key + ": " + str(value) + "\n")
    outFile.write("----------------------------\n")

# check to see if the next template is an options template (and ignore)
    tmpl_next = buf.next_template()

    if ( tmpl_next.scope ):
        outFile.write("STATS Record:\n")
        buf.next_record(rec)

sys.stdout.write("Finished. Processed " + str(flowcount) + " flows.\n")

