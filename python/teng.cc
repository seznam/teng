/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: tengmodule.cc,v 1.15 2008-11-18 13:24:01 burlog Exp $
 *
 * DESCRIPTION
 * Teng python module.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 */

#include <Python.h>
#include <new>
#include <set>
#include <memory>

#include <cstdlib>

#include "teng.h"
#include "tengudf.h"

#include <iostream>
#include <stdexcept>

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

using namespace Teng;

/** @short version of python */
#define MY_PYTHON_VER (PY_MAJOR_VERSION * 10 + PY_MINOR_VERSION)

#if MY_PYTHON_VER < 16
#define IS_SEQUENCE(data) (PySequence_Check(data) && !PyString_Check(data))
#define SEQUENCE_SIZE(seq) (PySequence_Length(seq))
#else
#define IS_SEQUENCE(data) (PySequence_Check(data) &&\
    !(PyString_Check(data) || PyUnicode_Check(data)))
#define SEQUENCE_SIZE(seq) (PySequence_Size(seq))
#endif

/** @short Encoding used when no encoding supplied */
static const std::string DEFAULT_DEFAULT_ENCODING = "utf-8";

/** @short Format used when no format supplied */
static const std::string DEFAULT_DEFAULT_CONTENT_TYPE = "";

/**
 * @short Python Teng object (teng.Teng).
 *
 * Contains C++ classes! They must be constructed && destructed
 * inplace!!!
 */
struct TengObject {
    PyObject_HEAD

    /**
     * @short Teng engine.
     */
    Teng_t teng;

    /**
     * @short Default encoding for unicode objects.
     */
    std::string defaultEncoding;

    /**
     * @short Default content type of template.
     */
    std::string defaultContentType;
};

/**
 * @short Deallocates TengObject.
 * @param self dealocated object
 */
static void Teng_dealloc(TengObject *self);

/**
 * @short Gets attributes of teng object.
 * @param self this object
 * @param name name of attribute
 * @return found attribute or 0 (==exception)
 */
static PyObject* Teng_getattr(TengObject *self, char *name);

/**
 * @short Creates new Teng object.
 * @param self this object
 * @param args arguments
 * @param keywds keyword arguments
 * @return new Teng object or 0 (==exception)
 */
static PyObject* Teng_Teng(TengObject *self, PyObject *args,
                           PyObject *keywds);

static PyObject* Teng_createDataRoot(TengObject *self, PyObject *args,
                                     PyObject *keywds);

/**
 * @short Definition of type of Teng object
 */
static PyTypeObject Teng_Type = {
    PyObject_HEAD_INIT(&Teng_Type)
    0,                                 /*ob_size*/
    "Teng",                            /*tp_name*/
    sizeof (TengObject),               /*tp_basicsize*/
    0,                                 /*tp_itemsize*/
    /* methods */
    (destructor) Teng_dealloc,         /*tp_dealloc*/
    0,                                 /*tp_print*/
    (getattrfunc) Teng_getattr,        /*tp_getattr*/
    0,                                 /*tp_setattr*/
    0,                                 /*tp_compare*/
    0,                                 /*tp_repr*/
    0,                                 /*tp_as_number*/
    0,                                 /*tp_as_sequence*/
    0,                                 /*tp_as_mapping*/
    0,                                 /*tp_hash*/
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    0,                                 /* tp_flags */
    0,                                 /* tp_doc */
};

void Teng_dealloc(TengObject *self) {
    using std::string;
    // call (inplace) destructor for engine
    self->teng.~Teng_t();
    // call (inplace) destructor for default encoding
    self->defaultEncoding.~string();
    // call (inplace) destructor for default content type
    self->defaultContentType.~string();
#if (MY_PYTHON_VER >= 22)
    // destroy memory
    PyObject_Del(self);
#endif
}

PyObject* Teng_Teng(TengObject *self, PyObject *args, PyObject *keywds) {
    // allowed keywords
    static const char *kwlist[] = {"root", "encoding", "contentType",
                                   "logToOutput", "errorFragment",
                                   "validate", "templateCacheSize",
                                   "dictionaryCacheSize", 0};

    // argument values
    const char *root = 0;
    const char *encoding = 0;
    const char *contentType = 0;
    int logToOutput = 0;
    int errorFragment = 0;
    int validate = 0;
    int templateCacheSize = 0;
    int dictionaryCacheSize = 0;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|zzziiiii:Teng",
                                    (char **)kwlist,
                                     &root, &encoding, &contentType,
                                     &logToOutput, &errorFragment,
                                     &validate, &templateCacheSize,
                                     &dictionaryCacheSize))
        return 0;

    if (templateCacheSize < 0) templateCacheSize = 0;
    if (dictionaryCacheSize < 0) dictionaryCacheSize = 0;

#if (MY_PYTHON_VER < 20)
    // create new memory for object
    TengObject *s = (TengObject *) _PyObject_New(&Teng_Type);
#else
    // create new memory for object
    TengObject *s = PyObject_New(TengObject, &Teng_Type);
#endif

    // check for error
    if (!s) return 0;

    // create settings
    Teng_t::Settings_t settings(0, false, templateCacheSize,
                                dictionaryCacheSize);

    try {
        // create teng object
        new (&s->teng) Teng_t((root) ? root : std::string(), settings);

        // set default encoding
        new (&s->defaultEncoding) std::string(encoding ? encoding
                                              : DEFAULT_DEFAULT_ENCODING);

        // set default contentType
        new (&s->defaultContentType) std::string(
                contentType ? contentType : DEFAULT_DEFAULT_CONTENT_TYPE);
        // OK
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }

    // return created teng object
    return reinterpret_cast<PyObject*>(s);
}

static int stringFromPythonString(PyObject *pystr, std::string &str) {
#if MY_PYTHON_VER < 16
    // get plain string
    char *value = PyString_AsString(pystr);
    if (!value) return -1;
    // append it to given string
    str.append(value);
#else
    // get plain string
    char *value;
    Py_ssize_t valueLength;
    if (PyString_AsStringAndSize(pystr, &value, &valueLength))
        return -1;
    // append it to given string
    str.append(value, valueLength);
#endif
    // OK
    return 0;
}

/**
 * @short Converts python structures into Teng data (fragments).
 */
class DataConverter_t {
public:
    /**
     * @short Creates new encoder.
     * @param encoding encoding for converting unicode data
     */
    DataConverter_t(const std::string &encoding)
        : encoding(encoding), objects()
    {}

    /**
     * @short Converts python structures into Teng data.
     *
     * Data can be list of dictionaries or dictionary. List of
     * dictionaries is merged into one dictionary.
     *
     * @param data source of fragment tree
     * @param fragment root fragment
     * @return 0 OK, !0 exception
     */
    int operator() (PyObject *data, Fragment_t &root) {
        if (IS_SEQUENCE(data)) {
            // if root is list, run through it
            int size = SEQUENCE_SIZE(data);
            if (size < 0) return -1;
            for (int pos = 0; pos < size; ++pos) {
                // get next item (new reference!)
                PyObject *value = PySequence_GetItem(data, pos);
                if (!value) return -1;
                // if not dictionary, report as error
                if (!PyDict_Check(value)) {
                    PyErr_SetString(PyExc_TypeError,
                                    "Root list must contain only "
                                    "dictionaries.");
                    return -1;
                }
                // make fragment from this value
                if (makeFragment(value, root)) {
                    // forget reference to value
                    Py_XDECREF(value);
                    return -1;
                }
                // forget reference to value
                Py_XDECREF(value);
            }
        } else if (PyDict_Check(data)) {
            // dictionary -> make fragment from data
            if (makeFragment(data, root))
                return -1;
        } else {
            // other data => error
            PyErr_SetString(PyExc_TypeError,
                            "Root fragment must be dictionary or "
                            "list of dictionaries only.");
            return -1;
        }

        // OK
        return 0;
    }

    /**
     * @short Converts python structures into child Teng fragment
     *        inserted into parent.
     *
     * Used in Fragment.addFragment()
     *
     * @param data source of fragment tree
     * @param name name of child fragment
     * @param fragment root fragment
     * @return 0 OK, !0 exception
     */
    Fragment_t* operator() (PyObject *data, const std::string &name,
                            Fragment_t &parent)
    {
        Fragment_t &child = parent.addFragment(name);
        if (makeFragment(data, child))
            return 0;
        return &child;
    }

    /**
     * @short Adds new variable into fragment.
     * @param name name of variable
     * @param data value of variable
     * @param framgnet destination fragmengt
     * @return 0 OK, !0 exception
     */
    int addVariable(const std::string &name, PyObject *data,
                    Fragment_t &fragment)
    {
        if (PyString_Check(data)) {
            // string object -> get data and assign
            std::string value;
            if (stringFromPythonString(data, value))
                return -1;
            // add variable
            fragment.addVariable(name, value);
#if MY_PYTHON_VER > 15
        } else if (PyUnicode_Check(data)) {
            // UNICODE is supported in versions > 1.5!!!
            // unicode object -> get data, convert to propper encoding and
            // assign

            PyObject *str = PyUnicode_AsEncodedString(data, encoding.c_str(),
                                                      "replace");
            if (!str) return -1;
            std::string value;
            if (stringFromPythonString(str, value)) {
                Py_XDECREF(str);
                return -1;
            }
            // add variable
            fragment.addVariable(name, value);
            // destroy temporary string
            Py_XDECREF(str);
#endif
        } else if (PyInt_Check(data)) {
            // int object -> get int, convert to string and assign
            Teng::IntType_t i = PyInt_AsLong(data);
            // check for error
            if (PyErr_Occurred()) return -1;
            // add variable
            fragment.addVariable(name, i);
        } else if (PyLong_Check(data)) {
            // int object -> get int, convert to string and assign
            Teng::IntType_t i = PyLong_AsLongLong(data);
            // check for error
            if (PyErr_Occurred()) return -1;
            // add variable
            fragment.addVariable(name, i);
        } else if (PyFloat_Check(data)) {
            // double object -> get double, convert to string and assign
            double d = PyFloat_AsDouble(data);
            fragment.addVariable(name, d);
        } else if (data == Py_None) {
            // none object -> assing empty string
            fragment.addVariable(name, std::string());
        } else {
            // other object -> get string representation and assign
            PyObject *str = PyObject_Str(data);
            if (!str) return -1;
            std::string value;
            if (stringFromPythonString(str, value)) {
                Py_XDECREF(str);
                return -1;
            }
            // add variable
            fragment.addVariable(name, value);
            // destroy temporary string
            Py_XDECREF(str);
        }

        // OK
        return 0;
    }

private:
    /**
     * @short Creates new fragment list from given sequence.
     * @param data sequence of fragments
     * @param fragmentList fragment list being filled with data
     * @return 0 OK, !0 exception
     */
    int makeFragmentList(PyObject *data, FragmentList_t &fragmentList) {
        // check for cycles
        if (objects.find(data) != objects.end()) {
            PyErr_SetString(PyExc_AttributeError,
                            "Data contain cycles! Processing aborted.");
            return -1;
        }
        // remember this object
        objects.insert(data);
        // run through sequence and add fragments to the list
        int size = SEQUENCE_SIZE(data);
        if (size < 0) return -1;
        for (int pos = 0; pos < size; ++pos) {
            // get next item (new reference!)
            PyObject *value = PySequence_GetItem(data, pos);
            if (!value) return -1;
            // create new fragment
            Fragment_t &subFragment = fragmentList.addFragment();
            // fill it
            if (makeFragment(value, subFragment)) {
                // forget reference to value
                Py_XDECREF(value);
                return -1;
            }
            // forget reference to value
            Py_XDECREF(value);
        }
        // forget this object
        objects.erase(data);

        // OK
        return 0;
    }

    /**
     * @short Creates new fragment
     * @param data source of fragment (dictionary)
     * @param fragment fragment being filled with data
     * @return 0 OK, !0 exception
     */
    int makeFragment(PyObject *data, Fragment_t &fragment) {
        if (!PyDict_Check(data)) {
            PyErr_SetString(PyExc_AttributeError,
                            "Fragment must be dictionary");
            return -1;
        }
        // check for cycles
        if (objects.find(data) != objects.end()) {
            PyErr_SetString(PyExc_AttributeError,
                            "Data contain cycles! Processing aborted.");
            return -1;
        }
        // remember this object
        objects.insert(data);
        PyObject *key;
        PyObject *value;
        // run through dictionary and fill in fragment
        for (Py_ssize_t pos = 0; PyDict_Next(data, &pos, &key, &value); ) {
            // only string keys are allowed
            if (!PyString_Check(key)) continue;
            // get key value
            std::string name;
            if (stringFromPythonString(key, name))
                return -1;
            if (IS_SEQUENCE(value)) {
                // sequence (not string!) -> fragment list
                if (makeFragmentList(value, fragment.addFragmentList(name)))
                    return -1;
            } else if (PyDict_Check(value)) {
                // dictionary -> singe fragment
                if (makeFragment(value, fragment.addFragment(name)))
                    return -1;
            } else {
                // variable
                if (addVariable(name, value, fragment))
                    return -1;
            }
        }
        // forget this object
        objects.erase(data);

        // OK
        return 0;
    }

    /**
     * @short Encoding for converting unicode -> string.
     */
    const std::string encoding;

    /**
     * @short Set of so far encountered object. Used in cycle
     * detection.
     */
    std::set<PyObject*> objects;
};

/**
 * @short Crete python error log from C++ error log.
 * @param err C++ error log
 * @return python error log or 0
 */
static PyObject* createErrorLog(Error_t &err) {
    // get entries from error log
    const std::vector<Error_t::Entry_t> &entries = err.getEntries();
    // allocate appropriate tuple
    PyObject *log = PyTuple_New(entries.size());
    if (!log) return 0;

    // run through entries
    int pos = 0;
    for (std::vector<Error_t::Entry_t>::const_iterator
             ientries = entries.begin();
         ientries != entries.end(); ++ientries, ++pos) {
        // create new python entry from C++ entry
        PyObject *entry =
            Py_BuildValue("{s:i,s:s#,s:i,s:i,s:s#}",
                          "level", ientries->level,
                          "filename", ientries->pos.filename.data(),
                          ientries->pos.filename.length(),
                          "line", ientries->pos.lineno,
                          "column", ientries->pos.col,
                          "message", ientries->message.data(),
                          ientries->message.length());
        if (!entry) {
            // on error destroy python error log
            Py_XDECREF(log);
            return 0;
        }

        // add entry into python error log
        if (PyTuple_SetItem(log, pos, entry)) {
            // on error destroy python error log and entry
            Py_XDECREF(log);
            Py_XDECREF(entry);
            return 0;
        }
    }

    // OK
    return log;
}

static char Teng_Teng__doc__[] =
"Create new teng engine\n"
"arguments:\n"
"    root                      Root path for relative paths\n"
"    encoding                  Default encoding for generatePage()\n"
"    contentType               Defautl contentType for generatePage()\n"
"    templateCacheSize         Specifies maximal number of templates in the\n"
"                              cache.\n"
"    dictionaryCacheSize       Specifies maximal number of dictionaries\n"
"                              in the cache.\n"
;

static char Teng_generatePage__doc__[] =
"Generate page from template, dictionaries and data.\n"
"\n"
"arguments:\n"
"1. source is:\n"
"    string templateFilename   Path to template.\n"
"    string skin               Skin -- appended after last dot of filename.\n"
"                              (..../x.html + std -> ..../x.std.html)\n"
"                              Accepted only together with templateFilename.\n"
"   or:\n"
"    string templateString     Template.\n"
"\n"
"2. output is\n"
"    string outputFilename     Path to output file.\n"
"   or:\n"
"    string outputFile         File object (must be open for writing)\n"
"                              or file-like object (must have write() method\n"
"   or:\n"
"    Returned as item in result structure (see below)\n"
"\n"
"3. dictionaries are:\n"
"    string dictionaryFilename File with language dictionary.\n"
"    string language           Language variation.\n"
"                              Appended after last dot of filename.\n"
"                              (..../x.dict en -> ..../x.en.dict)\n"
"    string configFilename     File with configuration.\n"
"\n"
"4. and other arguments are:\n"
"    string contentType        Content-type of template.\n"
"                              Use teng.listSupportedContentTypes()\n"
"                              for accepted values\n"
"    string encoding           Encoding of data. Used for conversion\n"
"                              od Unicode object to 8bit strings.\n"
"\n"
"5. Data are:\n"
"    dict, list or Fragment data  Hierarchical data (see documentation).\n"
"\n"
"Or above measn exclusive or! 'templateFilename' and 'templateString'\n"
"cannot be specified together. Arguments'outputFilename' and 'outputFile'\n"
"collide too. When no output specified result will be stored in\n"
"result structure.\n"
"\n"
"Result:\n"
"    dict {\n"
"        string output         Result of page generation when no output\n"
"                              specified\n"
"        int status            Indicates error/no error during generation.\n"
"        tuple errorLog        error log\n"
"    }\n"
"\n"
;

static char Teng_listSupportedContentTypes__doc__[] =
"List content types supported by this engine.\n"
"\n"
"Returns tuple of two-item tuples with name of content type and \n"
"comment on the content type\n"
"\n"
;

static char Teng_dictionaryLookup__doc__[] =
"Finds item in dictionary.\n"
"\n"
"arguments:\n"
"    string dictionaryFilename  File with language dictionary.\n"
"    string language            Language variation.\n"
"                               Appended after last dot of filename.\n"
"                               (..../x.dict en -> ..../x.en.dict)\n"
"    string key                 item name.\n"
"    string configFilename = "" File with configuration.\n"
"\n"
"result:\n"
"    string or None \n"
"\n"
;

static char Teng_createDataRoot__doc__[] =
"Create root fragment.\n"
"\n"
"arguments:\n"
"    dict data                 Root fragment's variables and subfragmens.\n"
"    string encoding           Encoding for unicode conversions.\n"
"                              If ommitted use Teng's default.\n"
"\n"
"result:\n"
"    Created fragment (type Fragment).\n"
"\n"
;

static char Fragment_addFragment__doc__[] =
"Add sub fragment to this fragment.\n"
"\n"
"arguments:\n"
"    string name               Name new fragment.\n"
"    dict data                 Fragment's variables and subfragmens.\n"
"\n"
"result:\n"
"    Created sub fragment (type Fragment).\n"
"\n"
;

static char Fragment_addVariable__doc__[] =
"Add variable to this fragment.\n"
"\n"
"arguments:\n"
"    string name               Name of variable.\n"
"    object value              Value of variable.\n"
"\n"
"result:\n"
"    None.\n"
"\n"
;

static char Teng_registerUdf__doc__[] =
"Registers user-defined function.\n"
"\n"
"arguments:\n"
"    string functionName        Name of the function in udf. namespace.\n"
"    callable handler           Callable object.\n"
"\n"
"result:\n"
"    True or False \n"
"\n"
;


/**
 * @short finds item in dictionary
 * @param self python Teng object
 * @param args positional arguments
 * @param keywds keyword arguments
 * @return -1 error or 0 OK
 */
static PyObject* Teng_dictionaryLookup(TengObject *self, PyObject *args,
                                       PyObject *keywds)
{
    // allowed keywords
    static const char *kwlist[] = {"dictionaryFilename", "language", "key",
                             "configFilename", 0};

    // values of arguments
    const char *configFilename = "";
    const char *dictionaryFilename;
    const char *language;
    const char *key;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "sss|s:dictionaryLookup", (char **)kwlist,
                                     &dictionaryFilename, &language, &key,
                                     &configFilename))
        return 0;
    try {
        std::string s;
        if (self->teng.dictionaryLookup(configFilename, dictionaryFilename,
                                        language, key, s)) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return Py_BuildValue("s#", s.data(), s.length());
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }
}


/**
 * @short Generate page from template, dictionaries and data.
 * @param self python Teng object
 * @param args positional arguments
 * @param keywds keyword arguments
 * @return status or 0
 */
static PyObject* Teng_generatePage(TengObject *self,
                                   PyObject *args, PyObject *keywds);

/**
 * @short Mapping of Teng objects methods to C functions
 */
static PyMethodDef Teng_methods[] = {
    {
        "generatePage",
        (PyCFunction) Teng_generatePage,
        METH_VARARGS | METH_KEYWORDS,
        Teng_generatePage__doc__,
    }, {
        "createDataRoot",
        (PyCFunction) Teng_createDataRoot,
        METH_VARARGS | METH_KEYWORDS,
        Teng_createDataRoot__doc__
    }, {
        "dictionaryLookup",
        (PyCFunction) Teng_dictionaryLookup,
        METH_VARARGS | METH_KEYWORDS,
        Teng_dictionaryLookup__doc__
    },
    { 0, 0 } // end of map
};

PyObject* Teng_getattr(TengObject *self, char *name) {
    // find entry in Teng_methods
    return Py_FindMethod(Teng_methods, (PyObject *)self, name);
}

/**
 * @short List supported content types.
 * @param self teng module
 * @param args positional arguments
 * @return list of supported content types or 0
 */
static PyObject* listSupportedContentTypes(PyObject *self, PyObject *args) {
    // parse arguments
    if (!PyArg_ParseTuple(args, ":listSupportedContentTypes"))
        return 0;

    try {
        std::vector<std::pair<std::string, std::string> > vcontentTypes;
        Teng_t::listSupportedContentTypes(vcontentTypes);

        // create output list
        PyObject *contentTypes = PyTuple_New(vcontentTypes.size());
        if (!contentTypes) return 0;

        // run through entries
        int pos = 0;
        for (std::vector<std::pair<std::string, std::string> >::const_iterator
                 icontentTypes = vcontentTypes.begin();
             icontentTypes != vcontentTypes.end(); ++icontentTypes, ++pos) {
            // create new python string from C++ string
            PyObject *contentType =
                Py_BuildValue("(s#s#)",
                              icontentTypes->first.data(),
                              icontentTypes->first.length(),
                              icontentTypes->second.data(),
                              icontentTypes->second.length());
            if (!contentType) {
                // on error destroy list
                Py_XDECREF(contentTypes);
                return 0;
            }

            // add entry into python error log
            if (PyTuple_SetItem(contentTypes, pos, contentType)) {
                // on error destroy list and entry
                Py_XDECREF(contentTypes);
                Py_XDECREF(contentType);
                return 0;
            }
        }

        // return created tuple
        return contentTypes;
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }
}

// ===================================================================
// teng.Fragment object
// ===================================================================

class TengTree_t;

/**
 * @short Python Fragment object.
 *
 * Holds data tree and points to one fragment.
 */
struct FragmentObject {
    // python stuff
    PyObject_HEAD

    /** @short
     * @param
     * @return
     */
    TengTree_t *dataTree;

    /** @short
     * @param
     * @return
     */
    Fragment_t *fragment;
};

/** @short Teng tree with referencing support.
 */
class TengTree_t {
public:
    /** @short Create new tree.
     * @param encoding encoding passed to data builder
     */
    TengTree_t(const std::string &encoding)
        : referrers(), rootFragment(new Fragment_t),
          encoding(encoding)
    {}

    /** @short Add referring fragment.
     * @param referrer referring fragment
     */
    void addReferrer(FragmentObject *referrer) {
        referrers.insert(referrer);
    }

    /** @short Remove referring fragment.
     * @param referrer referring fragment
     */
    void removeReferrer(FragmentObject *referrer) {
        referrer->dataTree = 0;
        referrer->fragment = 0;
        referrers.erase(referrer);
    }

    /** @short Determine whether this tree is refferenced by given
     *         fragment.
     * @param referrer referring fragment
     * @return referrer refers to this tree
     */
    bool isReferredBy(FragmentObject *referrer) {
        return (referrers.find(referrer) != referrers.end());
    }

    /** @short Determine whether given fragment refers to root
     *         fragment.
     * @param fragment fragment
     * @return fragment is root
     */
    bool isRoot(FragmentObject *fragment) {
        return fragment->fragment == rootFragment;
    }

    /** @short Get root fragment.
     * @return root fragment
     */
    Fragment_t* getRoot() {
        return rootFragment;
    }

    /** @short Get encoding.
     * @return encoding
     */
    const std::string& getEncoding() {
        return encoding;
    }

    /** @short Destroy the tree and invalidate all referrers.
     */
    ~TengTree_t() {
        // invalidate all fragments
        for (std::set<FragmentObject *>::iterator
                 ireferrers = referrers.begin();
             ireferrers != referrers.end(); ++ireferrers) {
            (*ireferrers)->dataTree = 0;
            (*ireferrers)->fragment = 0;
        }

        // delete the tree
        delete rootFragment;
    }

private:
    /** @short Referring fragments.
     */
    std::set<FragmentObject *> referrers;

    /** @short The tree.
     */
    Fragment_t *rootFragment;

    /** @short Encoding.
     */
    std::string encoding;
};

/**
 * @short Deallocates FragmentObject.
 * @param self dealocated object
 */
static void Fragment_dealloc(FragmentObject *self);

/**
 * @short Gets attributes of fragment object.
 * @param self this object
 * @param name name of attribute
 * @return found attribute or 0 (==exception)
 */
static PyObject* Fragment_getattr(FragmentObject *self, char *name);

/**
 * @short Definition of type of Fragment object
 */
static PyTypeObject Fragment_Type = {
    PyObject_HEAD_INIT(&Fragment_Type)
    0,                                 /*ob_size*/
    "Fragment",                        /*tp_name*/
    sizeof (FragmentObject),           /*tp_basicsize*/
    0,                                 /*tp_itemsize*/
    /* methods */
    (destructor) Fragment_dealloc,     /*tp_dealloc*/
    0,                                 /*tp_print*/
    (getattrfunc) Fragment_getattr,    /*tp_getattr*/
    0,                                 /*tp_setattr*/
    0,                                 /*tp_compare*/
    0,                                 /*tp_repr*/
    0,                                 /*tp_as_number*/
    0,                                 /*tp_as_sequence*/
    0,                                 /*tp_as_mapping*/
    0,                                 /*tp_hash*/
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    0,                                 /* tp_flags */
    0,                                 /* tp_doc */
};

void Fragment_dealloc(FragmentObject *self) {
    // if fragment points to tree this tree knows about it
    if (self->dataTree && self->dataTree->isReferredBy(self)) {
        // if fragment is root remove whole tree
        if (self->dataTree->isRoot(self)) {
            delete self->dataTree;
        } else {
            // else remove it from referrers
            self->dataTree->removeReferrer(self);
        }
    }
#if (MY_PYTHON_VER >= 22)
    // destroy memory
    PyObject_Del(self);
#endif
}

PyObject* Teng_createDataRoot(TengObject *self, PyObject *args,
                                  PyObject *keywds)
{
    // allowed keywords
    static const char *kwlist[] = {"data", "encoding", 0};

    // values of arguments
    PyObject *data = 0;
    const char *pencoding = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|z:createDataRoot", (char **)kwlist,
                                     &data, &pencoding))
        return 0;

    try {
        // determine encoding
        std::string encoding = (pencoding ? std::string(pencoding)
                                : self->defaultEncoding);

        // create (empty) data tree
        std::auto_ptr<TengTree_t> dataTree(new TengTree_t(encoding));

        // if any data given
        if (data) {
            // process them
            if (DataConverter_t(encoding)(data, *dataTree->getRoot()))
                return 0;
        }

#if (MY_PYTHON_VER < 20)
        // create new memory for object
        FragmentObject *fragment =
            reinterpret_cast<FragmentObject *>(_PyObject_New(&Fragment_Type));
#else
        // create new memory for object
        FragmentObject *fragment = PyObject_New(FragmentObject, &Fragment_Type);
#endif

        // check for error
        if (!fragment) return 0;

        // fill fragment with data tree and fragment pointer
        fragment->dataTree = dataTree.release();
        fragment->fragment = fragment->dataTree->getRoot();
        // remember this fragment
        fragment->dataTree->addReferrer(fragment);

        // OK
        return reinterpret_cast<PyObject*>(fragment);
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }
}
/**
 * @short Generate page from template, dictionaries and data.
 * @param self python Teng object
 * @param args positional arguments
 * @param keywds keyword arguments
 * @return status or 0
 */
static PyObject* Fragment_addFragment(FragmentObject *self,
                                      PyObject *args, PyObject *keywds)
{
    // allowed keywords
    static const char *kwlist[] = {"name", "data", 0};

    // values of arguments
    const char *name = 0;
    PyObject *data = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "sO:addFragment", (char **)kwlist,
                                     &name, &data))
        return 0;


    try {
        // create child fragment
        Fragment_t *childFragment =
            DataConverter_t(self->dataTree->getEncoding())
            (data, std::string(name), *self->fragment);

        if (!childFragment) return 0;

#if (MY_PYTHON_VER < 20)
        // create new memory for object
        FragmentObject *child = (FragmentObject *)_PyObject_New(&Fragment_Type);
#else
        // create new memory for object
        FragmentObject *child = PyObject_New(FragmentObject, &Fragment_Type);
#endif

        // check for error
        if (!child) return 0;

        // fill fragment with data tree and fragment pointer
        child->dataTree = self->dataTree;
        child->fragment = childFragment;
        // remember this fragment
        self->dataTree->addReferrer(child);

        return reinterpret_cast<PyObject*>(child);
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }
}

/**
 * @short Generate page from template, dictionaries and data.
 * @param self python Teng object
 * @param args positional arguments
 * @param keywds keyword arguments
 * @return status or 0
 */
static PyObject* Fragment_addVariable(FragmentObject *self,
                                      PyObject *args, PyObject *keywds)
{
    // allowed keywords
    static const char *kwlist[] = {"name", "value", 0};

    // values of arguments
    const char *name = 0;
    PyObject *value = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "sO:addVariable",
                                     (char **)kwlist, &name, &value))
        return 0;

    try {
        if (DataConverter_t(self->dataTree->getEncoding())
            .addVariable(name, value, *self->fragment))
            return 0;
    } catch (std::bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }

    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

/**
 * @short Mapping of Fragment objects methods to C functions
 */
static PyMethodDef Fragment_methods[] = {
    {
        "addFragment",
        (PyCFunction) Fragment_addFragment,
        METH_VARARGS | METH_KEYWORDS,
        Fragment_addFragment__doc__,
    },
    {
        "addVariable",
        (PyCFunction) Fragment_addVariable,
        METH_VARARGS | METH_KEYWORDS,
        Fragment_addVariable__doc__,
    },
    { 0, 0 } // end of map
};

PyObject* Fragment_getattr(FragmentObject *self, char *name) {
    // find entry in Fragment_methods
    return Py_FindMethod(Fragment_methods, (PyObject *)self, name);
}

namespace {
    /** @short Exception used to escape from teng is some python
     *         exception occures.
     *
     */
    struct PyException_t {
        PyException_t() {}
    };

    /** @short Output writer. Writes to the associated python
     *         file-like object.
     */
    class PyWriter_t : public Writer_t {
    public:
        /** @short Creates new writer. Associates py file-like object.
         *  @param obj py file-like object
         */
        PyWriter_t(PyObject *obj);

        /** @short Destroy writer. If exception occured, hold it here.
         */
        virtual ~PyWriter_t();

        /** @short Write given string to output.
         *  @param str string to be written
         *  @return 0 OK, !0 error
         */
        virtual int write(const std::string &str);

        /** @short Write given string to output.
         *  @param str string to be written
         *  @return 0 OK, !0 error
         */
        virtual int write(const char *str);

        /** @short Write given string to output.
         *  @param str string to be written
         *  @param interval iterators to given string, only this part
         *                  shall be written
         *  @return 0 OK, !0 error
         */
        virtual int write(const std::string &str,
                          std::pair<std::string::const_iterator,
                          std::string::const_iterator> interval);

        /** @short Flush buffered data to the output.
         *  No-op.
         *  @return 0 OK, !0 error
         */
        virtual int flush();

    private:
        int bufferData(const char *data, int length);

        int writeData(const char *data, int length);

        int fetchException();

        /** @short Associated py file-like object.
         */
        PyObject *obj;

        /** Flags whether underlying file-like object has method
         *  flush().
         */
        bool hasFlush;

        /** Buffer size.
         */
        static const int BUFF_SIZE = 1 << 12;

        /** Data buffer used for buffered writing.
         */
        char buffer[BUFF_SIZE];

        /** Length of used space in buffer.
         */
        int bufferUsed;

        /** Exception.
         */
        PyObject *exc_type;
        PyObject *exc_value;
        PyObject *exc_traceback;
    };

    // definition
    const int PyWriter_t::BUFF_SIZE;

    inline bool hasMethod(PyObject *obj, const char *method_) {
        // grrrr :-(
        char *method(const_cast<char*>(method_));

        // check whether we have such attribute
        if (!PyObject_HasAttrString(obj, method)) return false;
        PyObject *pymethod(PyObject_GetAttrString(obj, method));
        if (!pymethod) throw PyException_t();

        // check whether attributes is callable
        bool res = PyCallable_Check(pymethod);
        Py_DECREF(pymethod);

        return res;
    }

    PyWriter_t::PyWriter_t(PyObject *obj)
        : obj(obj), hasFlush(false), bufferUsed(0),
          exc_type(), exc_value(), exc_traceback()
    {
        // check whether obj has write() method
        if (!hasMethod(obj, "write")) {
            PyErr_SetString(PyExc_AttributeError,
                            "outputFile doesn't have write() method.");
            throw PyException_t();
        }

        // determine whether obj has method flush
        hasFlush = hasMethod(obj, "flush");
    }

    PyWriter_t::~PyWriter_t() {
        // flush
        flush();

        // restore exception (if any)
        if (exc_type && !std::uncaught_exception()) {
            PyErr_Restore(exc_type, exc_value, exc_traceback);
            throw PyException_t();
        }

        // get rid of exception
        Py_XDECREF(exc_type);
        Py_XDECREF(exc_value);
        Py_XDECREF(exc_traceback);
    }

    int PyWriter_t::flush() {
        // check for error
        if (exc_type) return -1;

        // flush buffer's content if any
        if (bufferUsed) {
            if (writeData(buffer, bufferUsed)) return -1;
            bufferUsed = 0;
        }

        // no-op if no flush method
        if (!hasFlush) return 0;

        PyObject *res = PyObject_CallMethod(obj, (char *)"flush", 0);
        if (!res) return fetchException();

        // ignore result
        Py_DECREF(res);
        return 0;
    }

    int PyWriter_t::bufferData(const char *data, int length) {
        // check for error
        if (exc_type) return -1;

        // check for buffer overflow
        if ((BUFF_SIZE - bufferUsed) < length) {
            // flush buffer
            if (flush()) return -1;

            // check whether string fits to data buffer
            if (length > BUFF_SIZE) {
                // write data without buffering and return
                if (writeData(data, length)) return -1;
                return 0;
            }
        }

        // remember data
        ::memcpy(buffer + bufferUsed, data, length);
        bufferUsed += length;

        // OK
        return 0;
    }

    int PyWriter_t::writeData(const char *data_, int length) {
        // sanity check
        if (length <= 0) return 0;

        // grrrr
        void *data(const_cast<char*>(data_));

        // create buffer from memory
        PyObject *pydata = PyBuffer_FromMemory(data, length);
        if (!pydata) return fetchException();

        // call obj.write(pydata)
        PyObject *res = PyObject_CallMethod(obj, (char *)"write",
                                            (char *)"O", pydata);
        // get rid of py-data-buffer
        Py_DECREF(pydata);
        if (!res) return fetchException();

        // OK
        return 0;
    }

    int PyWriter_t::fetchException() {
        // fetch exception
        PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);

        // log error
        err.logError(Error_t::LL_FATAL, Error_t::Position_t(),
                     "Python exception occured.");

        // always fail
        return -1;
    }

    int PyWriter_t::write(const std::string &str) {
        // buffer string's content
        return bufferData(str.data(), str.size());
    }

    int PyWriter_t::write(const char *str) {
        // buffer string's content
        return bufferData(str, ::strlen(str));
    }

    int PyWriter_t::write(const std::string &str,
                          std::pair<std::string::const_iterator,
                          std::string::const_iterator> interval)
    {
        // buffer string's content in the range
        // <interval.first, interval.second)
        return bufferData(str.data() + std::distance(str.begin(),
                                                     interval.first),
                          std::distance(interval.first, interval.second));
    }
}

PyObject* Teng_generatePage(TengObject *self,
                            PyObject *args, PyObject *keywds)
{
    // allowed keywords
    static const char *kwlist[] = {"templateFilename", "skin", "templateString",
                                   "dataDefinitionFilename",
                                   "dictionaryFilename", "language",
                                   "configFilename", "contentType", "data",
                                   "outputFilename", "outputFile", "encoding",
                                   0};

    // values of arguments
    const char *templateFilename = 0;
    const char *skin = 0;
    const char *templateString = 0;
    int templateLength = 0;
    const char *dataDefinitionFilename = 0;
    const char *dictionaryFilename = 0;
    const char *language = 0;
    const char *configFilename = 0;
    const char *contentType = 0;
    PyObject *data = 0;
    const char *outputFilename = 0;
    PyObject *outputFile = 0;
    const char *pencoding = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "|zzz#zzzzzOzOz:generatePage",
                                     (char **)kwlist,
                                     &templateFilename, &skin,
                                     &templateString, &templateLength,
                                     &dataDefinitionFilename,
                                     &dictionaryFilename, &language,
                                     &configFilename, &contentType, &data,
                                     &outputFilename, &outputFile,
                                     &pencoding))
        return 0;

    // status of page generation
    int status = 0;

    // check template single source
    if (templateFilename && templateString) {
        // both sources
        PyErr_SetString(PyExc_AttributeError,
                        "You cannot supply both 'templateFilename'"
                        " and 'templateString'.");
        return 0;
    } else if (!(templateFilename || templateString)) {
        // no source
        PyErr_SetString(PyExc_AttributeError,
                        "You must supply 'templateFilename'"
                        " or 'templateString'.");
        return 0;
    }

    // check that language supplied only together with dictionary
    if (language && !dictionaryFilename) {
        PyErr_SetString(PyExc_AttributeError,
                        "Language without dictionary has no meaning.");
        return 0;
    }

    // check output single destination (none is OK)
    if (outputFilename && outputFile) {
        // both outputs
        PyErr_SetString(PyExc_AttributeError,
                        "You cannot supply both 'outputFilename'"
                        " and 'outputFile'.");
        return 0;
    }

    try {
        // output writer
        std::auto_ptr<Writer_t> writer(0);
        // indicates that writer is string writer
        bool stringOutput = false;
        // the output from string writer
        std::string output;

        if (outputFilename) {
            // output to file -> create new file writer
            writer.reset(new FileWriter_t(outputFilename));
        } else if (outputFile) {
            if (PyFile_Check(outputFile)) {
                // create new file writer
                writer.reset(new FileWriter_t(PyFile_AsFile(outputFile)));
            } else {
                // probably file-like object (throws exception
                // PyException_t if no write() method is found
                writer.reset(new PyWriter_t(outputFile));
            }
        } else {
            // output to string
            writer.reset(new StringWriter_t(output));
            // indicate string writer
            stringOutput = true;
        }

        std::string encoding = (pencoding ? std::string(pencoding)
                                : self->defaultEncoding);

        // root fragment
        Fragment_t defaultRoot;

        // root defaults to (empty) default-root
        Fragment_t *root = &defaultRoot;

        // this smart-pointer will guard temporary root built from
        // python native data (if any)
        std::auto_ptr<Fragment_t> rootGuard(0);

        // if any data given, convert then to fragment
        if (data) {
            if (data->ob_type == &Fragment_Type) {
                FragmentObject *
                    fragment = reinterpret_cast<FragmentObject*>(data);
                if (!fragment->dataTree) {
                    PyErr_SetString(PyExc_AttributeError,
                                    "Fragment object points to deleted data.");
                    return 0;
                }
                if (!fragment->dataTree->isRoot(fragment)) {
                    PyErr_SetString(PyExc_AttributeError,
                                    "This fragment is not a root fragment.");
                    return 0;
                }
                root = fragment->dataTree->getRoot();
                encoding = fragment->dataTree->getEncoding();
            } else {
                root = new Fragment_t();
                // remember new root;
                rootGuard.reset(root);
                // convert data
                if (DataConverter_t(encoding)(data, *root))
                    return 0;
            }
        }
        // error log
        Error_t err;

        // this macro converts C string into C++ string
        // NULL pointer is converted as empty string
#define S(str) (str ? std::string(str) : std::string())
        // this macro converts C string into C++ string
        // NULL pointer is converted as defaul value
#define SD(str, default) (str ? std::string(str) : default)
        // this macro converts C string into C++ string
        // NULL pointer is converted as empty string
        // len is length of string
#define SL(str, len) (str ? std::string(str, len) : std::string())
        // generate page from file
        if (templateFilename) {
            status = self->teng.generatePage(S(templateFilename), S(skin),
                                             S(dictionaryFilename), S(language),
                                             S(configFilename),
                                             SD(contentType,
                                                self->defaultContentType),
                                             encoding, *root, *writer, err);
        } else {
            status = self->teng.generatePage(SL(templateString, templateLength),
                                             S(dictionaryFilename), S(language),
                                             S(configFilename),
                                             SD(contentType,
                                                self->defaultContentType),
                                             encoding, *root, *writer, err);
        }
#undef SL
#undef SD
#undef S

        // get rid of writer (we have to catch any exception it can
        // throw)
        delete writer.release();

        // process error log
        PyObject *errorLog = createErrorLog(err);
        if (!errorLog) return 0;

        // create result object
        PyObject *result = Py_BuildValue("{s:i,s:s#,s:O}", "status", status,
                                         "output", output.data(),
                                         output.length(),
                                         "errorLog", errorLog);
        Py_XDECREF(errorLog);

        // return result
        return result;
    } catch (const std::bad_alloc &ba) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    } catch (const PyException_t &pe) {
        return 0;
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        return 0;
    } catch (...) {
        PyErr_SetString(PyExc_Exception, "Unknown C++ exception.");
        return 0;
    }
}

class PythonUdf_t {
    protected:
        std::string m_name;
        PyObject *m_callback;

        std::string getErrorMessage() {
            PyObject *pyErrType = 0, *pyErrValue = 0, *pyErrTB = 0;
            PyObject *pyStr;
            std::string errMsg;

            PyErr_Fetch(&pyErrType, &pyErrValue, &pyErrTB);
            if ( (pyErrType != 0) && ((pyStr = PyObject_Repr(pyErrType)) != 0) ) {
                errMsg = std::string(PyString_AsString(pyStr));
                Py_DECREF(pyStr);
                if ( (pyErrValue != 0) && ((pyStr = PyObject_Repr(pyErrValue)) != 0) ) {
                    errMsg += " : " + std::string(PyString_AsString(pyStr));
                    Py_DECREF(pyStr);
                }
            }
            Py_XDECREF(pyErrType);
            Py_XDECREF(pyErrValue);
            Py_XDECREF(pyErrTB);
            return errMsg;
        }

    public:
        PythonUdf_t(const std::string &name, PyObject *callback)
            : m_name (name), m_callback(callback) {
            Py_INCREF(m_callback);
        }

        PythonUdf_t(const PythonUdf_t &o)
        : m_name(o.m_name), m_callback(o.m_callback) {
            Py_INCREF(m_callback);
        }

        UDFValue_t operator()(const std::vector<UDFValue_t> &args) {
            PyObject *pyArgs = PyTuple_New(args.size()), *obj;
            PyObject *pyRes = 0;
            Py_ssize_t pos = 0;
            UDFValue_t result(std::string("undefined"));

            if ( pyArgs == 0 ) {
                throw std::runtime_error("Unable to allocate arg tuple");
            }

            for (std::vector<UDFValue_t>::const_iterator it = args.begin(); it != args.end(); it++) {

                switch ( it->getType() ) {
                    case UDFValue_t::Integer:
                        obj = PyLong_FromLongLong(it->getInt());
                        break;
                    case UDFValue_t::Real:
                        obj = PyFloat_FromDouble(it->getReal());
                        break;
                    case UDFValue_t::String:
                        obj = PyString_FromString(it->getString().c_str());
                        break;
                    default:
                        obj = 0;
                        break;
                }

                if ( obj == 0 ) {
                    Py_DECREF(pyArgs);
                    throw std::runtime_error("Unable to pass argument");
                }
                PyTuple_SetItem(pyArgs, pos++, obj);
            }

            try {
                pyRes = PyObject_Call(m_callback, pyArgs, 0);
                Py_DECREF(pyArgs);

                if ( pyRes == 0 ) {
                    throw std::runtime_error(getErrorMessage());
                }

                if ( PyInt_Check(pyRes) ) {
                    result.setInt(PyInt_AsLong(pyRes));
                } else if ( PyLong_Check(pyRes) ) {
                    result.setInt(PyLong_AsLong(pyRes));
                } else if ( PyFloat_Check(pyRes) ) {
                    result.setReal(PyFloat_AsDouble(pyRes));
                } else if ( PyString_Check(pyRes) ) {
                    result.setString(std::string(PyString_AsString(pyRes)));
                } else {
                    Py_DECREF(pyRes);
                    std::runtime_error("Return value can be int, float or string");
                }
            } catch (...) {
                Py_XDECREF(pyRes);
                Py_DECREF(pyArgs);
                throw std::runtime_error(getErrorMessage());
            }

            Py_DECREF(pyArgs);
            Py_DECREF(pyRes);

            return result;
        }

        ~PythonUdf_t() {
            Py_DECREF(m_callback);
        }
};


static PyObject* registerUdf(PyObject *self, PyObject *args) {
    const char *name;
    PyObject *callback = 0;

    if (!PyArg_ParseTuple(args, "sO:registerUdf", &name, &callback))
        return 0;

    if ( !PyCallable_Check(callback) ) {
        PyErr_SetString(PyExc_ValueError, "Invalid callback");
        return 0;
    }

    if ( findUDF("udf." + std::string(name)) ) {
        //PyErr_SetString(PyExc_ValueError, "Duplicated callback name");
        Py_INCREF(Py_False);
        return Py_False;
    }

    registerUDF(name, PythonUdf_t(name, callback));

    Py_INCREF(Py_True);
    return Py_True;
}

// ===================================================================
// module methods
// ===================================================================

/**
 * @short Mapping of teng module methods to C functions
 */
static PyMethodDef teng_methods[] = {
    {
        "Teng",
        (PyCFunction) Teng_Teng,
        METH_VARARGS | METH_KEYWORDS,
        Teng_Teng__doc__,
    }, {
        "listSupportedContentTypes",
        (PyCFunction) listSupportedContentTypes,
        METH_VARARGS,
        Teng_listSupportedContentTypes__doc__,
    }, {
        "registerUdf",
        (PyCFunction) registerUdf,
        METH_VARARGS,
        Teng_registerUdf__doc__,
    },
    { 0, 0 } // end of map
};

/**
 * @short Initialize teng module
 */
extern "C" DL_EXPORT(void) initteng(void) {
    /* Create the module and add the functions */
    Py_InitModule("teng", teng_methods);
}
