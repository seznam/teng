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
 * $Id: tengmodule.cc,v 1.2 2004-12-30 12:42:01 vasek Exp $
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

#include "teng.h"

#include <iostream>

using namespace std;

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
static const string DEFAULT_DEFAULT_ENCODING = "utf-8";

/** @short Format used when no format supplied */
static const string DEFAULT_DEFAULT_CONTENT_TYPE = "";

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
    string defaultEncoding;

    /**
     * @short Default content type of template.
     */
    string defaultContentType;
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
    static char *kwlist[] = {"root", "encoding", "contentType",
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
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|zzziiiii:Teng", kwlist,
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
    Teng_t::Settings_t settings(templateCacheSize, dictionaryCacheSize);

    try {
        // create teng object
        new (&s->teng) Teng_t((root) ? root : string(), settings);
        
        // set default encoding
        new (&s->defaultEncoding) string(encoding ? encoding
                                         : DEFAULT_DEFAULT_ENCODING);
        
        // set default contentType
        new (&s->defaultContentType) string(contentType ? contentType
                                            : DEFAULT_DEFAULT_CONTENT_TYPE);
        // OK
    } catch (bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }

    // return created teng object
    return reinterpret_cast<PyObject*>(s);
}

static int stringFromPythonString(PyObject *pystr, string &str) {
#if MY_PYTHON_VER < 16
    // get plain string
    char *value = PyString_AsString(pystr);
    if (!value) return -1;
    // append it to given string
    str.append(value);
#else
    // get plain string
    char *value;
    int valueLength;
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
    DataConverter_t(const string &encoding)
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
                if (!PyMapping_Check(value)) {
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
        } else if (PyMapping_Check(data)) {
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
    Fragment_t* operator() (PyObject *data, const string &name,
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
    int addVariable(const string &name, PyObject *data,
                    Fragment_t &fragment)
    {
        if (PyString_Check(data)) {
            // string object -> get data and assign
            string value;
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
            string value;
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
            long i = PyInt_AsLong(data);
            // add variable
            fragment.addVariable(name, i);
        } else if (PyFloat_Check(data)) {
            // double object -> get double, convert to string and assign
            double d = PyFloat_AsDouble(data);
            fragment.addVariable(name, d);
        } else if (data == Py_None) {
            // none object -> assing empty string
            fragment.addVariable(name, string());
        } else {
            // other object -> get string representation and assign
            PyObject *str = PyObject_Str(data);
            if (!str) return -1;
            string value;
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
        if (!PyMapping_Check(data)) {
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
        for (int pos = 0; PyDict_Next(data, &pos, &key, &value); ) {
            // only string keys are allowed
            if (!PyString_Check(key)) continue;
            // get key value
            string name;
            if (stringFromPythonString(key, name))
                return -1;
            if (IS_SEQUENCE(value)) {
                // sequence (not string!) -> fragment list
                if (makeFragmentList(value, fragment.addFragmentList(name)))
                    return -1;
            } else if (PyMapping_Check(value)) {
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
    const string encoding;

    /**
     * @short Set of so far encountered object. Used in cycle
     * detection.
     */
    set<PyObject*> objects;
};

/**
 * @short Crete python error log from C++ error log.
 * @param err C++ error log
 * @return python error log or 0
 */
static PyObject* createErrorLog(Error_t &err) {
    // get entries from error log
    const vector<Error_t::Entry_t> &entries = err.getEntries();
    // allocate appropriate tuple
    PyObject *log = PyTuple_New(entries.size());
    if (!log) return 0;

    // run through entries
    int pos = 0;
    for (vector<Error_t::Entry_t>::const_iterator
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
"    string outputFile         File object (must beopen for writing).\n"
"   or:\n"
"    Returned as item in result structure (see below)\n"
"\n"
"3. dictionaries are:\n"
"    string definitionFilename File with data definition.\n"
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
    static char *kwlist[] = {"dictionaryFilename", "configFilename",
                             "language", "key", 0};

    // values of arguments
    const char *configFilename = "";
    const char *dictionaryFilename;
    const char *language;
    const char *key;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "sss|s:dictionaryLookup", kwlist,
                                     &dictionaryFilename, &language, &key,
                                     &configFilename))
        return 0;
    try {
        string s;
        if (self->teng.dictionaryLookup(configFilename, dictionaryFilename,
                                        language, key, s)) {
            Py_INCREF(Py_None);
            return Py_None;
        }
        return Py_BuildValue("s#", s.data(), s.length());
    } catch (bad_alloc &e) {
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
        vector<pair<string, string> > vcontentTypes;
        Teng_t::listSupportedContentTypes(vcontentTypes);
        
        // create output list
        PyObject *contentTypes = PyTuple_New(vcontentTypes.size());
        if (!contentTypes) return 0;
        
        // run through entries
        int pos = 0;
        for (vector<pair<string, string> >::const_iterator
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
    } catch (bad_alloc &e) {
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
    TengTree_t(const string &encoding)
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
    const string& getEncoding() {
        return encoding;
    }

    /** @short Destroy the tree and invalidate all referrers.
     */
    ~TengTree_t() {
        // invalidate all fragments
        for (set<FragmentObject *>::iterator ireferrers = referrers.begin();
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
    set<FragmentObject *> referrers;

    /** @short The tree.
     */
    Fragment_t *rootFragment;

    /** @short Encoding.
     */
    string encoding;
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
    static char *kwlist[] = {"data", "encoding", 0};

    // values of arguments
    PyObject *data = 0;
    const char *pencoding = 0;
    
    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|z:createDataRoot", kwlist,
                                     &data, &pencoding))
        return 0;

    try {
        // determine encoding
        string encoding = (pencoding ? string(pencoding)
                           : self->defaultEncoding);
        
        // create (empty) data tree
        TengTree_t *dataTree = new TengTree_t(encoding);
        
        // if any data given
        if (data) {
            // process them
            if (DataConverter_t(encoding)(data, *dataTree->getRoot())) {
                // on error destroy data tree
                delete dataTree;
                return 0;
            }
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
        if (!fragment) {
            // on error destroy data tree
            delete dataTree;
            return 0;
        }
        
        // fill fragment with data tree and fragment pointer
        fragment->dataTree = dataTree;
        fragment->fragment = dataTree->getRoot();
        // remember this fragment
        dataTree->addReferrer(fragment);
        
        // OK
        return reinterpret_cast<PyObject*>(fragment);
    } catch (bad_alloc &e) {
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
    static char *kwlist[] = {"name", "data", 0};

    // values of arguments
    const char *name = 0;
    PyObject *data = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds, 
                                     "sO:addFragment", kwlist, &name, &data))
        return 0;


    try {
        // create child fragment
        Fragment_t *childFragment = 
            DataConverter_t(self->dataTree->getEncoding())
            (data, string(name), *self->fragment);
        
        if (!childFragment) return 0;
        
#if (MY_PYTHON_VER < 20)
        // create new memory for object
        FragmentObject *child = (FragmentObject *) _PyObject_New(&Fragment_Type);
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
    } catch (bad_alloc &e) {
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
    static char *kwlist[] = {"name", "value", 0};

    // values of arguments
    const char *name = 0;
    PyObject *value = 0;

    // parse arguments from input
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "sO:addVariable",
                                     kwlist, &name, &value))
        return 0;

    try {
        if (DataConverter_t(self->dataTree->getEncoding())
            .addVariable(name, value, *self->fragment))
            return 0;
    } catch (bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
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

PyObject* Teng_generatePage(TengObject *self,
                            PyObject *args, PyObject *keywds)
{
    // allowed keywords
    static char *kwlist[] = {"templateFilename", "skin", "templateString",
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
                                     "|zzz#zzzzzOzO!z:generatePage", kwlist,
                                     &templateFilename, &skin,
                                     &templateString, &templateLength,
                                     &dataDefinitionFilename,
                                     &dictionaryFilename, &language,
                                     &configFilename, &contentType, &data,
                                     &outputFilename,
                                     &PyFile_Type, &outputFile,
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
        Writer_t *writer = 0;
        // indicates that writer is string writer
        bool stringOutput = false;
        // the output from string writer
        string output;
        
        if (outputFilename) {
            // output to file -> create new file writer
            writer = new FileWriter_t(outputFilename);
        } else if (outputFile) {
            // create new file writer
            writer = new FileWriter_t(PyFile_AsFile(outputFile));
        } else {
            // output to string
            writer = new StringWriter_t(output);
            // indicate string writer
            stringOutput = true;
        }
        
        string encoding = (pencoding ? string(pencoding)
                           : self->defaultEncoding);
        
        // root fragment
        Fragment_t defaultRoot;
        
        Fragment_t *root = &defaultRoot;
        bool deleteRoot = false;
        // if any data given, convert then to fragment
        if (data) {
            if (data->ob_type == &Fragment_Type) {
                FragmentObject *fragment = reinterpret_cast<FragmentObject*>(data);
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
                deleteRoot = true;
                if (DataConverter_t(encoding)(data, *root)) {
                    delete root;
                    return 0;
                }
            }
        }
        // error log
        Error_t err;
        
        // this macro converts C string into C++ string
        // NULL pointer is converted as empty string
#define S(str) (str ? string(str) : string())
        // this macro converts C string into C++ string
        // NULL pointer is converted as defaul value
#define SD(str, default) (str ? string(str) : default)
        // this macro converts C string into C++ string
        // NULL pointer is converted as empty string
        // len is length of string
#define SL(str, len) (str ? string(str, len) : string())
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
        
        if (deleteRoot) delete root;
        
        // process error log
        PyObject *errorLog = createErrorLog(err);
        if (!errorLog) {
            delete writer;
            return 0;
        }
        
        // create result object
        PyObject *result = Py_BuildValue("{s:i,s:s#,s:O}", "status", status,
                                         "output", output.data(),
                                         output.length(),
                                         "errorLog", errorLog);
        Py_XDECREF(errorLog);
        
        // destroy writer
        delete writer;
        
        // return result
        return result;
    } catch (bad_alloc &e) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;
    }
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
