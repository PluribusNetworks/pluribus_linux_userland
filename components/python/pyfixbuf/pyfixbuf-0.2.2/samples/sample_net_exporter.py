#!/usr/bin/env python
## ------------------------------------------------------------------------
## sample_net_exporter.py
##
## Export IPFIX via TCP.
## ------------------------------------------------------------------------
## Copyright (C) 2013-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt
## ------------------------------------------------------------------------

import sys
import pyfixbuf

# Test that the argument number is correct
if (len(sys.argv) != 4):
    print "Must supply a hostname, port, and transport protocol."
    print "Usage: sample_net_exporter.py hostname port transport"
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()

# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)

# create the "outer" template
tmpl = pyfixbuf.Template(infomodel)

# create a template that will be exported in the STML
sub_tmpl = pyfixbuf.Template(infomodel)

# create a template that will be exported in the STL
stl_tmpl = pyfixbuf.Template(infomodel)

# add elements we want in our "outer" template
export_list = [
    pyfixbuf.InfoElementSpec("sourceIPv4Address"),
    pyfixbuf.InfoElementSpec("destinationIPv4Address"),
    pyfixbuf.InfoElementSpec("sourceTransportPort"),
    pyfixbuf.InfoElementSpec("destinationTransportPort"),
    pyfixbuf.InfoElementSpec("protocolIdentifier"),
    pyfixbuf.InfoElementSpec("packetTotalCount"),
    pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
    pyfixbuf.InfoElementSpec("subTemplateMultiList")]

# Adding 1 subtemplatelist & 2 basiclists that will be exported in our STML
export_sub_list = [
    pyfixbuf.InfoElementSpec("subTemplateList"),
    pyfixbuf.InfoElementSpec("basicList"),
    pyfixbuf.InfoElementSpec("basicList")]

# Add 2 elements to the subtemplatelist template

stl_item_list = [
    pyfixbuf.InfoElementSpec("octetTotalCount"),
    pyfixbuf.InfoElementSpec("reverseOctetTotalCount")]

# add elements to our template

tmpl.add_spec_list(export_list)

# add elements to our sub template

sub_tmpl.add_spec_list(export_sub_list)

# adding STL elements to our subtemplatelist template

stl_tmpl.add_spec_list(stl_item_list)

# create the exporter

exporter = pyfixbuf.Exporter()

#create the IPFIX file to write to

exporter.init_net(hostname=sys.argv[1], port=sys.argv[2], transport=sys.argv[3])

#create the session

session = pyfixbuf.Session(infomodel)

# for exporters we need to create internal and external templates

session.add_internal_template(template_id=999, template=tmpl)
session.add_external_template(template_id=999, template=tmpl)
session.add_internal_template(template_id=1000, template=sub_tmpl)
session.add_external_template(template_id=1000, template=sub_tmpl)
session.add_internal_template(template_id=1001, template=stl_tmpl)
session.add_external_template(template_id=1001, template=stl_tmpl)

# create the record to fill to export
# we can just give it the template since we don't have duplicate elements
rec = pyfixbuf.Record(infomodel, tmpl)

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

# create a rec for our sub template multilist
# we use 2 basicLists so add elements separately
sub_rec = pyfixbuf.Record(infomodel)
sub_rec.add_element("subTemplateList")
sub_rec.add_element("BL", pyfixbuf.BASICLIST)
sub_rec.add_element("BL2", pyfixbuf.BASICLIST)

# create a rec for our subtemplatelist

stl_rec = pyfixbuf.Record(infomodel)
stl_rec.add_element_list(["octetTotalCount", "reverseOctetTotalCount"])

# now we are ready to export! (export 10 flows)

count = 0

while count < 10:
    rec['sourceIPv4Address'] = "127.0.0.1"
    rec['destinationIPv4Address'] = "192.168.1.1"
    rec['sourceTransportPort'] = 1 + count
    rec['destinationTransportPort'] = 15 - count
    rec['protocolIdentifier'] = 17
    rec['packetTotalCount'] = 98
    rec['reversePacketTotalCount'] = 111

    stl_rec.set_template(stl_tmpl)

    stl_rec["octetTotalCount"] = 74653
    stl_rec["reverseOctetTotalCount"] = 898989

    sub_rec["subTemplateList"] = [stl_rec]
    sub_rec.set_template(sub_tmpl)
    sub_rec.init_basic_list("BL", 2, "httpUserAgent")
    sub_rec["BL"] = ['Mozilla', 'Safari']
    bl = pyfixbuf.BL(infomodel, "httpServerString", 1)
    bl[0] = "www.wikipedia.com"
    sub_rec['BL2'] = bl

    stml = [sub_rec, sub_rec]

    rec["subTemplateMultiList"] = stml

    #done! just append the record
    buf.append(rec)
    count += 1

    # get the stml from the rec
    for entry in rec["subTemplateMultiList"]:
        entry.set_record(sub_rec)
        for item in entry:
            item.clear_all_lists()

    # clear the stml - or you can call rec.clearAllLists()
    rec["subTemplateMultiList"].clear()

# now emit the buffer so we write everything that's in the buffer
buf.emit()
sys.stdout.write("Finished.\n")


