/* ----------------------------------------------------------------------------
 *  C library to provide a 'python module interface' to Mandlebrot Set
 *  calculators
 */


#include <stdio.h>

#define MANDLEBROT_MODULE
#include "mandlebrot.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject *SpamError;

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
static PyObject * float64(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int maxiter = 0;
    double Xs, Xe, Ys, Ye;

    if (!PyArg_ParseTuple(args, "(ii)idddd", &wsize, &hsize, &maxiter, &Xs, &Xe, &Ys, &Ye))
        return NULL;

    /* check bounds of parameters */
    if((wsize <= 0) || (hsize <= 0) || (maxiter <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, check display size.");
        return NULL;
    }
    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int *bytearray; //[wsize * hsize * 3];
    bytearray = (int*)calloc((size_t)(wsize * hsize * 3), sizeof(int));

    /* call mandlebrot_bytearray */
    mandlebrot_bytearray_c(wsize, hsize, maxiter, Xs, Xe, Ys, Ye, &bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }
    free(bytearray);
    
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
static PyObject * mpfr(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int maxiter = 0;

    if (!PyArg_ParseTuple(args, "(ii)i", &wsize, &hsize, &maxiter))
        return NULL;

    /* check bounds of parameters */
    if((wsize <= 0) || (hsize <= 0) || (maxiter <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, check display size.");
        return NULL;
    }
    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int *bytearray; //[wsize * hsize * 3];
    bytearray = (int*)calloc((size_t)(wsize * hsize * 3), sizeof(int));

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }
    free(bytearray);

    return points;
}

/* ----------------------------------------------------------------------------
 */
static PyObject * mpfr_slice(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int nslice = 0;
    int slice = 0;
    int maxiter = 0;

    if (!PyArg_ParseTuple(args, "(ii)iii", &wsize, &hsize, &nslice, &slice, &maxiter))
        return NULL;

    /* check bounds of parameters */
    if((wsize <= 0) || (hsize <= 0) || (nslice < 1) || (slice > nslice) || (maxiter <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, check display size and slice(s).");
        return NULL;
    }
    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int *bytearray; //[wsize * hsize/nslice * 3];
    bytearray = (int*)calloc((size_t)(wsize * hsize * 3), sizeof(int));
    printf("bytearray length = %d\n",wsize * hsize/nslice * 3);

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_slice_c(wsize, hsize, nslice, slice, maxiter, &bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize/nslice * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }
    free(bytearray);

    return points;
}

/* ----------------------------------------------------------------------------
 */
static PyObject * mpfr_thread(PyObject *self, PyObject *args)
{
    int wsize = 0;
    int hsize = 0;
    int maxiter = 0;

    if (!PyArg_ParseTuple(args, "(ii)i", &wsize, &hsize, &maxiter))
        return NULL;

    /* check bounds of parameters */
    if((wsize <= 0) || (hsize <= 0) || (maxiter <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, check display size.");
        return NULL;
    }
    PyObject *points = PyList_New(0);

    /* create an array of integers to store the result of the mandlebrot calculation */
    int *bytearray; //[wsize * hsize * 3];
    bytearray = (int*)calloc((size_t)(wsize * hsize * 3), sizeof(int));

    /* call mandlebrot_bytearray */
    mandlebrot_mpfr_thread_c(wsize, hsize, maxiter, &bytearray);

    /* transfer returned values into PyList for return */
    for(int i = 0; i < (wsize * hsize * 3); i++)
    {
        PyList_Append(points, PyLong_FromLong((long) bytearray[i]));
    }
    free(bytearray);

    return points;
}

/* ----------------------------------------------------------------------------
 */
static PyObject * zoom_in(PyObject *self, PyObject *args)
{
    unsigned int pX, pY, w, h, factor;

    if (!PyArg_ParseTuple(args, "(ii)(ii)i", &pX, &pY, &w, &h, &factor))
        return NULL;

    /* check bounds of parameters */
    if((w <= 0) || (h <= 0) || (pX > w) || (pY > h) || (factor <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, position must be within display.");
        return NULL;
    }
    mpfr_zoom_in(pX, pY, w, h, factor);

    return PyLong_FromLong(0);
}

/* ----------------------------------------------------------------------------
 */
static PyObject * zoom_out(PyObject *self, PyObject *args)
{
    unsigned int pX, pY, w, h, factor;

    if (!PyArg_ParseTuple(args, "(ii)(ii)i", &pX, &pY, &w, &h, &factor))
        return NULL;

    /* check bounds of parameters */
    if((w <= 0) || (h <= 0) || (pX > w) || (pY > h) || (factor <= 0))
    {
        PyErr_SetString(SpamError, "Invalid Inputs, position must be within display.");
        return NULL;
    }
    mpfr_zoom_out(pX, pY, w, h, factor);

    return PyLong_FromLong(0);
}
/* ----------------------------------------------------------------------------
 */
static PyObject * init(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) /* no arguments */
        return NULL;

    setup_c();
    printf("setup_c() called.\n");
    //return PyLong_FromLong(0);
    
    /* return None */
    Py_INCREF(Py_None);
    return Py_None;
}
/* ----------------------------------------------------------------------------
 */
static PyObject * setup(PyObject *self, PyObject *args)
{
    const char *Xs, *Xe, *Ys, *Ye; /* do i need to free these at the end of the func */
    char *Cx = "99.9";
    char *Cy = "99.9";

    if (!PyArg_ParseTuple(args, "ssss|ss", &Xs, &Xe, &Ys, &Ye, &Cx, &Cy))
        return NULL;

    initialize_c(Xs, Xe, Ys, Ye, Cx, Cy);

    //return PyLong_FromLong(0); 
    
    /* return None */
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * tidyup(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "")) /* no arguments */
        return NULL;
    
    free_mpfr_mem_c();
    return PyLong_FromLong(0);
}

/* ----------------------------------------------------------------------------
 * Python Module Stuff  - from docs.python.org/3/extending/extending.html
 */
static PyMethodDef MandlebrotMethods[] = {
    //{"mandlebrot",           mandlebrot,             METH_VARARGS, "calculate mandlebrot set" },
    {"float64",          float64,          METH_VARARGS, PyDoc_STR("calculate mandlebrot set into a bytearray") },
    {"mpfr",             mpfr,             METH_VARARGS, PyDoc_STR("calculate mandlebrot set using mpfr lib") },
    {"mpfr_slice",       mpfr_slice,       METH_VARARGS, PyDoc_STR("calculate mandlebrot set slice using mpfr lib") },
    {"mpfr_thread",      mpfr_thread,      METH_VARARGS, PyDoc_STR("calculate mandlebrot set using mpfr lib threaded") },
    {"zoom_in",          zoom_in,          METH_VARARGS, PyDoc_STR("calculate next mandlebrot set zoom values") },
    {"zoom_out",         zoom_out,         METH_VARARGS, PyDoc_STR("calculate previous mandlebrot set zoom values") },
    {"init",             init,             METH_VARARGS, PyDoc_STR("init() -> None") },   /*  */
    {"setup",            setup,            METH_VARARGS, PyDoc_STR("setup() -> None") },  /*  */
    {"tidyup",           tidyup,           METH_VARARGS, PyDoc_STR("tidyup() -> None") }, /* tidyup the mpfr memory */
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

    SpamError = PyErr_NewException("mandlebrot.error", NULL, NULL);
    Py_XINCREF(SpamError);
    if (PyModule_AddObject(m, "error", SpamError) < 0) {
        Py_XDECREF(SpamError);
        Py_CLEAR(SpamError);
        Py_DECREF(m);
        return NULL;
    }
    /* call setup routine */
    setup_c();

    return m;
}

