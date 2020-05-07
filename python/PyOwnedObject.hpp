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

