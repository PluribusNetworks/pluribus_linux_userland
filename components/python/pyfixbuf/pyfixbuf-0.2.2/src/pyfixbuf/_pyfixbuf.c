/**
 ** _pyfixbuf.c
 ** ------------------------------------------------------------------------
 ** Copyright (C) 2006-2015 Carnegie Mellon University. All Rights Reserved.
 ** ------------------------------------------------------------------------
 ** Authors: Brian Trammell, Dan Ruef, Emily Ecoff
 ** ------------------------------------------------------------------------
 ** @OPENSOURCE_HEADER_START@
 ** Use of the libfixbuf system and related source code is subject to the terms
 ** of the following licenses:
 **
 ** GNU Lesser GPL (LGPL) Rights pursuant to Version 2.1, February 1999
 ** Government Purpose License Rights (GPLR) pursuant to DFARS 252.227.7013
 **
 ** NO WARRANTY
 **
 ** ANY INFORMATION, MATERIALS, SERVICES, INTELLECTUAL PROPERTY OR OTHER
 ** PROPERTY OR RIGHTS GRANTED OR PROVIDED BY CARNEGIE MELLON UNIVERSITY
 ** PURSUANT TO THIS LICENSE (HEREINAFTER THE "DELIVERABLES") ARE ON AN
 ** "AS-IS" BASIS. CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED AS TO ANY MATTER INCLUDING, BUT NOT
 ** LIMITED TO, WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE,
 ** MERCHANTABILITY, INFORMATIONAL CONTENT, NONINFRINGEMENT, OR ERROR-FREE
 ** OPERATION. CARNEGIE MELLON UNIVERSITY SHALL NOT BE LIABLE FOR INDIRECT,
 ** SPECIAL OR CONSEQUENTIAL DAMAGES, SUCH AS LOSS OF PROFITS OR INABILITY
 ** TO USE SAID INTELLECTUAL PROPERTY, UNDER THIS LICENSE, REGARDLESS OF
 ** WHETHER SUCH PARTY WAS AWARE OF THE POSSIBILITY OF SUCH DAMAGES.
 ** LICENSEE AGREES THAT IT WILL NOT MAKE ANY WARRANTY ON BEHALF OF
 ** CARNEGIE MELLON UNIVERSITY, EXPRESS OR IMPLIED, TO ANY PERSON
 ** CONCERNING THE APPLICATION OF OR THE RESULTS TO BE OBTAINED WITH THE
 ** DELIVERABLES UNDER THIS LICENSE.
 **
 ** Licensee hereby agrees to defend, indemnify, and hold harmless Carnegie
 ** Mellon University, its trustees, officers, employees, and agents from
 ** all claims or demands made against them (and any related losses,
 ** expenses, or attorney's fees) arising out of, or relating to Licensee's
 ** and/or its sub licensees' negligent use or willful misuse of or
 ** negligent conduct or willful misconduct regarding the Software,
 ** facilities, or other rights or assistance granted by Carnegie Mellon
 ** University under this License, including, but not limited to, any
 ** claims of product liability, personal injury, death, damage to
 ** property, or violation of any laws or regulations.
 **
 ** Carnegie Mellon University Software Engineering Institute authored
 ** documents are sponsored by the U.S. Department of Defense under
 ** Contract FA8721-05-C-0003. Carnegie Mellon University retains
 ** copyrights in all material produced under this contract. The U.S.
 ** Government retains a non-exclusive, royalty-free license to publish or
 ** reproduce these documents, or allow others to do so, for U.S.
 ** Government purposes only pursuant to the copyright license under the
 ** contract clause at 252.227.7013.
 ** Government purposes only pursuant to the copyright license under the
 ** contract clause at 252.227.7013.
 **
 ** @OPENSOURCE_HEADER_END@
 ** ------------------------------------------------------------------------
 */
#include <Python.h>

#include <stdint.h>
#include <fixbuf/public.h>
#include <glib.h>

#define GET_GLOBALS(m) (&fixbufpy_globals)
#define GLOBALS GET_GLOBALS(PyState_FindModule(&fixbufpy_globals))
#define MAX_NAME 200
#define MAX_ELEMENTS 100
#define IPV6_TYPE 6
#define IS_STRING(o) (PyUnicode_Check(o) || PyString_Check(o) || fixbufPyVarfield_Check(o))
#define IS_INT(o) (PyInt_Check(o) || PyLong_Check(o))
#define IS_BYTE(o) (PyByteArray_Check(o))

#ifndef Py_TYPE
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

#  define LONG_AS_UNSIGNED_LONGLONG(o)          \
    (PyLong_Check(o) ?                          \
     PyLong_AsUnsignedLongLong(o) :             \
     PyLong_AsUnsignedLong(o))

typedef struct fixbufPyInfoModel_st {
    PyObject_HEAD
    fbInfoModel_t *infoModel;
} fixbufPyInfoModel;

static PyTypeObject fixbufPyInfoModelType;

#define fixbufPyInfoModel_Check(op) PyObject_TypeCheck(op, &fixbufPyInfoModelType)

typedef struct fixbufPyInfoElement_st {
    PyObject_HEAD
    fbInfoElement_t *infoElement;
    const fbInfoElement_t *ptr;
    char infoElementName[MAX_NAME];
    char description[500];
} fixbufPyInfoElement;

static PyTypeObject fixbufPyInfoElementType;

#define fixbufPyInfoElement_Check(op) PyObject_TypeCheck(op, &fixbufPyInfoElementType)

typedef struct fixbufPyfBuf_st {
    PyObject_HEAD
    fBuf_t *fbuf;
    int ignore;
} fixbufPyfBuf;

static PyTypeObject fixbufPyfBufType;

#define fixbufPyfBuf_Check(op) PyObject_TypeCheck(op, &fixbufPyfBufType)

typedef struct fixbufPySession_st {
    PyObject_HEAD
    fbSession_t *session;
} fixbufPySession;

static PyTypeObject fixbufPySessionType;

#define fixbufPySession_Check(op) PyObject_TypeCheck(op, &fixbufPySessionType)

typedef struct fixbufPyExporter_st {
    PyObject_HEAD
    fbExporter_t *exporter;
} fixbufPyExporter;

static PyTypeObject fixbufPyExporterType;

#define fixbufPyExporter_Check(op) PyObject_TypeCheck(op, &fixbufPyExporterType)

typedef struct fixbufPyCollector_st {
    PyObject_HEAD
    fbCollector_t *collector;
} fixbufPyCollector;

static PyTypeObject fixbufPyCollectorType;

#define fixbufPyCollector_Check(op) PyObject_TypeCheck(op, &fixbufPyCollectorType)

typedef struct fixbufPyTemplate_st {
    PyObject_HEAD
    fbTemplate_t *template;
    uint16_t template_id;
} fixbufPyTemplate;

static PyTypeObject fixbufPyTemplateType;

#define fixbufPyTemplate_Check(op) PyObject_TypeCheck(op, &fixbufPyTemplateType)

typedef struct fixbufPyInfoElementSpec_st {
    PyObject_HEAD
    fbInfoElementSpec_t *spec;
    char infoElementName[MAX_NAME];
} fixbufPyInfoElementSpec;

static PyTypeObject fixbufPyInfoElementSpecType;

#define fixbufPyInfoElementSpec_Check(op) PyObject_TypeCheck(op, &fixbufPyInfoElementSpecType)

typedef struct fixbufPyRecord_st {
    PyObject_HEAD
    uint8_t *rec;
    size_t reclen;
    gboolean memalloc;
} fixbufPyRecord;

static PyTypeObject fixbufPyRecordType;

#define fixbufPyRecord_Check(op) PyObject_TypeCheck(op, &fixbufPyRecordType)

typedef struct fixbufPySTML_st {
    PyObject_HEAD
    fbSubTemplateMultiList_t *stml;
    fbSubTemplateMultiListEntry_t *entry;
    gboolean stml_alloc;
} fixbufPySTML;

static PyTypeObject fixbufPySTMLType;

#define fixbufPySTML_Check(op) PyObject_TypeCheck(op, &fixbufPySTMLType)

typedef struct fixbufPySTMLEntry_st {
    PyObject_HEAD
    fbSubTemplateMultiListEntry_t *entry;
} fixbufPySTMLEntry;

static PyTypeObject fixbufPySTMLEntryType;

#define fixbufPySTMLEntry_Check(op) PyObject_TypeCheck(op, &fixbufPySTMLEntryType)

typedef struct fixbufPySTL_st {
    PyObject_HEAD
    fbSubTemplateList_t *stl;
    gboolean stl_alloc;
} fixbufPySTL;

static PyTypeObject fixbufPySTLType;

#define fixbufPySTL_Check(op) PyObject_TypeCheck(op, &fixbufPySTLType)

typedef struct fixbufPyVarfield_st {
    PyObject_HEAD
    fbVarfield_t *varfield;
} fixbufPyVarfield;

static PyTypeObject fixbufPyVarfieldType;

#define fixbufPyVarfield_Check(op) PyObject_TypeCheck(op, &fixbufPyVarfieldType)


typedef struct fixbufPyListener_st {
    PyObject_HEAD
    fbConnSpec_t conn;
    fbListener_t *listener;
} fixbufPyListener;

static PyTypeObject fixbufPyListenerType;

#define fixbufPyListener_Check(op) PyObject_TypeCheck(op, &fixbufPyListenerType)

typedef struct fixbufPyBL_st {
    PyObject_HEAD
    fbBasicList_t *bl;
    gboolean bl_alloc;
    gboolean init;
} fixbufPyBL;

static PyTypeObject fixbufPyBLType;

#define fixbufPyBL_Check(op) PyObject_TypeCheck(op, &fixbufPyBLType)

static PyObject *ignoreList = NULL;

static void obj_dealloc(
    PyObject   *obj)
{
    Py_TYPE(obj)->tp_free(obj);
}

/**
 * fbInfoElement
 *
 **/

static PyObject *fixbufPyInfoElement_new(
    PyTypeObject *type,
    PyObject     *args,
    PyObject     *kwds)
{
    fixbufPyInfoElement *self;

    self = (fixbufPyInfoElement *)type->tp_alloc(type, 0);

    if (self != NULL) {
        self->infoElement = PyMem_Malloc(sizeof(fbInfoElement_t));
        if (self->infoElement == NULL) {
            Py_XDECREF((PyObject *)self);
            return PyErr_NoMemory();
        }
        memset(self->infoElement, 0, sizeof(fbInfoElement_t));
    }

    return (PyObject *)self;
}

static void fixbufPyInfoElement_dealloc(
    fixbufPyInfoElement *obj)
{

    if (obj->infoElement) {
        PyMem_Free(obj->infoElement);
    }
    Py_TYPE(obj)->tp_free((PyObject *)obj);
}

static int fixbufPyInfoElement_init(
    fixbufPyInfoElement *self,
    PyObject            *args,
    PyObject            *kwds)
{

    static char *kwlist[] = {"name", "enterprise_number", "id", "length",
                             "reversible", "endian", "type", "min", "max",
                             "units", "semantic", "description", NULL};

    char  *name;
    char  *description = NULL;
    int   ent;
    /* id */
    int   num;
    int   len = 0;
    int   reversible = 0;
    int   endian = 0;
    int   fb_flags = 0;
    long  min = 0;
    long  max = 0;
    int   type = 0;
    int   units = 0;
    int   semantic = 0;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sii|iiiilliis", kwlist,
                                     &name,
                                     &ent, &num, &len, &reversible, &endian,
                                     &type, &min, &max, &units, &semantic,
                                     &description))
    {
        return -1;
    }

    if (self->infoElement == NULL) {
        return -1;
    }

    if (reversible) {
        fb_flags |= FB_IE_F_REVERSIBLE;
    }

    if (endian) {
        fb_flags |= FB_IE_F_ENDIAN;
    }

    if (len == 0) {
        /* if length is not given, but type is given, use length for that
           type */
        if (type > 0) {
            switch (type) {
              case FB_UINT_8:
              case FB_INT_8:
              case FB_BOOL:
                len = 1;
                break;
              case FB_UINT_16:
              case FB_INT_16:
                len = 2;
                break;
              case FB_UINT_32:
              case FB_INT_32:
              case FB_FLOAT_32:
              case FB_DT_SEC:
              case FB_IP4_ADDR:
                len = 4;
                break;
              case FB_MAC_ADDR:
                len = 6;
                break;
              case FB_UINT_64:
              case FB_INT_64:
              case FB_FLOAT_64:
              case FB_DT_MILSEC:
              case FB_DT_MICROSEC:
              case FB_DT_NANOSEC:
                len = 8;
                break;
              case FB_IP6_ADDR:
                len = 16;
                break;
              default:
                /* lists and string */
                len = 65535;
            }
        } else {
            len = 65535;
        }
    }


    switch (semantic) {
      case 0:
        break;
      case 1:
        fb_flags |= FB_IE_QUANTITY;
        break;
      case 2:
        fb_flags |= FB_IE_TOTALCOUNTER;
        break;
      case 3:
        fb_flags |= FB_IE_DELTACOUNTER;
        break;
      case 4:
        fb_flags |= FB_IE_IDENTIFIER;
        break;
      case 5:
        fb_flags |= FB_IE_FLAGS;
        break;
      case 6:
        fb_flags |= FB_IE_LIST;
        break;
      default:
        fprintf(stderr, "Invalid Semantic Value.\n");
        break;
    }

    switch (units) {
      case 0:
        break;
      case 1:
        fb_flags |= FB_UNITS_BITS;
        break;
      case 2:
        fb_flags |= FB_UNITS_OCTETS;
        break;
      case 3:
        fb_flags |= FB_UNITS_PACKETS;
        break;
      case 4:
        fb_flags |= FB_UNITS_FLOWS;
        break;
      case 5:
        fb_flags |= FB_UNITS_SECONDS;
        break;
      case 6:
        fb_flags |= FB_UNITS_MILLISECONDS;
        break;
      case 7:
        fb_flags |= FB_UNITS_MICROSECONDS;
        break;
      case 8:
        fb_flags |= FB_UNITS_NANOSECONDS;
        break;
      case 9:
        fb_flags |= FB_UNITS_WORDS;
        break;
      case 10:
        fb_flags |= FB_UNITS_MESSAGES;
        break;
      case 11:
        fb_flags |= FB_UNITS_HOPS;
        break;
      case 12:
        fb_flags |= FB_UNITS_ENTRIES;
        break;
      default:
        fprintf(stderr, "Invalid UNITS.\n");
        break;
    }


    if (strlen(name) >= MAX_NAME) {
        name[MAX_NAME-1]='0';
    }
    strcpy(self->infoElementName, name);
    self->infoElement->ref.name = self->infoElementName;
    self->infoElement->midx = 0;
    self->infoElement->ent = ent;
    self->infoElement->num = num;
    self->infoElement->len = len;
    self->infoElement->flags = fb_flags;
    self->infoElement->type = type;
    self->infoElement->min = min;
    self->infoElement->max = max;

    if (description) {
        int desc_len = strlen(description);
        if (desc_len > 499) {
            desc_len = 499;
        }
        strncpy(self->description, description, desc_len);
        /* make sure this is null terminated */
        self->description[desc_len]='\0';
        self->infoElement->description = self->description;
    }

    /* need to add description */

    return 0;
}

static PyObject *fixbufPyInfoElement_getlength(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->len);
}

static PyObject *fixbufPyInfoElement_getname(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    char name[MAX_NAME];

    if (obj->infoElement->ref.name) {
        memcpy(name, obj->infoElement->ref.name, strlen(obj->infoElement->ref.name));
        return PyString_FromStringAndSize(name, strlen(obj->infoElement->ref.name));
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }

}

static PyObject *fixbufPyInfoElement_gettemplatename(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    char name[MAX_NAME];

    if (obj->infoElement->ref.canon->ref.name) {
        memcpy(name, obj->infoElement->ref.canon->ref.name,
               strlen(obj->infoElement->ref.canon->ref.name));
        return PyString_FromStringAndSize(name,
                                       strlen(obj->infoElement->ref.canon->ref.name));
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }

}

static PyObject *fixbufPyInfoElement_getid(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->num);
}

static PyObject *fixbufPyInfoElement_getdescription(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    if (obj->infoElement->description) {
        return PyString_FromStringAndSize(obj->infoElement->description, strlen(obj->infoElement->description));
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject *fixbufPyInfoElement_getent(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->ent);
}

static PyObject*fixbufPyInfoElement_gettype(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->type);
}

static PyObject*fixbufPyInfoElement_getmin(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->min);
}

static PyObject*fixbufPyInfoElement_getmax(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->infoElement->max);
}

static PyObject *fixbufPyInfoElement_getunits(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(FB_IE_UNITS(obj->infoElement->flags));
}

static PyObject *fixbufPyInfoElement_getsemantic(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    return PyInt_FromLong(FB_IE_SEMANTIC(obj->infoElement->flags));
}

static PyObject *fixbufPyInfoElement_getreversible(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    if (obj->infoElement->flags & FB_IE_F_REVERSIBLE) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject *fixbufPyInfoElement_getendian(
    fixbufPyInfoElement *obj,
    void *cbdata)
{
    if (obj->infoElement->flags & FB_IE_F_ENDIAN) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}
/**
 * InfoElement GetSetters
 *
 *
 */

static PyGetSetDef fixbufPyInfoElement_getsetters[] = {
    {"length", (getter)fixbufPyInfoElement_getlength, NULL,
     "Info Element Length", NULL},
    {"id", (getter)fixbufPyInfoElement_getid, NULL,
     "Info Element ID", NULL},
    {"name", (getter)fixbufPyInfoElement_getname, NULL,
     "Info Element Name", NULL},
    {"tname", (getter)fixbufPyInfoElement_gettemplatename, NULL,
     "Info Element Name when accessing IE from Template.", NULL},
    {"ent", (getter)fixbufPyInfoElement_getent, NULL,
     "Info Element Enterprise Number", NULL},
    {"type", (getter)fixbufPyInfoElement_gettype, NULL,
     "Info Element Data Type", NULL},
    {"min", (getter)fixbufPyInfoElement_getmin, NULL,
     "Info Element Minimum Value", NULL},
    {"max", (getter)fixbufPyInfoElement_getmax, NULL,
     "Info Element Maximum Value", NULL},
    {"semantic", (getter)fixbufPyInfoElement_getsemantic, NULL,
     "Info Element Semantic Value", NULL},
    {"units", (getter)fixbufPyInfoElement_getunits, NULL,
     "Info Element Units", NULL},
    {"description", (getter)fixbufPyInfoElement_getdescription, NULL,
     "Info Element Human-Readable Description", NULL},
    {"reversible", (getter)fixbufPyInfoElement_getreversible, NULL,
     "Info Element Reversible Flag True/False", NULL},
    {"endian", (getter)fixbufPyInfoElement_getendian, NULL,
     "Info Element Endian Flag True/False", NULL},
   {NULL, NULL, NULL, NULL, NULL}
};

static PyTypeObject fixbufPyInfoElementType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /*ob_size */
    "pyfixbuf.fbInfoElementBase", /*tp_name*/
    sizeof(fixbufPyInfoElement),  /*tp_basicsize*/
    0,                            /*tp_itemsize*/
    (destructor)fixbufPyInfoElement_dealloc, /*tp_dealloc*/
    0,                            /*tp_print*/
    0,                            /*tp_getattr*/
    0,                            /*tp_setattr*/
    0,                            /*tp_compare*/
    0,                            /*tp_repr*/
    0,                            /*tp_as_number*/
    0,                            /*tp_as_sequence*/
    0,                            /*tp_as_mapping*/
    0,                            /*tp_hash */
    0,                            /*tp_call*/
    0,                            /*tp_str*/
    0,                            /*tp_getattro*/
    0,                            /*tp_setattro*/
    0,                            /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbInfoElement", /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    fixbufPyInfoElement_getsetters, /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    (initproc)fixbufPyInfoElement_init,   /* tp_init */
    0,                            /* tp_alloc */
    fixbufPyInfoElement_new,     /* tp_new */
    0                             /* tp_free */
};

/**
 *
 * fbInfoModel
 *
 **/

static PyObject *fixbufPyInfoModelAddElement(
    fixbufPyInfoModel   *self,
    PyObject *args)

{
    fixbufPyInfoElement *ie;

    if (!PyArg_ParseTuple(args, "O!", &fixbufPyInfoElementType, &ie)) {
        return NULL;
    }

    if (!fixbufPyInfoElement_Check(ie)) {
        PyErr_SetString(PyExc_ValueError, "Expected pyfixbuf.fbInfoElement");
        return NULL;
    }

    fbInfoModelAddElement(self->infoModel, ie->infoElement);

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPyInfoModelGetElementLength(
    fixbufPyInfoModel *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"name", "type", NULL};
    char *name = NULL;
    const fbInfoElement_t *ie;
    uint16_t len = 0;
    long type = 0;
    uint16_t num;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|l", kwlist, &name, &type)) {
        return NULL;
    }

    if (name == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Need a valid info element name");
        return NULL;
    }

    if (type == FB_BASIC_LIST) {
        len = sizeof(fbBasicList_t);
    } else if (type == FB_SUB_TMPL_LIST) {
        len = sizeof(fbSubTemplateList_t);
    } else if (type == FB_SUB_TMPL_MULTI_LIST) {
        len = sizeof(fbSubTemplateMultiList_t);
    } else {

        ie = fbInfoModelGetElementByName(self->infoModel,
                                         name);
        if (ie == NULL) {
            PyErr_Format(PyExc_StandardError,
                            "Information Element %s does not exist\n",
                            name);
            return NULL;
        }

        len = ie->len;
        num = ie->num;

        /* return the length of the element */
        if (len == FB_IE_VARLEN) {
            len = sizeof(fbVarfield_t);
            if (num == FB_IE_BASIC_LIST) {
                len = sizeof(fbBasicList_t);
            } else if (num == FB_IE_SUBTEMPLATE_LIST) {
                len = sizeof(fbSubTemplateList_t);
            } else if (num == FB_IE_SUBTEMPLATE_MULTILIST) {
                len = sizeof(fbSubTemplateMultiList_t);
            }
        }
    }

    return PyInt_FromLong(len);
}

static PyObject *fixbufPyInfoModelIsVarfield(
    fixbufPyInfoModel *self,
    PyObject *args)
{
    char *element_name = NULL;
    const fbInfoElement_t *ie;
    int len;

    if (!PyArg_ParseTuple(args, "s", &element_name)) {
        PyErr_SetString(PyExc_AttributeError, "element name");
        return NULL;
    }

    ie = fbInfoModelGetElementByName(self->infoModel,
                                     (const char *)element_name);
    if (ie == NULL) {
        PyErr_SetString(PyExc_StandardError, "Information Element does not Exist");
        return NULL;
    }

    len = ie->len;

    if (len == FB_IE_VARLEN) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}


static PyObject *fixbufPyInfoModelGetElementType(
    fixbufPyInfoModel *self,
    PyObject *args)
{
    char *element_name = NULL;
    const fbInfoElement_t *ie;
    uint16_t len;
    uint16_t num;
    uint16_t type = 0;

    if (!PyArg_ParseTuple(args, "s", &element_name)) {
        PyErr_SetString(PyExc_AttributeError, "element name");
        return NULL;
    }

    ie = fbInfoModelGetElementByName(self->infoModel,
                                     element_name);
    if (!ie) {
        PyErr_Format(PyExc_StandardError,
                     "Information Element %s Does Not Exist.",
                     element_name);
        return NULL;
    }
    num = ie->num;
    type = ie->type;
    len = ie->len;

    /* in case it's not defined */
    if (len == FB_IE_VARLEN) {
        /* this has to be VARLEN so that we know to return variable length
           fields */
        type = FB_IE_VARLEN;
        if (num == FB_IE_BASIC_LIST) {
            type = FB_BASIC_LIST;
        } else if (num == FB_IE_SUBTEMPLATE_LIST) {
            type = FB_SUB_TMPL_LIST;
        } else if (num == FB_IE_SUBTEMPLATE_MULTILIST) {
            type = FB_SUB_TMPL_MULTI_LIST;
        }
    }

    return PyInt_FromLong(type);
}

static PyObject *fixbufPyInfoModelGetElementTrueType(
    fixbufPyInfoModel *self,
    PyObject *args)
{
    char *element_name = NULL;
    const fbInfoElement_t *ie;
    uint16_t type = 0;

    if (!PyArg_ParseTuple(args, "s", &element_name)) {
        PyErr_SetString(PyExc_AttributeError, "element name");
        return NULL;
    }

    ie = fbInfoModelGetElementByName(self->infoModel,
                                     element_name);
    type = ie->type;

    return PyInt_FromLong(type);
}


static PyObject *fixbufPyInfoModelGetElement(
    fixbufPyInfoModel *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"name", "id", "ent", NULL};
    char *name = NULL;
    fixbufPyInfoElement *element = NULL;
    const fbInfoElement_t *ie = NULL;
    int ent = 0;
    int id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|sii", kwlist, &name, &id, &ent)) {
        return NULL;
    }

    if (name == NULL) {
        if (id == 0) {
            PyErr_SetString(PyExc_AttributeError,
                            "Expected either name or ID.");
            return NULL;
        }
    }

    if (name) {
        ie = fbInfoModelGetElementByName(self->infoModel, name);
    } else {
        ie = fbInfoModelGetElementByID(self->infoModel, id, ent);
    }

    element = (fixbufPyInfoElement *)fixbufPyInfoElementType.tp_new(&fixbufPyInfoElementType, NULL, NULL);

    if (element == NULL) {
        Py_XDECREF(element);
        return PyErr_NoMemory();
    }

    if (ie) {
        memcpy(element->infoElement, ie, sizeof(fbInfoElement_t));
    }

    /* keep orig ptr so it can be passed to fbbasiclistinit */
    element->ptr = ie;

    /*Py_INCREF(element);*/

    return (PyObject *)element;
}


static PyMethodDef fixbufPyInfoModel_methods[] = {
    {"addElement", (PyCFunction)fixbufPyInfoModelAddElement, METH_VARARGS,
     ("Add the info element to the info model")},
    {"getElementLength", (PyCFunction)fixbufPyInfoModelGetElementLength,
     METH_KEYWORDS, ("Return the Info Element Length")},
    {"getElementType", (PyCFunction)fixbufPyInfoModelGetElementType,
     METH_VARARGS, ("Return the Info Element Type - for retrieving offset")},
    {"getElementTrueType", (PyCFunction)fixbufPyInfoModelGetElementTrueType,
     METH_VARARGS, ("Retrun the true Info Element Type as defined in the Info Model")},
    {"is_varfield", (PyCFunction)fixbufPyInfoModelIsVarfield,
     METH_VARARGS, ("Find out if the element is a variable length field")},
    {"getElement", (PyCFunction)fixbufPyInfoModelGetElement,
     METH_KEYWORDS, ("Return Information Element with given name or id")},
    {NULL, NULL, 0, NULL}
};


static int fixbufPyInfoModel_init(
    fixbufPyInfoModel *self,
    PyObject *args,
    PyObject *kwds)
{

    if (self != NULL) {
        self->infoModel = fbInfoModelAlloc();
    }

    return 0;
}


static void fixbufPyInfoModel_dealloc(
    fixbufPyInfoModel *obj)
{

    if (obj->infoModel) {
        fbInfoModelFree(obj->infoModel);
    }

    Py_TYPE(obj)->tp_free((PyObject *)obj);
}

static PyTypeObject fixbufPyInfoModelType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbInfoModelBase",    /*tp_name*/
    sizeof(fixbufPyInfoModel), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPyInfoModel_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbInfoModel", /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyInfoModel_methods,  /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPyInfoModel_init,  /* tp_init */
    0,                          /* tp_alloc */
    0,                          /*tp_new */
    0                            /* tp_free */
};

/**
 * fBuf
 *
 **/

static PyObject *fixbufPyfBuf_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{

    fixbufPyfBuf *self;

    self = (fixbufPyfBuf *)type->tp_alloc(type, 0);

    return (PyObject *)self;
}


static int fixbufPyfBuf_init(
    fixbufPyfBuf        *self,
    PyObject            *args,
    PyObject            *kwds)
{
    self->ignore = 0;

    return 0;
}

static PyObject *fixbufPyfBufAllocForCollection(
    fixbufPyfBuf *self,
    PyObject *args)
{

    fixbufPySession *session = NULL;
    fixbufPyCollector *collector = NULL;
    int rv;


    rv = PyArg_ParseTuple(args, "OO", (PyObject *)&session, (PyObject *)&collector);
    if (!rv) {
        return NULL;
    }

    if (!fixbufPyfBuf_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.fBuf");
        return NULL;
    }

    if (!fixbufPySession_Check(session)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.fbSession");
        return NULL;
    }

    if (!fixbufPyCollector_Check(collector)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.fbCollector");
        return NULL;
    }

    self->fbuf = fBufAllocForCollection(session->session,
                                        collector->collector);

    if (!self->fbuf) {
        PyErr_SetString(PyExc_ValueError, "Error allocating fBuf for Collection");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPyfBufAllocForExport(
    fixbufPyfBuf *self,
    PyObject *args)
{

    fixbufPySession *session = NULL;
    fixbufPyExporter *exporter = NULL;
    int rv;

    rv = PyArg_ParseTuple(args, "OO", (PyObject *)&session,
                          (PyObject *)&exporter);
    if (!rv) {
        return NULL;
    }

    if (!fixbufPyfBuf_Check(self)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.fBuf");
        return NULL;
    }

    if (!fixbufPySession_Check(session)) {
        PyErr_SetString(PyExc_TypeError, "Expected valid pyfixbuf.fbSession");
        return NULL;
    }

    if (!fixbufPyExporter_Check(exporter)) {
        PyErr_SetString(PyExc_TypeError, "Expected valid pyfixbuf.fbExporter");
        return NULL;
    }

    self->fbuf = fBufAllocForExport(((fixbufPySession *)session)->session,
                                    ((fixbufPyExporter *)exporter)->exporter);

    if (!self->fbuf) {
        PyErr_SetString(PyExc_ValueError, "Error Allocating fbuf for Export");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;


}

static PyObject *fixbufPyfBufSetInternalTemplate(
    fixbufPyfBuf *self,
    PyObject *args)
{

    int tid;
    GError *err = NULL;

    if (!PyArg_ParseTuple(args, "i", &tid)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Template ID");
        return NULL;
    }

    if (self->fbuf == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    if (!fBufSetInternalTemplate(self->fbuf, tid, &err)) {
        PyErr_Format(PyExc_StandardError,
                     "Fixbuf Error Setting Internal Template on Buffer: %s\n",
                     err->message);
        Py_INCREF(Py_False);
        return Py_False;
    }

    Py_INCREF(Py_True);

    return Py_True;
}


static PyObject *fixbufPyfBufSetExportTemplate(
    fixbufPyfBuf *self,
    PyObject *args)
{

    int tid;
    GError *err = NULL;

    if (!PyArg_ParseTuple(args, "i", &tid)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Template ID");
        return NULL;
    }

    if (self->fbuf == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    if (!fBufSetExportTemplate(self->fbuf, tid, &err)) {
        PyErr_Format(PyExc_StandardError,
                     "Fixbuf Error Setting External Template on Buffer: %s",
                     err->message);
        Py_INCREF(Py_False);
        return Py_False;
    }

    Py_INCREF(Py_True);

    return Py_True;

}

static PyObject *fixbufPyfBufNext(
    fixbufPyfBuf *self,
    PyObject *args)
{

    GError *err = NULL;
    int rv;
    fixbufPyRecord *fixrec = NULL;
    fbTemplate_t *tmpl = NULL;
    uint16_t tid = 0;
    int options = 1;
    size_t rec_len = 0;

    if (!PyArg_ParseTuple(args, "O", (PyObject *)&fixrec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(fixrec)) {
        PyErr_SetString(PyExc_AttributeError, "Expected pyfixbuf.Record");
        return NULL;
    }

    if (self->fbuf == NULL) {
        PyErr_SetString(PyExc_StopIteration, "End of File - NULL Buffer");
        return NULL;
    }

    if (fixrec->rec == NULL) {
        fixrec->rec = PyMem_Malloc(fixrec->reclen);
        if (fixrec->rec == NULL) {
            Py_XDECREF((PyObject *)self);
            return PyErr_NoMemory();
        }
        fixrec->memalloc = TRUE;
    }

    if (self->ignore) {
        do {
            tmpl = fBufNextCollectionTemplate(self->fbuf, &tid, &err);
            if (tmpl) {
                if (fbTemplateGetOptionsScope(tmpl)) {
                    rv = fBufNext(self->fbuf, fixrec->rec, &(fixrec->reclen), &err);
                } else {
                    options = 0;
                }
            } else {
                goto err;
            }
        } while (options);
    }

    rec_len = fixrec->reclen;
    rv = fBufNext(self->fbuf, fixrec->rec, &rec_len, &err);
    if (rv == 0) {
        goto err;
    }
    Py_INCREF(Py_None);
    return Py_None;

err:
    if (!strncmp(err->message, "End of file", strlen("End of file"))) {
        g_clear_error(&err);
        fBufFree(self->fbuf);
        self->fbuf = NULL;
        PyErr_SetString(PyExc_StopIteration, "End of File");
        return NULL;
    } else {
        PyErr_Format(PyExc_StandardError, "Error retrieving fBuf: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }
}

static PyObject *fixbufPyfBufAppend(
    fixbufPyfBuf *self,
    PyObject *args)
{
    int len = 0;
    GError *err = NULL;
    fixbufPyRecord *fixrec = NULL;
    int rv;

    if (!PyArg_ParseTuple(args, "O|i", (PyObject *)&fixrec, &len)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Record and"
                        " Optional Record Length");
        return NULL;
    }

    if (!fixbufPyRecord_Check(fixrec)) {
        return NULL;
    }

    if (len == 0) {
        len = fixrec->reclen;
    }

    if (fixrec->rec == NULL) {
        PyErr_SetString(PyExc_StandardError, "No Record Available to Append");
        return NULL;
    }

    rv = fBufAppend(self->fbuf, fixrec->rec, (size_t)len, &err);
    if (rv == FALSE) {
        PyErr_Format(PyExc_StandardError, "Error appending Buffer: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *fixbufPyfBufEmit(
    fixbufPyfBuf *self)
{

    GError *err = NULL;

    if (!fBufEmit(self->fbuf, &err)) {
        PyErr_Format(PyExc_StandardError, "Error emiting Buffer: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;

}

static PyObject *fixbufPyfBufNextCollectionTemplate(
    fixbufPyfBuf *self,
    PyObject *args)
{
    uint16_t tid;
    GError *err = NULL;
    fixbufPyTemplate *tmpl = NULL;

    if (self->fbuf == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Buffer is NULL");
        return NULL;
    }

    tmpl = (fixbufPyTemplate *)PyObject_New(fixbufPyTemplate,
                                            &fixbufPyTemplateType);
    if (tmpl == NULL) {
        Py_XDECREF(tmpl);
        return PyErr_NoMemory();
    }

    tmpl->template = fBufNextCollectionTemplate(self->fbuf, &tid, &err);

    tmpl->template_id = tid;

    /*Py_INCREF(tmpl);*/

    return (PyObject *)tmpl;
}


static PyObject *fixbufPyfBufGetCollectionTemplate(
    fixbufPyfBuf *self,
    PyObject     *args)
{

    uint16_t tid;
    fixbufPyTemplate *tmpl = NULL;

    if (self->fbuf == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Buffer is NULL");
        return NULL;
    }

    tmpl = (fixbufPyTemplate *)PyObject_New(fixbufPyTemplate,
                                            &fixbufPyTemplateType);
    if (tmpl == NULL) {
        Py_XDECREF(tmpl);
        return PyErr_NoMemory();
    }

    tmpl->template = fBufGetCollectionTemplate(self->fbuf, &tid);

    tmpl->template_id = tid;


    /*Py_INCREF(tmpl);*/

    return (PyObject *)tmpl;
}



static PyObject *fixbufPyfBufFree(
    fixbufPyfBuf *self)
{

    if (self->fbuf) {
        fBufFree(self->fbuf);
        self->fbuf = NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;

}

static PyObject *fixbufPyfBufWriteOptionsRecord(
    fixbufPyfBuf *self,
    PyObject *args)
{

    GError *err = NULL;
    fixbufPyInfoModel *pymodel = NULL;
    fbInfoModel_t *model = NULL;
    int  tid = 0;
    char *name;

    if (!PyArg_ParseTuple(args, "O!si", &fixbufPyInfoModelType,
                          &pymodel, &name, &tid))
    {
        return NULL;
    }

    if (!fixbufPyInfoModel_Check(pymodel)) {
        PyErr_SetString(PyExc_ValueError, "Need an InfoModel");
        return NULL;
    }

    if (tid == 0) {
        PyErr_SetString(PyExc_AttributeError, "Need a valid Template ID");
        return NULL;
    }

    if (name == NULL) {
        PyErr_SetString(PyExc_AttributeError,"Need a valid info element name");
        return NULL;
    }

    if (self->fbuf == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Buffer is NULL");
        return NULL;
    }


    model = pymodel->infoModel;

    if (fbInfoElementWriteOptionsRecord(self->fbuf,
                                      fbInfoModelGetElementByName(model, name),
                                         tid, &err)) {
        Py_INCREF(Py_None);
        return Py_None;
    } else {
        PyErr_Format(PyExc_StandardError, "Unable to append Options Record: "
                     "%s", err->message);
        g_clear_error(&err);
        return NULL;
    }
}

static PyObject *fixbufPySetAutoInsert(
    fixbufPyfBuf *self)
{
    GError *err = NULL;

    if (fBufSetAutomaticInsert(self->fbuf, &err)) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        PyErr_Format(PyExc_StandardError, "Unable to set Buffer in Auto"
                     " Insert Mode: %s", err->message);
        g_clear_error(&err);
        return NULL;
    }

}

static PyObject *fixbufPySetIgnoreOptions(
    fixbufPyfBuf *self,
    PyObject *args)
{
    int ignore;

    if (!PyArg_ParseTuple(args, "i", &ignore)) {
        return NULL;
    }

    self->ignore = ignore;

    Py_INCREF(Py_None);

    return Py_None;
}


static PyMethodDef fixbufPyfBuf_methods[] = {
    {"allocForCollection", (PyCFunction)fixbufPyfBufAllocForCollection,
     METH_VARARGS, ("Allocate an fbuf for collection")},
    {"allocForExport", (PyCFunction)fixbufPyfBufAllocForExport, METH_VARARGS,
     ("Allocate an fbuf for export")},
    {"setInternalTemplate", (PyCFunction)fixbufPyfBufSetInternalTemplate,
     METH_VARARGS, ("Set an Internal Template on the fBuf for export")},
    {"setExportTemplate", (PyCFunction)fixbufPyfBufSetExportTemplate,
     METH_VARARGS, ("Set an External Template on a buffer.  The external "
    "tmpl describes the record that will be written to the IPFIX message")},
    {"nextRecord", (PyCFunction)fixbufPyfBufNext, METH_VARARGS,
     ("Get the next record")},
    {"nextTemplate", (PyCFunction)fixbufPyfBufNextCollectionTemplate,
     METH_VARARGS, ("Retrieve the external tmpl that will be used to read "
     "the next record from the buffer")},
    {"getTemplate", (PyCFunction)fixbufPyfBufGetCollectionTemplate,
     METH_VARARGS, ("Retrieve the external template that was used to read "
                    "the last record from the buffer")},
    {"append", (PyCFunction)fixbufPyfBufAppend, METH_VARARGS,
     ("Append the record to the buffer")},
    {"emit", (PyCFunction)fixbufPyfBufEmit, METH_NOARGS, ("Emit the buffer")},
    {"free", (PyCFunction)fixbufPyfBufFree, METH_NOARGS, ("Free the buffer")},
    {"writeOptionsRecord", (PyCFunction)fixbufPyfBufWriteOptionsRecord,
     METH_VARARGS, ("Write an Info Element Type Information Options"
                    " Record for the given IE name")},
    {"setAutoInsert", (PyCFunction)fixbufPySetAutoInsert, METH_NOARGS,
     ("Set the Buffer in Auto Insert Mode.")},
    {"ignoreOptions", (PyCFunction)fixbufPySetIgnoreOptions, METH_VARARGS,
     ("Set the Buffer to ignore Options Templates and Records.")},
    {NULL, NULL, 0, NULL}
};


static PyTypeObject fixbufPyfBufType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fBufBase",           /*tp_name*/
    sizeof(fixbufPyfBuf),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,               /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fBuf",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyfBuf_methods,       /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPyfBuf_init, /* tp_init */
    0,                          /* tp_alloc */
    fixbufPyfBuf_new,           /* tp_new */
    0                           /* tp_free */
};


/**
 * fbRecord
 *
 *
 */

static int fixbufPyRecord_init(
    fixbufPyRecord *self,
    PyObject *args,
    PyObject *kwds)
{
    int len;

    if (!PyArg_ParseTuple(args, "i", &len)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Record Length");
        return -1;
    }
    if (self != NULL) {
        self->rec = NULL;
        self->reclen = (size_t)len;
        self->memalloc = FALSE;
    }

    return 0;
}

static void fixbufPyRecord_dealloc(
    fixbufPyRecord *self)
{
    if (self->memalloc) {
        PyMem_Free(self->rec);
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *fixbufPyRecordSetOffset(
    fixbufPyRecord *self,
    PyObject *args)
{

    PyObject *value = NULL;
    fbVarfield_t varfield;
    int offset;
    int len;
    int type;
    char *string;
    uint64_t val;

    if (!PyArg_ParseTuple(args, "Oiii", &value, &offset, &len, &type)) {
        return NULL;
    }

    if (self->rec == NULL) {
        self->rec = PyMem_Malloc(self->reclen);
        if (!self->rec) {
            Py_XDECREF((PyObject *)self);
            return PyErr_NoMemory();
        }
        /* initialize memory to 0 */
        memset(self->rec, 0, self->reclen);
        self->memalloc = TRUE;
    }

    if (IS_STRING(value) && (type != FB_IP6_ADDR)) {
        if (fixbufPyVarfield_Check(value)) {
            memcpy(self->rec + offset,
                   ((fixbufPyVarfield *)(value))->varfield,
                   sizeof(fbVarfield_t));
        } else {
            string = PyString_AsString(value);
            varfield.len = strlen(string);
            varfield.buf = (uint8_t*)string;
            /*Py_INCREF(value);*/
            memcpy(self->rec + offset, &varfield, sizeof(fbVarfield_t));
        }

    } else if (IS_INT(value) || (type == FB_IP6_ADDR)) {
        if (len != 16) {
            val = LONG_AS_UNSIGNED_LONGLONG(value);
        }
        if (PyErr_Occurred()) {
            PyErr_Format(PyExc_ValueError,
                         "Value provided is not of long type.  "
                         "Try to convert using long()");
            return NULL;
        }
        if (len == 1) {
            if (val > (uint64_t)UINT8_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 8-bit integer");
                return NULL;
            } else {
                *(self->rec + offset) = (uint8_t)val;
            }
        } else if (len == 2) {
            if (val > (uint64_t)UINT16_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 16-bit integer");
                return NULL;
            } else {
                *((uint16_t *)(self->rec + offset)) = (uint16_t)val;
            }
        } else if (len == 4) {
            if (val > (uint64_t)UINT32_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 32-bit integer");
                return NULL;
            } else {
                *((uint32_t *)(self->rec + offset)) = (uint32_t)val;
            }
        } else if (len == 8) {
            if (val > (uint64_t)UINT64_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 64-bit integer");
                return NULL;
            } else {
                memcpy((self->rec + offset), &val, sizeof(uint64_t));
            }
        } else if (len == 16) {
            string = PyString_AsString(value);
            memcpy((self->rec + offset), string, len);
        } else {
            PyString_AsStringAndSize(value, &string, (Py_ssize_t *)&len);
            memcpy((self->rec + offset), string, len);
        }
            /*} else {
            PyErr_SetString(PyExc_AttributeError,
                            "Invalid Length");
            return NULL;
            }*/

    } else if (type == FB_SUB_TMPL_MULTI_LIST) {
        if (!fixbufPySTML_Check(value)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Value must be a STML");
            return NULL;
        }
        memcpy((self->rec + offset), ((fixbufPySTML *)(value))->stml,
               sizeof(fbSubTemplateMultiList_t));
        if (((fixbufPySTML *)(value))->stml_alloc) {
            /* free fbsubtemplatemultilist*/
            PyMem_Free(((fixbufPySTML *)(value))->stml);
            ((fixbufPySTML *)(value))->stml_alloc = FALSE;
        }
        ((fixbufPySTML *)(value))->stml = (fbSubTemplateMultiList_t*)(self->rec + offset);

    } else if (type == FB_SUB_TMPL_LIST) {
        if (!fixbufPySTL_Check(value)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Value must be a STL");
            return NULL;
        }
        memcpy((self->rec + offset), ((fixbufPySTL *)(value))->stl,
               sizeof(fbSubTemplateList_t));
        if (((fixbufPySTL *)(value))->stl_alloc) {
            /* free fbsubtemplatelist*/
            PyMem_Free(((fixbufPySTL *)(value))->stl);
            ((fixbufPySTL *)(value))->stl_alloc = FALSE;
        }
        ((fixbufPySTL *)(value))->stl = (fbSubTemplateList_t*)(self->rec + \
                                                               offset);
    } else if (type == FB_BASIC_LIST) {
        if (!fixbufPyBL_Check(value)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Value must be a valid BL");
            return NULL;
        }
        memcpy((self->rec + offset), ((fixbufPyBL *)(value))->bl,
               sizeof(fbBasicList_t));
        if (((fixbufPyBL *)(value))->bl_alloc) {
            PyMem_Free(((fixbufPyBL *)(value))->bl);
            ((fixbufPyBL *)(value))->bl_alloc = FALSE;
        }
        ((fixbufPyBL *)(value))->bl = (fbBasicList_t *)(self->rec + offset);
    } else {
        PyErr_SetString(PyExc_AttributeError,
                        "value argument must be string or number");
        return NULL;
    }

    if (PyErr_Occurred()) {
        return NULL;
    }


    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *fixbufPyRecordClearBasicList(
    fixbufPyRecord *self,
    PyObject *args)
{

    int offset;

    if (!PyArg_ParseTuple(args, "i", &offset)) {
        return NULL;
    }

    if (self->rec == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No fixbuf Record to Clear");
        return NULL;
    }

    fbBasicListClear((fbBasicList_t *)(self->rec + offset));

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPyRecordGetBL(
    fixbufPyRecord *self,
    PyObject *args)
{

    fixbufPyBL *bl;
    int offset;

    if (!PyArg_ParseTuple(args, "O!i", &fixbufPyBLType, &bl, &offset)) {
        return NULL;
    }

    if (self->rec == NULL) {
        offset = 0;
        /* No record - return 0 */
        return PyInt_FromLong(offset);
    }

    memcpy(bl->bl, (self->rec + offset), sizeof(fbBasicList_t));

    bl->init = TRUE;

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPyRecordGetBasicListItems(
    fixbufPyRecord *self,
    PyObject *args)
{

    PyObject *list;
    fbBasicList_t *bl = NULL;
    Py_ssize_t i;
    int offset;
    int len;
    long type;
    int w;

    if (!PyArg_ParseTuple(args, "iil", &offset, &len, &type)) {
        return NULL;
    }

    if (self->rec == NULL) {
        len = 0;
        /* No record - return 0 */
        return PyInt_FromLong(len);
    }


    bl = (fbBasicList_t *)(self->rec + offset);

    list = PyList_New(bl->numElements);

    if (list == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not create List Object");
        return NULL;
    }

    if (type == FB_IE_VARLEN) {

        fbVarfield_t *item;
        for (w=0;(item=(fbVarfield_t *)fbBasicListGetIndexedDataPtr(bl,w));w++)
        {
            i = w;
            PyList_SetItem(list, i,
                           PyString_FromStringAndSize((char *)item->buf,
                                                      item->len));
        }
    } else {

        switch (type) {
          case FB_UINT_8:
          case FB_BOOL:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyInt_FromLong(*item));
                }
                break;
            }
          case FB_UINT_16:
            {
                uint16_t *item;
                for (w=0; (item =(uint16_t *)fbBasicListGetIndexedDataPtr(bl, w)); w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyLong_FromLong(*item));
                }
                break;
            }
          case FB_UINT_32:
          case FB_IP4_ADDR:
          case FB_DT_SEC:
            {
                uint32_t *item;
                for (w=0;
                     (item =(uint32_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    PyList_SetItem(list,i, PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_UINT_64:
          case FB_DT_MILSEC:
          case FB_DT_MICROSEC:
          case FB_DT_NANOSEC:
            {
                uint64_t *item;
                for (w = 0;
                     (item = (uint64_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    PyList_SetItem(list,i, PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_INT_8:
            {
                int8_t *item;
                for (w=0; (item =
                           (int8_t *)fbBasicListGetIndexedDataPtr(bl, w)); w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyInt_FromLong(*item));
                }
                break;

            }
          case FB_INT_16:
            {
                int16_t *item;
                for (w=0; (item =
                           (int16_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyLong_FromLong(*item));
                }
                break;
            }
          case FB_INT_32:
            {
                int32_t *item;
                for (w=0; (item =
                           (int32_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_INT_64:
            {
                int64_t *item;
                for (w=0; (item =
                           (int64_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_FLOAT_32:
          case FB_FLOAT_64:
            {
                uint8_t *item;
                PyObject *o;
                for (w=0;
                     (item = (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    o = PyString_FromStringAndSize((char *)item, len);
                    if (PyErr_Occurred()) {
                        Py_XDECREF(o);
                        return NULL;
                    }
                    PyList_SetItem(list, i,
                                   PyFloat_FromString(o, NULL));
                }
                break;
            }
          case FB_IP6_ADDR:
          case FB_STRING:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,
                                   PyString_FromStringAndSize((char *)item,
                                                              len));
                }
                break;
            }
          case FB_OCTET_ARRAY:
          case FB_MAC_ADDR:
          default:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,
                                   PyByteArray_FromStringAndSize((char *)item,
                                                                 len));
                }
                break;
            }
        }
    }

    return list;
}



static PyObject *fixbufPyRecordGetOffset(
    fixbufPyRecord *self,
    PyObject *args)
{

    int len;
    int offset;
    long type = 0;
    PyObject *rv;

    if (!PyArg_ParseTuple(args, "ii|l", &offset, &len, &type)) {
        return NULL;
    }

    if (self->rec == NULL) {
        len = 0;
        /* No record - return 0 */
        return PyInt_FromLong(len);
    }

    if (type == FB_IE_VARLEN) {
        /* variable length field */
        fixbufPyVarfield *varf = PyObject_New(fixbufPyVarfield,
                                              &fixbufPyVarfieldType);
        if (varf == NULL) {
            return PyErr_NoMemory();
        }
        varf->varfield = (fbVarfield_t *)(self->rec + offset);
        /*Py_INCREF(varf);*/
        return (PyObject *)varf;
    }

    switch (type) {
      case FB_UINT_8:
      case FB_BOOL:
        {
            uint8_t grab;
            if (len != 1) {
                break;
            }
            grab = *(self->rec + offset);
            return PyLong_FromLong(grab);
        }
      case FB_UINT_16:
        {
            uint16_t grab;
            if (len != 2) {
                break;
            }
            grab = *((uint16_t *)(self->rec + offset));
            return PyLong_FromLong(grab);
        }
      case FB_UINT_32:
      case FB_IP4_ADDR:
      case FB_DT_SEC:
        {
            uint32_t grab;
            if (len != 4) {
                break;
            }
            grab = *((uint32_t *)(self->rec + offset));
            return PyLong_FromUnsignedLongLong(grab);
        }
      case FB_UINT_64:
      case FB_DT_MILSEC:
      case FB_DT_MICROSEC:
      case FB_DT_NANOSEC:
        {
            uint64_t grab;
            if (len != 8) {
                break;
            }
            memcpy(&grab, self->rec + offset, sizeof(uint64_t));
            rv = PyLong_FromUnsignedLongLong(grab);
            if (PyErr_Occurred()) {
                Py_XDECREF(rv);
                return NULL;
            } else {
                /*Py_INCREF(rv);*/
                return rv;
            }
        }
      case FB_INT_8:
        {
            int8_t grab;
            if (len != 1) {
                break;
            }
            grab = *(self->rec + offset);
            return PyLong_FromLong(grab);
        }
      case FB_INT_16:
        {
            int16_t grab;
            if (len != 2) {
                break;
            }
            grab = *((int16_t *)(self->rec + offset));
            return PyLong_FromLong(grab);
        }
      case FB_INT_32:
        {
            int32_t grab;
            if (len != 4) {
                break;
            }
            grab = *((int32_t *)(self->rec + offset));
            /*memcpy(&grab, self->rec + offset, len);*/
            return PyLong_FromUnsignedLongLong(grab);
        }
      case FB_INT_64:
        {
            int64_t grab;
            if (len != 8) {
                break;
            }
            memcpy(&grab, self->rec + offset, sizeof(int64_t));
            rv = PyLong_FromUnsignedLongLong(grab);
            if (PyErr_Occurred()) {
                Py_XDECREF(rv);
                return NULL;
            } else {
                /*Py_INCREF(rv);*/
                return rv;
            }
        }
      case FB_FLOAT_32:
        {
            uint8_t grab[4];
            if (len != 4) {
                break;
            }
            memcpy(grab, self->rec + offset, 4);
            rv = PyString_FromStringAndSize((char *)grab, len);
            if (PyErr_Occurred()) {
                Py_XDECREF(rv);
                return NULL;
            } else {
                return PyFloat_FromString(rv, NULL);
            }
        }
      case FB_FLOAT_64:
        {
            uint8_t grab[8];
            if (len != 8) {
                break;
            }
            memcpy(grab, self->rec + offset, 8);
            rv = PyString_FromStringAndSize((char *)grab, len);
            if (PyErr_Occurred()) {
                Py_XDECREF(rv);
                return NULL;
            } else {
                return PyFloat_FromString(rv, NULL);
            }
        }
      case FB_IP6_ADDR:
        {
            uint8_t grab[16];
            if (len != 16) {
                break;
            }
            memcpy(grab, self->rec + offset, len);
            return PyString_FromStringAndSize((char *)grab, len);
        }
      case FB_STRING:
        {
            uint8_t grab[65535];
            memcpy(grab, self->rec + offset, len);
            return PyString_FromStringAndSize((char *)grab, len);
        }
      case FB_MAC_ADDR:
        {
            uint8_t grab[6];
            if (len != 6) {
                break;
            }
            memcpy(grab, self->rec + offset, len);
            return PyString_FromStringAndSize((char *)grab, len);
        }
      case FB_OCTET_ARRAY:
      default:
        {
            uint8_t grab[65535];
            memcpy(grab, self->rec + offset, len);
            rv = PyByteArray_FromStringAndSize((const char *)grab, len);
            if (PyErr_Occurred()) {
                Py_XDECREF(rv);
                return NULL;
            } else {
                /*Py_INCREF(rv);*/
                return rv;
            }
        }
    }

    /* length override? */

    if (len == 1) {
        uint8_t grab;
        grab = *(self->rec + offset);
        return PyLong_FromLong(grab);
    } else if (len == 2) {
        uint16_t grab;
        grab = *((uint16_t *)(self->rec + offset));
        return PyLong_FromLong(grab);
    } else if (len == 4) {
        uint32_t grab;
        uint32_t *ptr = (uint32_t *)(self->rec + offset);
        memcpy(&grab, ptr, sizeof(uint32_t));
        return PyLong_FromUnsignedLongLong(grab);
    } else if (len == 8) {
        uint64_t grab;
        PyObject *rv;
        memcpy(&grab, self->rec + offset, len);
        rv = PyLong_FromUnsignedLongLong(grab);
        if (PyErr_Occurred()) {
            Py_XDECREF(rv);
            return NULL;
        } else {
            /*Py_INCREF(rv);*/
            return rv;
        }
    } else if (len == 16) {
        uint8_t grab[16];
        memcpy(grab, self->rec + offset, len);
        return PyString_FromStringAndSize((char *)grab, len);
    } else {
        uint8_t grab[65535];
        PyObject *rv;
        memcpy(grab, self->rec + offset, len);
        rv = PyByteArray_FromStringAndSize((const char *)grab, len);
        if (PyErr_Occurred()) {
            Py_XDECREF(rv);
            return NULL;
        } else {
            /*Py_INCREF(rv);*/
            return rv;
        }
    }


    Py_INCREF(Py_None);
    return Py_None;

}

static PyObject *fixbufPyRecord_getlen(
    fixbufPyRecord *self,
    void *cbdata)
{
    return PyInt_FromLong(self->reclen);
}

static int fixbufPyRecord_setlen(
    fixbufPyRecord *self,
    PyObject *value,
    void *cbdata)
{
    PyObject *pyint_val;
    size_t new_len;

    if (!PyNumber_Check(value)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Positive Number");
        return -1;
    }

    pyint_val = PyNumber_Int(value);
    if (pyint_val == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Expected Positive Number");
        return -1;
    }

    new_len = PyLong_AsLong(pyint_val);

    if (self->memalloc) {
        if (new_len > self->reclen) {
            /* need to realloc */
            self->rec = PyMem_Realloc(self->rec, new_len);
            if (!self->rec) {
                Py_XDECREF(pyint_val);
                return -1;
            }
        }
    }

    self->reclen = new_len;

    Py_XDECREF(pyint_val);

    return 0;
}

static PyObject *fixbufPyRecordClear(
    fixbufPyRecord *self)
{

    if (self->memalloc == FALSE) {
        self->rec = NULL;
    } else {
        memset(self->rec, 0, self->reclen);
    }


    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef fixbufPyRecord_methods[] = {
    {"getOffset", (PyCFunction)fixbufPyRecordGetOffset,
     METH_VARARGS, ("Get offset into buffer")},
    {"setOffset", (PyCFunction)fixbufPyRecordSetOffset,
     METH_VARARGS, ("Set offset into buffer")},
    {"getBL", (PyCFunction)fixbufPyRecordGetBL,
     METH_VARARGS, ("Get basiclist items")},
    {"getBasicListItems", (PyCFunction)fixbufPyRecordGetBasicListItems,
     METH_VARARGS, ("Get basiclist items")},
    {"clear", (PyCFunction)fixbufPyRecordClear, METH_NOARGS,
     ("Set Record to NULL")},
    {"basicListClear", (PyCFunction)fixbufPyRecordClearBasicList,
     METH_VARARGS, ("Clear given basiclist")},
    {NULL, NULL, 0, NULL}
};


static PyGetSetDef fixbufPyRecord_getsetters[] = {
    {"length", (getter)fixbufPyRecord_getlen, (setter)fixbufPyRecord_setlen,
     "Record Length", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};


static PyTypeObject fixbufPyRecordType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbRecordBase",           /*tp_name*/
    sizeof(fixbufPyRecord),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPyRecord_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "Record",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyRecord_methods,        /* tp_methods */
    0,                          /* tp_members */
    fixbufPyRecord_getsetters,     /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPyRecord_init,           /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};



static int fixbufPyTemplate_init(
    fixbufPyTemplate *self,
    PyObject *args,
    PyObject *kwds)
{

    fixbufPyInfoModel *model = NULL;
    fixbufPyTemplate *template = NULL;
    int type = 0;
    static char *kwlist[] = {"model", "type", "template", NULL};
    GError *err = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|iO", kwlist,
                                     &fixbufPyInfoModelType, &model, &type, &template))
    {
        return -1;
    }

    if (!fixbufPyInfoModel_Check(model)) {
        PyErr_SetString(PyExc_ValueError, "Need an InfoModel");
        return -1;
    }

    if (template) {
        self->template = template->template;
        self->template_id = template->template_id;
        return 0;
    }

    if (type) {
        self->template =
            fbInfoElementAllocTypeTemplate(((fixbufPyInfoModel *)model)->infoModel, &err);
        if (self->template == NULL) {
            PyErr_Format(PyExc_StandardError,
                         "Could not create Information Type Template: %s\n",
                         err->message);
            g_clear_error(&err);
            return -1;
        }
    } else {
        self->template =
            fbTemplateAlloc(((fixbufPyInfoModel *)model)->infoModel);
    }

    if (self->template == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Error Allocating Template");
        return -1;
    }

    return 0;
}

static void fixbufPyTemplate_dealloc(
    fixbufPyTemplate *self)
{
    /*if (self->template) {
        fbTemplateFreeUnused(self->template);
        }*/
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *fixbufPyTemplateAddSpec(
    fixbufPyTemplate   *self,
    fixbufPyInfoElementSpec *spec)
{

    GError *err = NULL;

    if (!fixbufPyInfoElementSpec_Check(spec)) {
        return PyErr_Format(PyExc_TypeError, "Expected Info Element Spec");
    }

    if (self->template == NULL) {
        PyErr_SetString(PyExc_AttributeError, "NULL Template");
        return NULL;
    }

    if (spec->spec == NULL) {
        PyErr_SetString(PyExc_AttributeError, "NULL InfoElementSpec");
        return NULL;
    }

    if (!fbTemplateAppendSpec(self->template, spec->spec, 0, &err)) {
        PyErr_Format(PyExc_StandardError,
                     "Error adding info element spec to template: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPyTemplate_getscope(
    fixbufPyTemplate *self,
    void *cbdata)
{

    if (self->template == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    return PyInt_FromLong(fbTemplateGetOptionsScope(self->template));

}

static PyObject *fixbufPyTemplateCountElements(
    fixbufPyTemplate *self)
{
    long rv = 0;
    if (self->template == NULL) {
        return PyInt_FromLong(rv);
    }

    return PyInt_FromLong(fbTemplateCountElements(self->template));
}

static int fixbufPyTemplate_setscope(
    fixbufPyTemplate *self,
    PyObject *value,
    void *cbdata)
{
    uint16_t scope_count;
    int num_elements;
    PyObject *pyint_val;

    if (!PyNumber_Check(value)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Scope Count");
        return -1;
    }

    pyint_val = PyNumber_Int(value);
    if (pyint_val == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Expected Scope Count");
        return -1;
    }

    scope_count = PyLong_AsLong(pyint_val);
    Py_XDECREF(pyint_val);

    num_elements = fbTemplateCountElements(self->template);

    if (num_elements < scope_count) {
        PyErr_SetString(PyExc_RuntimeError, "Scope count must be greater"
                        " than or equal to the number of elements in the "
                        "template");
        return -1;
    }


    fbTemplateSetOptionsScope(self->template, scope_count);

    return 0;
}

static PyObject *fixbufPyTemplate_gettid(
    fixbufPyTemplate *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->template_id);
}

static PyObject *fixbufPyTemplate_gettype(
    fixbufPyTemplate *obj,
    void *cbdata)
{
    if (fbInfoModelTypeInfoRecord(obj->template)) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject *fixbufPyTemplateContainsElement(
    fixbufPyTemplate *self,
    PyObject *arg)
{
    char *name = NULL;
    fbInfoElementSpec_t spec;

    if (fixbufPyInfoElementSpec_Check(arg)) {
        spec.name = ((fixbufPyInfoElementSpec *)arg)->spec->name;
        if (fbTemplateContainsElementByName(self->template, &spec)) {
            Py_INCREF(Py_True);
            return Py_True;
        } else {
            Py_INCREF(Py_False);
            return Py_False;
        }
    } else if (fixbufPyInfoElement_Check(arg)) {
        if (fbTemplateContainsElement(self->template,
                                   ((fixbufPyInfoElement *)arg)->infoElement))
        {
            Py_INCREF(Py_True);
            return Py_True;
        } else {
            Py_INCREF(Py_False);
            return Py_False;
        }

    } else if (PyString_Check(arg)) {
        name = PyString_AsString(arg);
        if (name) {
            spec.name = name;
            if (fbTemplateContainsElementByName(self->template, &spec)) {
                Py_INCREF(Py_True);
                return Py_True;
            } else {
                Py_INCREF(Py_False);
                return Py_False;
            }
        }
    }

    PyErr_SetString(PyExc_AttributeError,
                    "Expected Either Name, InfoElement, or "
                    " InfoElementSpec");
    return NULL;

}

static PyObject *fixbufPyTemplateGetIndexedIE(
    fixbufPyTemplate *self,
    PyObject *arg)
{
    PyObject *pyint_val;
    uint32_t index;
    fixbufPyInfoElement *element = NULL;
    fbInfoElement_t *ie = NULL;

    if (!PyNumber_Check(arg)) {
        PyErr_SetString(PyExc_AttributeError, "Expected Integer");
        return NULL;
    }

    pyint_val = PyNumber_Int(arg);
    if (pyint_val == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Expected Valid Integer");
        return NULL;
    }

    index = PyLong_AsLong(pyint_val);
    Py_XDECREF(pyint_val);

    ie = fbTemplateGetIndexedIE(self->template, index);

    if (!ie) {
        PyErr_SetString(PyExc_IndexError, "Index Out of Bounds");
        return NULL;
    }

    element = (fixbufPyInfoElement *)fixbufPyInfoElementType.tp_new(&fixbufPyInfoElementType, NULL, NULL);

    if (element == NULL) {
        Py_XDECREF(element);
        return PyErr_NoMemory();
    }

    memcpy(element->infoElement, ie, sizeof(fbInfoElement_t));

    /*    Py_INCREF(element);*/

    return (PyObject *)element;
}



static PyGetSetDef fixbufPyTemplate_getsetters[] = {
    {"template_id", (getter)fixbufPyTemplate_gettid, NULL, "Template ID",NULL},
    {"scope", (getter)fixbufPyTemplate_getscope,
     (setter)fixbufPyTemplate_setscope, "Scope", NULL},
    {"type", (getter)fixbufPyTemplate_gettype, NULL, "Type", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyMethodDef fixbufPyTemplate_methods[] = {
    {"addSpec", (PyCFunction)fixbufPyTemplateAddSpec, METH_O,
     ("Add an Information Element Spec to the template")},
    {"__len__", (PyCFunction)fixbufPyTemplateCountElements, METH_NOARGS,
     ("returns number of elements in template")},
    {"containsElement", (PyCFunction)fixbufPyTemplateContainsElement, METH_O,
     ("Determine if an Information Element exists in the Template")},
    {"getIndexedIE", (PyCFunction)fixbufPyTemplateGetIndexedIE, METH_O,
     ("Get the Information Element at the given index in the Template")},
    {NULL, NULL, 0, NULL}
};


static PyTypeObject fixbufPyTemplateType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbTemplateBase",           /*tp_name*/
    sizeof(fixbufPyTemplate),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPyTemplate_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    "fbTemplate",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyTemplate_methods,  /* tp_methods */
    0,                          /* tp_members */
    fixbufPyTemplate_getsetters, /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                     /* tp_dictoffset */
    (initproc)fixbufPyTemplate_init,      /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};


static int fixbufPySession_init(
    fixbufPySession *self,
    PyObject *args,
    PyObject *kwds)
{

    fixbufPyInfoModel *info = NULL;
    static char *kwlist[] = {"model", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwlist,
                                     &fixbufPyInfoModelType, &info))
    {
        PyErr_SetString(PyExc_TypeError, "Expected InfoModel Object");
        return -1;
    }

    if (!fixbufPyInfoModel_Check(info)) {
        PyErr_SetString(PyExc_ValueError, "Expects an InfoModel");
        return -1;
    }

    self->session = fbSessionAlloc(info->infoModel);

    if (self->session == NULL) {
        PyErr_SetString(PyExc_ValueError, "Error creating Session");
        return -1;
    }

    return 0;
}

static PyObject *fixbufPySessionAddTemplate(
    fixbufPySession *self,
    PyObject *args,
    PyObject *kwds)
{

    static char *kwlist[] = {"template", "template_id", "internal", NULL};
    fixbufPyTemplate *tmpl = NULL;
    GError *err = NULL;
    int internal = 1;
    int tid = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii", kwlist,
                                     (PyObject *)&tmpl, &tid, &internal))
    {
        return NULL;
    }
    if (!fixbufPyTemplate_Check(tmpl)) {
        PyErr_SetString(PyExc_TypeError, "Expected Template");
        return NULL;
    }

    tmpl->template_id = fbSessionAddTemplate(self->session, internal, tid,
                                     tmpl->template, &err);

    if ( tmpl->template_id == 0 ) {
        PyErr_Format(PyExc_StandardError,
                     "Error adding template to the session: %s\n",
                     err->message);
        g_clear_error(&err);
    }

    return PyInt_FromLong(tmpl->template_id);
}

static PyObject *fixbufPySessionGetTemplate(
    fixbufPySession *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"template_id", "internal", NULL};
    fixbufPyTemplate *tmpl = NULL;
    GError *err = NULL;
    int tid;
    int internal = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|i", kwlist,
                                     &tid, &internal)) {
        return NULL;
    }

    tmpl = (fixbufPyTemplate *)PyObject_New(fixbufPyTemplate,
                                            &fixbufPyTemplateType);
    if (tmpl == NULL) {
        Py_XDECREF(tmpl);
        return PyErr_NoMemory();
    }

    tmpl->template = fbSessionGetTemplate(self->session, internal, tid, &err);
    if (tmpl->template == NULL) {
        g_clear_error(&err);
        Py_XDECREF(tmpl);
        Py_INCREF(Py_None);
        return Py_None;
    }

    tmpl->template_id = tid;

    /*Py_INCREF(tmpl);*/

    return (PyObject *)tmpl;
}

static int fixbufPySession_setdomain(
    fixbufPySession *self,
    PyObject *value,
    void *cbdata)
{
    uint32_t domain;

    if (!IS_INT(value)) {
        PyErr_SetString(PyExc_TypeError, "Expected an integer");
        return -1;
    }

    domain = PyLong_AsUnsignedLong(value);
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_OverflowError,
                        "Domain value must be a 32-bit integer");
        return -1;
    }

    fbSessionSetDomain(self->session, domain);

    return 0;
}

static PyObject *fixbufPySession_getdomain(
    fixbufPySession *self,
    void *cbdata)
{
    return PyLong_FromUnsignedLong(fbSessionGetDomain(self->session));
}

static PyObject *fixbufPySessionExportTemplates(
    fixbufPySession *self)
{
    GError *err = NULL;

    if (!fbSessionExportTemplates(self->session, &err)) {
        PyErr_Format(PyExc_StandardError, "Error exporting templates: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}
/*
void decodeTemplateCallback(
    fbSession_t *session,
    uint16_t     tid,
    fbTemplate_t *tmpl)
{

    size_t decodesize = PyList_Size(decodeOnlyList);
    PyObject *item;
    int loop;
    int new_tid;

    for (loop = 0; loop < decodesize; loop++) {
        item = PyList_GetItem(decodeOnlyList, loop);
        if (IS_INT(item)) {
            new_tid = PyInt_AsLong(item);
            if (tid == new_tid) {
                fbSessionAddTemplatePair(session, tid, tid);
                return;
            }
        }
    }

}
*/
void ignoreTemplateCallback(
    fbSession_t    *session,
    uint16_t       tid,
    fbTemplate_t   *tmpl)
{
    size_t ignore_size = PyList_Size(ignoreList);
    PyObject *item;
    int loop;
    int new_tid;

    for (loop = 0; loop < ignore_size; loop++) {
        item = PyList_GetItem(ignoreList, loop);
        if (IS_INT(item)) {
            new_tid = PyInt_AsLong(item);
            if (tid == new_tid) {
                fbSessionAddTemplatePair(session, tid, 0);
                return;
            }
        }
    }

    fbSessionAddTemplatePair(session, tid, tid);
}


static PyObject *fixbufPySessionAddDecodeList(
    fixbufPySession *self,
    PyObject *args)
{
    PyObject *list = NULL;
    size_t list_size;
    PyObject *item;
    int loop;
    int new_tid;

    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    if (!PyList_Check(list)) {
        PyErr_SetString(PyExc_AttributeError, "Expected List Object");
        return NULL;
    }

    list_size = PyList_Size(list);

    for (loop = 0; loop < list_size; loop++) {
        item = PyList_GetItem(list, loop);
        if (IS_INT(item)) {
            new_tid = PyInt_AsLong(item);
            fbSessionAddTemplatePair(self->session, new_tid, new_tid);
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *fixbufPySessionAddIgnoreList(
    fixbufPySession *self,
    PyObject *args)
{
    PyObject *list = NULL;

    if (!PyArg_ParseTuple(args, "O", &list)) {
        return NULL;
    }

    if (!PyList_Check(list)) {
        PyErr_SetString(PyExc_AttributeError, "Expected List Object");
        return NULL;
    }

    ignoreList = list;

    Py_INCREF(ignoreList);

    fbSessionAddTemplateCallback(self->session, ignoreTemplateCallback);

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySessionAddTemplatePair(
    fixbufPySession *self,
    PyObject *args,
    PyObject *kwds)
{

    static char *kwlist[] = {"external_template_id", "internal_template_id",
                             NULL};
    int ext_tid;
    int int_tid;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &ext_tid,
                                     &int_tid))
    {
        return NULL;
    }

    if (self->session != NULL) {
        fbSessionAddTemplatePair(self->session, ext_tid, int_tid);
   }

    Py_INCREF(Py_None);

    return Py_None;

}

static PyMethodDef fixbufPySession_methods[] = {
    {"addTemplate", (PyCFunction)fixbufPySessionAddTemplate, METH_KEYWORDS,
     ("Add a template to the session")},
    {"getTemplate", (PyCFunction)fixbufPySessionGetTemplate, METH_KEYWORDS,
     ("Get template in session with template_id")},
    {"export_templates", (PyCFunction)fixbufPySessionExportTemplates,
     METH_NOARGS, ("Export templates associated with a session")},
    {"addTemplatePair", (PyCFunction)fixbufPySessionAddTemplatePair,
     METH_KEYWORDS, ("add an external-internal pair to the session in order "
                     "to decode certain templates appropriately")},
    {"addDecodeList", (PyCFunction)fixbufPySessionAddDecodeList, METH_VARARGS,
     ("Add a list of template ids that should only be transcoded.  Ignore "
      "the rest")},
    {"addIgnoreList", (PyCFunction)fixbufPySessionAddIgnoreList, METH_VARARGS,
     ("Add a list of template ids that should be ingored.  Decode the rest.")},
   {NULL, NULL, 0, NULL}
};

static PyGetSetDef fixbufPySession_getsetters[] = {
    {"domain", (getter)fixbufPySession_getdomain,
     (setter)fixbufPySession_setdomain,
     "Current Observation Domain on Session", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};


static PyTypeObject fixbufPySessionType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbSessionBase",           /*tp_name*/
    sizeof(fixbufPySession),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,               /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbSession",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPySession_methods,    /* tp_methods */
    0,                          /* tp_members */
    fixbufPySession_getsetters, /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPySession_init,       /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};


static PyObject *fixbufPyCollectorAllocFile(
    fixbufPyCollector *self,
    PyObject *args)
{

    char *filename = NULL;
    GError *err = NULL;

    if(!PyArg_ParseTuple(args,"et", Py_FileSystemDefaultEncoding, &filename)) {
        PyErr_SetString(PyExc_AttributeError, "filename");
        return NULL;
    }

    if (filename) {
        self->collector = fbCollectorAllocFile(NULL, filename, &err);
    }

    if (self->collector == NULL) {
        PyErr_Format(PyExc_StandardError,
                     "Error allocating file collector: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyMethodDef fixbufPyCollector_methods[] = {
    {"allocCollectorFile", (PyCFunction)fixbufPyCollectorAllocFile,
     METH_VARARGS,
     ("Allocate a collector for an IPFIX file given input filename")},
    {NULL, NULL, 0, NULL}
};



static PyTypeObject fixbufPyCollectorType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbCollectorBase",     /*tp_name*/
    sizeof(fixbufPyCollector), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,               /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbCollector",              /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyCollector_methods,   /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};

/**
 * fbListener
 *
 */

static PyObject *fixbufPyListenerAlloc(
    fixbufPyListener *self,
    PyObject         *args,
    PyObject         *kwds)
{

    static char *kwlist[] = {"transport", "host", "port", "session", NULL};
    char *transport;
    char *host;
    char *port;
    fixbufPySession *session;
    GError *err = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sssO", kwlist, &transport,
                                     &host, &port, &session)) {
        return NULL;
    }

    if (!fixbufPySession_Check(session)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.fbSession");
        return NULL;
    }

    if (strcmp(transport, "tcp") == 0) {
        self->conn.transport = FB_TCP;
    } else if (strcmp(transport, "udp") == 0) {
        self->conn.transport = FB_UDP;
    } else {
        PyErr_Format(PyExc_StandardError,
                     "%s is not a supported transport.\n",
                     transport);
        return NULL;
    }

    self->conn.svc = port;
    self->conn.host = host;
    self->conn.ssl_ca_file = NULL;
    self->conn.ssl_cert_file = NULL;
    self->conn.ssl_key_file = NULL;
    self->conn.ssl_key_pass = NULL;
    self->conn.vai = NULL;
    self->conn.vssl_ctx = NULL;

    self->listener = fbListenerAlloc(&(self->conn), session->session,
                                     NULL, NULL, &err);

    if (self->listener == NULL) {
        PyErr_Format(PyExc_StandardError,
                     "Error allocating listener: %s\n",
                     err->message);
        g_clear_error(&err);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}



static PyObject *fixbufPyListenerWait(
    fixbufPyListener *self,
    PyObject         *args)
{

    fixbufPyfBuf *buf = NULL;
    fixbufPySession *session = NULL;
    fBuf_t *ret_buf = NULL;
    GError *err = NULL;

    if (!PyArg_ParseTuple(args, "OO", &buf, &session)) {
        return NULL;
    }

    if (!fixbufPyfBuf_Check(buf)) {
        PyErr_SetString(PyExc_TypeError, "Expected Buffer");
        return NULL;
    }

    if (!fixbufPySession_Check(session)) {
        PyErr_SetString(PyExc_TypeError, "Expected Session");
        return NULL;
    }

    if (self->listener == NULL) {
        PyErr_SetString(PyExc_TypeError, "Invalid Listener");
        return NULL;
    }

    ret_buf = fbListenerWait(self->listener, &err);

    if (PyErr_CheckSignals()) {
        return NULL;
    }

    if (ret_buf == NULL) {
        PyErr_Format(PyExc_StandardError, "Error: %s\n", err->message);
        g_clear_error(&err);
        return NULL;
    }

    buf->fbuf = ret_buf;

    Py_INCREF(buf);

    session->session = fBufGetSession(ret_buf);

    return (PyObject *)buf;
}

static PyMethodDef fixbufPyListener_methods[] = {
    {"allocListener", (PyCFunction)fixbufPyListenerAlloc, METH_KEYWORDS,
     ("Allocate a listener for a given host and port")},
    {"listenerWait", (PyCFunction)fixbufPyListenerWait, METH_VARARGS,
     ("Listen on the interface")},
    {NULL, NULL, 0, NULL}
};




static PyTypeObject fixbufPyListenerType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbListenerBase",     /*tp_name*/
    sizeof(fixbufPyListener), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,               /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbListener",              /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyListener_methods,   /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};



static PyObject *fixbufPyExporterAllocFile(
    fixbufPyExporter *self,
    PyObject *args)
{

    char *filename = NULL;

    if (!PyArg_ParseTuple(args, "et", Py_FileSystemDefaultEncoding, &filename))
    {
        return NULL;
    }

    if (filename) {
        self->exporter = fbExporterAllocFile(filename);
    }

    if (self->exporter == NULL) {
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPyExporterAllocNet(
    fixbufPyExporter *self,
    PyObject         *args,
    PyObject         *kwds)
{

    static char *kwlist[] = {"transport", "host", "port", NULL};
    char *transport;
    char *host;
    char *port;
    fbConnSpec_t conn;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwlist, &transport,
                                     &host, &port)) {
        return NULL;
    }

    if (strcmp(transport, "tcp") == 0) {
        conn.transport = FB_TCP;
    } else if (strcmp(transport, "udp") == 0) {
        conn.transport = FB_UDP;
    } else {
        PyErr_Format(PyExc_StandardError,
                     "%s is not a supported transport.\n", transport);
        return NULL;
    }

    conn.svc = port;
    conn.host = host;
    conn.ssl_ca_file = NULL;
    conn.ssl_cert_file = NULL;
    conn.ssl_key_file = NULL;
    conn.ssl_key_pass = NULL;
    conn.vai = NULL;
    conn.vssl_ctx = NULL;


    self->exporter = fbExporterAllocNet(&conn);

    if (self->exporter == NULL) {
        PyErr_Format(PyExc_StandardError,
                     "Problem setting up the Exporter on host %s:%d",
                     host, atoi(port));
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;

}


static PyMethodDef fixbufPyExporter_methods[] = {
    {"allocFile", (PyCFunction)fixbufPyExporterAllocFile, METH_VARARGS,
     ("Allocate an Exporter for an IPFIX file given input filename")},
    {"allocNet", (PyCFunction)fixbufPyExporterAllocNet, METH_KEYWORDS,
     ("Allocate an Exporter for a given hostname and port")},
    {NULL, NULL, 0, NULL}
};


static PyTypeObject fixbufPyExporterType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbExporterBase",     /*tp_name*/
    sizeof(fixbufPyExporter), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    0,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbExporter",              /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyExporter_methods,   /* tp_methods */
    0,                          /* tp_members */
    0,                          /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};

static PyObject *fixbufPyInfoElementSpec_new(
    PyTypeObject *type,
    PyObject     *args,
    PyObject     *kwds)
{
    fixbufPyInfoElementSpec *self;

    self = (fixbufPyInfoElementSpec *)type->tp_alloc(type, 0);

    if (self != NULL) {
        self->spec = PyMem_Malloc(sizeof(*self->spec));
        if (self->spec == NULL) {
            Py_XDECREF((PyObject *)self);
            return PyErr_NoMemory();
        }
        memset(self->spec, 0, sizeof(*self->spec));
    }

    return (PyObject *)self;
}

static void fixbufPyInfoElementSpec_dealloc(
    fixbufPyInfoElementSpec *obj)
{
    if (obj->spec) {
        PyMem_Free(obj->spec);
    }
    Py_TYPE(obj)->tp_free((PyObject *)obj);
}



static int fixbufPyInfoElementSpec_init(
    fixbufPyInfoElementSpec *self,
    PyObject            *args,
    PyObject            *kwds)
{

    static char *kwlist[] = {"name", "length",  NULL};
    char *name;
    int  len = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &name,
                                     &len))
    {
        return -1;
    }

    if (self->spec) {
        if (strlen(name) >= MAX_NAME) {
            name[MAX_NAME-1]='0';
        }
        strcpy(self->infoElementName, name);
        self->spec->name = self->infoElementName;
        self->spec->len_override = len;
    } else {
        return -1;
    }

    return 0;
}

static PyObject *fixbufPyInfoElementSpec_getlength(
    fixbufPyInfoElementSpec *obj,
    void *cbdata)
{
    return PyInt_FromLong(obj->spec->len_override);
}

static PyObject *fixbufPyInfoElementSpec_getname(
    fixbufPyInfoElementSpec *obj,
    void *cbdata)
{
    char name[MAX_NAME];

    memcpy(name, obj->spec->name, strlen(obj->spec->name));
    return PyString_FromStringAndSize(name, strlen(obj->spec->name));
}



static PyGetSetDef fixbufPyInfoElementSpec_getsetters[] = {
    {"length", (getter)fixbufPyInfoElementSpec_getlength, NULL,
     "Info Element Spec Length", NULL},
    {"name", (getter)fixbufPyInfoElementSpec_getname, NULL,
     "Info Element Spec Name", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};


static PyTypeObject fixbufPyInfoElementSpecType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbInfoElementSpecBase",  /*tp_name*/
    sizeof(fixbufPyInfoElementSpec), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPyInfoElementSpec_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbInfoElementSpec",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    fixbufPyInfoElementSpec_getsetters, /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPyInfoElementSpec_init,  /* tp_init */
    0,                          /* tp_alloc */
    fixbufPyInfoElementSpec_new,  /* tp_new */
    0                           /* tp_free */
};


static PyObject *fixbufPySTML_getsemantic(
    fixbufPySTML *self,
    void *cbdata)
{
    long rv = 0;

    if (self->stml) {
        return PyInt_FromLong(fbSubTemplateMultiListGetSemantic(self->stml));
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyObject *fixbufPySTML_getnumelements(
    fixbufPySTML *self,
    void *cbdata)
{
    long rv = 0;


    if (self->stml) {
        return PyInt_FromLong(self->stml->numElements);
    } else {
        return PyInt_FromLong(rv);
    }
}

static int fixbufPySTML_setsemantic(
    fixbufPySTML *self,
    PyObject *value,
    void *cbdata)
{
    uint8_t semantic;

    if (!IS_INT(value)) {
        PyErr_SetString(PyExc_AttributeError, "Expected An Integer");
        return -1;
    }

    semantic = PyLong_AsLong(value);
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_OverflowError,
                        "Semantic value must be an 8-bit integer");
        return -1;
    }

    if (self->stml) {
        fbSubTemplateMultiListSetSemantic(self->stml, semantic);
    }

    return 0;
}


static PyObject *fixbufPySTML_clear(
    fixbufPySTML      *self,
    PyObject          *args)
{

    int offset = 0;
    fixbufPyRecord *rec = NULL;

    if (!PyArg_ParseTuple(args, "|Oi", &rec, &offset)) {
        return NULL;
    }

    if (offset) {
        if (!fixbufPyRecord_Check(rec)) {
            PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.Record");
            return NULL;
        }
        self->stml = (fbSubTemplateMultiList_t *)(rec->rec + offset);
    }


    fbSubTemplateMultiListClear(self->stml);
    self->entry = NULL;
    self->stml = NULL;

    Py_INCREF(Py_None);
    return Py_None;
}


/*
static PyObject *fixbufPySTML_getRecord(
    fixbufPySTML *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;

    if (!PyArg_ParseTuple(args, "O", &rec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected pyfixbuf.rec");
        return NULL;
    }

    if (rec == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Invalid Record - Null record");
        return NULL;
    }

    if (!self) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateMultiList - NULL STML");
        return NULL;
    }

    if (self->entry == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    rec->rec = fbSubTemplateMultiListEntryNextDataPtr(self->entry, rec->rec);

    if (rec->rec == NULL) {
          Py_INCREF(Py_False);
        return Py_False;
    }

    Py_INCREF(Py_True);

    return Py_True;
}

*/
static PyObject *fixbufPySTML_getNextEntry(
    fixbufPySTML        *self,
    PyObject            *args)
{

    fixbufPyRecord *rec = NULL;
    int offset = 0;

    if (!PyArg_ParseTuple(args, "Oi", &rec, &offset)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    self->stml = (fbSubTemplateMultiList_t *)(rec->rec + offset);

    self->entry =
        fbSubTemplateMultiListGetNextEntry(self->stml, self->entry);

    if (self->entry == NULL) {
        /* We are done processing this list */
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPySTML_getFirstEntry(
    fixbufPySTML        *self,
    PyObject            *args)
{

    fixbufPyRecord *rec = NULL;
    int offset = 0;

    if (!PyArg_ParseTuple(args, "Oi", &rec, &offset)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    self->stml = (fbSubTemplateMultiList_t *)(rec->rec + offset);

    self->entry =
        fbSubTemplateMultiListGetFirstEntry(self->stml);

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySTML_getIndex(
    fixbufPySTML        *self,
    PyObject            *args)
{

    int index = 0;
    fbSubTemplateMultiListEntry_t *entry = NULL;

    if (!PyArg_ParseTuple(args, "i", &index)) {
        return NULL;
    }

    if (self->stml == NULL) {
        PyErr_SetString(PyExc_ValueError, "STML was not initialized.");
        return NULL;
    }

    entry = fbSubTemplateMultiListGetIndexedEntry(self->stml, index);

    if (entry == NULL) {
        PyErr_SetString(PyExc_IndexError, "Index Out of Bounds");
        return NULL;
    }

    self->entry = entry;

    Py_INCREF(Py_None);

    return Py_None;
}

static void fixbufPySTML_dealloc(
    fixbufPySTML *obj)
{
    if (obj->stml_alloc) {
        PyMem_Free(obj->stml);
    }
    Py_TYPE(obj)->tp_free((PyObject *)obj);
}

static int fixbufPySTML_init(
    fixbufPySTML *self,
    PyObject     *args,
    PyObject     *kwds)
{
    int tmplcount = -1;
    int offset = 0;
    fixbufPyRecord *rec = NULL;

    if (!PyArg_ParseTuple(args, "|Oii", &rec, &offset, &tmplcount)) {
        return -1;
    }

    /* get stml */
    if (fixbufPyRecord_Check(rec)) {
        self->stml = (fbSubTemplateMultiList_t *)(rec->rec + offset);
        self->entry = NULL;
        return 0;
    }

    if (tmplcount > -1) {

        /*if (rec->rec == NULL) {
          rec->rec = PyMem_Malloc(rec->reclen);
          if (!rec->rec) {
          Py_DECREF((PyObject *)rec);
          PyErr_NoMemory();
          return -1;
          }
          rec->memalloc = TRUE;
                }*/
        self->stml = PyMem_Malloc(sizeof(fbSubTemplateMultiList_t));
        if (!self->stml) {
            Py_XDECREF((PyObject *)self->stml);
            PyErr_NoMemory();
            return -1;
        }
        memset(self->stml, 0, sizeof(fbSubTemplateMultiList_t));
        self->stml_alloc = TRUE;
        fbSubTemplateMultiListInit(self->stml, 0, tmplcount);

    } else {

        self->stml = NULL;
    }

    self->entry = NULL;

    return 0;
}


static PyObject *fixbufPySTML_rewind(
    fixbufPySTML *self)
{

    self->entry = NULL;

    Py_INCREF(Py_None);
    return Py_None;

}


static PyMethodDef fixbufPySTML_methods[] = {
    {"getNextEntry", (PyCFunction)fixbufPySTML_getNextEntry, METH_VARARGS,
     ("Get Next SubTemplateMultiList in the list - or NULL if done")},
    {"clear", (PyCFunction)fixbufPySTML_clear, METH_VARARGS,
     ("Clear the STML")},
    {"getFirstEntry", (PyCFunction)fixbufPySTML_getFirstEntry, METH_VARARGS,
     ("Set the entry pointer to the first item in the list.")},
    {"rewind", (PyCFunction)fixbufPySTML_rewind, METH_NOARGS,
     ("Rewind entry pointer to first item in the list.")},
    {"getIndex", (PyCFunction)fixbufPySTML_getIndex, METH_VARARGS,
     ("Get the Indexed Entry in the list.")},
    {NULL, NULL, 0, NULL}
};


static PyGetSetDef fixbufPySTML_getsetters[] = {
    {"semantic", (getter)fixbufPySTML_getsemantic,
     (setter)fixbufPySTML_setsemantic, "Get/Set STML Semantic", NULL},
    {"count", (getter)fixbufPySTML_getnumelements,
     NULL, "Get the number of entries in the list.", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyTypeObject fixbufPySTMLType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbSTMLBase",     /*tp_name*/
    sizeof(fixbufPySTML),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPySTML_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbSTML",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPySTML_methods,       /* tp_methods */
    0,                          /* tp_members */
    fixbufPySTML_getsetters,    /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPySTML_init,  /* tp_init */
    0,                          /* tp_alloc */
    0,                          /* tp_new */
    0                           /* tp_free */
};




static PyObject *fixbufPySTMLEntry_entryInit(
    fixbufPySTMLEntry *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"record", "template", "template_id", "count",
                             NULL};
    fixbufPyTemplate *tmpl = NULL;
    fixbufPyRecord *rec = NULL;
    int tid;
    int count;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOii", kwlist, &rec,
                                     &tmpl, &tid, &count))
    {
        return NULL;
    }

    if (!fixbufPyTemplate_Check(tmpl)) {
        PyErr_SetString(PyExc_TypeError, "Expected Template");
        return NULL;
    }

    fbSubTemplateMultiListEntryInit(self->entry, tid, tmpl->template,
                                    count);
    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySTMLEntry_getNextRecord(
    fixbufPySTMLEntry *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;

    if (!PyArg_ParseTuple(args, "O", &rec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (rec == NULL) {
        PyErr_SetString(PyExc_AttributeError, "Invalid Record - Null record");
        return NULL;
    }

    if (!self) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateMultiList "
                        " - NULL STML");
        return NULL;
    }

    if (self->entry == NULL) {
        /* getNextEntry - should have returned False */
        Py_INCREF(Py_False);
        return Py_False;
    }

    rec->rec = fbSubTemplateMultiListEntryNextDataPtr(self->entry, rec->rec);

    if (rec->rec == NULL) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySTMLEntry_setIndexedEntry(
    fixbufPySTMLEntry *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;
    uint8_t *dataptr;
    int index;

    if (!PyArg_ParseTuple(args, "iO", &index, &rec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (!self) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateMultiList "
                        " - NULL STML");
        return NULL;
    }

    if (self->entry == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    dataptr = fbSubTemplateMultiListEntryGetIndexedPtr(self->entry, index);

    if (dataptr == NULL) {
        PyErr_SetString(PyExc_IndexError, "Out of Bounds");
        return NULL;
    }

    memcpy(dataptr, rec->rec, rec->reclen);

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPySTMLEntry_getIndexedEntry(
    fixbufPySTMLEntry *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;
    int index;

    if (!PyArg_ParseTuple(args, "Oi", &rec, &index)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (!self) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateMultiList "
                        " - NULL STML");
        return NULL;
    }

    if (self->entry == NULL) {
        Py_INCREF(Py_False);
        return Py_False;
    }

    rec->rec = fbSubTemplateMultiListEntryGetIndexedPtr(self->entry, index);

    if (rec->rec == NULL) {
        PyErr_SetString(PyExc_IndexError, "Index Out of Bounds");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySTMLEntry_containsElement(
    fixbufPySTMLEntry *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"model", "name", NULL};
    char *name;
    fixbufPyInfoModel *model;
    PyObject *result;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os", kwlist, &model,
                                     &name))
    {
        return NULL;
    }

    if (!fixbufPyInfoModel_Check(model)) {
        PyErr_SetString(PyExc_TypeError, "Expected InfoModel");
        return NULL;
    }

    if (!self->entry) {
        result = Py_False;
        goto done;
    }

    if (!model->infoModel) {
        PyErr_SetString(PyExc_AttributeError, "Invalid InfoModel - NULL");
        return NULL;
    }

    if (fbSubTemplateMultiListEntryGetTemplate(self->entry)) {
        if (fbTemplateContainsElement(
                (fbTemplate_t *)fbSubTemplateMultiListEntryGetTemplate(self->entry),
                fbInfoModelGetElementByName(model->infoModel, (const char *)name)))
        {
            result = Py_True;
        } else {
            result = Py_False;
        }
    } else {
        result = Py_False;
    }

done:
    Py_INCREF(result);

    return result;

}

static int fixbufPySTMLEntry_init(
    fixbufPySTMLEntry *self,
    PyObject     *args,
    PyObject     *kwds)
{
    fixbufPySTML *stml = NULL;
    static char *kwlist[] = {"stml", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &stml))
    {
        return -1;
    }

    if (!fixbufPySTML_Check(stml)) {
        PyErr_SetString(PyExc_TypeError, "Expected STML");
        return -1;
    }

    self->entry = stml->entry;

    return 0;
}


static PyObject *fixbufPySTMLEntry_gettid(
    fixbufPySTMLEntry *self,
    void *cbdata)
{
    long rv = 0;

    if (self) {
        return PyInt_FromLong(fbSubTemplateMultiListEntryGetTemplateID(self->entry));
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyObject *fixbufPySTMLEntry_getcount(
    fixbufPySTMLEntry *self,
    void *cbdata)
{
    long rv = 0;

    if (self) {
        return PyInt_FromLong(self->entry->numElements);
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyGetSetDef fixbufPySTMLEntry_getsetters[] = {
    {"template_id", (getter)fixbufPySTMLEntry_gettid, NULL,
     "SubTemplateMultiList Entry Template ID", NULL},
    {"count", (getter)fixbufPySTMLEntry_getcount, NULL,
     "Get the number of entries in the list", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyMethodDef fixbufPySTMLEntry_methods[] = {
    {"containsElement", (PyCFunction)fixbufPySTMLEntry_containsElement,
     METH_KEYWORDS,
     ("Find out if the entry contains the element, returns True or False")},
    {"getNextRecord", (PyCFunction)fixbufPySTMLEntry_getNextRecord,
     METH_VARARGS,
     ("Get the element we want out of the STML, returns 0 if Does not exist")},
    {"setIndexedEntry", (PyCFunction)fixbufPySTMLEntry_setIndexedEntry,
     METH_VARARGS,
     ("Set the given Record at the appropriate entry")},
    {"getIndexedEntry", (PyCFunction)fixbufPySTMLEntry_getIndexedEntry,
     METH_VARARGS,
     ("Get the Record at the appropriate entry")},
/*    {"clear", (PyCFunction)fixbufPySTML_clear, METH_VARARGS,
      ("Clear the STML")},*/
    {"entryInit", (PyCFunction)fixbufPySTMLEntry_entryInit, METH_KEYWORDS,
     ("Initialize Entry in STML for Export")},
    {NULL, NULL, 0, NULL}
};



static PyTypeObject fixbufPySTMLEntryType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbSTMLEntryBase",  /*tp_name*/
    sizeof(fixbufPySTMLEntry), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,                /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "fbSTMLEntry",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPySTMLEntry_methods,       /* tp_methods */
    0,                          /* tp_members */
    fixbufPySTMLEntry_getsetters,    /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPySTMLEntry_init,  /* tp_init */
    0,                          /* tp_alloc */
    0,  /* tp_new */
    0                           /* tp_free */
};



static PyObject *fixbufPyVarfield_getvarfield(
    fixbufPyVarfield *self,
    void *cbdata)
{
    return PyString_FromStringAndSize((char *)self->varfield->buf,
                                      self->varfield->len);
}

static PyObject *fixbufPyVarfield_getlength(
    fixbufPyVarfield *self,
    void *cbdata)
{
    return PyLong_FromLong(self->varfield->len);
}


static PyGetSetDef fixbufPyVarfield_getset[] = {
    {"string", (getter)fixbufPyVarfield_getvarfield, NULL,
     "Get Variable Length Field Buffer as String", NULL},
    {"length", (getter)fixbufPyVarfield_getlength, NULL,
     "Get Variable Length Field Length", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};


static PyTypeObject fixbufPyVarfieldType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbVarfieldBase",  /*tp_name*/
    sizeof(fixbufPyVarfield), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    obj_dealloc,                /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "fbVarfield",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    0,                          /* tp_methods */
    0,                          /* tp_members */
    fixbufPyVarfield_getset,    /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    0,                          /* tp_init */
    0,                          /* tp_alloc */
    0,  /* tp_new */
    0                           /* tp_free */
};


static int fixbufPySTL_init(
    fixbufPySTL *self,
    PyObject     *args,
    PyObject     *kwds)
{

    static char *kwlist[] = {"rec", "offset", NULL};
    fixbufPyRecord *rec = NULL;
    int offset = 0;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oi", kwlist, &rec,
                                     &offset)) {
        return -1;
    }

    if (rec) {
        if (fixbufPyRecord_Check(rec)) {
            /* set the subtemplate list to the first one */
            if (rec->rec == NULL) {
                rec->rec = PyMem_Malloc(rec->reclen);
                if (!rec->rec) {
                    Py_XDECREF((PyObject *)rec);
                    PyErr_NoMemory();
                    return -1;
                }
                rec->memalloc = TRUE;
            }
            self->stl = (fbSubTemplateList_t *)(rec->rec + offset);
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected Valid Record");
            return -1;
        }
    } else {
        self->stl = PyMem_Malloc(sizeof(fbSubTemplateList_t));
        if (!self->stl) {
            Py_XDECREF((PyObject *)self->stl);
            PyErr_NoMemory();
            return -1;
        }
        self->stl_alloc = TRUE;
        memset(self->stl, 0, sizeof(fbSubTemplateList_t));
    }

    return 0;
}


static void fixbufPySTL_dealloc(
    fixbufPySTL *obj)
{
    if (obj->stl_alloc) {
        PyMem_Free(obj->stl);
    }
    Py_TYPE(obj)->tp_free((PyObject *)obj);
}


static PyObject *fixbufPySTL_containsElement(
    fixbufPySTL *self,
    PyObject *args,
    PyObject *kwds)
{
    static char *kwlist[] = {"model", "name", NULL};
    char *name;
    fixbufPyInfoModel *model;
    PyObject *result;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Os", kwlist, &model,
                                     &name))
    {
        return NULL;
    }

    if (!fixbufPyInfoModel_Check(model)) {
        PyErr_SetString(PyExc_TypeError, "Expected InfoModel");
        return NULL;
    }

    if (!self->stl) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateList = NULL");
        return NULL;
    }

    if (!model->infoModel) {
        PyErr_SetString(PyExc_AttributeError, "Invalid InfoModel = NULL");
        return NULL;
    }

    if (fbTemplateContainsElement(
            (fbTemplate_t *)fbSubTemplateListGetTemplate(self->stl),
            fbInfoModelGetElementByName(model->infoModel, (const char *)name)))
    {
        result = Py_True;
    } else {
        result = Py_False;
    }

    Py_INCREF(result);

    return result;

}


static PyObject *fixbufPySTL_getNextEntry(
    fixbufPySTL        *self,
    PyObject            *args,
    PyObject            *kwds)
{

    static char *kwlist[] = {"rec", NULL};
    fixbufPyRecord *rec;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &rec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (!self->stl) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    rec->rec =
        fbSubTemplateListGetNextPtr((fbSubTemplateList_t *)
                                    self->stl,
                                    rec->rec);

    if (!rec->rec) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}


static PyObject *fixbufPySTL_clear(
    fixbufPySTL      *self,
    PyObject         *args)
{

    int offset = -1;
    fixbufPyRecord *rec = NULL;


    if (!PyArg_ParseTuple(args, "|Oi", &rec, &offset)) {
        return NULL;
    }

    if (offset != -1) {
        self->stl = (fbSubTemplateList_t *)(rec->rec + offset);
    }

    fbSubTemplateListClear(self->stl);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *fixbufPySTL_gettid(
    fixbufPySTL *self,
    void *cbdata)
{
    long rv = 0;

    if (self->stl) {
        return PyInt_FromLong(fbSubTemplateListGetTemplateID(self->stl));
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyObject *fixbufPySTL_entryInit(
    fixbufPySTL *self,
    PyObject *args,
    PyObject *kwds)
{

    static char *kwlist[] = {"template", "template_id", "count", NULL};
    fixbufPyTemplate *tmpl = NULL;
    int tid;
    int count;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oii", kwlist,
                                     &tmpl, &tid, &count))
    {
        return NULL;
    }

    if (!fixbufPyTemplate_Check(tmpl)) {
        PyErr_SetString(PyExc_TypeError, "Expected Template");
        return NULL;
    }

    fbSubTemplateListInit(self->stl, 0, tid, tmpl->template, count);

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPySTL_getcount(
    fixbufPySTL *self,
    void *cbdata)
{
    long rv = 0;

    if (self->stl) {
        return PyInt_FromLong(self->stl->numElements);
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyObject *fixbufPySTL_getsemantic(
    fixbufPySTL *self,
    void *cbdata)
{
    if (self->stl) {
        return PyInt_FromLong(fbSubTemplateListGetSemantic(self->stl));
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static int fixbufPySTL_setsemantic(
    fixbufPySTL *self,
    PyObject *value,
    void *cbdata)
{
    uint8_t semantic;

    if (!IS_INT(value)) {
        PyErr_SetString(PyExc_AttributeError, "Expected An Integer");
        return -1;
    }

    semantic = PyLong_AsLong(value);
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_OverflowError,
                        "Semantic value must be an 8-bit integer");
        return -1;
    }

    if (self->stl) {
        fbSubTemplateListSetSemantic(self->stl, semantic);
    } else {
        PyErr_SetString(PyExc_StandardError, "entry_init() must be called "
                        " on STL before semantic can be set.");
        return -1;
    }

    return 0;
}

static PyObject *fixbufPySTL_getIndexedEntry(
    fixbufPySTL *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;
    int index;

    if (!PyArg_ParseTuple(args, "Oi", &rec, &index)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (!self->stl) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateList "
                        " - NULL STL");
        return NULL;
    }

    rec->rec = fbSubTemplateListGetIndexedDataPtr(self->stl, index);

    if (rec->rec == NULL) {
        PyErr_SetString(PyExc_IndexError, "Index Out of Bounds");
        return NULL;
    }

    Py_INCREF(Py_None);

    return Py_None;
}

static PyObject *fixbufPySTL_setIndexedEntry(
    fixbufPySTL *self,
    PyObject *args)
{
    fixbufPyRecord *rec = NULL;
    uint8_t *dataptr;
    int index;

    if (!PyArg_ParseTuple(args, "iO", &index, &rec)) {
        return NULL;
    }

    if (!fixbufPyRecord_Check(rec)) {
        PyErr_SetString(PyExc_TypeError, "Expected Record");
        return NULL;
    }

    if (!self->stl) {
        PyErr_SetString(PyExc_AttributeError, "Invalid SubTemplateList "
                        " - NULL STL");
        return NULL;
    }

    dataptr = fbSubTemplateListGetIndexedDataPtr(self->stl, index);

    if (dataptr == NULL) {
        PyErr_SetString(PyExc_IndexError, "Out of Bounds");
        return NULL;
    }

    memcpy(dataptr, rec->rec, rec->reclen);

    Py_INCREF(Py_None);

    return Py_None;
}


static PyMethodDef fixbufPySTL_methods[] = {
    {"getNext", (PyCFunction)fixbufPySTL_getNextEntry, METH_KEYWORDS,
     ("Get Next SubTemplateList Entry in the list - or NULL if done")},
    {"clear", (PyCFunction)fixbufPySTL_clear, METH_VARARGS,
     ("Clear the STL")},
    {"getIndexedEntry", (PyCFunction)fixbufPySTL_getIndexedEntry, METH_VARARGS,
     ("get the data ptr for the given index.")},
    {"setIndexedEntry", (PyCFunction)fixbufPySTL_setIndexedEntry, METH_VARARGS,
     ("set the data ptr at the index to the given record.")},
    {"containsElement", (PyCFunction)fixbufPySTL_containsElement,METH_KEYWORDS,
     ("Find out if the entry contains the element, returns True or False")},
    {"entryInit", (PyCFunction)fixbufPySTL_entryInit, METH_KEYWORDS,
     ("Initialize the STL for Export")},
    {NULL, NULL, 0, NULL}
};


static PyGetSetDef fixbufPySTL_getsetters[] = {
    {"template_id", (getter)fixbufPySTL_gettid, NULL,
     "SubTemplateList Template ID", NULL},
    {"count", (getter)fixbufPySTL_getcount, NULL,
     "Get number of entries in list", NULL},
    {"semantic", (getter)fixbufPySTL_getsemantic,
     (setter)fixbufPySTL_setsemantic,
     "Get/Set the Semantic Value on the SubTemplateList", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};


static PyTypeObject fixbufPySTLType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbSTLBase",  /*tp_name*/
    sizeof(fixbufPySTL), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPySTL_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "fbSTL",               /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPySTL_methods,        /* tp_methods */
    0,                          /* tp_members */
    fixbufPySTL_getsetters,     /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPySTL_init, /* tp_init */
    0,                          /* tp_alloc */
    0,  /* tp_new */
    0                           /* tp_free */
    };


static PyObject *fixbufPyBL_new(
    PyTypeObject *type,
    PyObject     *args,
    PyObject     *kwds)
{
    fixbufPyBL *self;

    self = (fixbufPyBL *)type->tp_alloc(type, 0);

    if (self != NULL) {
        self->bl = PyMem_Malloc(sizeof(fbBasicList_t));
        if (self->bl == NULL) {
            Py_XDECREF((PyObject *)self);
            return PyErr_NoMemory();
        }
        memset(self->bl, 0, sizeof(fbBasicList_t));
    }
    self->bl_alloc = TRUE;
    return (PyObject *)self;
}

static void fixbufPyBL_dealloc(
    fixbufPyBL *obj)
{
    if (obj->bl_alloc) {
        PyMem_Free(obj->bl);
    }
    Py_TYPE(obj)->tp_free((PyObject *)obj);
}

static int fixbufPyBL_init(
    fixbufPyBL   *self,
    PyObject     *args,
    PyObject     *kwds)
{

    static char *kwlist[] = {"element", "count", "semantic", NULL};
    fixbufPyInfoElement *ie = NULL;
    int count = 0;
    int semantic = 0;


    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oii", kwlist, &ie,
                                     &count, &semantic)) {
        return -1;
    }

    if (count < 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Count must be greater or equal to 0.");
        return -1;
    }

    if (ie == NULL) {
        return 0;
    }

    if (!(fixbufPyInfoElement_Check(ie))) {
        return 0;
    } else {
        if (self->bl_alloc) {
            fbBasicListInit(self->bl, semantic, ie->ptr, count);
            self->init = TRUE;
        }
    }

    return 0;
}

static PyObject *fixbufPyBL_getitems(
    fixbufPyBL *self,
    PyObject *args)
{

    PyObject *list;
    fbBasicList_t *bl = self->bl;
    const fbInfoElement_t *ie = NULL;
    Py_ssize_t i;
    uint8_t type;
    int w;
    uint16_t len;

    if (self->bl == NULL) {
        PyErr_SetString(PyExc_ValueError, "Null BasicList");
        return NULL;
    }

    list = PyList_New(bl->numElements);

    if (list == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not create List Object");
        return NULL;
    }

    ie = fbBasicListGetInfoElement(bl);

    if (ie == NULL) {
        PyErr_SetString(PyExc_ValueError,
                        "Null InfoElement associated with BL");
        return NULL;
    }

    type = ie->type;
    len = ie->len;

    if (len == FB_IE_VARLEN) {

        fbVarfield_t *item;
        for (w=0;(item=(fbVarfield_t *)fbBasicListGetIndexedDataPtr(bl,w));w++)
        {
            i = w;
            PyList_SetItem(list, i,
                           PyString_FromStringAndSize((char *)item->buf,
                                                      item->len));
        }
    } else {

        switch (type) {
          case FB_UINT_8:
          case FB_BOOL:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyInt_FromLong(*item));
                }
                break;
            }
          case FB_UINT_16:
            {
                uint16_t *item;
                for (w=0; (item =(uint16_t *)fbBasicListGetIndexedDataPtr(bl, w)); w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyLong_FromLong(*item));
                }
                break;
            }
          case FB_UINT_32:
          case FB_IP4_ADDR:
          case FB_DT_SEC:
            {
                uint32_t *item;
                for (w=0;
                     (item =(uint32_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    PyList_SetItem(list,i, PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_UINT_64:
          case FB_DT_MILSEC:
          case FB_DT_MICROSEC:
          case FB_DT_NANOSEC:
            {
                uint64_t *item;
                for (w = 0;
                     (item = (uint64_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    PyList_SetItem(list,i, PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_INT_8:
            {
                int8_t *item;
                for (w=0; (item =
                           (int8_t *)fbBasicListGetIndexedDataPtr(bl, w)); w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyInt_FromLong(*item));
                }
                break;

            }
          case FB_INT_16:
            {
                int16_t *item;
                for (w=0; (item =
                           (int16_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i, PyLong_FromLong(*item));
                }
                break;
            }
          case FB_INT_32:
            {
                int32_t *item;
                for (w=0; (item =
                           (int32_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_INT_64:
            {
                int64_t *item;
                for (w=0; (item =
                           (int64_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,PyLong_FromUnsignedLongLong(*item));
                }
                break;
            }
          case FB_FLOAT_32:
          case FB_FLOAT_64:
            {
                uint8_t *item;
                PyObject *o;
                for (w=0;
                     (item = (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));
                     w++)
                {
                    i = w;
                    o = PyString_FromStringAndSize((char *)item, len);
                    if (PyErr_Occurred()) {
                        Py_XDECREF(o);
                        return NULL;
                    }
                    PyList_SetItem(list, i,
                                   PyFloat_FromString(o, NULL));
                }
                break;
            }
          case FB_IP6_ADDR:
          case FB_STRING:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,
                                   PyString_FromStringAndSize((char *)item,
                                                              len));
                }
                break;
            }
          case FB_OCTET_ARRAY:
          case FB_MAC_ADDR:
          default:
            {
                uint8_t *item;
                for (w=0; (item =
                           (uint8_t *)fbBasicListGetIndexedDataPtr(bl, w));w++)
                {
                    i = w;
                    PyList_SetItem(list, i,
                                   PyByteArray_FromStringAndSize((char *)item,
                                                                 len));
                }
                break;
            }
        }
    }

    return list;
}

static PyObject *fixbufPyBL_setitems(
    fixbufPyBL *self,
    PyObject *args)
{

    PyObject *value;
    fbVarfield_t *varfield;
    const fbInfoElement_t * ie;
    int index = 0;
    char *string;
    uint8_t type;
    uint16_t len;
    fbBasicList_t *bl = self->bl;
    uint64_t val = 0;

    if (!PyArg_ParseTuple(args, "iO", &index, &value)) {
        return NULL;
    }

    if (self->bl == NULL) {
        PyErr_SetString(PyExc_ValueError, "Null BasicList: BL must be "
                        "initialized before setting.");
        return NULL;
    }

    if (self->init == FALSE) {
        PyErr_SetString(PyExc_AttributeError, "Basic List must be initialized "
                        "with InfoElement before setting.");
        return NULL;
    }


    if ( index >= self->bl->numElements ) {
        PyErr_Format(PyExc_ValueError,
                     "Invalid: Trying to add item %d to BasicList of "
                     "size %d", index + 1,
                     self->bl->numElements);
        return NULL;
    }

    ie = self->bl->infoElement;

    if (ie == NULL) {
        PyErr_SetString(PyExc_ValueError, "No InfoElement associated with "
                        "basicList.");
        return NULL;
    }


    len = ie->len;
    type = ie->type;

    if (IS_STRING(value) && (type != FB_IP6_ADDR)) {
        if (fixbufPyVarfield_Check(value)) {
            varfield  = (fbVarfield_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (varfield == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }
            memcpy(varfield,
                   ((fixbufPyVarfield *)(value))->varfield,
                   sizeof(fbVarfield_t));
        } else {
            varfield = (fbVarfield_t *)fbBasicListGetIndexedDataPtr(bl, index);
            string = PyString_AsString(value);
            varfield->len = strlen(string);
            varfield->buf = (uint8_t*)string;
        }

    } else if (IS_INT(value) || (type == FB_IP6_ADDR)) {
        if (len != 16) {
            val = LONG_AS_UNSIGNED_LONGLONG(value);
        }
        if (PyErr_Occurred()) {
            PyErr_Format(PyExc_ValueError,
                         "Value provided is not of long type.  "
                         "Try to convert using long()");
            return NULL;
        }
        if (len == 1) {
            uint8_t *blval = (uint8_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }
            if (val > (uint64_t)UINT8_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 8-bit integer");
                return NULL;
            } else {
                *blval = val;
            }
        } else if (len == 2) {
            uint16_t *blval = (uint16_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }
            if (val > (uint64_t)UINT16_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 16-bit integer");
                return NULL;
            } else {
                *blval = (uint16_t)val;
            }
        } else if (len == 4) {
            uint32_t *blval = (uint32_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);

                return NULL;
            }
            if (val > (uint64_t)UINT32_MAX) {
                PyErr_SetString(PyExc_ValueError, "Expected a 32-bit integer");
                return NULL;
            } else {
                *blval = (uint32_t)val;
            }
        } else if (len == 8) {
            uint64_t *blval = (uint64_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }
            memcpy(blval, &val, sizeof(uint64_t));
        } else if (len == 16) {
            uint8_t *blval = (uint8_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }
            string = PyString_AsString(value);
            memcpy(blval, string, len);
        } else {
            uint8_t *blval = (uint8_t *)fbBasicListGetIndexedDataPtr(bl, index);
            if (blval == NULL) {
                PyErr_Format(PyExc_ValueError,
                             "Invalid Set for Item %d in BasicList", index);
                return NULL;
            }

            PyString_AsStringAndSize(value, &string, (Py_ssize_t *)&len);
            memcpy(blval, string, len);
        }
    } else if (IS_BYTE(value)) {
        char *bytes = PyByteArray_AsString(value);
        uint8_t *blval =
            (uint8_t *)fbBasicListGetIndexedDataPtr(bl, index);
        if (blval == NULL) {
            PyErr_Format(PyExc_ValueError,
                         "Invalid Set for Item %d in BasicList",index);

            return NULL;
        }
        memcpy(blval, (uint8_t*)bytes, len);
    } else if (type == FB_SUB_TMPL_MULTI_LIST) {
        fbSubTemplateMultiList_t *stml = NULL;
        if (!fixbufPySTML_Check(value)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Value must be a STML");
            return NULL;
        }
        stml = (fbSubTemplateMultiList_t *)fbBasicListGetIndexedDataPtr(bl, index);
        if (stml == NULL) {
            PyErr_Format(PyExc_ValueError,
                         "Invalid Set for Item %d in BasicList", index);
            return NULL;
        }
        memcpy(stml, ((fixbufPySTML *)(value))->stml,
               sizeof(fbSubTemplateMultiList_t));
        if (((fixbufPySTML *)(value))->stml_alloc) {
            /* free fbsubtemplatemultilist*/
            PyMem_Free(((fixbufPySTML *)(value))->stml);
            ((fixbufPySTML *)(value))->stml_alloc = FALSE;
        }
        ((fixbufPySTML *)(value))->stml = (fbSubTemplateMultiList_t*)(stml);

    } else if (type == FB_SUB_TMPL_LIST) {
        fbSubTemplateList_t *stl = NULL;

        if (!fixbufPySTL_Check(value)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Value must be a STL");
            return NULL;
        }

        stl = (fbSubTemplateList_t *)fbBasicListGetIndexedDataPtr(bl, index);
        if (stl == NULL) {
            PyErr_Format(PyExc_ValueError,
                         "Invalid Set for Item %d in BasicList", index);
            return NULL;
        }
        memcpy(stl, ((fixbufPySTL *)(value))->stl,
               sizeof(fbSubTemplateList_t));

        if (((fixbufPySTL *)(value))->stl_alloc) {
            /* free fbsubtemplatelist*/
            PyMem_Free(((fixbufPySTL *)(value))->stl);
            ((fixbufPySTL *)(value))->stl_alloc = FALSE;
        }
        ((fixbufPySTL *)(value))->stl = (fbSubTemplateList_t*)(stl);
    } else {
        PyErr_SetString(PyExc_AttributeError,
                        "value argument must not of appropriate type for basicList");
        return NULL;
    }

    if (PyErr_Occurred()) {
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *fixbufPyBL_getcount(
    fixbufPyBL *self,
    void *cbdata)
{
    long rv = 0;

    if (self->bl) {
        return PyInt_FromLong(self->bl->numElements);
    } else {
        return PyInt_FromLong(rv);
    }
}

static PyObject *fixbufPyBL_getsemantic(
    fixbufPyBL *self,
    void *cbdata)
{
    if (self->bl) {
        return PyInt_FromLong(fbBasicListGetSemantic(self->bl));
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static int fixbufPyBL_setsemantic(
    fixbufPyBL *self,
    PyObject *value,
    void *cbdata)
{
    uint8_t semantic;

    if (!IS_INT(value)) {
        PyErr_SetString(PyExc_AttributeError, "Expected An Integer");
        return -1;
    }

    semantic = PyLong_AsLong(value);
    if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_OverflowError,
                        "Semantic value must be an 8-bit integer");
        return -1;
    }

    if (self->bl) {
        fbBasicListSetSemantic(self->bl, semantic);
    } else {
        PyErr_SetString(PyExc_StandardError, "BL must be initialized "
                        "before semantic can be set.");
        return -1;
    }

    return 0;
}

static PyObject *fixbufPyBL_getelement(
    fixbufPyBL *self,
    void *cbdata)
{
    fixbufPyInfoElement *element = NULL;
    const fbInfoElement_t *ie = NULL;

    if (self->bl) {
        ie = fbBasicListGetInfoElement(self->bl);
    }


    if (ie) {
        element = (fixbufPyInfoElement *)fixbufPyInfoElementType.tp_new(&fixbufPyInfoElementType, NULL, NULL);
        if (element == NULL) {
            Py_XDECREF(element);
            return PyErr_NoMemory();
        }

        memcpy(element->infoElement, ie, sizeof(fbInfoElement_t));

        /*Py_INCREF(element);*/

        return (PyObject *)element;
    } else {

        Py_INCREF(Py_None);

        return Py_None;
    }
}

static PyObject *fixbufPyBLClear(
    fixbufPyBL *self,
    PyObject *args)
{

    if (self->bl == NULL) {
        PyErr_SetString(PyExc_AttributeError, "No basicList to Clear");
        return NULL;
    }

    fbBasicListClear(self->bl);

    Py_INCREF(Py_None);

    return Py_None;
}


static PyMethodDef fixbufPyBL_methods[] = {
    {"clear", (PyCFunction)fixbufPyBLClear, METH_VARARGS,
     ("Clear the BasicList")},
    {"getitems", (PyCFunction)fixbufPyBL_getitems, METH_VARARGS,
     ("returns the Python list for the basicList")},
    {"setitems", (PyCFunction)fixbufPyBL_setitems, METH_VARARGS,
     ("set the basicList")},
    {NULL, NULL, 0, NULL}
};


static PyGetSetDef fixbufPyBL_getsetters[] = {
    {"count", (getter)fixbufPyBL_getcount, NULL,
     "Get number of entries in list", NULL},
    {"semantic", (getter)fixbufPyBL_getsemantic,(setter)fixbufPyBL_setsemantic,
     "Get/Set the Semantic Value on the basicList", NULL},
    {"element", (getter)fixbufPyBL_getelement, NULL,
     "Get the InfoElement set on the basicList", NULL},
    {NULL, NULL, NULL, NULL, NULL}
};

static PyTypeObject fixbufPyBLType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size */
    "pyfixbuf.fbBLBase",       /*tp_name*/
    sizeof(fixbufPyBL), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)fixbufPyBL_dealloc,        /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "fbBL",                     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    fixbufPyBL_methods,        /* tp_methods */
    0,                          /* tp_members */
    fixbufPyBL_getsetters,     /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)fixbufPyBL_init, /* tp_init */
    0,                          /* tp_alloc */
    fixbufPyBL_new,  /* tp_new */
    0                           /* tp_free */
};


void init_pyfixbuf(void)
{

    PyObject *fixbuf;
    int rv;

    fixbuf = Py_InitModule3("_pyfixbuf", NULL, "Fixbuf Extension Module");

    if (fixbuf == NULL) {
        printf("Could not create module pyfixbuf\n");
        goto err;
    }

    /* adding info model type */

    fixbufPyInfoModelType.tp_new = PyType_GenericNew;

    if (PyType_Ready(&fixbufPyInfoModelType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyInfoModelType);
    rv = PyModule_AddObject(fixbuf, "fbInfoModelBase",
                            (PyObject *)&fixbufPyInfoModelType);
    assert(rv == 0);

    /* adding info element type */

    if (PyType_Ready(&fixbufPyInfoElementType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPyInfoElementType);
    rv = PyModule_AddObject(fixbuf, "fbInfoElementBase",
                            (PyObject *)&fixbufPyInfoElementType);
    assert(rv == 0);

    /* adding session type */

    fixbufPySessionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPySessionType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPySessionType);
    rv = PyModule_AddObject(fixbuf, "fbSessionBase",
                            (PyObject *)&fixbufPySessionType);
    assert(rv == 0);

    /* adding collector type */

    fixbufPyCollectorType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPyCollectorType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPyCollectorType);
    rv = PyModule_AddObject(fixbuf, "fbCollectorBase",
                            (PyObject *)&fixbufPyCollectorType);
    assert(rv == 0);

    /* adding listener type */

    fixbufPyListenerType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPyListenerType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPyListenerType);
    rv = PyModule_AddObject(fixbuf, "fbListenerBase",
                            (PyObject *)&fixbufPyListenerType);
    assert(rv == 0);

    /* adding exporter type */

    fixbufPyExporterType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPyExporterType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyExporterType);
    rv = PyModule_AddObject(fixbuf, "fbExporterBase",
                            (PyObject *)&fixbufPyExporterType);
    assert(rv == 0);

    /* adding Template Type */

    fixbufPyTemplateType.tp_new = PyType_GenericNew;

    if (PyType_Ready(&fixbufPyTemplateType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyTemplateType);
    rv = PyModule_AddObject(fixbuf, "fbTemplateBase",
                            (PyObject *)&fixbufPyTemplateType);
    assert(rv == 0);

    if (PyType_Ready(&fixbufPyfBufType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyfBufType);
    rv = PyModule_AddObject(fixbuf, "fBufBase",
                            (PyObject *)&fixbufPyfBufType);
    assert(rv == 0);

    /*adding info element spec type */

    if (PyType_Ready(&fixbufPyInfoElementSpecType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyInfoElementSpecType);
    rv = PyModule_AddObject(fixbuf, "fbInfoElementSpecBase",
                            (PyObject *)&fixbufPyInfoElementSpecType);
    assert (rv == 0);


    fixbufPyRecordType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPyRecordType) < 0) {
        goto err;
    }

    Py_INCREF(&fixbufPyRecordType);
    rv = PyModule_AddObject(fixbuf, "fbRecordBase",
                            (PyObject *)&fixbufPyRecordType);
    assert(rv == 0);


    fixbufPySTMLType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPySTMLType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPySTMLType);
    rv = PyModule_AddObject(fixbuf, "fbSTMLBase",
                            (PyObject *)&fixbufPySTMLType);
    assert(rv == 0);

    fixbufPySTMLEntryType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPySTMLEntryType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPySTMLEntryType);
    rv = PyModule_AddObject(fixbuf, "fbSTMLEntryBase",
                            (PyObject *)&fixbufPySTMLEntryType);
    assert(rv == 0);

    fixbufPyVarfieldType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPyVarfieldType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPyVarfieldType);
    rv = PyModule_AddObject(fixbuf, "fbVarfieldBase",
                            (PyObject *)&fixbufPyVarfieldType);
    assert(rv == 0);

    fixbufPySTLType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&fixbufPySTLType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPySTLType);
    rv = PyModule_AddObject(fixbuf, "fbSTLBase", (PyObject *)&fixbufPySTLType);
    assert(rv == 0);

    /* adding basicList type */
    if (PyType_Ready(&fixbufPyBLType) < 0) {
        goto err;
    }
    Py_INCREF(&fixbufPyBLType);
    rv = PyModule_AddObject(fixbuf, "fbBLBase",
                            (PyObject *)&fixbufPyBLType);
    assert(rv == 0);



err:
    if (PyErr_Occurred()) {
        PyErr_Print();
    }
}
