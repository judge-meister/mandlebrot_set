/* ----------------------------------------------------------------------------
 *  C library to provide a 'python module interface' to Mandlebrot Set
 *  calculators
 */


#include <stdio.h>

#include "mandlebrot.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>


/* ----------------------------------------------------------------------------
 * Python function interface - mandlebrot_bytearray
 * mandlebrot_bytearray - return the results as a list of color values to be
 *                        converted to a bytesarray by the caller
 *
 * Params - (contained in PyObject *args)
 * wsize (int) - width of display screen/window
 * hsize (int) - height of display screen/window
 * Xs, Xe, Ys, Ye (double) bounds of mandlebrot set to calculate
 * maxiter (int) - escape value for mandlebrot calc
 *
 * Returns a PyList containing color values for all the calculated points
 */
static PyObject * mandlebrot_bytearray(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int maxiter = 0;
    double Xs, Xe, Ys, Ye;

    if (!PyArg_ParseTuple(args, "iiidddd", &wsize, &hsize, &maxiter, &Xs, &Xe, &Ys, &Ye))
        return NULL;

    /*printf("params, %d,%d %1.20lf,%1.20lf %1.20lf,%1.20lf  %d\n", wsize,hsize, Xs,Xe, Ys,Ye, maxiter);*/
    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int bytearray[wsize * hsize * 3];

    /* call mandlebrot_bytearray */
    mandlebrot_bytearray_c(wsize, hsize, maxiter, Xs, Xe, Ys, Ye, bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }

    return points;
}

/* ----------------------------------------------------------------------------
 * Python function interface - mandlebrot_bytearray
 * mandlebrot_bytearray - return the results as a list of color values to be
 *                        converted to a bytesarray by the caller
 *
 * Params - (contained in PyObject *args)
 * wsize (int) - width of display screen/window
 * hsize (int) - height of display screen/window
 * Xs, Xe, Ys, Ye (char[]) bounds of mandlebrot set to calculate as strings
 * maxiter (int) - escape value for mandlebrot calc
 *
 * Returns a PyList containing color values for all the calculated points
 */
static PyObject * mandlebrot_mpfr(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int maxiter = 0;

    if (!PyArg_ParseTuple(args, "iii", &wsize, &hsize, &maxiter))
        return NULL;

    //printf("params, %d x %d, %d %s, %s  %s, %s\n", wsize,hsize, maxiter, Xs,Xe, Ys,Ye);

    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int bytearray[wsize * hsize * 3];

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_c(wsize, hsize, maxiter, bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }

    return points;
}

/* ----------------------------------------------------------------------------
 */
static PyObject * mandlebrot_zoom_in(PyObject *self, PyObject *args)
{
    unsigned int pX, pY, w, h, factor;

    if (!PyArg_ParseTuple(args, "iiiii", &pX, &pY, &w, &h, &factor))
        return NULL;

    mpfr_zoom_in(pX, pY, w, h, factor);

    return PyLong_FromLong(0);
}

/* ----------------------------------------------------------------------------
 */
static PyObject * mandlebrot_zoom_out(PyObject *self, PyObject *args)
{
    unsigned int pX, pY, w, h;

    if (!PyArg_ParseTuple(args, "iiii", &pX, &pY, &w, &h))
        return NULL;

    //mpfr_zoom_out(pX, pY, w, h);

    return PyLong_FromLong(0);
}
/* ----------------------------------------------------------------------------
 */
static PyObject * setup(PyObject *self, PyObject *args)
{
    setup_c();
    printf("setup_c()\n");
    return PyLong_FromLong(0);
}
/* ----------------------------------------------------------------------------
 */
static PyObject * initialize(PyObject *self, PyObject *args)
{
    const char *Xs, *Xe, *Ys, *Ye; /* do i need to free these at the end of the func */

    if (!PyArg_ParseTuple(args, "ssss", &Xs, &Xe, &Ys, &Ye))
        return NULL;

    initialize_c(Xs, Xe, Ys, Ye);

    return PyLong_FromLong(0);
}


/* ----------------------------------------------------------------------------
 * Python Module Stuff  - from docs.python.org/3/extending/extending.html
 */
static PyMethodDef MandlebrotMethods[] = {
    //{"mandlebrot",           mandlebrot,             METH_VARARGS, "calculate mandlebrot set" },
    {"mandlebrot_bytearray",   mandlebrot_bytearray,  METH_VARARGS, "calculate mandlebrot set into a bytearray" },
    {"mandlebrot_mpfr",        mandlebrot_mpfr,       METH_VARARGS, "calculate mandlebrot set using mpfr lib" },
    {"mandlebrot_zoom_in",     mandlebrot_zoom_in,    METH_VARARGS, "calculate next mandlebrot set zoom values" },
    {"mandlebrot_zoom_out",    mandlebrot_zoom_out,   METH_VARARGS, "calculate previous mandlebrot set zoom values" },
    {"initialize",    initialize,   METH_VARARGS, "" },
    {"setup",         setup,        METH_VARARGS, "" },
    {NULL, NULL, 0 , NULL}
};

/* ----------------------------------------------------------------------------
*/
static struct PyModuleDef mandlebrotmodule = {
    PyModuleDef_HEAD_INIT,
    "mandlebrot",
    "Python interface for the mandlebrot set C library", /* module documentation */
    -1,
    MandlebrotMethods
};

/* ----------------------------------------------------------------------------
*/
PyMODINIT_FUNC PyInit_mandlebrot(void)
{
    PyObject *m;

    m = PyModule_Create(&mandlebrotmodule);
    if (m == NULL)
        return NULL;

    /*SpamError = PyErr_NewException("spam.error", NULL, NULL);
    Py_XINCREF(SpamError);
    if (PyModule_AddObject(m, "error", SpamError) < 0) {
        Py_XDECREF(SpamError);
        Py_CLEAR(SpamError);
        Py_DECREF(m);
        return NULL;
    }*/

    return m;
}

/* ----------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    /* Add a built-in module, before Py_Initialize */
    if (PyImport_AppendInittab("mandlebrot", PyInit_mandlebrot) == -1) {
        fprintf(stderr, "Error: could not extend in-built modules table\n");
        exit(1);
    }

    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(program);

    /* Initialize the Python interpreter.  Required.
       If this step fails, it will be a fatal error. */
    Py_Initialize();

    /* Optionally import the module; alternatively,
       import can be deferred until the embedded script
       imports it. */
    /*PyObject *pmodule = PyImport_ImportModule("mandlebrot");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'mandlebrot'\n");
    }*/

    /*...*/
    setup_c();

    PyMem_RawFree(program);
    return 0;
}
