//===- SwappedArgsExt.cpp ---------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
// This material is based on research sponsored by the Department of Homeland
// Security (DHS) Office of Procurement Operations, S&T acquisition Division via
// contract number 70RSAT19C00000056. The views and conclusions contained herein
// are those of the authors and should not be interpreted as necessarily
// representing the official policies or endorsements, either expressed or
// implied, of the Department of Homeland Security.
//
//===----------------------------------------------------------------------===//
#define PY_SSIZE_T_CLEAN
#include "PyOwnedObject.hpp"
#include "SwappedArgChecker.hpp"

struct CheckerObject : PyObject {
  swapped_arg::Checker checker;
};

/// Tries to convert a Python unicode object to a C++ string.
static std::optional<std::string> PyStrToStr(PyObject* pystr) {
  Py_ssize_t size;
  const char* result = PyUnicode_AsUTF8AndSize(pystr, &size);
  if (!result)
    return std::nullopt;
  return std::string(result, size);
}

/// Converts a std::set of morphemes to a Python set.
static PyOwnedObject MorphemeSetToPy(const std::set<std::string>& morphemes) {
  PyOwnedObject result(PySet_New(nullptr));
  if (!result)
    return PyOwnedObject();
  for (const std::string& m : morphemes) {
    PyOwnedObject mPy(PyUnicode_FromStringAndSize(m.c_str(), m.size()));
    if (!mPy)
      return nullptr;
    if (PySet_Add(result.get(), mPy.get()) < 0)
      return nullptr;
  }

  return result;
}

/// Converts a Result into a Python dict.
static PyOwnedObject ResultToPy(const swapped_arg::Result& result) {
  PyOwnedObject resultDict(PyDict_New());
  if (!resultDict)
    return nullptr;

  PyOwnedObject arg1(PyLong_FromSize_t(result.arg1));
  if (!arg1)
    return nullptr;
  if (PyDict_SetItemString(resultDict.get(), "arg1", arg1.get()) < 0)
    return nullptr;

  PyOwnedObject arg2(PyLong_FromSize_t(result.arg2));
  if (!arg2)
    return nullptr;
  if (PyDict_SetItemString(resultDict.get(), "arg2", arg2.get()) < 0)
    return nullptr;

  PyOwnedObject morphemes1 = MorphemeSetToPy(result.morphemes1);
  if (!morphemes1)
    return nullptr;
  if (PyDict_SetItemString(resultDict.get(), "morphemes1", morphemes1.get()) <
      0)
    return nullptr;

  PyOwnedObject morphemes2 = MorphemeSetToPy(result.morphemes2);
  if (!morphemes2)
    return nullptr;
  if (PyDict_SetItemString(resultDict.get(), "morphemes2", morphemes2.get()) <
      0)
    return nullptr;

  return resultDict;
}

static PyObject* Checker_New(PyTypeObject* type, PyObject* args,
                             PyObject* kwds) {
  CheckerObject* self =
      reinterpret_cast<CheckerObject*>(type->tp_alloc(type, 0));
  if (self) {
    new (self) CheckerObject;
  }
  return self;
}

static void Checker_Dealloc(CheckerObject* self) {
  self->~CheckerObject();
  Py_TYPE(self)->tp_free(self);
}

static PyObject* Checker_Check(CheckerObject* self, PyObject* args,
                               PyObject* kwargs) {
  const char* callee = nullptr;
  PyObject* arguments = nullptr;
  PyObject* paramNames = nullptr;

  static const char* kwlist[] = {"arguments", "callee", "parameters",
                                 nullptr};
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|zO:check2",
                                   const_cast<char**>(kwlist), &arguments,
                                   &callee, &paramNames))
    return nullptr;

  // Create the call site from all of the arguments passed into this
  // function.
  swapped_arg::CallSite site;
  {
    PyOwnedObject iterator(PyObject_GetIter(arguments));
    if (!iterator) {
      PyErr_SetString(PyExc_TypeError, "arguments must be an iterable of str");
      return nullptr;
    }

    while (PyOwnedObject item = PyOwnedObject(PyIter_Next(iterator.get()))) {
      if (!PyUnicode_Check(item.get())) {
        PyErr_SetString(PyExc_TypeError,
                        "arguments must be an iterable of str");
        return nullptr;
      }
      std::optional<std::string> itemStr = PyStrToStr(item.get());
      if (!itemStr)
        return nullptr;
      site.positionalArgNames.push_back({*itemStr});
    }

    if (PyErr_Occurred())
      return nullptr;
  }
  if (paramNames && paramNames != Py_None) {
    PyOwnedObject iterator(PyObject_GetIter(paramNames));
    if (!iterator) {
      PyErr_SetString(PyExc_TypeError, "parameters must be an iterable of str");
      return nullptr;
    }

    site.callDecl.paramNames = std::vector<std::string>();
    while (PyOwnedObject item = PyOwnedObject(PyIter_Next(iterator.get()))) {
      if (!PyUnicode_Check(item.get())) {
        PyErr_SetString(PyExc_TypeError,
                        "parameters must be an iterable of str");
        return nullptr;
      }
      std::optional<std::string> itemStr = PyStrToStr(item.get());
      if (!itemStr)
        return nullptr;
      site.callDecl.paramNames->push_back(*itemStr);
    }

    if (PyErr_Occurred())
      return nullptr;
  }
  if (callee) {
    site.callDecl.fullyQualifiedName = callee;
  }

  PyOwnedObject resultList(PyList_New(0));
  if (!resultList)
    return nullptr;

  for (const auto& result : self->checker.CheckSite(site)) {
    PyOwnedObject resultDict = ResultToPy(result);
    if (PyList_Append(resultList.get(), resultDict.get()) < 0)
      return nullptr;
  }

  return resultList.release();
}

static PyMethodDef Checker_methods[] = {
    {"check_call", (PyCFunction)Checker_Check, METH_VARARGS | METH_KEYWORDS,
     "Checks a call site for swapped arguments.\n\n"
     ":param arguments: The expression names of each positional argument "
     "passed.\n"
     ":param callee: The name of the function being called.\n"
     ":param parameters: The names for the formal parameters of the callee.\n"
     ":returns: A list of dicts describing swaps (or an empty list if no "
     "swaps were found)."},
    {nullptr}};

static PyTypeObject Checker_Type = {
    PyVarObject_HEAD_INIT(nullptr, 0) //
    "swappedargs.Checker",
    sizeof(CheckerObject),
    0, /* tp_itemsize */
    reinterpret_cast<destructor>(&Checker_Dealloc),
    0,       /* tp_vectorcall_offset */
    nullptr, /* tp_getattr */
    nullptr, /* tp_setattr */
    nullptr, /* tp_as_async */
    nullptr, /* tp_repr */
    nullptr, /* tp_as_number */
    nullptr, /* tp_as_sequence */
    nullptr, /* tp_as_mapping */
    nullptr, /* tp_hash */
    nullptr, /* tp_call */
    nullptr, /* tp_str */
    nullptr, /* tp_getattro */
    nullptr, /* tp_setattro */
    nullptr, /* tp_as_buffer */
    0,       /* tp_flags */
    "The swapped argument checker",
    nullptr, /* tp_traverse */
    nullptr, /* tp_clear */
    nullptr, /* tp_richcompare */
    0,       /* tp_weaklistoffset */
    nullptr, /* tp_iter */
    nullptr, /* tp_iternext */
    Checker_methods,
    nullptr, /* tp_members */
    nullptr, /* tp_getset */
    nullptr, /* tp_base */
    nullptr, /* tp_dict */
    nullptr, /* tp_descr_get */
    nullptr, /* tp_descr_set */
    0,       /* tp_dictoffset */
    nullptr, /* tp_init */
    nullptr, /* tp_alloc */
    Checker_New,
};

static struct PyModuleDef Checker_Module = {
    PyModuleDef_HEAD_INIT,
    "swappedargs",
    "Checker module",
    -1,
};

PyMODINIT_FUNC PyInit_swappedargs(void) {
  if (PyType_Ready(&Checker_Type) < 0)
    return nullptr;

  PyOwnedObject m(PyModule_Create(&Checker_Module));
  if (!m)
    return nullptr;

  Py_INCREF(&Checker_Type);
  if (PyModule_AddObject(m.get(), "Checker", (PyObject*)&Checker_Type) < 0) {
    Py_DECREF(&Checker_Type);
    return nullptr;
  }

  return m.release();
}
