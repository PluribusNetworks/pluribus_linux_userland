=================================================
:mod:`pyfixbuf` API Documentation
=================================================

.. automodule:: pyfixbuf

InfoElement
=======================

Information Elements make up the IPFIX Information Model and
IPFIX templates.  All Information Elements consist of a unique
and meaningful name, a private enterprise number (PEN), a numeric identifier,
a length, and a data type.  libfixbuf adds, by default, the IANA
approved Information Elements to the Information Model.  IANA's Information Elements
have a private enterprise
number of 0.  pyfixbuf groups the YAF-defined Information Elements, CERT PEN
6871, by protocol.  YAF_LIST and YAF_STATS are necessary for collecting
default input streams from YAF.  See the tables at the bottom of this page
for a list of the YAF pre-defined information elements by protocol.

If an Information Element (IE) is initialized with the ENDIAN flag set, the
IE is an integer and will be endian-converted on transcode.
If the REVERSIBLE flag is set, a second, reverse information element
will be added to the Information Model.

If an Information Element is initialized with a DataType then
the appropriate Python data type will be returned.  Otherwise, the
value of the Information Element retrieved will be in a Byte Array.
If the Information Element is of type STRING or LIST, the IE length
should be VARLEN. OCTET_ARRAYS may or may not be variable length.
The following is a list of acceptable data types, which are stored
as an enumeration in libfixbuf.  When defining an Information Element
both the type and integer value are accepted.

.. list-table::
  :header-rows: 1
  :widths: 20, 8, 8, 20

  * - Type
    - Integer Value
    - Length
    - Python Return Type
  * - DataType.OCTET_ARRAY
    - 0
    - VARLEN
    - Byte Array
  * - DataType.UINT8
    - 1
    - 1
    - Integer
  * - DataType.UINT16
    - 2
    - 2
    - Long
  * - DataType.UINT32
    - 3
    - 4
    - Long
  * - DataType.UINT64
    - 4
    - 8
    - Long
  * - DataType.INT8
    - 5
    - 1
    - Long
  * - DataType.INT16
    - 6
    - 2
    - Long
  * - DataType.INT32
    - 7
    - 4
    - Long
  * - DataType.INT64
    - 8
    - 8
    - Long
  * - DataType.FLOAT32
    - 9
    - 4
    - Float
  * - DataType.FLOAT64
    - 10
    - 8
    - Float
  * - DataType.BOOL
    - 11
    - 1
    - Bool
  * - DataType.MAC_ADDR
    - 12
    - 6
    - String
  * - DataType.STRING
    - 13
    - VARLEN
    - String
  * - DataType.SECONDS
    - 14
    - 4
    - Long
  * - DataType.MILLISECONDS
    - 15
    - 8
    - Long
  * - DataType.MICROSECONDS
    - 16
    - 8
    - Long
  * - DataType.NANOSECONDS
    - 17
    - 8
    - Long
  * - DataType.IP4ADDR
    - 18
    - 4
    - String
  * - DataType.IP6ADDR
    - 19
    - 16
    - String
  * - DataType.BASIC_LIST
    - 20
    - VARLEN
    - BL
  * - DataType.SUB_TMPL_LIST
    - 21
    - VARLEN
    - STL
  * - DataType.SUB_TMPL_MULTI_LIST
    - 22
    - VARLEN
    - STML

Units, min, max, semantic, and description are all optional parameters
to further describe an information element.  If the process is exporting
Information Element Type Option Records (:rfc:`5610`), this information will help
the collecting process identify the type of information contained in the value of an
Information Element.  Valid Units are listed in the table below.

============================   =============
Units                          Integer Value
============================   =============
Units.NONE                     0
Units.BITS                     1
Units.OCTETS                   2
Units.PACKETS                  3
Units.FLOWS                    4
Units.SECONDS                  5
Units.MILLISECONDS             6
Units.MICROSECONDS             7
Units.NANOSECONDS              8
Units.WORDS                    9
Units.MESSAGES                 10
Units.HOPS                     11
Units.ENTRIES                  12
============================   =============


The following table lists the available Semantic values:

============================   =============
Semantic                       Integer Value
============================   =============
Semantic.DEFAULT               0
Semantic.QUANTITY              1
Semantic.TOTALCOUNTER          2
Semantic.DELTACOUNTER          3
Semantic.IDENTIFIER            4
Semantic.FLAGS                 5
Semantic.LIST                  6
============================   =============

.. autoclass:: InfoElement(name : str, enterprise_number : int, id : int , [length=VARLEN, reversible=False, endian=False, type=DataType.OCTET_ARRAY, units=Units.NONE, min=0, max=0, semantic=Semantic.DEFAULT, description=None])

   .. attribute:: name

      The name, a string, associated with the InfoElement.

   .. attribute:: ent

      The Private Enterprise Number (PEN) associated with the InfoElement.  Default Information
      Elements will have a PEN of 0. An integer with max value 2^32.

   .. attribute:: id

      The Information Element ID that, with the PEN,  uniquely identifies
      the Information Element. ID is an integer with max value 65535.

   .. attribute:: length

      The length associated with the Information Element.
      This is the amount of memory allocated
      for the Information Element.  If the Information Element is of variable length,
      length will contain the size of the fbVarfield struct.

   .. attribute:: type

      The data type associated with the Information Element.  This is stored as an enumeration
      in libfixbuf and can have values 0-23.  If type is not defined, the default type is 0,
      DataType.OCTET_ARRAY.  If the Information Element is defined as VARLEN,
      the default type is 14, DataType.STRING.

   .. attribute:: units

      The units associated with the Information Eleemnt.  This is stored as an enumeration in
      libfixbuf and can have values 0-13.  If units are not defined, the default is Units.NONE.

   .. attribute:: min

      If a range is defined with the Information Element, min is the mininum value accepted.
      Valid values are 0 - 2^64.

   .. attribute:: max

      If a range is defined for an Information Element, max is the maximum value accepted.
      Valid values are 0 - 2^64.

   .. attribute:: semantic

      Semantic value for an Information Element.  This is stored as an enumeration in libfixbuf
      and can have values 0 - 6.  The default semantic is 0, Semantic.DEFAULT.

   .. attribute:: description

      Description of an Information Element.  This is a string.  Default is None.

   .. attribute:: reversible

      True if an Information Element is defined as reversible.

   .. attribute:: endian

      True if an Information Element is defined as endian.

Examples::

	>>> foo = pyfixbuf.InfoElement('fooname', CERT_PEN, 722, units=pyfixbuf.Units.WORDS)
 	>>> bar = pyfixbuf.InfoElement('barname', 123, 565, 1, reversible=True, endian=True)
 	>>> foo2 = pyfixbuf.InfoElement('fooname2', 0, 888, 3, type=pyfixbuf.DataType.OCTET_ARRAY)
      	>>> flo = pyfixbuf.InfoElement('flo_element', 0, 452, 8, endian=True, type=8)


InfoElementSpec
===========================

Information Element Specifications (:class:`InfoElementSpec`) are used
to name an information element for inclusion in a template.  The
Information Element must have already been defined and added to
the Information Model.  An :class:`InfoElementSpec` contains the exact
name of the defined Information Element and an optional length override.

.. autoclass:: InfoElementSpec(name : str , [length=0])

   .. attribute:: name

	The Information Element Specification name.

   .. attribute:: length

        The length override for the Information Element Specification.
	If a length override is not specified at initialization, the value
	of the :class:`InfoElementSpec` length will be 0, and the length used
	for transcode will be the length as defined in the :class:`InfoModel`.


InfoModel
======================

The InfoModel type implements an IPFIX Information Model,
adding the IANA managed Information Elements by default.

.. autoclass:: InfoModel()

   .. automethod:: add_element(element : InfoElement)

   .. automethod:: add_element_list(list: List)

   .. automethod:: get_element_length(name: str , [type: int]) -> length

   .. automethod:: get_element([name: str, id: int, ent: int]) ->  InfoElement

   .. automethod:: get_element_type(name: str) -> type

   .. automethod:: add_options_element(rec : Record)

Examples::

	>>> model = pyfixbuf.InfoModel()
	>>> model.add_element(foo);
	>>> model.add_element_list([foo, bar, flo])
	>>> model.add_element_list(pyfixbuf.YAF_DNS_LIST) # adds all YAF DNS DPI elements
	>>> length = model.get_element_length("sourceTransportPort")
	>>> print length
	2

Template
======================

The :class:`Template` type implements an IPFIX Template or an IPFIX Options
Template.  IPFIX templates contain one or more Information Elements.
If a certain sequence of elements is desired, each Information Element
(:class:`InfoElementSpec`) must be added to the template in the desired
order.  Templates are stored by Template ID and type (internal, external)
per domain in a :class:`Session`.  Template IDs of data sets are numbered from
256 to 65535.  Templates are given a template ID when they are added to
a :class:`Session`. The only difference between Data Templates and Options
Templates is that Options Templates have a scope associated with them,
which gives the context of reported Information Elements in the Data
Records.

An Internal Template is how fixbuf decides what the data should look
like when it is transcoded.  For this reason, an internal template should
match the corresponding :class:`Record`, in terms of the order of Information
Elements. An External Template is sent before
the exported data so that the Collecting Process is able to process
IPFIX messages without necessarily knowing the interpretation of all data
records.

.. autoclass:: Template(InfoModel)

   An Information Model (:class:`InfoModel`) is needed to allocate and
   initialize a new Template.

   .. automethod:: add_spec(spec : InfoElementSpec)

   .. automethod:: add_spec_list(list : List)

   .. automethod:: add_element(name : str)

   .. automethod:: build_spec_list()

   .. method:: getIndexedIE(key: int) -> InfoElement

      Returns the :class:`InfoElement` at the given index *key* in
      the :class:`Template`.  Unlike the __getitem__ method which
      returns the :class:`InfoElementSpec`, this method
      returns the :class:`InfoElement` at a particular index.

   .. automethod:: __contains__([name: str, ie: InfoElement]) -> bool

   .. automethod:: __getitem__(key: str or int)

   .. method:: __len__() -> int

      Returns the number of elements in the template.

   .. attribute:: scope

      Returns the scope associated with the template (integer). Scope can
      and should be changed if template is an Options Template.

   .. attribute:: tid

      Returns the template ID associated with the template.  Template ID
      can only be changed by adding a new template to a (:class: `Session`).

   .. attribute:: type

      Returns `True` if template is an Information Element Type Information
      Template.  Returns `False` otherwise.  This element may not be changed.

Examples::

   >>> tmpl = pyfixbuf.Template(model)
   >>> spec = pyfixbuf.InfoElementSpec("sourceTransportPort")
   >>> spec2 = pyfixbuf.InfoElementSpec("destinationTransportPort")
   >>> tmpl.add_spec(spec)
   >>> tmpl.add_spec(spec2)
   >>> tmpl2 = pyfixbuf.Template(model)
   >>> tmpl2.add_spec_list([pyfixbuf.InfoElementSpec("fooname"),
   		       pyfixbuf.InfoElementSpec("barname")])
   >>> tmpl2.scope = 2
   >>> if "sourceTransportPort" in tmpl:
   >>> 	  print "yes"
   yes


Session
=====================

The state of an IPFIX Transport Session is maintained in the :class:`Session`
object.  This includes all IPFIX Message Sequence Number tracking,
and internal and external template management.  Templates must be added
before collecting or exporting any data.

.. autoclass:: Session(InfoModel)

   .. automethod:: add_template(template : Template , [template_id=0]) -> int

   .. automethod:: add_internal_template(template : Template , [template_id=0]) -> int

   .. automethod:: add_external_template(template : Template , [template_id=0]) -> int

   .. automethod:: decode_only(list: List)

   .. automethod:: ignore_templates(list: List)

   .. automethod:: add_template_pair(external_template_id : int, internal_template_id : int)

   .. method:: export_templates()

      Export the templates associated with this session.  This is necessary
      for an exporting session and must be called before any records are appended to
      the :class:`Buffer`.  :class:`Buffer` must already have a :class:`Session`
      associated with it using :meth:`init_export`.

   .. method:: get_template(template_id : int[, internal=False]) -> Template

      Return the template with the given template_id.  By default it returns
      the external template in the session with the given template_id.
      Returns None if the Template doesn't exist.  The returned template cannot
      be modified.  Set *internal* to True to retrieve the internal template
      with the given *template_id*.

Examples::

	>>> session = pyfixbuf.Session(model)
	>>> session.add_internal_template(289, tmpl)
	>>> auto_id = session.add_external_template(0, tmpl)
	>>> session.decode_only([256, 257])

Exporter
=====================

An Exporter maintains the information needed for its connection
to a corresponding Collecting Process.  An Exporter can be created to
connect via the network using one of the supported IPFIX transport
protocols, or to write to IPFIX files.  Depending on the type of
Exporter desired, one will use one of the following methods:

.. autoclass:: Exporter()

   .. automethod:: init_file(filename: str)

   .. automethod:: init_net(hostname: str, [transport="tcp", port=4739])

Examples::

  >>> exporter = pyfixbuf.Exporter()
  >>> exporter.init_file("/path/to/out.ipfix")
  >>> exporter2 = pyfixbuf.Exporter()
  >>> exporter2.init_net("localhost", "udp", 18000)


Collector
======================

An :class:`Collector` maintains the necessary information for
the connection to a corresponding Exporting Process.  A
:class:`Collector` is used for reading from an IPFIX file.  See
:class:`Listener` for collecting IPFIX over a network.

.. autoclass:: Collector()

   .. automethod:: init_file(filename: str)

Examples::

   >>> collector = pyfixbuf.Collector()
   >>> collector.init_file("path/to/in.ipfix")

Record
================

A :class:`Record` is one of the "core" interaces to the IPFIX data through
libfixbuf.  This is the main object for manipulating the data prior
to export and following import.

.. autoclass:: Record(model : InfoModel , [template=None], [record=None])

   .. automethod:: add_element(key_name : str, [type=0, element_name=None, length=0])

   .. automethod:: add_element_list(list : List)

   .. automethod:: clear_all_lists()

   .. automethod:: clear()

   .. automethod:: init_basic_list(basic_list_key : str, [count=0, element_name=None])

   .. automethod:: clear_basic_list(basic_list_key : str)

   .. method:: __getitem__(key : str, int)

      Returns the value of the element with the given key.  The return type depends 
      on the Information Element type which was defined when initializing the
      :class:`InfoElement`. *key* may be a string which corresponds to the key_name
      given to add_element() or the :class:`InfoElement` name.  *key* may also be
      an integer, which corresponds to the index in the :class:`Record`.

      ============================   =============
      Element Type 		     Return Type
      ============================   =============
      UINT*, INT*		     Long
      FLOAT* 			     Float
      MILLOSECONDS, MICROSECONDS     Long
      NANOSECONDS, SECONDS	     Long
      OCTET_ARRAY  		     Byte Array
      BASICLIST			     :class:`BL`
      VARLEN 			     String
      IP (v4 or v6)		     IP String
      MACADDR                        MAC Address String xx:xx:xx:xx:xx:xx
      SUBTEMPLATELIST                :class:`STL`
      SUBTEMPLATEMULTILIST	     :class:`STML`
      Default (Undefined Type)	     Byte Array
      ============================   =============

   .. automethod:: __setitem__ (key : str, value : int or str)

   .. automethod:: copy(other : Record)

   .. automethod:: is_list(key_name : str) -> bool

   .. automethod:: get_stl_list_entry(key_name : str) -> STL

   .. automethod:: get_stml_list_entry(key_name: str) -> STML

   .. automethod:: as_dict() -> dictionary

   .. automethod:: __len__() -> int

   .. automethod:: __contains__(item: str) -> bool

   .. automethod:: set_template(template: Template)

   .. automethod:: __iter__() -> Record

   .. automethod:: next()
   
   .. automethod:: matches_template(template: Template) -> bool
   
   .. automethod:: count(key_name: str) -> int

Buffer
==============

The :class:`Buffer` implements a transcoding IPFIX Message buffer for
both export and collection.  The :class:`Buffer` is one of the "core" interfaces
to the fixbuf library.  Each :class:`Buffer` must be initialized to do either
collecting or exporting.

.. autoclass:: Buffer(record : Record)

   .. automethod:: init_collection(session : Session, collector : Collector)

   .. automethod:: init_export(session : Session, exporter : Exporter)

   .. automethod:: set_internal_template(v : int)

   .. automethod:: set_export_template(v : int)

   .. automethod:: next_record(record : Record) -> Record

   .. automethod:: next() -> Record

   .. automethod:: __iter__() -> Buffer

   .. automethod:: set_record(record : Record)

   .. automethod:: next_template() -> Template

   .. automethod:: get_template() -> Template

   .. automethod:: append(Record, [v : int])

   .. automethod:: write_ie_options_record(name: str, template: Template)

   .. automethod:: auto_insert()

   .. automethod:: ignore_options(ignore: bool)

   .. method:: emit()

      Emit all records in the buffer.

Examples::

	>>> buf = pyfixbuf.Buffer(my_rec)
	>>> buf.init_collection(session, collector)
	>>> buf.set_internal_template(999)
	>>> for data in buf:
	... 	data = data.as_dict()
	...	for key,value in data.items()
	...	    print key + ":" + str(value) + '\n'

Examples::

	>>> buf = pyfixbuf.Buffer(my_rec)
	>>> buf.init_export(session, exporter)
	>>> buf.set_internal_template(999)
	>>> buf.set_external_template(999)
	>>> session.export_templates()
	>>> while count < 10:
	... 	  my_rec['sourceIPv4Address'] = "192.168.3.2"
	...	  my_rec['destinationIPv4Address'] = "192.168.4.5"
	...	  buf.append(my_rec)
        >>> buf.emit()

Examples::
	
	>> buf = pyfixbuf.Buffer(auto=True)
	>> buf.init_collection(session, collector)
	>> for data in buf:
	...    data = data.as_dict()
	...    for key,value in data.items()
	...        print key + ":" + str(value) + '\n'

STML
=================

A subTemplateMultiList is a list of zero or more instances of
a structured data record, where the data records do not necessarily
have to reference the same template.  A subTemplateMultiList is made
up of one or more :class:`STMLEntry`.  Each entry in the STML should
(but are not required) have a different template associated with it.  
The data in the STML is accessed by iterating through each :class:`STMLEntry` in the
list and setting a :class:`Record` on the :class:`STMLEntry`.

.. autoclass:: STML([record=None, key_name=None, type_count=-1])

   .. automethod:: clear()

   .. automethod:: __iter__() -> STML

   .. automethod:: next() -> STMLEntry

   .. method:: __len__() -> int

      Returns the number of entries in the subTemplateMultiList.

   .. automethod:: __contains__(name: str) -> bool

   .. automethod:: __getitem__(index: int) -> STMLEntry

   .. automethod:: __setitem__(key: int, value: list)

   .. attribute:: semantic

      The semantic value of the list of subTemplates.

Decode Examples::

     >>> stml = my_rec["subTemplateMultiList"]
     >>> for entry in stml:
     ... 	if "tcpSequenceNumber" in entry:
     ...           entry.set_record(tcprec)
     ...	   for tcp_record in entry:
     ...               tcp_record = tcp_record.as_dict()
     ...	       for key,value in tcp_record.items()
     ...	       	   print key + ": " + str(value) + '\n'

Encode Examples::
	
     >>> stml = STML(type_count=3)
     >>> stml.entry_init(rec, template, 2) #init first entry to 2 with template
     >>> rec["sourceTransportPort"] = 3
     >>> rec["destinationTransportPort"] = 5
     >>> stml[0][0] = rec
     >>> rec["sourceTransportPort"] = 6
     >>> rec["destinationTransportPort"] = 7
     >>> stml[0][1] = rec
     >>> stml[1][0] = rec2       #init second entry to 1 item using rec2
     >>> stml[2].entry_init(rec3, template3, 0) #init third entry to 0


STMLEntry
=====================

Each :class:`STML` consists of one or more :class:`STMLEntry`.  Each
:class:`STMLEntry` is associated with a template, and therefore should have
a corresponding :class:`Record`.  An :class:`STMLEntry`
can contain zero or more instances of the associated :class:`Record`.

.. autoclass:: STMLEntry(stml : STML)

   .. automethod:: entry_init(record : Record, template : Template, [count=0])

   .. automethod:: set_record(record : Record)

   .. automethod:: __contains__(name : str) -> bool
   
   .. automethod:: set_template(template: Template)

   .. automethod:: __iter__() -> STMLEntry
   
   .. automethod:: next() -> Record

   .. automethod:: __getitem__(item : str or int) -> Record

   .. automethod:: __setitem__(key : int, value : Record)

   .. method:: __len__() -> int

      The number of items in this entry.

   .. attribute:: template_id

      The Template ID of the template that corresponds to this entry in the list.

Examples::

	>>> stml = my_rec["subTemplateMultiList"]
	>>> for entry in stml:
	... 	if "tcpSequenceNumber" in entry:
	...	   entry.set_record(tcp_rec)
	...	   for tcp_record in entry:
	...	       tcp_record = tcp_record.as_dict()
	...	       for key,value in tcp_record.items():
	...	       	   print key + ": " + str(value) + '\n'
	...     elif entry.template_id == 0xCE00:
	...	   entry.set_record(dns_rec)
	...
	>>> stml.clear()


STL
===============

A subTemplateList is a list of zero or more instances of a
structured data type where each entry corresponds to a
single template.  Since a single template is associated
with an :class:`STL`, a :class:`Record` must also be associated with the
:class:`STL`.  Access each entry (a :class:`Record`) in the list by
iterating through the :class:`STL`.

.. autoclass:: STL([record=None, key_name=None])

   .. automethod:: set_record(record : Record)

   .. automethod:: __contains__(name : str) -> bool

   .. automethod:: entry_init(record : Record, template : Template[, count=0])

   .. automethod:: __iter__() -> STL

   .. automethod:: next() -> Record

   .. automethod:: clear()

   .. automethod:: __getitem__(item: int or str) -> Record

   .. automethod:: __setitem__(key: int, value: Record)

   .. method:: __len__()

       The number of entries in the subTemplateList.

   .. attribute:: template_id

	The template ID used for this subTemplateList.

   .. attribute:: semantic

       The semantic value for the subTemplateList.

Decoding Examples::

	>>> stl = rec["dnsList"]
	>>> stl.set_record(dnsRecord)
	>>> for dnsRecord in stl:
	... 	dnsRecord = dnsRecord.as_dict()
	...	for key,value in dnsRecord.items():
	...	    print key + ": " + str(value) + '\n'
	... stl.clear()

Encoding Examples::
	 
	 >>> stl = STL()
	 >>> stl.entry_init(dnsRecord, dnsTemplate, 2)
	 >>> dnsRecord["dnsQName"] = "google.com"
	 >>> dnsRecord["rrType"] = 1
	 >>> stl[0] = dnsRecord
	 >>> dnsRecord["dnsQName"] = "ns.google.com"
	 >>> dnsRecord["rrType"] = 2
	 >>> stl[1] = dnsRecord
	 >>> rec["subTemplateList"] = stl

BL
===============

A basicList is a list of zero or more instances of an Information Element. 
Examples include a list of port numbers, or a list of host names.
The BL object acts similar to a Python list with additional attributes.

.. autoclass:: BL(model : InfoModel, element : str, InfoElementSpec [, count=0, semantic=0])

   .. method:: __len__()

       The number of entries in the basicList.   

   .. automethod:: __iter__()

   .. automethod:: next()

   .. automethod:: __getitem__(index: int)

   .. automethod:: __setitem__(key: int, value: str, int)
   
   .. automethod:: copy(other : list)

   .. automethod:: __contains__(item : str, int) -> bool

   .. automethod:: __str__() -> str

   .. automethod:: __eq__(other : list) -> bool
   
   .. method:: clear()
   
      Clears and frees the basicList data.

   .. attribute:: semantic

       The semantic value for the basicList.

   .. attribute:: element
   
       The element associated with the basicList.  This returns an InfoElement.

Decoding Examples::

        >>> bl = rec["basicList"]
	>>> for items in bl:
	...    print str(items) + '\n'
	... bl.clear()

Encoding Examples::

         >>> bl = BL(model, "httpUserAgent", 2)
	 >>> bl[0] = "Mozilla/Firefox"
	 >>> bl[1] = "Safari5.0"
	 >>> rec["basicList"] = bl
	 >>> if "Safari5.0" in bl:
	 ...     print "Apple"
	 Apple
	 >>> print bl
	 ["Mozilla/Firefox", "Safari5.0"]
	 

Listener
====================

The Listener manages the passive collection used to listen
for connections from Exporting Processes.

.. autoclass:: Listener(session : Session, hostname : str[, transport="tcp", port=4739])

   .. automethod:: wait([record : Record])

Pre-defined Information Element Lists
======================================

pyfixbuf groups the YAF-defined Information Elements, CERT PEN 6871, by protocol.
YAF_LIST and YAF_STATS are necessary for collecting default input streams
from YAF.  Adding the following lists to the :class:`InfoModel` will result
in adding the following Information Elements to the :class:`InfoModel`.

YAF_LIST
----------

.. list-table::
   :header-rows: 1
   :widths: 60, 1, 20, 100

   * - Information Element
     - ID
     - TYPE
     - Description
   * - initialTCPFlags
     - 14
     - UINT8
     - Initial sequence number of the forward direction of the flow
   * - unionTCPFlags
     - 15
     - UINT8
     - Union of TCP flags of all packets other than the initial packet in the forward direction of the flow
   * - reverseFlowDeltaMilliseconds
     - 21
     - UINT32
     - Difference in time in milliseconds between first packet in forward direction and first packet in reverse direction
   * - silkAppLabel
     - 33
     - UINT16
     - Application label, defined as the primary well-known port associated with a given application.
   * - osName
     - 36
     - STRING
     - p0f OS Name for the forward flow based on the SYN packet and p0f SYN Fingerprints.
   * - payload
     - 36
     - OCTET ARRAY
     - Initial n bytes of forward direction of flow payload.
   * - osVersion
     - 37
     - STRING
     - p0f OS Version for the forward flow based on the SYN packet and p0f SYN Fingerprints.
   * - firstPacketBanner
     - 38
     - OCTET ARRAY
     - IP and transport headers for first packet in forward direction to be used for external OS Fingerprinters.
   * - secondPacketBanner
     - 39
     - OCTET ARRAY
     - IP and transport headers for first packet in forward direction to be used for external OS Fingerprinters.
   * - flowAttributes
     - 40
     - UINT16
     - Miscellaneous flow attributes for the forward direction of the flow
   * - osFingerPrint
     - 107
     - STRING
     - p0f OS Fingerprint for the forward flow based on the SYN packet and p0f SYN fingerprints.
   * - yafFlowKeyHash
     - 106
     - UINT32
     - The 32 bit hash of the 5-tuple and VLAN that is used as they key to YAF's internal flow table.

YAF_STATS_LIST
--------------

.. list-table::
   :header-rows: 1
   :widths: 50, 1, 20, 100

   * - Information Element
     - ID
     - TYPE
     - Description
   * - expiredFragmentCount
     - 100
     - UINT32
     - Total amount of fragments that have been expired since yaf start time.
   * - assembledFragmentCount
     - 101
     - UINT32
     - Total number of packets that been assembled from a series of fragments since yaf start time.
   * - meanFlowRate
     - 102
     - UINT32
     - The mean flow rate of the yaf flow sensor since yaf start time, rounded to the nearest integer.
   * - meanPacketRate
     - 103
     - UINT32
     - The mean packet rate of the yaf flow sensor since yaf start time, rounded to the nearest integer.
   * - flowTableFlushEventCount
     - 104
     - UINT32
     - Total number of times the yaf flow table has been flushed since yaf start time.
   * - flowTablePeakCount
     - 105
     - UINT32
     - The maximum number of flows in the yaf flow table at any one time since yaf start time.

YAF_FLOW_STATS_LIST
--------------------

.. list-table::
   :header-rows: 1
   :widths: 50, 1, 20, 100

   * - Information Element
     - ID
     - TYPE
     - Description
   * - smallPacketCount
     - 500
     - UINT32
     - The number of packets that contain less than 60 bytes of payload.
   * - nonEmptyPacketCount
     - 501
     - UINT32
     - The number of packets that contain at least 1 byte of payload.
   * - dataByteCount
     - 502
     - UINT64
     - Total bytes transferred as payload.
   * - averageInterarrivalTime
     - 503
     - UINT64
     - Average number of milliseconds between packets.
   * - standardDeviationInterarrivalTime
     - 504
     - UINT64
     - Standard deviation of the interarrival time for up to the first ten packets.
   * - firstNonEmptyPacketSize
     - 505
     - UINT16
     - Payload length of the first non-empty packet.
   * - maxPacketSize
     - 506
     - UINT16
     - The largest payload length transferred in the flow.
   * - firstEightNonEmptyPacketDirections
     - 507
     - UINT8
     - Represents directionality for the first 8 non-empty packets. 0 for forward direction, 1 for reverse direction.
   * - standardDeviationPayloadLength
     - 508
     - UINT16
     - The standard deviation of the payload length for up to the first 10 non empty packets.
   * - tcpUrgCount
     - 509
     - UINT32
     - The number of TCP packets that have the URGENT Flag set.
   * - largePacketCount
     - 510
     - UINT32
     - The number of packets that contain at least 220 bytes of payload.


YAF_HTTP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - httpServerString
     - 110
     - STRING
   * - httpUserAgent
     - 111
     - STRING
   * - httpGet
     - 112
     - STRING
   * - httpConnection
     - 113
     - STRING
   * - httpVersion
     - 114
     - STRING
   * - httpReferer
     - 115
     - STRING
   * - httpLocation
     - 116
     - STRING
   * - httpHost
     - 117
     - STRING
   * - httpContentLength
     - 118
     - STRING
   * - httpAge
     - 119
     - STRING
   * - httpAccept
     - 120
     - STRING
   * - httpAcceptLanguage
     - 121
     - STRING
   * - httpContentType
     - 122
     - STRING
   * - httpResponse
     - 123
     - STRING
   * - httpCookie
     - 220
     - STRING
   * - httpSetCookie
     - 221
     - STRING
   * - httpAuthorization
     - 252
     - STRING
   * - httpVia
     - 253
     - STRING
   * - httpX-Forwarded-For
     - 254
     - STRING
   * - httpRefresh
     - 256
     - STRING
   * - httpIMEI
     - 257
     - STRING
   * - httpIMSI
     - 258
     - STRING
   * - httpMSISDN
     - 259
     - STRING
   * - httpSubscriber
     - 260
     - STRING
   * - httpExpires
     - 255
     - STRING
   * - httpAcceptCharset
     - 261
     - STRING
   * - httpAcceptEncoding
     - 262
     - STRING
   * - httpAllow
     - 263
     - STRING
   * - httpDate
     - 264
     - STRING
   * - httpExpect
     - 265
     - STRING
   * - httpFrom
     - 266
     - STRING
   * - httpProxyAuthentication
     - 267
     - STRING
   * - httpUpgrade
     - 268
     - STRING
   * - httpWarning
     - 269
     - STRING
   * - httpDNT
     - 270
     - STRING
   * - httpX-Forwarded-Proto
     - 271
     - STRING
   * - httpX-Forwarded-Host
     - 272
     - STRING
   * - httpX-Forwarded-Server
     - 273
     - STRING
   * - httpX-DeviceID
     - 274
     - STRING
   * - httpX-Profile
     - 275
     - STRING
   * - httpLastModified
     - 276
     - STRING
   * - httpContentEncoding
     - 277
     - STRING
   * - httpContentLanguage
     - 278
     - STRING
   * - httpContentLocation
     - 279
     - STRING
   * - httpX-UA-Compatible
     - 280
     - STRING

YAF_SLP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - slpVersion
     - 128
     - UINTE8
   * - slpMessageType
     - 129
     - UINT8
   * - slpString
     - 130
     - STRING

YAF_FTP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - ftpReturn
     - 131
     - STRING
   * - ftpUser
     - 132
     - STRING
   * - ftpPass
     - 133
     - STRING
   * - ftpType
     - 134
     - STRING
   * - ftpRespCode
     - 135
     - STRING

YAF_IMAP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - imapCapability
     - 136
     - STRING
   * - imapLogin
     - 137
     - STRING
   * - imapStartTLS
     - 138
     - STRING
   * - imapAuthenticate
     - 139
     - STRING
   * - imapCommand
     - 140
     - STRING
   * - imapExists
     - 141
     - STRING
   * - imapRecent
     - 142
     - STRING

YAF_RTSP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - rtspURL
     - 143
     - STRING
   * - rtspVersion
     - 144
     - STRING
   * - rtspReturnCode
     - 145
     - STRING
   * - rtspContentLength
     - 146
     - STRING
   * - rtspCommand
     - 147
     - STRING
   * - rtspContentType
     - 148
     - STRING
   * - rtspTransport
     - 149
     - STRING
   * - rtspCSeq
     - 150
     - STRING
   * - rtspLocation
     - 151
     - STRING
   * - rtspPacketsReceived
     - 152
     - STRING
   * - rtspUserAgent
     - 153
     - STRING
   * - rtspJitter
     - 154
     - STRING

YAF_SIP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - sipInvite
     - 155
     - STRING
   * - sipCommand
     - 156
     - STRING
   * - sipVia
     - 157
     - STRING
   * - sipMaxForwards
     - 158
     - STRING
   * - sipAddress
     - 159
     - STRING
   * - sipContentLength
     - 160
     - STRING
   * - sipUserAgent
     - 161
     - STRING


YAF_SMTP_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - smtpHello
     - 162
     - STRING
   * - smtpFrom
     - 163
     - STRING
   * - smtpTo
     - 164
     - STRING
   * - smtpContentType
     - 165
     - STRING
   * - smtpSubject
     - 166
     - STRING
   * - smtpFilename
     - 167
     - STRING
   * - smtpContentDisposition
     - 168
     - STRING
   * - smtpResponse
     - 169
     - STRING
   * - smtpEnhanced
     - 170
     - STRING
   * - smtpSize
     - 222
     - STRING
   * - smtpDate
     - 251
     - STRING


YAF_DNS_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - dnsQueryResponse
     - 174
     - UINT8
   * - dnsQRType
     - 175
     - UINT16
   * - dnsAuthoritative
     - 176
     - UINT8
   * - dnsNXDomain
     - 177
     - UINT8
   * - dnsRRSection
     - 178
     - UINT8
   * - dnsQName
     - 179
     - STRING
   * - dnsCName
     - 180
     - STRING
   * - dnsMXPreference
     - 181
     - UINT16
   * - dnsMXExchange
     - 182
     - STRING
   * - dnsNSDName
     - 183
     - STRING
   * - dnsPTRDName
     - 184
     - STRING
   * - dnsTTL
     - 199
     - UINT32
   * - dnsTXTData
     - 208
     - STRING
   * - dnsSOASerial
     - 209
     - UINT32
   * - dnsSOARefresh
     - 210
     - UINT32
   * - dnsSOARetry
     - 211
     - UINT32
   * - dnsSOAExpire
     - 212
     - UINT32
   * - dnsSOAMinimum
     - 213
     - UINT32
   * - dnsSOAMName
     - 214
     - STRING
   * - dnsSOARName
     - 215
     - STRING
   * - dnsSRVPriority
     - 216
     - UINT16
   * - dnsSRVWeight
     - 217
     - UINT16
   * - dnsSRVPort
     - 218
     - UINT16
   * - dnsSRVTarget
     - 219
     - STRING
   * - dnsID
     - 226
     - UINT16
   * - dnsAlgorithm
     - 227
     - UINT8
   * - dnsKeyTag
     - 228
     - UINT16
   * - dnsSigner
     - 229
     - STRING
   * - dnsSignature
     - 230
     - OCTET ARRAY
   * - dnsDigest
     - 231
     - OCTET ARRAY
   * - dnsPublicKey
     - 232
     - OCTET ARRAY
   * - dnsSalt
     - 233
     - OCTET ARRAY
   * - dnsHashData
     - 234
     - OCTET ARRAY
   * - dnsIterations
     - 235
     - UINT16
   * - dnsSignatureExpiration
     - 236
     - UINT32
   * - dnsSignatureInception
     - 237
     - UINT32
   * - dnsDigestType
     - 238
     - UINT8
   * - dnsLabels
     - 239
     - UINT8
   * - dnsTypeCovered
     - 240
     - UINT16
   * - dnsFlags
     - 241
     - UINT16

YAF_SSL_LIST
--------------

Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - sslCipher
     - 185
     - UINT32
   * - sslClientVersion
     - 186
     - UINT8
   * - sslServerCipher
     - 187
     - UINT32
   * - sslCompressionMethod
     - 188
     - UINT8
   * - sslCertVersion
     - 189
     - UINT8
   * - sslCertSignature
     - 190
     - STRING
   * - sslCertIssuerCountryName
     - 191
     - STRING
   * - sslCertIssuerOrgName
     - 192
     - STRING
   * - sslCertIssuerOrgUnitName
     - 193
     - STRING
   * - sslCertIssuerZipCode
     - 194
     - STRING
   * - sslCertIssuerState
     - 195
     - STRING
   * - sslCertIssuerCommonName
     - 196
     - STRING
   * - sslCertIssuerLocalityName
     - 197
     - STRING
   * - sslCertIssuerStreetAddress
     - 198
     - STRING
   * - sslCertSubCountryName
     - 200
     - STRING
   * - sslCertSubOrgName
     - 201
     - STRING
   * - sslCertSubOrgUnitName
     - 202
     - STRING
   * - sslCertSubZipCode
     - 203
     - STRING
   * - sslCertSubState
     - 204
     - STRING
   * - sslCertSubCommonName
     - 205
     - STRING
   * - sslCertSubLocalityName
     - 206
     - STRING
   * - sslCertSubStreetAddress
     - 207
     - STRING
   * - sslCertSerialNumber
     - 208
     - STRING
   * - sslObjectType
     - 245
     - UINT8
   * - sslObjectValue
     - 246
     - STRING
   * - sslCertValidityNotBefore
     - 247
     - STRING
   * - sslCertValidityNotAfter
     - 248
     - STRING
   * - sslCertPublicKeyAlgorithm
     - 249
     - STRING
   * - sslCertPublicKeyLength
     - 250
     - UINT16
   * - sslRecordVersion
     - 288
     - UINT16

YAF_DPI_LIST
--------------

This list contains miscellaneous Information Elements from the remaining protocols YAF decodes.  Descriptions of each Information Element can be found at http://tools.netsa.cert.org/yaf/yafdpi.html.

.. list-table::
   :header-rows: 1
   :widths: 50, 20, 40

   * - Information Element
     - ID
     - TYPE
   * - mysqlUsername
     - 223
     - STRING
   * - mysqlCommandCode
     - 224
     - UINT8
   * - mysqlCommandText
     - 225
     - STRING
   * - pop3TextMessage
     - 124
     - STRING
   * - ircTextMessage
     - 125
     - STRING
   * - tftpFilename
     - 126
     - STRING
   * - tftpMode
     - 127
     - STRING
   * - dhcpFingerPrint
     - 242
     - STRING
   * - dhcpVendorCode
     - 243
     - STRING
   * - dnp3SourceAddress
     - 281
     - UINT16
   * - dnp3DestinationAddress
     - 282
     - UINT16
   * - dnp3Function
     - 283
     - UINT8
   * - dnp3ObjectData
     - 284
     - OCTET_ARRAY
   * - modbusData
     - 285
     - OCTET_ARRAY
   * - ethernetIPData
     - 286
     - OCTET_ARRAY
   * - rtpPayloadType
     - 287
     - UINT8




