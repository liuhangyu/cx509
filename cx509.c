/* -*- mode: C++; fill-column: 100; -*-
 *
 * This extension module exports the cx509 type. The underlying implementation is provided by
 * sset.c.
 *
 * dmb - Feb 2012 - Copyright (C) 2012 Arcode Corporation
 */
#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "structmember.h"

/* root X.509 type header file; generated by asn1c */
#include "Certificate.h"

/* extension types we know about: */
#include "BasicConstraints.h"
#include "KeyUsage.h"
#include "SubjectAltName.h"

/* other types we need */
#include "GeneralName.h"

typedef struct {
    PyObject_HEAD
    Certificate_t *certificate;
} cx509;

/* forward declarations */
static PyTypeObject cx509Type;
static PyObject *cx509_parse(cx509 *self, PyObject *args, PyObject *kw);
static char *_oid_to_string(OBJECT_IDENTIFIER_t *oid);


static PyObject *
cx509_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
    cx509 *self = (cx509 *) type->tp_alloc(type, 0);
    self->certificate = NULL;
    return (PyObject *) self;
}

static int
cx509_init(cx509 *self, PyObject *args, PyObject *kw)
{
    PyObject *result = cx509_parse(self, args, kw);
    if (result) {
	Py_DECREF(result);
	return 0;
    }
    return -1;
}

static PyObject *
cx509_parse(cx509 *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "data", "format", NULL };
    PyObject *data = NULL;
    Py_ssize_t len;
    char *format = NULL;
    Certificate_t *certificate = NULL;
    asn_dec_rval_t rval;

    if (!PyArg_ParseTupleAndKeywords(args, kw, "|s#O", kwlist, &data, &len, &format))
	return NULL;

    /* free existing data (if any) */
    asn_DEF_Certificate.free_struct(&asn_DEF_Certificate, self->certificate, 0);
    self->certificate = NULL;

    if (data) {
	/* parse new data */
	if (format == NULL || 
	    !strcmp(format, "ber") || !strcmp(format, "BER") ||
	    !strcmp(format, "cer") || !strcmp(format, "CER") ||
	    !strcmp(format, "der") || !strcmp(format, "DER")) {
	    rval = ber_decode(0, &asn_DEF_Certificate, (void **) &certificate, (const void *) data, (size_t) len);
	}
	else if (!strcmp(format, "xer") || !strcmp(format, "XER")) {
	    rval = xer_decode(0, &asn_DEF_Certificate, (void **) &certificate, (const void *) data, (size_t) len);
	}
	if (rval.code == RC_OK) {
	    /* decoding succeeded */
	    self->certificate = certificate;
	} 
	else {
	    /* Free partially decoded certificate */
	    asn_DEF_Certificate.free_struct(&asn_DEF_Certificate, certificate, 0);
	    self->certificate = NULL;
	}
    }

    Py_INCREF(self);
    return (PyObject *) self;
}

/* count up total number of bytes in string output */
static int _print2count(const void *buffer, size_t size, void *app_key)
{
    *((size_t *) app_key) += size;
    return 0;
}

/* copy the string output to the specified output buffer */
static int _print2buffer(const void *buffer, size_t size, void *app_key)
{
    void **output = (void **) app_key;
    memcpy(*output, buffer, size);
    *output += size;
    return 0;
}

static PyObject *
cx509___str__(cx509 *self)
{
    size_t count = 0;
    void *allocated, *output;
    PyObject *s = NULL;

    /* just count the number of bytes in the output */
    if (asn_DEF_Certificate.print_struct(&asn_DEF_Certificate, self->certificate, 1, _print2count, (void *) &count))
	return NULL;

    /* allocate and print to allocated buffer */
    allocated = output = PyMem_Malloc(count);

    /* write the output */
    if (asn_DEF_Certificate.print_struct(&asn_DEF_Certificate, self->certificate, 1, _print2buffer, (void *) &output)) {
	free(output);
	return NULL;
    }

    s = PyString_FromStringAndSize(allocated, count);
    PyMem_Free(allocated);
    return s;
}

static PyObject *
cx509_get_version(cx509 *self)
{
    TBSCertificate_t tbsCertificate;
    Version_t *version;
    long v = 0;

    if (!self->certificate) {
	PyErr_Format(PyExc_ValueError, "empty certificate");
	return NULL;
    }

    tbsCertificate = self->certificate->tbsCertificate;
    version = tbsCertificate.version;
    if (version) {
	if (asn_INTEGER2long(version, &v) != 0) {
	    PyErr_Format(PyExc_ValueError, strerror(errno));
	    return NULL;
	}
    }
    return PyInt_FromLong(v);
}

static PyObject *
cx509_extensions(cx509 *self)
{
    TBSCertificate_t tbsCertificate;
    struct Extensions *extensions;
    struct Extension *ext;
    PyObject *L;
    PyObject *dict;
    PyObject *tmp;
    asn_dec_rval_t rval;
    char *oid;
    int i, j;

    /* extension types we know about: */
    BasicConstraints_t *basicConstraints = NULL;
    long basicConstraints_pathlen = 0;

    KeyUsage_t *keyUsage = NULL;
    PyObject *keyUsageFlags = NULL;

    SubjectAltName_t *subjectAltName = NULL;
    GeneralName_t *gn = NULL;
    PyObject *dNSName, *dNSNames;

    if (!self->certificate) {
	PyErr_Format(PyExc_ValueError, "empty certificate");
	return NULL;
    }

    L = PyList_New(0);

    tbsCertificate = self->certificate->tbsCertificate;
    extensions = tbsCertificate.extensions;
    if (extensions) {
	for (i = 0; i < extensions->list.count; i++) {
	    dict = PyDict_New();
	    ext = extensions->list.array[i];
	    oid = _oid_to_string(&ext->extnID);

	    PyDict_SetItemString(dict, "critical", (ext->critical && *ext->critical) ? Py_True : Py_False); /* does not steal reference */

	    /* parse known extensions */
	    if (oid) {
		if (!strcmp(oid, "{ 2.5.29.15 }")) {
		    tmp = PyString_FromString("keyUsage");
		    PyDict_SetItemString(dict, "name", tmp);
		    Py_DECREF(tmp);
		    if (ext->extnValue.size) {
			rval = ber_decode(0, &asn_DEF_KeyUsage, (void **) &keyUsage,
					  (const void *) ext->extnValue.buf, (size_t) ext->extnValue.size);
			if (rval.code == RC_OK && keyUsage) {
			    keyUsageFlags = PyFrozenSet_New(NULL);
			    if (keyUsage->size > 0) {
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_digitalSignature))) {
				    tmp = PyString_FromString("digitalSignature");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_nonRepudiation))) {
				    tmp = PyString_FromString("nonRepudiation");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_keyEncipherment))) {
				    tmp = PyString_FromString("keyEncipherment");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_dataEncipherment))) {
				    tmp = PyString_FromString("dataEncipherment");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_keyAgreement))) {
				    tmp = PyString_FromString("keyAgreement");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_keyCertSign))) {
				    tmp = PyString_FromString("keyCertSign");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_cRLSign))) {
				    tmp = PyString_FromString("cRLSign");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
				if (keyUsage->buf[0] & (1 << (7 - KeyUsage_encipherOnly))) {
				    tmp = PyString_FromString("encipherOnly");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
			    }
			    if (keyUsage->size > 1) {
				if (keyUsage->buf[1] & (1 << (7 - (KeyUsage_digitalSignature - 8)))) {
				    tmp = PyString_FromString("decipherOnly");
				    PySet_Add(keyUsageFlags, tmp);
				    Py_DECREF(tmp);
				}
			    }
			    PyDict_SetItemString(dict, "keyUsage", keyUsageFlags);
			}
		    }
		}
		else if (!strcmp(oid, "{ 2.5.29.17 }")) {
		    /* NOTE: we only check for dNSName type here; we just ignore the others */
		    tmp = PyString_FromString("subjectAltName");
		    PyDict_SetItemString(dict, "name", tmp);
		    Py_DECREF(tmp);
		    if (ext->extnValue.size) {
			rval = ber_decode(0, &asn_DEF_SubjectAltName, (void **) &subjectAltName,
					  (const void *) ext->extnValue.buf, (size_t) ext->extnValue.size);
			if (rval.code == RC_OK && subjectAltName) {
			    dNSNames = PyList_New(0);
			    for (j = 0; j < subjectAltName->list.count; j++) {
				gn = subjectAltName->list.array[j];
				if (gn && gn->present == GeneralName_PR_dNSName && gn->choice.dNSName.buf) {
				    dNSName = PyString_FromStringAndSize((void *) gn->choice.dNSName.buf, (size_t) gn->choice.dNSName.size);
				    PyList_Append(dNSNames, dNSName); /* does not steal reference */
				    Py_DECREF(dNSName);
				}
			    }
			    if (PyList_Size(dNSNames))
				PyDict_SetItemString(dict, "dNSName", dNSNames);
			    else
				Py_DECREF(dNSNames);
			}
		    }
		}
		else if (!strcmp(oid, "{ 2.5.29.19 }")) {
		    tmp = PyString_FromString("basicConstraints");
		    PyDict_SetItemString(dict, "name", tmp);
		    Py_DECREF(tmp);
		    if (ext->extnValue.size) {
			rval = ber_decode(0, &asn_DEF_BasicConstraints, (void **) &basicConstraints, 
					  (const void *) ext->extnValue.buf, (size_t) ext->extnValue.size);
			if (rval.code == RC_OK && basicConstraints) {
			    PyDict_SetItemString(dict, "cA", (basicConstraints->cA && *basicConstraints->cA) ? Py_True : Py_False); /* does not steal reference */
			    if (!asn_INTEGER2long(basicConstraints->pathLenConstraint, &basicConstraints_pathlen)) {
				tmp = PyInt_FromLong(basicConstraints_pathlen);
				PyDict_SetItemString(dict, "pathLenConstraint", tmp);
				Py_DECREF(tmp);
			    }
			}
		    }
		}
		else {
		    tmp = PyString_FromString(oid);
		    PyDict_SetItemString(dict, "name", tmp);
		    Py_DECREF(tmp);
		}
	    }

	    PyList_Append(L, dict); /* does not steal reference */
	    Py_DECREF(dict);

	    if (oid)
		PyMem_Free(oid);
	}
    }

    return L;
}

static char *
_oid_to_string(OBJECT_IDENTIFIER_t *oid)
{
    size_t count = 0;
    void *allocated, *output;

    if (OBJECT_IDENTIFIER_print(NULL, (const void *) oid, 0, _print2count, (void *) &count))
	return NULL;

    /* allocate and print to allocated buffer */
    allocated = output = PyMem_Malloc(count + 1);

    /* write the output */
    if (OBJECT_IDENTIFIER_print(NULL, (const void *) oid, 0, _print2buffer, (void *) &output)) {
	free(output);
	return NULL;
    }
    
    *((char *) output) = 0; /* null-terminate */

    return allocated;
}

static void
cx509_free(PyObject *self)
{
    Py_TYPE(self)->tp_free(self);
}

static PyMemberDef cx509_members[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef cx509_methods[] = {
    {"_parse", (PyCFunction) cx509_parse, METH_VARARGS|METH_KEYWORDS, "Parse the provided BER/DER/CER binary." },
    {"get_version", (PyCFunction) cx509_get_version, METH_NOARGS, "Return the certificate version." },
    {"extensions", (PyCFunction) cx509_extensions, METH_NOARGS, "Return list of extensions." },

#if 0
    {"__str__", (PyCFunction) cx509___str__, METH_VARARGS, "Return human-readable string representation of certificate." },
    {"__contains__", (PyCFunction) cx509_contains, METH_VARARGS, "Return True if the provided value is in the cx509." },
    {"__getitem__", (PyCFunction) cx509_getitem, METH_VARARGS, "Return the nth value in the set." },
    {"__eq__", (PyCFunction) cx509_eq, METH_VARARGS, "Return True if self and the provided sset are identical." },
    {"parse", (PyCFunction) cx509_parse, METH_VARARGS, "Parse the provided string; set the cx509 to match the string." },
    {"number_of_ranges", (PyCFunction) cx509_number_of_ranges, METH_NOARGS, "Return the length of this cx509 in range pairs." },
    {"min_and_max", (PyCFunction) cx509_min_and_max, METH_NOARGS, "Return a tuple (min, max) of values in the cx509." },
    {"has_star", (PyCFunction) cx509_has_star, METH_NOARGS, "Return True if the cx509 contains the INFINITY value; False otherwise." },
    {"clone", (PyCFunction) cx509_clone, METH_VARARGS|METH_KEYWORDS, "Make this cx509 a clone of the provided cx509. The new cx509 will have its own memory." },
    {"optimize", (PyCFunction) cx509_optimize, METH_VARARGS|METH_KEYWORDS, "Optimize ranges in cx509." },
    {"clear", (PyCFunction) cx509_clear, METH_NOARGS, "Clear the cx509." },
    {"invert", (PyCFunction) cx509_invert, METH_NOARGS, "Return a new cx509 with all the values that are not in self." },
    {"union", (PyCFunction) cx509_union, METH_VARARGS, "Return a new cx509 with elements from both self and t." },
    {"update", (PyCFunction) cx509_update, METH_VARARGS, "Update cx509 so it includes elements from both self and t." },
    {"as_tuple_list", (PyCFunction) cx509_as_tuple_list, METH_NOARGS, "Return ranges as a list of 2-item tuples." },
    {"set_from_list", (PyCFunction) cx509_set_from_list, METH_VARARGS|METH_KEYWORDS, "Set initial values from provided list of itegers (lo, hi, lo, hi ...)." },
#endif
    {NULL}  /* Sentinel */
};

static PyTypeObject cx509Type = {
    PyObject_HEAD_INIT(NULL)
    0,				/*ob_size*/
    "cx509.cx509",  		/*tp_name*/
    sizeof(cx509),  		/*tp_basicsize*/
    0,                         /*tp_itemsize*/
    cx509_free,     		/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0, 	                       /*tp_call*/
    (reprfunc) cx509___str__,  /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "cx509 objects",		/* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    cx509_methods,  		/* tp_methods */
    cx509_members,  		/* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)cx509_init,      /* tp_init */
    0,                        /* tp_alloc */
    cx509_new,     /* tp_new */
};


static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC
initcx509(void) 
{
    PyObject* m;

    if (PyType_Ready(&cx509Type) < 0)
        return;

    m = Py_InitModule3("cx509", module_methods, "X.509 certificate");
    if (m == NULL)
	return;

    Py_INCREF(&cx509Type);
    PyModule_AddObject(m, "cx509", (PyObject *) &cx509Type);
}
