=======================
Pyfixbuf Examples
=======================

Collector Example
""""""""""""""""""

The pyfixbuf API aims to follow the original C library.  
The following example follows the traditional method of 
collecting IPFIX with libfixbuf:

#. Create an information model
#. Add Private Enterprise Number (PEN) Information Elements to the model.
#. Create an IPFIX template(s).
#. Define what the template(s) will contain.
#. Add the elements to the template.
#. Create an IPFIX collector (file vs TCP vs UDP)
#. Create a session.
#. Add the template(s) to the session.
#. Create an incoming data buffer.
#. Associate the collector and the session to the buffer.
#. Set the internal template on the buffer.
#. Data is read into Records.

.. sourcecode:: python

   #!/usr/bin/env python

   import sys
   # Import pyfixbuf
   import pyfixbuf

   # Import times from netsa-python for nice timestamp formats
   from netsa.data.times import *

   # Test that the number of arguments is correct

   if ( len (sys.argv) != 2):
      print "Must supply ONLY an IPFIX file to read"
      sys.exit()

   # Create an InfoModel
   infomodel = pyfixbuf.InfoModel()

   # Add YAF basic and stats information elements
   infomodel.add_element_list(pyfixbuf.YAF_LIST)
   infomodel.add_element_list(pyfixbuf.YAF_STATS_LIST)

   # Create a Template
   tmpl = pyfixbuf.Template(infomodel)

   # Create a Stats Template to receive YAF Stats (Options) Records
   stats_tmpl = pyfixbuf.Template(infomodel)

   # Add some elements to the internal template
   # This is a normal YAF flow record

   data_list = [pyfixbuf.InfoElementSpec("flowStartMilliseconds"),
             pyfixbuf.InfoElementSpec("flowEndMilliseconds"),
   	     pyfixbuf.InfoElementSpec("octetTotalCount"),
             pyfixbuf.InfoElementSpec("reverseOctetTotalCount"),
             pyfixbuf.InfoElementSpec("packetTotalCount"),
             pyfixbuf.InfoElementSpec("reversePacketTotalCount"),
             pyfixbuf.InfoElementSpec("sourceIPv4Address"),
             pyfixbuf.InfoElementSpec("destinationIPv4Address"),
             pyfixbuf.InfoElementSpec("sourceTransportPort"),
             pyfixbuf.InfoElementSpec("destinationTransportPort"),
             pyfixbuf.InfoElementSpec("flowAttributes"),
             pyfixbuf.InfoElementSpec("reverseFlowAttributes"),
             pyfixbuf.InfoElementSpec("protocolIdentifier"),
             pyfixbuf.InfoElementSpec("flowEndReason"),
             pyfixbuf.InfoElementSpec("silkAppLabel"),
             pyfixbuf.InfoElementSpec("subTemplateMultiList")]

   tmpl.add_spec_list(data_list)

   # Add elements to the stats template (this is a subset of the YAF stats)

   stats_list = [pyfixbuf.InfoElementSpec("exportedFlowRecordTotalCount"),
   	         pyfixbuf.InfoElementSpec("packetTotalCount"),
         	 pyfixbuf.InfoElementSpec("droppedPacketTotalCount"),
           	 pyfixbuf.InfoElementSpec("ignoredPacketTotalCount")]

   stats_tmpl.add_spec_list(stats_list)

   # Create a collector

   collector = pyfixbuf.Collector()

   # Initialize the collector to read an IPFIX file

   collector.init_file(sys.argv[1])

   # create a session

   session = pyfixbuf.Session(infomodel)

   # Add your data template to the session

   session.add_internal_template(tmpl, 999)

   # Add the stats template to the session

   session.add_internal_template(stats_tmpl, 911)

   # Create a Record for each Template and/or each SubTemplate
   # The following rec will contain all the elements in the data template
   rec = pyfixbuf.Record(infomodel, tmpl)

   # The following rec will contain all the elements in the stats template
   statsrec = pyfixbuf.Record(infomodel, stats_tmpl)

   # Create a TCP Record, since YAF exports TCP information in the
   # subTemplateMultiList by default

   tcprec = pyfixbuf.Record(infomodel)

   # Since we don't need a template for this TCP Record because
   # it belongs in the subTemplateMultiList, we have to add
   # the TCP elements using the addElement method

   tcp_elements = ["tcpSequenceNumber", "initialTCPFlags", "unionTCPFlags",
             "reverseInitialTCPFlags", "reverseUnionTCPFlags", "reverseTcpSequenceNumber"]

   tcprec.add_element_list(tcp_elements)

   # create a new buffer for collection - rec matches our internal template
   buf = pyfixbuf.Buffer(rec)

   # initialize the buffer for collection
   buf.init_collection(session, collector)

   # set the internal template on the buffer
   buf.set_internal_template(999)

   # Now we can get the elements from the buffer

   for data in buf:
       data = data.as_dict()
       print "------FLOW-------"
       for key,value in data.items():
       if (key == "flowStartMilliseconds" or key == "flowEndMilliseconds"):
       	  # use netsa-python to print times
       	  print key + ": " + str(make_datetime(value/1000))
       # print every element that is not a subtemplatemultilist
       elif key != "subTemplateMultiList":
       	  print key + ": " + str(value)

       # retrieve STML
       stml = data["subTemplateMultiList"]
       # Iterate through entries in STML
       for entry in stml:
       	   # Is it a TCP Template?
       	   if "tcpSequenceNumber" in entry:
       	      # set the tcprec on the entry
              entry.set_record(tcprec)
	      # iterate through records in this entry of the stml
              for record in entry:
              	  record = record.as_dict()
             	  for key,value in record.items():
	     	      print key + ": " + str(value)
       # clear the STML
       stml.clear()

       # Now check to see if the next record is a stats record
       # by checking the next template on the buffer

       tmpl_next = buf.next_template()
       # if a template has scope - it's an options template
       if ( tmpl_next.scope ):
       	  # Set the internal template to the stats template
       	  buf.set_internal_template(911)
       	  # get the next record in the buffer as a stats record
       	  stats = buf.next_record(statsrec)
       	  print "----STATS----"
       	  if (stats != None):
             stats = stats.as_dict()
             # print all the items in stats
             for key,value in stats.items():
             	 print key + ": " + str(value)
       	    # Set the internal template back to the data template
       	  buf.set_internal_template(999)


It may be the case that the IPFIX data can change often and the application
needs to be able to collect everything that the records contain.  In that
case, pyfixbuf can be used to build Records on the fly based on the templates
that it receives.  This is slightly different than the traditional way of
reading IPFIX.  Typically, the application knows what kind of data it wants
and libfixbuf will populate only the fields the application cares about.
In the following example, the application wants to view the contents of every
IPFIX record in the file.

.. sourcecode:: python

   #!/usr/bin/env python

   import sys
   # Import pyfixbuf
   import pyfixbuf

   # Import times from netsa-python for nice timestamp formats
   from netsa.data.times import *

   # Test that the number of arguments is correct

   if ( len (sys.argv) != 2):
      print "Must supply ONLY an IPFIX file to read"
      sys.exit()

   # Create an InfoModel
   infomodel = pyfixbuf.InfoModel()

   # Create a collector

   collector = pyfixbuf.Collector()

   # Initialize the collector to read an IPFIX file

   collector.init_file(sys.argv[1])

   # create a session

   session = pyfixbuf.Session(infomodel)

   # create a new buffer for collection
   buf = pyfixbuf.Buffer(auto=True)

  # initialize the buffer for collection
   buf.init_collection(session, collector)

   for data in buf:

        print "------FLOW %d-------" % count
    	for key,value in data.as_dict().items():
            if (key == "flowStartMilliseconds" or key == "flowEndMilliseconds"):
               # use netsa-python to print times                                          
               print key + ": " + str(make_datetime(value/1000))
	    # print every element that is not a subtemplatemultilist                               
            elif key != "subTemplateMultiList":
            	 print str(key) + ": " + str(value)
    	# retrieve STML                                                                    
    	if "subTemplateMultiList" in data:
           stml = data["subTemplateMultiList"]
           # Iterate through entries in STML                                              
           for entry in stml:
               for record in entry:
                   record = record.as_dict()
                   for key,value in record.items():
                       if key != "subTemplateList":
                          print str(key) + ": " + str(value)
                   if "subTemplateList" in record:
                       stl = record["subTemplateList"]
                       for sub in stl:
                           for key, value in sub.as_dict().items():
                               print str(key) + ": " + str(value)
                       stl.clear()

 	    # clear the STML                                                               
            stml.clear()
        count += 1


Conversion Example
""""""""""""""""""

Pyfixbuf is often used for converting CSV records or non-IPFIX records
to IPFIX so they can be imported by an IPFIX collector tool, such as
Analysis Pipeline.  The following code provides an example of converting
CSV to IPFIX.  The CSV are DNS records that were converted from NMSG to CSV
with nmsgtool. Specifically, this example transforms the A and AAAA records from CSV
to IPFIX records so they can be read and analyzed by Analysis Pipeline.

.. sourcecode:: python

   
   #!/usr/bin/env python
   ## ------------------------------------------------------------------------
   ## nmsg_to_pipeline.py
   ## sample IPFIX converter/exporter using pyfixbuf.
   ## Takes a csv file that has format <domain name>,<type>,<ttl>,<ip>
   ## ------------------------------------------------------------------------

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

   # Create New Elements for a DNS IPv4 Address and IPv6 Address
   infomodel.add_element(pyfixbuf.InfoElement("dnsRRIPv4Address", pyfixbuf.CERT_PEN, 931, 4, type=pyfixbuf.DataType.IP4ADDR))
   infomodel.add_element(pyfixbuf.InfoElement("dnsRRIPv6Address", pyfixbuf.CERT_PEN, 932, 16, type=pyfixbuf.DataType.IP6ADDR))

   # create the "outer" template
   tmpl = pyfixbuf.Template(infomodel)

   # add elements we want in our template
   a_list = [
    	  pyfixbuf.InfoElementSpec("dnsQName"),
    	  pyfixbuf.InfoElementSpec("dnsRRIPv4Address"),
    	  pyfixbuf.InfoElementSpec("dnsTTL"),
    	  pyfixbuf.InfoElementSpec("dnsQRType")]

   # add elements to our template
   tmpl.add_spec_list(a_list)

   aaaa_list = [
    	  pyfixbuf.InfoElementSpec("dnsQName"),
	  pyfixbuf.InfoElementSpec("dnsRRIPv6Address"),
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

   #for exporters we need to create internal and external templates
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

   #now open NMSG CSV file
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

	   #some records have more than 1 IPv4Address on a line
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


See the other examples included with the pyfixbuf package in "samples."


