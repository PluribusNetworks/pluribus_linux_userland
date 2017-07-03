#!/usr/bin/env python
## ------------------------------------------------------------------------
## sample_exporter.py
## sample IPFIX exporter using pyfixbuf.
## writes to an IPFIX file.
## ------------------------------------------------------------------------
## Copyright (C) 2012-2015 Carnegie Mellon University. All Rights Reserved.
## ------------------------------------------------------------------------
## Authors: Emily Sarneso <ecoff@cert.org>
## ------------------------------------------------------------------------
## See license information in LICENSE-OPENSOURCE.txt

import sys
import pyfixbuf

# Test that the argument number is correct
if (len(sys.argv) != 2):
    print "Must supply an IPFIX file to write to."
    print "Usage: sample_exporter.py file.ipfix"
    sys.exit()


#create the information model with the standard IPFIX elements
infomodel = pyfixbuf.InfoModel()

# add YAF's HTTP IPFIX elements for a sub template
infomodel.add_element_list(pyfixbuf.YAF_HTTP_LIST)
infomodel.add_element(pyfixbuf.InfoElement("weird3byteElement", 756, 3, 3))

# create the "outer" template
tmpl = pyfixbuf.Template(infomodel)

otmpl = pyfixbuf.Template(infomodel, 1)

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
    pyfixbuf.InfoElementSpec("reverseOctetTotalCount"),
    pyfixbuf.InfoElementSpec("basicList")]

# add elements to our template

tmpl.add_spec_list(export_list)

# add elements to our sub template

sub_tmpl.add_spec_list(export_sub_list)

# adding STL elements to our subtemplatelist template

stl_tmpl.add_spec_list(stl_item_list)

# create the exporter

exporter = pyfixbuf.Exporter()

#create the IPFIX file to write to

exporter.init_file(sys.argv[1])

#create the session

session = pyfixbuf.Session(infomodel)

# for exporters we need to create internal and external templates

session.add_internal_template(tmpl, 999)
session.add_external_template(tmpl, 999)
session.add_internal_template(sub_tmpl, 1000)
session.add_external_template(sub_tmpl, 1000)
session.add_internal_template(stl_tmpl, 1001)
session.add_external_template(stl_tmpl, 1001)
session.add_template(otmpl, 888)

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
stl_rec.add_element("sourceMacAddress", pyfixbuf.BASICLIST)
# now we are ready to export! (export 10 flows)

buf.write_ie_options_record("weird3byteElement", otmpl) 

count = 0

while count < 10:
    rec['sourceIPv4Address']= "192.168.1.3"
    rec['destinationIPv4Address'] = "10.5.2.3"
    rec['sourceTransportPort'] = 1 + count
    rec['destinationTransportPort'] = 15 - count
    rec['protocolIdentifier'] = 17
    rec['packetTotalCount'] = 98
    rec['reversePacketTotalCount'] = 111

    # set stl_rec
    stl_rec["octetTotalCount"] = 74653
    stl_rec["reverseOctetTotalCount"] = 898989
    b1 = "00:00:00:01:01:01"
    b2 = "02:02:03:03:04:04"

    stl_rec["sourceMacAddress"] = [b1,b2]

    # need to do this bc stl_rec was not initialized with template
    stl_rec.set_template(stl_tmpl)

    sub_rec["subTemplateList"] = [stl_rec]

    # need to do this bc sub_rec was not initialized with template
    sub_rec.set_template(sub_tmpl)

    # this shows the 2 different ways to initalize a basicList
    sub_rec.init_basic_list("BL", 2, "httpUserAgent")
    sub_rec["BL"] = ['Mozilla','Safari']
    bl = pyfixbuf.BL(infomodel, "httpServerString", 1)
    bl[0] = "wikipedia"
    sub_rec['BL2'] = bl

    stml = [sub_rec, sub_rec]

    rec["subTemplateMultiList"] = stml

    #done! just append the record
    buf.append(rec)
    count += 1

    # get the stml from the rec so we can clear all the nested lists

    for entry in rec["subTemplateMultiList"]:
        entry.set_record(sub_rec)
        for item in entry:
            item.clear_all_lists()

    # clear the stml or you can use rec.clear_all_lists()
    rec["subTemplateMultiList"].clear()

# now emit the buffer so we write everything that's in the buffer
buf.emit()
sys.stdout.write("Finished.\n")


