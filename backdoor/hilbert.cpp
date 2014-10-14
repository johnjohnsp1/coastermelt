/*
 * Fast hilbert curve transform in C++
 *
 *  Adapted from http://en.wikipedia.org/wiki/Hilbert_curve
 *  Inspired by http://xkcd.com/195/
 *  Deterministically and contiguously maps an (x,y)
 *  coordinate in a 4096x4096 image to a 24-bit value.
 *
 * Copyright (c) 2014 Micah Elizabeth Scott
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Python.h>
#include <algorithm>


static unsigned hilbert(unsigned x, unsigned y, unsigned size)
{
    unsigned d = 0;
    for (unsigned s = size/2; s; s >>= 1) {
        unsigned rx = (x & s) > 0;
        unsigned ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);

        if (ry == 0) {
            if (rx == 1) {
                x = s-1 - x;
                y = s-1 - y;
            }
            std::swap(x,y);
        }
    }
    return d;
}

static PyObject* py_hilbert(PyObject *self, PyObject *args)
{
    unsigned x, y, size = 4096;

    if (!PyArg_ParseTuple(args, "II|I", &x, &y, &size)) {
        return 0;
    }

    return PyLong_FromUnsignedLong(hilbert(x, y, size));
}

static PyObject* py_test(PyObject *self)
{
    // Make sure the mapping is 1:1

    unsigned* buffer = new unsigned[0x1000000];
    if (!buffer) return PyErr_NoMemory();

    memset(buffer, 0, 0x4000000);

    for (unsigned x = 0; x < 4096; x++) {
        for (unsigned y = 0; y < 4096; y++) {

            unsigned a = hilbert(x, y, 4096);

            if (a > 0xFFFFFF) {
                PyErr_SetString(PyExc_ValueError, "Hilbert result out of range");
                delete buffer;
                return 0;
            }

            if (buffer[a]) {            
                PyErr_SetString(PyExc_ValueError, "Location used twice");
                delete buffer;
                return 0;
            }

            buffer[a] = 1;
        }
    }

    delete buffer;
    Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    { "hilbert", (PyCFunction) py_hilbert, METH_VARARGS,
      "hilbert(x, y, size=4096) -> address\n"
    },
    { "test", (PyCFunction) py_test, METH_NOARGS,
      "test() -> None\n"
    },
    {0}
};

PyMODINIT_FUNC inithilbert(void)
{
    Py_InitModule3("hilbert", module_methods, 0);
}
