/* -*- mode: C++; fill-column: 100; -*-
 *
 * This extension module exports the cx509 type. The underlying implementation is provided by
 * sset.c.
 *
 * See http://stackoverflow.com/questions/9394125/asn1c-der-decoder for alterations required for
 * compiling under MSVC.
 *
 * dmb - Nov 2012 - Copyright (C) 2012 Arcode Corporation
 */
#include <Python.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "structmember.h"

/* root X.509 type header file; generated by asn1c */
#include "Certificate.h"

/* extension types we know about: */
#include "BasicConstraints.h"
#include "KeyUsage.h"
#include "SubjectAltName.h"

/* PKCS1 types we need */
#include "RSAPublicKey.h"

/* other types we need */
#include "DirectoryString.h"
#include "GeneralName.h"

typedef struct {
    PyObject_HEAD
    Certificate_t *certificate;
} cx509;

/* 
 * OIDs we know about. These MUST be in lexicographic sorted order by dotted string.
 */
typedef struct {
    char *dotted;
    char *name;
} oid_t;
static int nOIDs = 0; /* set dynamically by find_oid */
static oid_t OIDs[] = {
    /* see: http://www.alvestrand.no/cgi-bin/hta/oidwordsearch */
    { "{ 1.2.840.10040.4.1 }", "id-dsa" },
    { "{ 1.2.840.10040.4.3 }", "id-dsa-with-sha1" },
    { "{ 1.2.840.10045.2.1 }", "id-ecPublicKey" }, /* Elliptic Curve public key */
    { "{ 1.2.840.10045.4.1 }", "ecdsa-with-SHA1" }, /* ECDSA signature with SHA-1 */
    { "{ 1.2.840.10046.2.1 }", "dhpublicnumber" }, /* Diffie-Hellman public key */
    { "{ 1.2.840.113549.1.1.1 }", "rsaEncryption" }, /* RSA public keys */
    { "{ 1.2.840.113549.1.1.10 }", "RSASSA-PSS" },
    { "{ 1.2.840.113549.1.1.11 }", "sha256WithRSAEncryption" },
    { "{ 1.2.840.113549.1.1.12 }", "sha384WithRSAEncryption" },
    { "{ 1.2.840.113549.1.1.13 }", "sha512WithRSAEncryption" },
    { "{ 1.2.840.113549.1.1.2 }", "md2WithRSAEncryption" }, /* RSA signature generated with MD2 hash */
    { "{ 1.2.840.113549.1.1.3 }", "md4WithRSAEncryption" },
    { "{ 1.2.840.113549.1.1.4 }", "md5WithRSAEncryption" }, /* RSA signature generated with MD5 hash */
    { "{ 1.2.840.113549.1.1.5 }", "sha1WithRSAEncryption" }, /* RSA signature generated with SHA1 hash */
    { "{ 1.2.840.113549.1.1.6 }", "rsaOAEPEncryptionSET" },
    { "{ 1.2.840.113549.1.1.7 }", "id-RSAES-OAEP" },
    { "{ 1.2.840.113549.1.9 }", "email" },
    { "{ 1.2.840.113549.1.9.1 }", "emailAddress" },
    { "{ 1.2.840.113549.2.2 }", "md2" }, /* MD2 hash function */
    { "{ 1.2.840.113549.2.26 }", "id-sha1" },
    { "{ 1.2.840.113549.2.5 }", "md5" }, /* MD5 hash function */
    { "{ 1.3.14.3.2.10 }", "desMAC" },
    { "{ 1.3.14.3.2.11 }", "rsaSignature" },
    { "{ 1.3.14.3.2.12 }", "dsa" },
    { "{ 1.3.14.3.2.13 }", "dsaWithSHA" },
    { "{ 1.3.14.3.2.14 }", "mdc2WithRSASignature" },
    { "{ 1.3.14.3.2.15 }", "shaWithRSASignature" },
    { "{ 1.3.14.3.2.16 }", "dhWithCommonModulus" },
    { "{ 1.3.14.3.2.17 }", "desEDE" },
    { "{ 1.3.14.3.2.18 }", "sha" },
    { "{ 1.3.14.3.2.19 }", "mdc-2" },
    { "{ 1.3.14.3.2.2 }", "md4WithRSA" },
    { "{ 1.3.14.3.2.20 }", "dsaCommon" },
    { "{ 1.3.14.3.2.21 }", "dsaCommonWithSHA" },
    { "{ 1.3.14.3.2.22 }", "rsaKeyTransport" },
    { "{ 1.3.14.3.2.23 }", "keyed-hash-seal" },
    { "{ 1.3.14.3.2.24 }", "md2WithRSASignature" },
    { "{ 1.3.14.3.2.25 }", "md5WithRSASignature" },
    { "{ 1.3.14.3.2.26 }", "sha-1" },
    { "{ 1.3.14.3.2.27 }", "dsa-sha1" },
    { "{ 1.3.14.3.2.28 }", "dsa-sha1-common-parameters" },
    { "{ 1.3.14.3.2.29 }", "sha1-with-RSA-signature" },
    { "{ 1.3.14.3.2.3 }", "md5WithRSA" },
    { "{ 1.3.14.3.2.4 }", "md4WithRSAEncryption" },
    { "{ 1.3.14.3.2.6 }", "desECB" },
    { "{ 1.3.14.3.2.7 }", "desCBC" },
    { "{ 1.3.14.3.2.8 }", "desOFB" },
    { "{ 1.3.14.3.2.9 }", "desCFB" },
    { "{ 2.16.840.1.101.2.1.1.22 }", "id-keyExchangeAlgorithm" }, /* KEA key */
    { "{ 2.16.840.1.113730.1.13 }", "comment" },
    { "{ 2.5.29.1 }", "oldAuthorityKeyIdentifier" },
    { "{ 2.5.29.14 }", "subjectKeyIdentifier" },
    { "{ 2.5.29.15 }", "keyUsage" },
    { "{ 2.5.29.16 }", "privateKeyUsagePeriod" },
    { "{ 2.5.29.17 }", "subjectAltName" },
    { "{ 2.5.29.18 }", "issuerAlternativeName" },
    { "{ 2.5.29.19 }", "basicConstraints" },
    { "{ 2.5.29.2 }", "oldPrimaryKeyAttributes" },
    { "{ 2.5.29.20 }", "cRLNumber" },
    { "{ 2.5.29.21 }", "reasonCode" },
    { "{ 2.5.29.23 }", "holdInstructionCode" },
    { "{ 2.5.29.24 }", "invalidityDate" },
    { "{ 2.5.29.27 }", "deltaCRLIndicator" },
    { "{ 2.5.29.28 }", "issuingDistributionPoint" },
    { "{ 2.5.29.29 }", "certificateIssuer" },
    { "{ 2.5.29.3 }", "certificatePolicies" },
    { "{ 2.5.29.30 }", "nameConstraints" },
    { "{ 2.5.29.31 }", "cRLDistributionPoints" },
    { "{ 2.5.29.32 }", "certificatePolicies" },
    { "{ 2.5.29.32.0 }", "anyPolicy" },
    { "{ 2.5.29.33 }", "policyMappings" },
    { "{ 2.5.29.35 }", "authorityKeyIdentifier" },
    { "{ 2.5.29.36 }", "policyConstraints" },
    { "{ 2.5.29.37 }", "extendedKeyUsage" },
    { "{ 2.5.29.4 }", "primaryKeyUsageRestriction" },
    { "{ 2.5.29.46 }", "freshestCRL" },
    { "{ 2.5.29.54 }", "inhibitAnyPolicy" },
    { "{ 2.5.4.0 }", "objectClass" },
    { "{ 2.5.4.1 }", "aliasedEntryName" },
    { "{ 2.5.4.10 }", "organizationName" },
    { "{ 2.5.4.11 }", "organizationalUnitName" },
    { "{ 2.5.4.11.1 }", "collectiveOrganizationalUnitName" },
    { "{ 2.5.4.12 }", "title" },
    { "{ 2.5.4.13 }", "description" },
    { "{ 2.5.4.14 }", "searchGuide" },
    { "{ 2.5.4.15 }", "businessCategory" },
    { "{ 2.5.4.16 }", "postalAddress" },
    { "{ 2.5.4.16.1 }", "collectivePostalAddress" },
    { "{ 2.5.4.17 }", "postalCode" },
    { "{ 2.5.4.17.1 }", "collectivePostalCode" },
    { "{ 2.5.4.18 }", "postOfficeBox" },
    { "{ 2.5.4.18.1 }", "collectivePostOfficeBox" },
    { "{ 2.5.4.19 }", "physicalDeliveryOfficeName" },
    { "{ 2.5.4.19.1 }", "collectivePhysicalDeliveryOfficeName" },
    { "{ 2.5.4.2 }", "knowledgeinformation" },
    { "{ 2.5.4.20 }", "telephoneNumber" },
    { "{ 2.5.4.20.1 }", "collectiveTelephoneNumber" },
    { "{ 2.5.4.21 }", "telexNumber" },
    { "{ 2.5.4.21.1 }", "collectiveTelexNumber" },
    { "{ 2.5.4.22 }", "telexTerminalIdentifier" },
    { "{ 2.5.4.22.1 }", "collectiveTelexTerminalIdentifer" },
    { "{ 2.5.4.23 }", "facsimileTelephoneNumber" },
    { "{ 2.5.4.23.1 }", "collectiveFacsimileTelephoneNumber" },
    { "{ 2.5.4.24 }", "x121Address" },
    { "{ 2.5.4.25 }", "internationalISDNNumber" },
    { "{ 2.5.4.25.1 }", "collectiveInternationalISDNNumber" },
    { "{ 2.5.4.26 }", "registeredAddress" },
    { "{ 2.5.4.27 }", "destinationIndicator" },
    { "{ 2.5.4.28 }", "preferredDeliveryMethod" },
    { "{ 2.5.4.29 }", "presentationAddress" },
    { "{ 2.5.4.3 }", "commonName" },
    { "{ 2.5.4.30 }", "supportedApplicationContext" },
    { "{ 2.5.4.31 }", "member" },
    { "{ 2.5.4.32 }", "owner" },
    { "{ 2.5.4.33 }", "roleOccupant" },
    { "{ 2.5.4.34 }", "seeAlso" },
    { "{ 2.5.4.35 }", "userPassword" },
    { "{ 2.5.4.36 }", "userCertificate" },
    { "{ 2.5.4.37 }", "cACertificate" },
    { "{ 2.5.4.38 }", "authorityRevocationList" },
    { "{ 2.5.4.39 }", "certificateRevocationList" },
    { "{ 2.5.4.4 }", "surname" },
    { "{ 2.5.4.40 }", "crossCertificatePair" },
    { "{ 2.5.4.41 }", "name" },
    { "{ 2.5.4.42 }", "givenName" },
    { "{ 2.5.4.43 }", "initials" },
    { "{ 2.5.4.44 }", "generationQualifier" },
    { "{ 2.5.4.45 }", "uniqueIdentifier" },
    { "{ 2.5.4.46 }", "dnQualifier" },
    { "{ 2.5.4.47 }", "enhancedSearchGuide" },
    { "{ 2.5.4.48 }", "protocolInformation" },
    { "{ 2.5.4.49 }", "distinguishedName" },
    { "{ 2.5.4.5 }", "serialNumber" },
    { "{ 2.5.4.50 }", "uniqueMember" },
    { "{ 2.5.4.51 }", "houseIdentifier" },
    { "{ 2.5.4.52 }", "supportedAlgorithms" },
    { "{ 2.5.4.53 }", "deltaRevocationList" },
    { "{ 2.5.4.58 }", "attributeCertificate" },
    { "{ 2.5.4.6 }", "countryName" },
    { "{ 2.5.4.65 }", "psuedonym" },
    { "{ 2.5.4.7 }", "localityName" },
    { "{ 2.5.4.7.1 }", "collectiveLocalityName" },
    { "{ 2.5.4.8 }", "stateOrProvinceName" },
    { "{ 2.5.4.8.1 }", "collectiveStateOrProvinceName" },
    { "{ 2.5.4.9 }", "streetAddress" },
    { "{ 2.5.4.9.1 }", "collectiveStreetAddress" },

    /* sentinel */
    { NULL,  NULL }
};

static int nOIDs_short_names = 0; /* set dynamically by find_oid */
static oid_t OID_short_names[] = {
    { "{ 2.5.4.10 }", "O" },
    { "{ 2.5.4.11 }", "OU" },
    { "{ 2.5.4.3 }", "CN" },
    { "{ 2.5.4.4 }", "SN" },
    { "{ 2.5.4.42 }", "GN" },
    { "{ 2.5.4.6 }", "CN" },
    { "{ 2.5.4.7 }", "L" },
    { "{ 2.5.4.8 }", "ST" },

    /* sentinel */
    { NULL,  NULL }
};

/* Forward declarations */
static PyTypeObject cx509Type;
static PyObject *cx509_parse(cx509 *self, PyObject *args, PyObject *kw);
static char *_oid_to_string(OBJECT_IDENTIFIER_t *oid);
static char *_integer_to_hex_string(INTEGER_t *I);
static void _populate_dict_from_rdn_sequence(PyObject *dict, RDNSequence_t *rdnSequence);
static void _add_directory_string_to_dict(ANY_t *any, PyObject *dict, const char *key_name);;
static PyObject *_directory_string_to_string(DirectoryString_t *ds, char encoding[16]);
static const char *find_oid(const char *dotted, int shortname);

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
	else {
	    PyErr_Format(PyExc_ValueError, "unknown format");
	    return NULL;
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
	PyMem_Free(output);
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

/* return valdity as (start_time, end_time); times are seconds since the epoch GMT */
static PyObject *
cx509_get_validity(cx509 *self)
{
    TBSCertificate_t tbsCertificate;
    PyObject *tuple;
    time_t time;

    if (!self->certificate) {
	PyErr_Format(PyExc_ValueError, "empty certificate");
	return NULL;
    }

    tuple = PyTuple_New(2);
    tbsCertificate = self->certificate->tbsCertificate;

    /* not before */
    if (tbsCertificate.validity.notBefore.present == Time_PR_utcTime) {
	time = asn_UT2time(&tbsCertificate.validity.notBefore.choice.utcTime, NULL, /*as_gmt:*/ 1);
	PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLongLong((PY_LONG_LONG) time)); /* steals the new reference */
    }
    else if (tbsCertificate.validity.notBefore.present == Time_PR_generalTime) {
	time = asn_GT2time(&tbsCertificate.validity.notBefore.choice.generalTime, NULL, /*as_gmt:*/ 1);
	PyTuple_SetItem(tuple, 0, PyLong_FromUnsignedLongLong((PY_LONG_LONG) time)); /* steals the new reference */
    }
    else {
	Py_INCREF(Py_None);
	PyTuple_SetItem(tuple, 0, Py_None);  /* steals the reference */
    }

    /* not after */
    if (tbsCertificate.validity.notAfter.present == Time_PR_utcTime) {
	time = asn_UT2time(&tbsCertificate.validity.notAfter.choice.utcTime, NULL, /*as_gmt:*/ 1);
	PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLongLong((PY_LONG_LONG) time)); /* steals the new reference */
    }
    else if (tbsCertificate.validity.notAfter.present == Time_PR_generalTime) {
	time = asn_GT2time(&tbsCertificate.validity.notAfter.choice.generalTime, NULL, /*as_gmt:*/ 1);
	PyTuple_SetItem(tuple, 1, PyLong_FromUnsignedLongLong((PY_LONG_LONG) time)); /* steals the new reference */
    }
    else {
	Py_INCREF(Py_None);
	PyTuple_SetItem(tuple, 1, Py_None);  /* steals the reference */
    }

    return tuple;
}

static PyObject *
cx509_get_issuer(cx509 *self)
{
    TBSCertificate_t tbsCertificate;
    PyObject *dict;

    if (!self->certificate) {
	PyErr_Format(PyExc_ValueError, "empty certificate");
	return NULL;
    }

    dict = PyDict_New();
    tbsCertificate = self->certificate->tbsCertificate;
    if (tbsCertificate.issuer.present == Name_PR_rdnSequence)
	_populate_dict_from_rdn_sequence(dict, &tbsCertificate.issuer.choice.rdnSequence);
    return dict;
}

static PyObject *
cx509_get_subject(cx509 *self)
{
    TBSCertificate_t tbsCertificate;
    PyObject *dict;

    if (!self->certificate) {
	PyErr_Format(PyExc_ValueError, "empty certificate");
	return NULL;
    }

    dict = PyDict_New();
    tbsCertificate = self->certificate->tbsCertificate;
    if (tbsCertificate.subject.present == Name_PR_rdnSequence)
	_populate_dict_from_rdn_sequence(dict, &tbsCertificate.subject.choice.rdnSequence);
    return dict;
}

static void
_populate_dict_from_rdn_sequence(PyObject *dict, RDNSequence_t *rdnSequence)
{
    AttributeTypeAndValue_t *atv;
    char *atype = NULL;
    int i, j;
    const char *name;

    for (i = 0; i < rdnSequence->list.count; i++) {
	for (j = 0; j < rdnSequence->list.array[i]->list.count; j++) {
	    atv = rdnSequence->list.array[i]->list.array[j];
	    atype = _oid_to_string(&atv->type);
	    name = find_oid(atype, /*shortname:*/ 0);
	    _add_directory_string_to_dict(&atv->value, dict, name ? name : atype);
	    if (atype)
		PyMem_Free(atype);
	}
    }
}

static void
_add_directory_string_to_dict(ANY_t *any, PyObject *dict, const char *key_name)
{
    PyObject *value;
    DirectoryString_t *ds = NULL;
    IA5String_t *ia5 = NULL;
    char encoding[16];	
    char *encoding_key_name;

#define ADD do {									\
    PyDict_SetItemString(dict, key_name, value);					\
    Py_DECREF(value);									\
    value = PyString_FromString(encoding);						\
    encoding_key_name = PyMem_Malloc(strlen(key_name) + strlen(":encoding") + 1);	\
    strcpy(encoding_key_name, key_name);						\
    strcat(encoding_key_name, ":encoding");						\
    PyDict_SetItemString(dict, encoding_key_name, value);				\
    PyMem_Free(encoding_key_name);							\
    Py_DECREF(value);									\
} while (0)

    if (!ANY_to_type(any, &asn_DEF_DirectoryString, (void *) &ds) && ds) {
	value = _directory_string_to_string(ds, encoding);
	if (value)
	    ADD;
    }
    else if (!ANY_to_type(any, &asn_DEF_IA5String, (void *) &ia5) && ia5) {
	strcpy(encoding, "ia5");
	value = PyString_FromStringAndSize((void *) ia5->buf, (size_t) ia5->size);
	if (value)
	    ADD;
    }
}

/*
 * From RFC 3280:
 *
 *   This specification requires only a subset of the name comparison functionality specified in the
 *   X.500 series of specifications.  Conforming implementations are REQUIRED to implement the
 *   following name comparison rules:
 *
 *      (a) attribute values encoded in different types (e.g., PrintableString and BMPString) MAY be
 *      assumed to represent different strings;
 *
 *      (b) attribute values in types other than PrintableString are case sensitive (this permits
 *      matching of attribute values as binary objects);
 *
 *      (c) attribute values in PrintableString are not case sensitive (e.g., "Marianne Swanson" is
 *      the same as "MARIANNE SWANSON"); and
 *
 *      (d) attribute values in PrintableString are compared after removing leading and trailing
 *      white space and converting internal substrings of one or more consecutive white space
 *      characters to a single space.
 *
 *   These name comparison rules permit a certificate user to validate certificates issued using
 *   languages or encodings unfamiliar to the certificate user.
 *
 * What this means for us:
 *
 * - we need to return both the string contents and the encoding for each string
 * - we need to normalize PrintableString values as noted above
 */
static PyObject *
_directory_string_to_string(DirectoryString_t *ds, char encoding[16])
{
    PyObject *retval = NULL;
    const char *from;
    char *allocated = NULL, *to;

    if (ds->present == DirectoryString_PR_printableString) {
	/*
	 * According to RFC 4517, PrintableString matches:
	 *
	 *   [A-Za-z0-9'()+,.=/:? -]+
	 *
	 * so we can just set the encoding to ascii here after doing the appropriate normalization.
	 */
	allocated = to = PyMem_Malloc(strlen((void *) ds->choice.printableString.buf) + 1);
	for (from = (const char *) ds->choice.printableString.buf; isspace(*from); ++from)
	    ; /* skip initial whitespace */
	for (; *from; ++from)
	    switch (*from) {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		    /* convert sequence of white space to single space */
		    while (1) {
			switch (*from) {
			    case ' ':
			    case '\t':
			    case '\r':
			    case '\n':
				++from;
				break;
			    default:
				--from;
				goto NonWS;
			}
		    }
		NonWS: *to++ = ' ';
		    break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': 
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': 
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		    /* 
		     * NOTE: I don't convert case here, because it seems better to preserve it for
		     * display purposes, but it's essential to do that when comparing for name
		     * equality.
		     */
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case '(': case ')': case '+': case ',': case '.': case '=': case '/': case ':': case '?': case '-':
		case '\'':
		    /* other characters we allow, despite the spec: */
		case '@':
		    *to++ = *from;
		    break;

		default:
		    /* illegal char - replace with asterisk */
		    *to++ = '*';
		    break;
	    }	
	if (to != allocated && to[-1] == ' ')
	    to[-1] = '\0'; /* strip trailing whitespace and null-terminate */
	else
	    *to = '\0'; /* null-terminate */
	strcpy(encoding, "ascii");
	retval = PyString_FromString(allocated);
    }
    else if (ds->present == DirectoryString_PR_utf8String) {
	strcpy(encoding, "utf8");
	retval = PyString_FromStringAndSize((void *) ds->choice.utf8String.buf, (size_t) ds->choice.utf8String.size);
    }
    else if (ds->present == DirectoryString_PR_teletexString) {
	/* obsolete, but still used (e.g., by Google) */
	strcpy(encoding, "x500-teletex");
	retval = PyString_FromStringAndSize((void *) ds->choice.teletexString.buf, (size_t) ds->choice.teletexString.size);
    }
    else if (ds->present == DirectoryString_PR_universalString) {
	/* obsolete */
	strcpy(encoding, "x500-universal");
	retval = PyString_FromStringAndSize((void *) ds->choice.universalString.buf, (size_t) ds->choice.universalString.size);
    }
    else if (ds->present == DirectoryString_PR_bmpString) {
	/* obsolete */
	strcpy(encoding, "x500-bmp");
	retval = PyString_FromStringAndSize((void *) ds->choice.bmpString.buf, (size_t) ds->choice.bmpString.size);
    }
    else
	strcpy(encoding, "x500-unknown");

    if (allocated)
	PyMem_Free(allocated);

    return retval;
}

/* get the list of extensions; note that we only parse the ones we understand, but get get the critical flag for all */
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
		/* 
		 * TBD: these might be necessary to validate some certs:
		 *
		 *  issuerAltName { 2.5.29.18 }
		 *  nameConstraints { 2.5.29.30 }
		 *  CRLDistributionPoints { 2.5.29.31 }
		 */
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
		    /* NOTE: we only check for the dNSName type here; we just ignore the others */
		    tmp = PyString_FromString("subjectAltName");
		    PyDict_SetItemString(dict, "name", tmp);
		    Py_DECREF(tmp);
		    if (ext->extnValue.size) {
			rval = ber_decode(0, &asn_DEF_SubjectAltName, (void **) &subjectAltName,
					  (const void *) ext->extnValue.buf, (size_t) ext->extnValue.size);
			if (rval.code == RC_OK && subjectAltName) {
			    dNSNames = PyFrozenSet_New(NULL);
			    for (j = 0; j < subjectAltName->list.count; j++) {
				gn = subjectAltName->list.array[j];
				if (gn && gn->present == GeneralName_PR_dNSName && gn->choice.dNSName.buf) {
				    dNSName = PyString_FromStringAndSize((void *) gn->choice.dNSName.buf, (size_t) gn->choice.dNSName.size);
				    PySet_Add(dNSNames, dNSName); /* does not steal reference */
				    Py_DECREF(dNSName);
				}
			    }
			    if (PySet_Size(dNSNames))
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

static PyObject *
cx509_get_public_key(cx509 *self)
{
    PyObject *dict, *tmp;
    const char *algorithm_oid;
    const char *algorithm_name;
    SubjectPublicKeyInfo_t *spki;
    RSAPublicKey_t *rsapk = NULL;
    asn_dec_rval_t rval;
    char *modulus;
    char *publicExponent;

    dict = PyDict_New();
    spki = &self->certificate->tbsCertificate.subjectPublicKeyInfo;

    algorithm_oid = _oid_to_string(&spki->algorithm.algorithm);
    /* TBD: make sure spki->algorithm.parameters is empty, otherwise fail */
    tmp = PyString_FromString((void *) algorithm_oid);
    PyDict_SetItemString(dict, "algorithm_oid", tmp);
    Py_DECREF(tmp);

    algorithm_name = find_oid(algorithm_oid, /*shortname:*/ 0);
    tmp = PyString_FromString((void *) algorithm_name);
    PyDict_SetItemString(dict, "algorithm", tmp);
    Py_DECREF(tmp);

    tmp = PyString_FromStringAndSize((void *) spki->subjectPublicKey.buf, spki->subjectPublicKey.size);
    PyDict_SetItemString(dict, "key", tmp);
    Py_DECREF(tmp);

    tmp = PyInt_FromLong(8 * spki->subjectPublicKey.size - spki->subjectPublicKey.bits_unused);
    PyDict_SetItemString(dict, "keylen", tmp);
    Py_DECREF(tmp);

    /* if we know about this algorithm, decode the key */
    if (!strcmp(algorithm_oid, "{ 1.2.840.113549.1.1.1 }")) {
	rval = ber_decode(0, &asn_DEF_RSAPublicKey, 
			  (void **) &rsapk,
			  (const void *) spki->subjectPublicKey.buf, 
			  (size_t) spki->subjectPublicKey.size);
	if (rval.code == RC_OK) {
	    /* the modulus is usually huge, so we need to convert it to hex, then have Python parse it into a long (bignum) */
	    modulus = _integer_to_hex_string(&rsapk->modulus);
	    if (modulus) {
		tmp = PyLong_FromString(modulus, NULL, 16);
		PyDict_SetItemString(dict, "modulus", tmp);
		PyMem_Free(modulus);
		Py_DECREF(tmp);
	    }

	    /* the public exponent is usually small (e.g., 3 or 65537), but we make no assumptions here */
	    publicExponent = _integer_to_hex_string(&rsapk->publicExponent);
	    if (publicExponent) {
		tmp = PyLong_FromString(publicExponent, NULL, 16);
		PyDict_SetItemString(dict, "public_exponent", tmp);
		PyMem_Free(publicExponent);
		Py_DECREF(tmp);
	    }
	}
    }
    
    return dict;
}

static PyObject *
cx509_get_signature_algorithm(cx509 *self)
{
    char *dotted;
    const char *algorithm_name;

    dotted = _oid_to_string(&self->certificate->signatureAlgorithm.algorithm);
    algorithm_name = find_oid(dotted, /*shortname:*/ 0);
    if (!algorithm_name) {
	Py_INCREF(Py_None);
	return Py_None;
    }
    return PyString_FromString(algorithm_name);
}

static PyObject *
cx509_get_signature(cx509 *self)
{
    return NULL;
}

static PyObject *
cx509_get_digest_info(cx509 *self)
{
    return NULL;
}

static char *
_oid_to_string(OBJECT_IDENTIFIER_t *oid)
{
    size_t count = 0;
    void *allocated, *output;

    if (!oid)
	return NULL;

    if (OBJECT_IDENTIFIER_print(NULL, (const void *) oid, 0, _print2count, (void *) &count))
	return NULL;

    /* allocate and print to allocated buffer */
    allocated = output = PyMem_Malloc(count + 1);

    /* write the output */
    if (OBJECT_IDENTIFIER_print(NULL, (const void *) oid, 0, _print2buffer, (void *) &output)) {
	PyMem_Free(output);
	return NULL;
    }
    
    *((char *) output) = 0; /* null-terminate */

    return allocated;
}

/**
 * Find OID name or shortname from dotted string.
 */
static const char *
find_oid(const char *dotted, int shortname)
{
    int lo, hi, len;
    oid_t *oids;

    if (shortname) {
	oids = OID_short_names;
	if (nOIDs_short_names) {
	    len = nOIDs_short_names;
	}
	else {
	    for (len = 0; OID_short_names[len].dotted; ++len);
	    nOIDs_short_names = len;
	}
    }
    else {
	oids = OIDs;
	if (nOIDs) {
	    len = nOIDs;
	}
	else {
	    for (len = 0; OIDs[len].dotted; ++len);
	    nOIDs = len;
	}
    }

    lo = 0;
    hi = len - 1;
    while (lo <= hi) {
	int mid = ((unsigned int) lo + (unsigned int) hi) >> 1;
	int cmp = strcmp(dotted, oids[mid].dotted);
	if (cmp > 0)
	    lo = mid + 1;
	else if (cmp < 0)
	    hi = mid - 1;
	else
	    return oids[mid].name;	/* found */
    }
    return NULL; /* not found */
}

/* convert an ASN.1 integer type to the equivalent sequence of hex digits */
static char *
_integer_to_hex_string(INTEGER_t *iptr)
{
    static const char *nybble = "0123456789ABCDEF";
    const unsigned char *b;
    char *allocated = NULL, *output;

    if(!iptr || !iptr->buf || !iptr->size)
	return NULL;

    allocated = output = PyMem_Malloc(iptr->size * 2 + 1);
    for (b = iptr->buf; b < iptr->buf + iptr->size; b++) {
	*output++ = nybble[(((unsigned int) *b) >> 4) & 0xF];
	*output++ = nybble[((unsigned int) *b) & 0xF];
    }
    *output++ = '\0';
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
    {"get_validity", (PyCFunction) cx509_get_validity, METH_NOARGS, "Return (earliest, latest) valid date/time." },
    {"get_issuer", (PyCFunction) cx509_get_issuer, METH_NOARGS, "Return a dict with information about the certificate issuer." },
    {"get_subject", (PyCFunction) cx509_get_subject, METH_NOARGS, "Return a dict with information about the certificate subject." },
    {"get_public_key", (PyCFunction) cx509_get_public_key, METH_NOARGS, "Return a dict with information about the public key." },
    {"get_signature_algorithm", (PyCFunction) cx509_get_signature_algorithm, METH_NOARGS, "Return the name of the signature algorithm." },
    {"get_signature", (PyCFunction) cx509_get_signature, METH_NOARGS, "Return the raw bytes of the signature." },
    {"extensions", (PyCFunction) cx509_extensions, METH_NOARGS, "Return list of extensions." },

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
