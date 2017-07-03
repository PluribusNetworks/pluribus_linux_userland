#!/usr/bin/env python
#######################################################################
# Copyright (C) 2013 by Carnegie Mellon University.
# See license information in LICENSE-OPENSOURCE.txt 
#######################################################################

import unittest
import warnings
import sys
import os
import pyfixbuf
from pyfixbuf import *

SAMPLE_FILE = os.path.join(os.path.dirname(__file__), "sampleipfix.ipfix")

class TestInfoElement(unittest.TestCase):
    
    def testInfoElementBasicConstruction(self):
        a = InfoElement("elementOne", CERT_PEN, 722)
        self.assertRaises(TypeError, InfoElement, "elementTwo")
        self.assertRaises(TypeError, InfoElement, "elementThree", CERT_PEN)
        self.assertRaises(TypeError, InfoElement, name="elementFour", id=899)
        self.assertRaises(TypeError, InfoElement, "elementFive", enterprise_number=999)
        self.assertRaises(TypeError, InfoElement, "elementSix", reversible=False, endian=True)
        b = InfoElement("elementSeven", 722, 11, 6, True, True)
        self.assertEqual(b.length, 6)
        self.assertEqual(b.endian, True)
        c = InfoElement("elementEight", 0, 11, True, reversible=False)
        ans = c.reversible
        self.assertEqual(ans, False)

    def testInfoElementAdvancedConstruction(self):
        a = InfoElement("elementFull", CERT_PEN, 14, 1, True, True, 0, 0, 0, 0, 0, "my full element")
        self.assertEqual(a.length, 1)
        d = InfoElement("elementEight", CERT_PEN, 14, True, True, 0, 0, 0, 0, 0, 0, "my eighth element")
        self.assertEqual(d.reversible, True)
        self.assertEqual(d.endian, False)
        e = InfoElement("elementNine", 1, 25, min=5, max=15, description="niner")
        self.assertEqual(e.min, 5)
        self.assertEqual(e.max, 15)
        self.assertEqual(e.id, 25)
        self.assertEqual(e.ent, 1)
        self.assertEqual(e.length, VARLEN)
        self.assertEqual(e.description, "niner")
        self.assertEqual(e.name, "elementNine")

        f = InfoElement("element10", 34, 99, reversible=True, endian=False, type=DataType.IP4ADDR, units=Units.PACKETS, semantic=2)
        self.assertEqual(f.units, Units.PACKETS)
        self.assertEqual(f.type, DataType.IP4ADDR)
        self.assertEqual(f.length, 4)
        self.assertEqual(f.semantic, 2)
        self.assertEqual(f.description, None)
        self.assertEqual(f.min, 0)
        self.assertEqual(f.max, 0)
        self.assertEqual(f.reversible, True)
        self.assertEqual(f.endian, False)


class TestInfoModel(unittest.TestCase):

    def setUp(self):
        self.ie1 = InfoElement("elementOne", CERT_PEN, 14, 2, True, True, DataType.UINT16, 0, 65535, 0, 0, "my number one")
        self.ie2 = InfoElement("elementTwo", CERT_PEN, 16, 4)
        self.ie3 = InfoElement("elementThree", 0, 999, 2)
        self.ie4 = InfoElement("elementReplace", 0, 999, 4)
        self.ie5 = InfoElement("elementReplacePEN", 32, 999, 6)
        self.ielist = [self.ie1, self.ie2, self.ie3]

    def testInfoModelConstruction(self):
        a = InfoModel()

    def testInfoModelAddElement(self):
        a = InfoModel()
        a.add_element(self.ie1)
        self.assertEqual(a.get_element_length(self.ie1.name), 2)
        self.assertEqual(a.get_element_type(self.ie1.name), DataType.UINT16)
        ie = a.get_element(self.ie1.name)
        self.assertEqual(ie.name, self.ie1.name)
        self.assertEqual(ie.units, self.ie1.units)
        self.assertEqual(ie.type, self.ie1.type)
        self.assertEqual(ie.length, self.ie1.length)
        self.assertEqual(ie.min, self.ie1.min)
        self.assertEqual(ie.max, self.ie1.max)
        self.assertEqual(ie.semantic, self.ie1.semantic)
        self.assertEqual(ie.id, self.ie1.id)
        self.assertEqual(ie.ent, self.ie1.ent)
        self.assertEqual(ie.description, self.ie1.description)
        
    def testInfoModelAddElementList(self):
        a = InfoModel()
        a.add_element_list(self.ielist)
        self.assertEqual(a.get_element_length(self.ie1.name), 2)
        self.assertEqual(a.get_element_type(self.ie1.name), DataType.UINT16)
        ie = a.get_element(self.ie1.name)
        self.assertEqual(ie.name, self.ie1.name)
        self.assertEqual(ie.units, self.ie1.units)
        self.assertEqual(ie.type, self.ie1.type)
        self.assertEqual(ie.length, self.ie1.length)
        self.assertEqual(ie.min, self.ie1.min)
        self.assertEqual(ie.max, self.ie1.max)
        self.assertEqual(ie.semantic, self.ie1.semantic)
        self.assertEqual(ie.id, self.ie1.id)
        self.assertEqual(ie.ent, self.ie1.ent)
        self.assertEqual(ie.description, self.ie1.description)
        self.assertEqual(a.get_element_length(self.ie2.name), 4)
        self.assertEqual(a.get_element_type(self.ie2.name), DataType.OCTET_ARRAY)
        ie = a.get_element(self.ie2.name)
        self.assertEqual(ie.name, self.ie2.name)
        self.assertEqual(ie.units, self.ie2.units)
        self.assertEqual(ie.type, self.ie2.type)
        self.assertEqual(ie.length, self.ie2.length)
        self.assertEqual(ie.min, self.ie2.min)
        self.assertEqual(ie.max, self.ie2.max)
        self.assertEqual(ie.semantic, self.ie2.semantic)
        self.assertEqual(ie.id, self.ie2.id)
        self.assertEqual(ie.ent, self.ie2.ent)
        self.assertEqual(ie.description, self.ie2.description)
        self.assertEqual(a.get_element_type(self.ie3.name), DataType.OCTET_ARRAY)
        ie = a.get_element(self.ie3.name)
        self.assertEqual(ie.name, self.ie3.name)
        self.assertEqual(ie.units, self.ie3.units)
        self.assertEqual(ie.type, self.ie3.type)
        self.assertEqual(ie.length, self.ie3.length)
        self.assertEqual(ie.min, self.ie3.min)
        self.assertEqual(ie.max, self.ie3.max)
        self.assertEqual(ie.semantic, self.ie3.semantic)
        self.assertEqual(ie.id, self.ie3.id)
        self.assertEqual(ie.ent, self.ie3.ent)
        self.assertEqual(ie.description, self.ie3.description)

    def testInfoModelAddElementReplace(self):
        a = InfoModel()
        a.add_element(self.ie3)
        a.add_element(self.ie4)
        self.assertEqual(a.get_element_length("elementReplace"), 4)
        self.assertEqual(a.get_element_length("elementThree"), 2)
        ie = a.get_element(id=999)
        self.assertEqual(ie.name, "elementReplace")
        self.assertEqual(ie.units, self.ie4.units)
        self.assertEqual(ie.type, self.ie4.type)
        self.assertEqual(ie.length, self.ie4.length)
        self.assertEqual(ie.min, self.ie4.min)
        self.assertEqual(ie.max, self.ie4.max)
        self.assertEqual(ie.semantic, self.ie4.semantic)
        self.assertEqual(ie.id, self.ie4.id)
        self.assertEqual(ie.ent, self.ie4.ent)
        self.assertEqual(ie.description, self.ie4.description)
        
        a.add_element(self.ie5)
        ie2 = a.get_element(id=999, ent=32)
        self.assertEqual(ie2.name, "elementReplacePEN")
        self.assertEqual(ie2.length, self.ie5.length)

        
        self.assertEqual(a.get_element_length("elementReplace"), 4)


    def testInfoModelInfoElementDNE(self):
        
        a = InfoModel()
        ie = a.get_element(id=999)
        self.assertEqual(ie.name, None)

        ie2 = a.get_element("mycoolElement")
        self.assertEqual(ie2.id, 0)
        self.assertEqual(ie2.description, None)
        self.assertEqual(ie2.ent, 0)

#    def testInfoModelOptionsElement(self):
#        a = InfoModel()
#        r = Record(a)
#        r.add_element("elementOne")
#        r.add_element("elementTwo")


class TestInfoElementSpec(unittest.TestCase):
    
    def testInfoElementSpecConstructor(self):
        
        i = InfoElementSpec("sourceTransportPort")
        self.assertEqual(i.name, "sourceTransportPort")
        self.assertEqual(i.length, 0)

        e = InfoElementSpec("sourceTransportPort", 4)
        self.assertEqual(e.name, "sourceTransportPort")
        self.assertEqual(e.length, 4)

        self.assertRaises(TypeError, InfoElementSpec, length=2)
        

class TestTemplate(unittest.TestCase):

    def setUp(self):
        self.model = InfoModel()
        self.spec = InfoElementSpec("sourceTransportPort")
        self.spec2 = InfoElementSpec("destinationTransportPort")
        self.nospec = InfoElementSpec("notanElement")
        self.speclist = [self.spec, self.spec2]

        self.speclist2=[InfoElementSpec("sourceIPv4Address"), 
                        InfoElementSpec("destinationIPv4Address"),
                        InfoElementSpec("octetTotalCount", 4)]

        self.invalidspec = InfoElementSpec("protocolIdentifier", 4)
        self.invalidspec2 = InfoElementSpec("sourceIPv4Address", 2)
        
        self.invalidspeclist = [self.invalidspec, self.invalidspec2]

    def testTemplateConstructor(self):
        tmpl = Template(self.model)

        self.assertRaises(TypeError, Template, self.spec)

        oTmpl = Template(self.model, 1)

        self.assertEqual(oTmpl.type, 1)
        self.assertEqual(tmpl.type, 0)

    def testTemplateScope(self):
        tmpl = Template(self.model)

        tmpl.add_spec(self.spec)
        tmpl.scope = 1

        self.assertEqual(tmpl.scope, 1)

        oTmpl = Template(self.model, 1)
        self.assertEqual(tmpl.scope, 1)

    def testAddSpecs(self):
        tmpl = Template(self.model)

        tmpl.add_spec(self.spec)

        tmpl.add_spec_list(self.speclist)
        tmpl.add_spec_list(self.speclist2)

        self.assertEqual(len(tmpl), 6)
        self.assertTrue("sourceTransportPort" in tmpl)
        self.assertTrue(self.spec in tmpl)
        self.assertFalse("fooElement" in tmpl)
        ie = InfoElement("myWeirdElement", CERT_PEN, 987)
        self.model.add_element(ie)
        tmpl.add_spec(InfoElementSpec("myWeirdElement"))
        self.assertTrue(ie in tmpl)
        self.assertRaises(StandardError, tmpl.add_spec, self.nospec)
        self.assertRaises(Exception, tmpl.add_spec, self.model)
        
        self.assertEqual(tmpl.template_id, 0)

        self.assertRaises(Exception, tmpl.add_spec, self.invalidspec)
        self.assertRaises(Exception, tmpl.add_spec_list, self.invalidspeclist)

    def testTemplateGetItem(self):
        tmpl = Template(self.model)
        tmpl.add_spec(self.spec)
        tmpl.add_spec_list(self.speclist2)

        self.assertEqual(tmpl["octetTotalCount"].length, 4)
        self.assertEqual(tmpl["sourceTransportPort"].length, 0)

        self.assertEqual(tmpl[0].name, "sourceTransportPort")
        self.assertRaises(IndexError, tmpl.__getitem__, 10)
        self.assertRaises(KeyError, tmpl.__getitem__, "yayayaya")
        self.assertRaises(TypeError, tmpl.__getitem__, self.spec)


    def testTemplateGetIndexedIE(self):
        tmpl = Template(self.model)
        
        tmpl.add_spec(self.spec)
        tmpl.add_spec(self.spec2)
        tmpl.add_spec_list(self.speclist2)

        ie = tmpl.getIndexedIE(0)
        self.assertEqual(self.spec.name, ie.tname)
        self.assertRaises(IndexError, tmpl.getIndexedIE, 9)
        self.assertRaises(AttributeError, tmpl.getIndexedIE, "keke")
        ie2 = tmpl.getIndexedIE(4)
        self.assertEqual(ie2.tname, "octetTotalCount")

class TestSession(unittest.TestCase):

    def setUp(self):
        self.model = InfoModel()
        self.spec = InfoElementSpec("sourceTransportPort")
        self.spec2 = InfoElementSpec("destinationTransportPort")
        self.nospec = InfoElementSpec("notanElement")
        self.speclist = [self.spec, self.spec2]

        self.speclist2=[InfoElementSpec("sourceIPv4Address"), 
                        InfoElementSpec("destinationIPv4Address"),
                        InfoElementSpec("octetTotalCount", 4)]
        self.tmpl = Template(self.model)
        self.tmpl.add_spec_list(self.speclist)
        self.tmpl.add_spec_list(self.speclist2)
        self.oTmpl = Template(self.model, 1)

    def testSessionConstructor(self):
        sess = Session(self.model)
        self.assertRaises(TypeError, Session, self.spec)
        sess2 = Session(InfoModel())
        
    def testSessionAddOptionsTemplate(self):
        sess = Session(self.model)
        
        sess.add_template(self.oTmpl, 259)
        self.assertEqual(len(self.oTmpl), 10)

    def testSessionAddTemplate(self):
        sess = Session(self.model)
        self.assertRaises(Exception, sess.add_template, self.tmpl, 20000000)
        
        sess.add_template(self.tmpl, 654)
        self.assertEqual(self.tmpl.template_id, 654)
        
        rv = sess.add_template(self.tmpl, 999)
        self.assertEqual(self.tmpl.template_id, rv)
        self.assertEqual(rv, 999)
        
    def testSessionAddInternal(self):
        sess = Session(self.model)

        self.assertRaises(Exception, sess.add_internal_template, self.tmpl, 20000000)

        sess.add_internal_template(self.tmpl, 654)
        self.assertEqual(self.tmpl.template_id, 654)

        rv = sess.add_internal_template(self.tmpl, 999)
        self.assertEqual(self.tmpl.template_id, rv)
        self.assertEqual(rv, 999)

    def testSessionAddExternal(self):
        sess = Session(self.model)

        self.assertRaises(Exception, sess.add_external_template, self.tmpl, 20000000)

        sess.add_external_template(self.tmpl, 654)
        self.assertEqual(self.tmpl.template_id, 654)

        rv = sess.add_external_template(self.tmpl, 999)
        self.assertEqual(self.tmpl.template_id, rv)
        self.assertEqual(rv, 999)

    def testSessionDecodeOnly(self):
        sess = Session(self.model)

        self.assertRaises(TypeError, sess.decode_only, 999)
        self.assertRaises(Exception, sess.decode_only, ["foo"])
        self.assertRaises(Exception, sess.decode_only, [999, "foo"])

        sess.decode_only([999, 1000])
        sess.decode_only([111])

    def testSessionIgnoreTemplates(self):
        sess = Session(self.model)

        self.assertRaises(TypeError, sess.ignore_templates, 999)
        self.assertRaises(Exception, sess.ignore_templates, ["foo"])
        self.assertRaises(Exception, sess.ignore_templates, [999, "foo"])

        sess.ignore_templates([999, 1000])
        sess.ignore_templates([111])

    def testSessionAddTemplatePair(self):
        sess = Session(self.model)

        self.assertRaises(Exception, sess.add_template_pair, 999, 785)

        sess.add_internal_template(self.tmpl, 999)

        sess.add_template_pair(876, 999)


class TestCollector(unittest.TestCase):
    
    def testCollectorConstructor(self):
        coll = Collector()
        self.assertRaises(TypeError, Collector, "myname")

    def testCollectorInitFile(self):
        coll = Collector()

        self.assertRaises(Exception, coll.init_file, "89734kigl.txt")
        coll.init_file(SAMPLE_FILE)
        

class TestExporter(unittest.TestCase):
    
    def testExporterConstructor(self):
        exporter = Exporter()
        self.assertRaises(TypeError, Exporter, "foo")

    def testExporterInitFile(self):
        exporter = Exporter()
        collector = Collector()
        self.assertRaises(TypeError, Exporter, collector)

        exporter.init_file("testout.ipfix")
        
    def testExporterInitNet(self):
        exporter = Exporter()

        self.assertRaises(Exception, exporter.init_net, "localhost", "tcp", 80)
        self.assertRaises(Exception, exporter.init_net, "localhost", "sctp")
        self.assertRaises(Exception, exporter.init_net, "localhost", "udp", 0)
        exporter.init_net("localhost")
        exporter.init_net("localhost", "udp")
        exporter.init_net("localhost", "tcp", "18000")
        exporter.init_net("127.0.0.1", "udp")

class TestListener(unittest.TestCase):
    
    def setUp(self):
        self.model = InfoModel()
        self.session = Session(self.model)

    def testListenerConstructor(self):
        list = Listener(self.session, "localhost")
        
        self.assertRaises(Exception, Listener, self.session, "localhost", "udp", 90)
        self.assertRaises(Exception, Listener, self.model, "localhost")
        self.assertRaises(Exception, Listener, self.session, "localhost", "http")
        
        list = Listener(self.session, hostname="localhost", transport="tcp", port=18000)
        list = Listener(self.session, "localhost", transport="udp")


class TestRecord(unittest.TestCase):
    
    def setUp(self):
        self.model = InfoModel()
        self.template = Template(self.model)
        self.spec = InfoElementSpec("sourceTransportPort")
        self.spec2 = InfoElementSpec("destinationTransportPort")
        self.nospec = InfoElementSpec("notanElement")
        self.speclist = [self.spec, self.spec2]

        self.speclist2=[InfoElementSpec("sourceIPv4Address"),
                        InfoElementSpec("destinationIPv4Address"),
                        InfoElementSpec("octetTotalCount", 4)]
        self.template.add_spec_list(self.speclist)
        self.template.add_spec_list(self.speclist2)
        
    def testRecordConstructor(self):
        
        rec = Record(self.model)
        
        self.assertRaises(Exception, Record, self.template)
        self.assertRaises(Exception, Record, self.model, self.model)
        rec = Record(self.model, self.template)
        recdict = rec.as_dict()
        self.assertTrue(isinstance(recdict, dict))
        self.assertEqual(len(recdict), 5)
        self.assertTrue("sourceTransportPort" in recdict)
        self.assertTrue("destinationTransportPort" in recdict)
        self.assertTrue("sourceIPv4Address" in recdict)
        self.assertTrue("destinationIPv4Address" in recdict)
        self.assertTrue("octetTotalCount" in recdict)

        self.assertEqual(rec.length, 16)
        
        rec2 = Record(self.model)
        rec2.add_element("sourceTransportPort")
        rec2.add_element("destinationTransportPort")
        rec2.add_element("octetTotalCount")
        rec2.add_element("packetTotalCount", length=4)
        self.assertEqual(rec2.length, 16)
        rec2dict = rec2.as_dict()
        self.assertTrue(isinstance(rec2dict, dict))
        self.assertEqual(len(rec2dict), 4)

    def testRecordAddElementReduced(self):
        
        rec = Record(self.model)
        self.assertRaises(Exception, rec.add_element, "sourceTransportPort", 0, None, 6)
        self.assertRaises(Exception, rec.add_element, "sourceIPv4Address", 0, None, 2)

    def testRecordAddElement(self):
        rec = Record(self.model)

        rec.add_element("sourceTransportPort")
        rec.add_element("octetTotalCount", length=4)
        rec.add_element("weirdList", BASICLIST, "octetTotalCount")
        rec.add_element("weirdList2", BASICLIST)
        rec.add_element("subTemplateList")
        rec.add_element("subTemplateMultiList")
        rec.add_element("sourceTransportPort")
        rec.add_element("sourceTransportPort2", 0, "sourceTransportPort")
        
        self.assertRaises(Exception, rec.add_element, "destinationTransportPort", 99)
        rec.add_element("basicList", BASICLIST, "noelem")
#        self.assertRaises(Exception, rec.add_element, "basicList", BASICLIST, "noelem")
        rec.add_element_list(["sourceIPv4Address", "destinationIPv4Address"])
        self.assertEqual(len(rec), 11)
        self.assertTrue("sourceIPv4Address" in rec)
        self.assertTrue("basicList" in rec)
        recdict = rec.as_dict()
        self.assertEqual(len(recdict), 11)
        self.assertTrue(("sourceTransportPort", 1) in rec)
        self.assertRaises(Exception, rec.copy, self.model)
        rec2 = Record(self.model)
        rec2.add_element_list(["sourceIPv4Address", "sourceTransportPort"])
        rec.copy(rec2)
        self.assertEqual(len(rec2), 2)
        self.assertTrue("sourceIPv4Address" in rec2)
        self.assertTrue("sourceTransportPort" in rec2)
        rec.clear()

    def testRecordIter(self):
        rec = Record(self.model)
        
        rec.add_element("sourceTransportPort")
        rec.add_element("octetTotalCount", length=4)
        rec.add_element("destinationTransportPort")
        rec.add_element("proto", 0, "protocolIdentifier")
        rec["sourceTransportPort"] = 15
        rec["octetTotalCount"] = 15
        rec["destinationTransportPort"] = 15
        rec["proto"] = 15

        for item in rec:
            self.assertEqual(item, 15)

    def testRecordInitBasicList(self):
        rec = Record(self.model)

        rec.add_element_list(["sourceIPv4Address", "destinationIPv4Address", "basicList"])
        rec.add_element("BL", BASICLIST, "octetTotalCount")
        rec.add_element("basicList2", BASICLIST)
        rec.add_element("sipbasic", BASICLIST)
        #no info element
        self.assertRaises(Exception, rec.init_basic_list, "basicList", 3)
        rec.init_basic_list("basicList", 2, "sourceTransportPort")
        rec["basicList"]=[23, 36]
        rec.init_basic_list("BL", 4)
        rec["BL"]=[long(99),long(456),long(345)]
        self.assertRaises(Exception, rec.init_basic_list, "basicList2")
        self.assertEqual(rec.length, 104)
        rec.add_element("destinationTransportPort")
        rec["destinationTransportPort"]=37
        self.assertEqual(rec.length, 106)
        
        self.assertEqual(rec["destinationTransportPort"], 37)
        self.assertEqual(len(rec["BL"]), 4)
        self.assertEqual(rec["basicList"],[23,36])
        self.assertEqual(rec[2], [23,36])
        self.assertEqual(rec["sourceIPv4Address"], '0.0.0.0')
        self.assertEqual(rec[0], '0.0.0.0')

        rec.init_basic_list("sipbasic", 1, "sourceIPv4Address")
        rec["sipbasic"] = "1.2.3.4"
        self.assertEqual(rec["sipbasic"], ["1.2.3.4"])

        rec.clear_basic_list("basicList")
        self.assertFalse(rec["basicList"])
        self.assertEqual(rec.length, 106)

        self.assertRaises(IndexError, rec.__getitem__, 10)

    def testRecordInitBL(self):
        rec = Record(self.model)

        rec.add_element_list(["sourceIPv4Address", "destinationIPv4Address", "basicList"])
        rec.add_element("BL", BASICLIST, "octetTotalCount")
        rec.add_element("basicList2", BASICLIST)
        self.assertRaises(TypeError, BL.__init__, "sourceTransportPort", 2)
        bl = BL(self.model, "sourceTransportPort", 2)
        bl[0] = 13
        bl[1] = 15
        self.assertRaises(TypeError, BL.__setitem__, 2, 14)
        self.assertEqual(len(bl), 2)
        self.assertEqual(bl.semantic, 0)
        rec["basicList"] = bl
        bl.semantic = 12
        self.assertEqual(bl.semantic, 12)
        bl = rec["basicList"]
        ie = bl.element
        self.assertEqual(ie.type, DataType.UINT16)
        self.assertEqual(ie.name, "sourceTransportPort")

        bl2 = BL(self.model, "sourceIPv4Address", 3, 6)
        bl2[0] = "1.2.3.4"
        bl2[1] = "2.3.4.5"
        bl2[2] = "3.4.5.6"

        rec["basicList2"] = bl2

        self.assertEqual(bl2.semantic, 6)
        blg = rec["basicList2"]

        self.assertEqual(len(blg), 3)
        self.assertEqual(rec["basicList2"], ["1.2.3.4", "2.3.4.5", "3.4.5.6"])

        blg.clear()
        self.assertEqual(len(blg), 0)
        bl3 = rec["BL"]
        self.assertEqual(len(bl3), 0)

        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        bl3 = pyfixbuf.BL(self.model, "httpUserAgent", 2)

        bl3[0] = "Mozilla/Firefox"
        bl3[1] = "Safari5.0"
        self.assertEqual(bl3[0], "Mozilla/Firefox")
        self.assertEqual(bl3[1], "Safari5.0")
        
        rec["BL"] = bl3

        self.assertEqual(rec["BL"], ["Mozilla/Firefox", "Safari5.0"])


    def testRecordInitSTML(self):
        rec = Record(self.model)
        subRec = Record(self.model, self.template)
        tmpl = Template(self.model)
        self.assertRaises(Exception, STML, rec, "subTemplateMultiList", 1)
        rec.add_element("subTemplateMultiList")
        
        stml0 = STML(type_count=2)
        stml0[0].entry_init(subRec, self.template, 2)
        subRec["sourceTransportPort"] = 3
        subRec["destinationTransportPort"] = 5
        stml0[0][0] = subRec
        subRec["sourceTransportPort"] = 6
        subRec["destinationTransportPort"] = 7
        stml0[0][1] = subRec
        self.assertEqual(stml0[0][1]["sourceTransportPort"], 6)
        rec["subTemplateMultiList"] = stml0
        # can't add elements after lists have been init'd
        self.assertRaises(Exception, rec.add_element, "protocolIdentifier")
        entry1 = rec["subTemplateMultiList"][0]
        entry1.set_record(subRec)
        newrec = entry1[0]
        self.assertEqual(newrec["sourceTransportPort"], 3)
        self.assertEqual(newrec["destinationTransportPort"], 5)
        newrec = entry1[1]
        self.assertEqual(newrec["sourceTransportPort"], 6)
        self.assertEqual(newrec["destinationTransportPort"], 7)
        self.assertRaises(IndexError, entry1.__getitem__, 2)
        
        # this shows how to create an entry on the fly without initializing
        # if you don't initialize, you have to use set_template and it defaults
        # to only 1 item in the list
        entry2 = stml0[1]
        self.assertRaises(IndexError, stml0.__getitem__, 2)
        recNoTmpl = Record(self.model)
        recNoTmpl.add_element("sourceTransportPort")
        recNoTmpl.add_element("packetTotalCount")
        recNoTmpl.add_element("protocolIdentifier")
        recNoTmpl["sourceTransportPort"] = 101
        recNoTmpl["protocolIdentifier"] = 20
        recNoTmpl["packetTotalCount"]=99999
        self.assertRaises(Exception, entry2.set_template, recNoTmpl)
        tmpl2 = Template(self.model)
        xspecs=[InfoElementSpec("sourceTransportPort"),
                        InfoElementSpec("protocolIdentifier"),
                        InfoElementSpec("packetTotalCount", 4)]
        tmpl2.add_spec_list(xspecs)
        entry2.set_template(tmpl2)
        #entry2.entry_init(recNoTmpl, tmpl2, 1)
        self.assertRaises(Exception, entry2.__setitem__, 0, subRec)
        entry2[0] = recNoTmpl
        #rec["subTemplateMultiList"] = stml0
        self.assertRaises(IndexError, entry2.__setitem__, 1, recNoTmpl)
        newrec = entry2[0]
        self.assertEqual(newrec["sourceTransportPort"], 101)
        self.assertEqual(newrec["protocolIdentifier"], 20)
        self.assertEqual(newrec["packetTotalCount"], 99999)
        
        # set items
        stml = STML(rec, "subTemplateMultiList", 2)
        stml.semantic = 5
        for entry in stml:
            self.assertRaises(Exception, entry.entry_init, rec, self.model) 
            self.assertRaises(Exception, entry.entry_init, rec, tmpl, 3)
            self.assertRaises(Exception, entry.entry_init, rec, self.template, 2)
            entry.entry_init(subRec, self.template, 2)
            for item in entry:
                item["sourceTransportPort"] = 3
                item["destinationTransportPort"]=5
                item["sourceIPv4Address"]="127.0.0.1"
                item["destinationIPv4Address"]="87.2.3.4"
                item["octetTotalCount"]=89
        self.assertEqual(stml.count, 2)
        self.assertEqual(stml.semantic, 5)
        stml.clear()
        self.assertEqual(stml.count, 0)
        # get items
        stml2 = rec["subTemplateMultiList"]
        for item in stml:
            self.assertTrue("sourceTransportPort" in item)
            self.assertRaises(Exception, item.set_record, self.model)
            self.assertEqual(item.count, 2)
            self.assertEqual(item.template_id, 0)
            item.set_record(subRec)
            for entry in item:
                self.assertEqual(entry["sourceTransportPort"], 3)
                self.assertEqual(entry["destinationTransportPort"], 5)
                self.assertEqual(entry["sourceIPv4Address"], "127.0.0.1")
                self.assertEqual(entry["destinationIPv4Address"], "87.2.3.4")
                self.assertEqual(entry["octetTotalCount"], 89)

    def testRecordInitSTL(self):
        rec = Record(self.model)
        subRec = Record(self.model, self.template)
        tmpl = Template(self.model)
        self.assertRaises(Exception, STL, rec, "subTemplateList", 1)
        rec.add_element("subTemplateList")
        rec.add_element("sourceTransportPort")
        rec.add_element("otherList", SUBTEMPLATELIST)
        rec.add_element("newList", 0, "subTemplateList")
        self.assertRaises(Exception, STL, rec, "sourceTransportPort")
        stl = STL(rec, "subTemplateList")
        # can't add elements after lists have been init'd
        self.assertRaises(Exception, rec.add_element, "protocolIdentifier")
        self.assertRaises(Exception, STL, subRec, tmpl)
        stl.entry_init(subRec, self.template, 2)
        stl.semantic=9
        for strec in stl:
            strec["octetTotalCount"] = 8888888
            strec["sourceTransportPort"] = 80
            strec["destinationTransportPort"] = 1137

        self.assertEqual(stl.template_id, 0)
        self.assertEqual(stl.count, 2)
        self.assertEqual(stl.semantic, 9)
        stl2 = STL(rec, "otherList")
        stl2.entry_init(subRec, self.template, 0)
        
        get_stl = rec["subTemplateList"]
        get_stl.set_record(subRec)
        self.assertEqual(get_stl.semantic, 9)
        self.assertEqual(get_stl.count, 2)
        for thing in get_stl:
            self.assertEqual(thing["octetTotalCount"], 8888888)
            self.assertEqual(thing["sourceTransportPort"], 80)
            self.assertEqual(thing["destinationTransportPort"], 1137)
            self.assertEqual(thing["sourceIPv4Address"], "0.0.0.0")
        
        get_stl.clear()
        self.assertEqual(get_stl.count, 0)
        
        #test other method
        newstl = STL()
        newstl.entry_init(subRec, self.template, 1)
        subRec["octetTotalCount"] = 50
        subRec["sourceTransportPort"] = 8080
        subRec["destinationTransportPort"] = 1024
        newstl[0] = subRec

        newRec = newstl[0]
        
        self.assertEqual(newRec["octetTotalCount"], 50)
        self.assertEqual(newRec["sourceTransportPort"], 8080)
        self.assertEqual(newRec["destinationTransportPort"], 1024)

        rec["newList"] = newstl
        
        rstl = rec["newList"]
        rstl.set_record(subRec)
        self.assertEqual(rstl["octetTotalCount"], 50)
        orec = rstl[0]
        self.assertEqual(orec["sourceTransportPort"], 8080)

    def testRecordSetSTLtoList(self):
        rec = Record(self.model)
        subRec = Record(self.model, self.template)
        tmpl = Template(self.model)
        rec.add_element("subTemplateList")
        rec.add_element("otherList", SUBTEMPLATELIST)
        rec.add_element("octetTotalCount")
        rec.add_element("packetTotalCount")
        tmpl.add_spec_list(self.speclist)
        subRecAlt = Record(self.model, tmpl)
        subRecNoTmpl = Record(self.model)
        subRecNoTmpl.add_element("sourceTransportPort")
        subRecNoTmpl.add_element("destinationTransportPort")

        stl = STL()
        subRec2 = Record(self.model, record=subRec)
        subRec3 = Record(self.model, record=subRec)
        
        subRec["sourceTransportPort"] = 1
        subRec["destinationTransportPort"] = 2
        subRec["sourceIPv4Address"] = "1.2.3.4"
        subRec["destinationIPv4Address"] = "2.3.4.5"
        subRec["octetTotalCount"]= 100

        subRec2["sourceTransportPort"] = 3
        subRec2["destinationTransportPort"] = 4
        subRec2["sourceIPv4Address"] = "3.4.5.6"
        subRec2["destinationIPv4Address"] = "4.5.6.7"
        subRec2["octetTotalCount"]= 200

        subRec3["sourceTransportPort"] = 5
        subRec3["destinationTransportPort"] = 6
        subRec3["sourceIPv4Address"] = "5.6.7.8"
        subRec3["destinationIPv4Address"] = "6.7.8.9"
        subRec3["octetTotalCount"]= 300

        subRecAlt["sourceTransportPort"]=8
        subRecAlt["destinationTransportPort"] = 9

        subRecNoTmpl["sourceTransportPort"]=11
        subRecNoTmpl["destinationTransportPort"] = 12

        # different records will raise exception
        self.assertRaises(Exception, rec["subTemplateList"], [subRec, subRec2, subRec3, subRecAlt])

        stl = [subRec, subRec2, subRec3]
        
        rec["subTemplateList"] = stl
        rec["otherList"] = [subRecNoTmpl, subRecAlt]

        newstl = rec["subTemplateList"]
        self.assertEqual(len(newstl), 3)
        newstl.set_record(subRec)
        self.assertEqual(newstl[1]["sourceTransportPort"], 3)
        self.assertEqual(newstl[1]["destinationTransportPort"], 4)
        self.assertEqual(newstl[1]["octetTotalCount"], 200)
        self.assertEqual(newstl[1]["sourceIPv4Address"], "3.4.5.6")

        self.assertEqual(newstl[2]["octetTotalCount"], 300)

    def testRecordSetSTMLEntrytoList(self):
        rec = Record(self.model)
        subRec = Record(self.model, self.template)

        tmpl = Template(self.model)
        rec.add_element("subTemplateMultiList")
        rec.add_element("octetTotalCount")
        rec.add_element("packetTotalCount")
        tmpl.add_spec_list(self.speclist)
        subRecAlt = Record(self.model, tmpl)        

        stml0 = STML(type_count=1)
        subRec2 = Record(self.model, record=subRec)
        subRec3 = Record(self.model, record=subRec)

        subRec["sourceTransportPort"] = 1
        subRec["destinationTransportPort"] = 2
        subRec["sourceIPv4Address"] = "1.2.3.4"
        subRec["destinationIPv4Address"] = "2.3.4.5"
        subRec["octetTotalCount"]= 100

        subRec2["sourceTransportPort"] = 3
        subRec2["destinationTransportPort"] = 4
        subRec2["sourceIPv4Address"] = "3.4.5.6"
        subRec2["destinationIPv4Address"] = "4.5.6.7"
        subRec2["octetTotalCount"]= 200

        subRec3["sourceTransportPort"] = 5
        subRec3["destinationTransportPort"] = 6
        subRec3["sourceIPv4Address"] = "5.6.7.8"
        subRec3["destinationIPv4Address"] = "6.7.8.9"
        subRec3["octetTotalCount"]= 300

        subRecAlt["sourceTransportPort"]=8
        subRecAlt["destinationTransportPort"] = 9
        self.assertRaises(Exception, stml0[0].__setitem__, [subRec, subRec2, subRec3, subRecAlt])
        stml0[0] = [subRec, subRec2, subRec3]

        rec["subTemplateMultiList"] = stml0

        newstml = rec["subTemplateMultiList"]
        self.assertEqual(len(newstml), 1)
        newstml[0].set_record(subRec)
        self.assertEqual(newstml[0][1]["sourceTransportPort"], 3)
        self.assertEqual(newstml[0][1]["destinationTransportPort"], 4)
        self.assertEqual(newstml[0][1]["octetTotalCount"], 200)
        self.assertEqual(newstml[0][1]["sourceIPv4Address"], "3.4.5.6")

        self.assertEqual(newstml[0][2]["octetTotalCount"], 300)

    def testRecordSetSTMLtoList(self):
        rec = Record(self.model)
        subRec = Record(self.model, self.template)
        tmpl = Template(self.model)
        rec.add_element("subTemplateMultiList")
        rec.add_element("octetTotalCount")
        rec.add_element("packetTotalCount")
        tmpl.add_spec_list(self.speclist)
        subRecAlt = Record(self.model, tmpl)
        subRecNoTmpl = Record(self.model)
        subRecNoTmpl.add_element("sourceTransportPort")
        subRecNoTmpl.add_element("destinationTransportPort")
        
        subRec2 = Record(self.model, record=subRec)
        subRec3 = Record(self.model, record=subRec)

        subRec["sourceTransportPort"] = 1
        subRec["destinationTransportPort"] = 2
        subRec["sourceIPv4Address"] = "1.2.3.4"
        subRec["destinationIPv4Address"] = "2.3.4.5"
        subRec["octetTotalCount"]= 100

        subRec2["sourceTransportPort"] = 3
        subRec2["destinationTransportPort"] = 4
        subRec2["sourceIPv4Address"] = "3.4.5.6"
        subRec2["destinationIPv4Address"] = "4.5.6.7"
        subRec2["octetTotalCount"]= 200

        subRec3["sourceTransportPort"] = 5
        subRec3["destinationTransportPort"] = 6
        subRec3["sourceIPv4Address"] = "5.6.7.8"
        subRec3["destinationIPv4Address"] = "6.7.8.9"
        subRec3["octetTotalCount"]= 300

        subRecAlt["sourceTransportPort"]=8
        subRecAlt["destinationTransportPort"] = 9

        subRecNoTmpl["sourceTransportPort"]=15
        subRecNoTmpl["destinationTransportPort"]=19
        self.assertRaises(Exception, subRecNoTmpl.set_template, self.template)
        subRecNoTmpl.set_template(tmpl)

        stml0 = [subRec, subRec2, subRec3, subRecAlt, subRecNoTmpl]

        rec["subTemplateMultiList"] = stml0

        newstml = rec["subTemplateMultiList"]
        self.assertEqual(len(newstml), 2)
        newstml[0].set_record(subRec)
        self.assertEqual(len(newstml[0]), 3)
        self.assertEqual(len(newstml[1]), 2)
        self.assertEqual(newstml[0][1]["sourceTransportPort"], 3)
        self.assertEqual(newstml[0][1]["destinationTransportPort"], 4)
        self.assertEqual(newstml[0][1]["octetTotalCount"], 200)
        self.assertEqual(newstml[0][1]["sourceIPv4Address"], "3.4.5.6")

        self.assertEqual(newstml[0][2]["octetTotalCount"], 300)

        self.assertRaises(Exception, newstml[1].__getitem__, "sourceTransportPort")
        newstml[1].set_record(subRecAlt)
        self.assertEqual(newstml[1][0]["sourceTransportPort"], 8)
        self.assertRaises(Exception, newstml[1][1].__getitem__, "octetTotalCount")
        self.assertEqual(newstml[1][1]["destinationTransportPort"], 19)

class TestBuffer(unittest.TestCase):
    
    def setUp(self):
        self.model = InfoModel()
        self.template = Template(self.model)
        self.spec = InfoElementSpec("sourceTransportPort")
        self.spec2 = InfoElementSpec("destinationTransportPort")
        self.nospec = InfoElementSpec("notanElement")
        self.speclist = [self.spec, self.spec2]

        self.speclist2=[InfoElementSpec("sourceIPv4Address"),
                        InfoElementSpec("destinationIPv4Address"),
                        InfoElementSpec("octetTotalCount", 4)]
        self.template.add_spec_list(self.speclist)
        self.template.add_spec_list(self.speclist2)

        self.session = Session(self.model)

    def testBufferConstructor(self):
        self.assertRaises(Exception, Buffer, self.model)
        rec = Record(self.model, self.template)
        buf = Buffer(rec)

    def testBufferInitExporter(self):
        rec = Record(self.model, self.template)
        exp = Exporter()
        buf = Buffer(rec)

        self.assertRaises(Exception, buf.init_export, self.model, exp)
        self.assertRaises(Exception, buf.init_export, self.session, rec)
        self.assertRaises(Exception, buf.init_export, self.session, exp)
        exp.init_file("myout.ipfix")
       
        buf.init_export(self.session, exp)
        
    def testBufferInitCollection(self):
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)

        self.assertRaises(Exception, buf.init_collection, self.model, coll)
        self.assertRaises(Exception, buf.init_collection, self.session, rec)
        self.assertRaises(Exception, buf.init_collection, self.session, coll)

        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        

    def testBufferSetTemplates(self):
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)

        self.assertRaises(Exception, buf.set_internal_template, 999)
        self.session.add_template(self.template, 999)

        self.assertRaises(Exception, buf.set_internal_template, 999)
        self.assertRaises(Exception, buf.set_export_template, 999)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        buf.set_internal_template(999)

        buf.set_export_template(999)

    def testBufferWriteOptions(self):
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        exp = Exporter()
        coll.init_file(SAMPLE_FILE)
        exp.init_file("myout.ipfix")
        ie = InfoElement("myReallyCoolElement", CERT_PEN, 6589, 2, True, True, DataType.IP4ADDR)
        self.model.add_element(ie)
        self.assertRaises(Exception, buf.write_ie_options_record, "notanElement", self.template)
        buf.init_collection(self.session, coll)
        self.assertRaises(Exception, buf.write_ie_options_record, "notanElement", self.template)

        buf.init_export(self.session, exp)
        self.assertRaises(Exception, buf.write_ie_options_record, "notanElement", self.template)
        
        # not an Options Template
        self.assertRaises(Exception, buf.write_ie_options_record, "myReallyCoolElement", self.template)

        oTmpl = Template(self.model, 1)
        #buf.write_ie_options_record("myReallyCoolElement", oTmpl)


    def testBufferNext(self):
        rec = Record(self.model)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)

        rec.add_element("sourceTransportPort")
        rec.add_element("sourceIPv4Address")
        rec.add_element("packetTotalCount")
        rec.add_element("subTemplateList")
        buf.init_collection(self.session, coll)
        self.assertRaises(Exception, buf.set_internal_template, 999)
        self.session.add_template(self.template, 1001)
        rec = Record(self.model, self.template)
        buf.set_record(rec)
        buf.set_internal_template(1001)

        buf.auto_insert()

        tmpl_next = buf.next_template()
        self.assertEqual(tmpl_next.template_id, 999)
        self.assertEqual(len(tmpl_next), 8)

        count = 0
        for data in buf:
            data = data.as_dict()
            count += 1
            self.assertEqual(data["sourceTransportPort"], count)
            self.assertEqual(data["sourceIPv4Address"], '192.168.1.3')
            self.assertEqual(data["destinationIPv4Address"], '10.5.2.3')
            self.assertEqual(data["octetTotalCount"], 0)
            self.assertEqual(data["destinationTransportPort"], 16-count)

    def testBufferSetRecord(self):
        coll = Collector()
        buf = Buffer()
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 1001)
        rec = Record(self.model, self.template)
        buf.set_record(rec)
        buf.auto_insert()
        count = 0
        for data in buf:
            data = data.as_dict()
            count += 1
            self.assertEqual(data["sourceTransportPort"], count)
            self.assertEqual(data["sourceIPv4Address"], '192.168.1.3')
            self.assertEqual(data["destinationIPv4Address"], '10.5.2.3')
            self.assertEqual(data["octetTotalCount"], 0)
            self.assertEqual(data["destinationTransportPort"], 16-count)

    def testBufferBlindNext(self):
        coll = Collector()
        buf = Buffer()
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)

        buf.auto_insert()
        tmpl_next = buf.next_template()
        # shouldn't have an internal template with this ID
        self.assertRaises(Exception, self.session.get_template, tmpl_next.template_id, True)
        # this returns the external template
        tmpl = self.session.get_template(tmpl_next.template_id)
        # this will create the spec list from the external template
        tmpl.build_spec_list()
        # now add it to the session
        self.session.add_template(tmpl, tmpl_next.template_id)
        # set buffer to the new template
        buf.set_internal_template(tmpl_next.template_id)
        # no record set on buffer 
        count = 0
        for data in buf:
            count += 1
            self.assertEqual(data["sourceTransportPort"], count)
            self.assertEqual(data["sourceIPv4Address"], '192.168.1.3')
            self.assertEqual(data["destinationIPv4Address"], '10.5.2.3')
            self.assertEqual(data["packetTotalCount"], 98)
            self.assertEqual(data["destinationTransportPort"], 16-count)
            self.assertTrue("subTemplateMultiList" in data)
            stml = data["subTemplateMultiList"]
            self.assertEqual(len(stml), 1)
            entry = stml[0]
            self.assertEqual(len(entry), 2)
            self.assertTrue("subTemplateList" in entry)
            stl = entry["subTemplateList"]
            self.assertEqual(len(stl), 1)
            self.assertTrue("basicList" in stl)
            stlrec = stl[0]
            self.assertEqual(len(stlrec), 3)
            # must call this basicList
            self.assertRaises(Exception, entry.__getitem__, "httpUserAgent")
            self.assertEqual(len(entry["basicList"]), 2)
            self.assertEqual(len(entry[("basicList", 1)]), 1)
            bl2 = entry[0][2]
            self.assertEqual(len(bl2), 1)
            self.assertEqual(bl2.element.name, "httpServerString")
            self.assertEqual(bl2, ["wikipedia.com"])
            self.assertEqual(entry[0].count("basicList"), 2)

        self.assertEqual(count, 10)

    def testBufferAutoMode(self):
        coll = Collector()
        buf = Buffer(auto=True)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)

        buf.auto_insert()

        count = 0
        for data in buf:
            count += 1
            self.assertEqual(data["sourceTransportPort"], count)
            self.assertEqual(data["sourceIPv4Address"], '192.168.1.3')
            self.assertEqual(data["destinationIPv4Address"], '10.5.2.3')
            self.assertEqual(data["packetTotalCount"], 98)
            self.assertEqual(data["destinationTransportPort"], 16-count)
            self.assertTrue("subTemplateMultiList" in data)
            stml = data["subTemplateMultiList"]
            self.assertEqual(len(stml), 1)
            entry = stml[0]
            self.assertEqual(len(entry), 2)
            self.assertTrue("subTemplateList" in entry)
            stl = entry["subTemplateList"]
            self.assertEqual(len(stl), 1)
            self.assertTrue("basicList" in stl)
            stlrec = stl[0]
            self.assertEqual(len(stlrec), 3)
            # must call this basicList
            self.assertRaises(Exception, entry.__getitem__, "httpUserAgent")
            self.assertEqual(len(entry["basicList"]), 2)
            self.assertEqual(len(entry[("basicList", 1)]), 1)
            bl2 = entry[0][2]
            self.assertEqual(len(bl2), 1)
            self.assertEqual(bl2.element.name, "httpServerString")
            self.assertEqual(bl2, ["wikipedia.com"])
            self.assertEqual(entry[0].count("basicList"), 2)
        
        self.assertEqual(count, 10)
        
    def testBufferNextSkipOptions(self):
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 1001)
        buf.set_internal_template(1001)

        tmpl_next = buf.next_template()
        self.assertEqual(tmpl_next.template_id, 888)
        self.assertEqual(tmpl_next.scope, 1)
        count = 0
        #skip options
        buf.next_record(rec)
        #get next
        for data in buf:
            self.assertEqual(buf.get_template().template_id, 999)
            data = data.as_dict()
            count += 1
            self.assertEqual(data["sourceTransportPort"], count)
            self.assertEqual(data["sourceIPv4Address"], '192.168.1.3')
            self.assertEqual(data["destinationIPv4Address"], '10.5.2.3')
            self.assertEqual(data["octetTotalCount"], 0)
            self.assertEqual(data["destinationTransportPort"], 16-count)

    def testBufferRetrieveSTML(self):
        self.template.add_element("subTemplateMultiList")
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 1001)
        buf.set_internal_template(1001)
        inrec = Record(self.model)
        inrec.add_element("subTemplateList")
        counter = 0
        buf.auto_insert()
        for data in buf:
            if "subTemplateMultiList" in data:
                stml = data["subTemplateMultiList"]
                counter += 1
                if "subTemplateList" in stml:
                    for entry in stml:
                        entry.set_record(inrec)
                        for item in entry:
                            stl = item["subTemplateList"]
                            self.assertEqual(len(stl), 1)
                    
                        stl.clear()
                stml.clear()
        # there are 11 data records, 1 for info element info record
        self.assertEqual(counter, 10)

    def testBufferRetrieveSTL(self):
        self.template.add_element("subTemplateMultiList")
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 444)
        buf.set_internal_template(444)
        inrec = Record(self.model)
        inrec.add_element("subTemplateList")
        inrec.add_element("httpUserAgent", BASICLIST)
        inrec.add_element("httpServerString", BASICLIST)
        counter = 0
        buf.auto_insert()
        for data in buf:
            if "subTemplateMultiList" in data:
                stml = data["subTemplateMultiList"]
                if "subTemplateList" in stml:
                    counter += 1
                    for entry in stml:
                        entry.set_record(inrec)
                        for item in entry:
                            self.assertTrue("Mozilla" in item["httpUserAgent"])
                            stl = item["subTemplateList"]
                            self.assertEqual(stl.template_id, 1001)
                            template = self.session.get_template(stl.template_id)
                            self.assertTrue("octetTotalCount" in template)
                            self.assertEqual(len(stl), 1)

                        stl.clear()
                stml.clear()
        # there are 11 data records, 1 for info element info record
        self.assertEqual(counter, 10)

    def testBufferIgnoreTemplates(self):
        self.template.add_element("subTemplateMultiList")
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 444)
        self.session.ignore_templates([1000])
        buf.set_internal_template(444)
        inrec = Record(self.model)
        inrec.add_element("subTemplateList")
        inrec.add_element("httpUserAgent", BASICLIST)
        inrec.add_element("httpServerString", BASICLIST)

        buf.auto_insert()
        for data in buf:
            template = buf.get_template()
            self.assertEqual(template.template_id, 999)
            stml = data["subTemplateMultiList"]
            for entry in stml:
                entry.set_record(inrec)
                for item in entry:
                    self.assertTrue("Safari" in item["httpUserAgent"])
                    stl = item["subTemplateList"]
                    self.assertEqual(len(stl), 0)

    def testBufferDecodeOnly(self):
        self.template.add_element("subTemplateMultiList")
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 444)
        self.session.decode_only([1000])
        buf.set_internal_template(444)
        inrec = Record(self.model)
        inrec.add_element("subTemplateList")
        inrec.add_element("httpUserAgent", BASICLIST)
        inrec.add_element("httpServerString", BASICLIST)

        buf.auto_insert()
        for data in buf:
            template = buf.get_template()
            self.assertEqual(template.template_id, 999)
            stml = data["subTemplateMultiList"]
            for entry in stml:
                entry.set_record(inrec)
                for item in entry:
                    self.assertTrue("wikipedia.com" in item["httpServerString"])
                    stl = item["subTemplateList"]
                    self.assertEqual(len(stl), 0)

    def testIgnoreOptions(self):
        self.template.add_element("subTemplateMultiList")
        self.model.add_element_list(pyfixbuf.YAF_HTTP_LIST)
        rec = Record(self.model, self.template)
        coll = Collector()
        buf = Buffer(rec)
        coll.init_file(SAMPLE_FILE)
        buf.init_collection(self.session, coll)
        self.session.add_template(self.template, 444)
        buf.set_internal_template(444)
        self.assertRaises(Exception, buf.ignore_options, -1)
        buf.ignore_options(True)
        inrec = Record(self.model)
        inrec.add_element("subTemplateList")
        inrec.add_element("httpUserAgent", BASICLIST)
        inrec.add_element("httpServerString", BASICLIST)

        for data in buf:
            template = buf.get_template()
            self.assertEqual(template.template_id, 999)
            stml = data["subTemplateMultiList"]
            for entry in stml:
                entry.set_record(inrec)
                for item in entry:
                    self.assertTrue("wikipedia.com" in item["httpServerString"])
                    stl = item["subTemplateList"]
                    self.assertEqual(len(stl), 1)        
        

"""
def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestInfoModel))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestInfoElement))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestInfoElementSpec))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestTemplate))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestSession))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestCollector))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestExporter))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestListener))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestRecord))
    suite.addTest(unittest.TestLoader().loadTestsFromTestCase(TestBuffer))

    return suite
"""

if __name__ == '__main__':
    unittest.main()
#    result = unittest.TextTestRunner(verbosity=3).run(suite())
#    if result.errors or result.failures:
#        sys.exit(1)
#    sys.exit(0)
