#!/usr/bin/env python
## ------------------------------------------------------------------------
## nmsg_to_pipeline.py
## sample IPFIX exporter using pyfixbuf.
## Takes a CSV file that has format <domain name>,<type>,<ttl>,<ip>
## ------------------------------------------------------------------------
## Copyright (C) 2017 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt

import sys
import pyfixbuf
import csv

# Test that the argument number is correct
if (len(sys.argv) < 3):
    print "Must supply an IPFIX file to write to."
    print "Usage: nmsg_to_pipeline.py nmsg_csv_file.txt <ipfix file or domain/ip> <port_number>"
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()

# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_DNS_LIST)

infomodel.add_element(pyfixbuf.InfoElement("dnsRRIPv4Address", pyfixbuf.CERT_PEN, 931, 4, type=pyfixbuf.DataType.IP4ADDR))
infomodel.add_element(pyfixbuf.InfoElement("dnsRRIPv6Address", pyfixbuf.CERT_PEN, 932, 16, type=pyfixbuf.DataType.IP6ADDR))

# create the "outer" template
tmpl = pyfixbuf.Template(infomodel)

# add elements we want in our template
export_list = [
    pyfixbuf.InfoElementSpec("dnsRRIPv4Address"),
    pyfixbuf.InfoElementSpec("dnsQName"),
    pyfixbuf.InfoElementSpec("dnsTTL"),
    pyfixbuf.InfoElementSpec("dnsQRType")]

# add elements to our template
tmpl.add_spec_list(export_list)

aaaa_list = [
    pyfixbuf.InfoElementSpec("dnsRRIPv6Address"),
    pyfixbuf.InfoElementSpec("dnsQName"),
    pyfixbuf.InfoElementSpec("dnsTTL"),
    pyfixbuf.InfoElementSpec("dnsQRType")]

tmplaaaa = pyfixbuf.Template(infomodel)
tmplaaaa.add_spec_list(aaaa_list)

# create the exporter
exporter = pyfixbuf.Exporter()

#create the IPFIX file to write to
if (len(sys.argv) == 3):
    exporter.init_file(sys.argv[2])
else:
    exporter.init_net(hostname=sys.argv[2], port=sys.argv[3], transport='tcp')

#create the session
session = pyfixbuf.Session(infomodel)

# for exporters we need to create internal and external templates
session.add_internal_template(tmpl, 999)
session.add_external_template(tmpl, 999)

session.add_internal_template(tmplaaaa, 1000)
session.add_external_template(tmplaaaa, 1000)

# create the record to fill to export
rec = pyfixbuf.Record(infomodel, tmpl)

reca = pyfixbuf.Record(infomodel, tmplaaaa)

#create the buffer for exporter
buf = pyfixbuf.Buffer(rec)

# make the buffer an export buffer
buf.init_export(session, exporter)

# set the internal template on the buffer
buf.set_internal_template(999)

# export the templates to the file
session.export_templates()

# now set the export template
buf.set_export_template(999)

#now open SIE file
f = open(sys.argv[1], 'r')

csv.field_size_limit(sys.maxsize)

c = csv.reader(f, delimiter=',')

count = 0

for row in c:
    if (row[1] == "A(1)" or row[1] == "1"):
        try:
            rec['dnsRRIPv4Address'] = row[3]
        except:
            print "row[3] is " + row[3]
        rec['dnsQName'] = row[0]
        rec['dnsTTL'] = int(row[2])
        rec['dnsQRType'] = 1
        
        buf.set_internal_template(999)
        buf.set_export_template(999)
        buf.append(rec)

        count += 1

        if (len(row) > 4):
            k = len(row) - 4
            while (k):
                rec['dnsRRIPv4Address'] = row[3+k]
                rec['dnsQName'] = row[0]
                rec['dnsTTL'] = int(row[2])
                rec['dnsQRType'] = 1
                
                buf.append(rec)

                count += 1
                k -= 1
    elif (row[1] == "AAAA(28)" or row[1] == "28"):
        try:
            reca['dnsRRIPv6Address'] = row[3]
        except:
            print "row[3] is " + row[3]
        reca['dnsQName'] = row[0]
        reca['dnsTTL'] = int(row[2])
        reca['dnsQRType'] = 28

        buf.set_internal_template(1000)
        buf.set_export_template(1000)
        buf.append(reca)

        count += 1

        if (len(row) > 4):
            k = len(row) - 4
            while (k):
                reca['dnsRRIPv6Address'] = row[3+k]
                reca['dnsQName'] = row[0]
                reca['dnsTTL'] = int(row[2])
                reca['dnsQRType'] = 28

                buf.append(reca)

                count += 1
                k -= 1

buf.emit()

print "FINISHED sending %d records" % count

f.close()




