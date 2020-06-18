//===- PyOwnedObject.hpp ----------------------------------------*- C++ -*-===//
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
#include <Python.h>

class PyOwnedObject
{
public:
    /// Constructs from a Python object and transfers ownership of it to
    /// this object, which will decrement the reference count when finished.
    PyOwnedObject(PyObject * obj = nullptr) : mObject(obj) {}

    PyOwnedObject(const PyOwnedObject & other) : mObject(other.mObject)
    {
        if (mObject)
            Py_INCREF(mObject);
    }

    PyOwnedObject & operator=(PyOwnedObject const & lhs)
    {
        if (&lhs != this)
        {
            if (mObject)
                Py_DECREF(mObject);
            mObject = lhs.mObject;
            if (mObject)
                Py_IncRef(mObject);
        }
        return *this;
    }

    explicit operator bool() const { return mObject; }

    PyObject * get() const { return mObject; }

    [[nodiscard]] PyObject * release()
    {
        PyObject * result = mObject;
        mObject = nullptr;
        return result;
    }

    ~PyOwnedObject()
    {
        if (mObject)
            Py_DECREF(mObject);
    }

private:
    PyObject * mObject;
};

