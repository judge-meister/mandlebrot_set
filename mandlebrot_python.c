/* ----------------------------------------------------------------------------
 *  C library to provide a 'python module interface' to Mandlebrot Set 
 *  calculators
 */


#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* ---------------------------------------------------------------------------- 
 * Header
 */
void mandlebrot_bytearray_c(const unsigned int wsize,   /* width of screen/display/window */
                            const unsigned int hsize,   /* height of screen/display/window */
                            const unsigned int maxiter, /* max iterations before escape */
                            const double Xs, /* string repr of mpfr_t for X top left */
                            const double Xe, /* string repr of mpfr_t for X top right */
                            const double Ys, /* string repr of mpfr_t for Y bottom left */
                            const double Ye, /* string repr of mpfr_t for Y bottom right */
                            int bytearray[] /* reference/pointer to result list of color values*/
                            );

void mandlebrot_mpfr_c(const unsigned int xsize,   /* width of screen/display/window */
                        const unsigned int ysize,   /* height of screen/display/window */
                        const unsigned int maxiter, /* max iterations before escape */
                        const char* Xs_str, /* string repr of mpfr_t for X top left */
                        const char* Xe_str, /* string repr of mpfr_t for X top right */
                        const char* Ys_str, /* string repr of mpfr_t for Y bottom left */
                        const char* Ye_str, /* string repr of mpfr_t for Y bottom right */
                        int bytearray[] /* reference/pointer to result list of color values*/
                        );

void mpfr_zoom_in(       const char** Xs_str, /* string repr of mpfr_t for X top left */
                         const char** Xe_str, /* string repr of mpfr_t for X top right */
                         const char** Ys_str, /* string repr of mpfr_t for Y bottom left */
                         const char** Ye_str, /* string repr of mpfr_t for Y bottom right */
                         const unsigned int pX, /* */
                         const unsigned int pY, /* */
                         const unsigned int w, /* */
                         const unsigned int h  /* */
                        );

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
	const char *Xs, *Xe, *Ys, *Ye;
	
	if (!PyArg_ParseTuple(args, "iiissss", &wsize, &hsize, &maxiter, &Xs, &Xe, &Ys, &Ye))
		return NULL;

	//printf("params, %d x %d, %d %s, %s  %s, %s\n", wsize,hsize, maxiter, Xs,Xe, Ys,Ye);
	
	PyObject *points = PyList_New(0);

	/* create an array of integers to store the result of the mandlebrot calculation */
	int bytearray[wsize * hsize * 3];
	
	/* call mandlebrot_bytearray */
	mandlebrot_mpfr_c(wsize, hsize, maxiter, Xs, Xe, Ys, Ye, bytearray);
	
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
	const char *Xs, *Xe, *Ys, *Ye; /* do i need to free these at the end of the func */
    unsigned int pX, pY, w, h;
	
	if (!PyArg_ParseTuple(args, "ssssiiii", &Xs, &Xe, &Ys, &Ye, &pX, &pY, &w, &h))
		return NULL;
	
	mpfr_zoom_in(&Xs, &Xe, &Ys, &Ye, pX, pY, w, h);

	printf("mandlebrot_zoom_in (out) \n%s\n%s\n%s\n%s\n", Xs, Xe, Ys, Ye);
	
	PyObject *coords = PyTuple_New(4);
	PyObject *pval;
	
	/* need to confirm whether I need to use PyDECREF() on pval */
	
	pval = Py_BuildValue("s#", Xs, (Py_ssize_t)strlen(Xs));
	PyTuple_SetItem(coords, 0, pval);

	pval = Py_BuildValue("s#", Xe, (Py_ssize_t)strlen(Xe));
	PyTuple_SetItem(coords, 1, pval);

	pval = Py_BuildValue("s#", Ys, (Py_ssize_t)strlen(Ys));
	PyTuple_SetItem(coords, 2, pval);

	pval = Py_BuildValue("s#", Ye, (Py_ssize_t)strlen(Ye));
	PyTuple_SetItem(coords, 3, pval);

	return coords;
}

/* ----------------------------------------------------------------------------
 */
static PyObject * mandlebrot_zoom_out(PyObject *self, PyObject *args)
{
	const char *Xs, *Xe, *Ys, *Ye; /* do i need to free these at the end of the func */

	if (!PyArg_ParseTuple(args, "ssss", &Xs, &Xe, &Ys, &Ye))
		return NULL;
	
	PyObject *coords = PyTuple_New(4);
	PyObject *pval;
	
	/* need to confirm whether I need to use PyDECREF() on pval */
	
	pval = Py_BuildValue("s#", Xs, (Py_ssize_t)strlen(Xs));
	PyTuple_SetItem(coords, 0, pval);

	pval = Py_BuildValue("s#", Xe, (Py_ssize_t)strlen(Xe));
	PyTuple_SetItem(coords, 1, pval);

	pval = Py_BuildValue("s#", Ys, (Py_ssize_t)strlen(Ys));
	PyTuple_SetItem(coords, 2, pval);

	pval = Py_BuildValue("s#", Ye, (Py_ssize_t)strlen(Ye));
	PyTuple_SetItem(coords, 3, pval);
	
	return coords;
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
	{NULL, NULL, 0 , NULL} 
};

static struct PyModuleDef mandlebrotmodule = {
	PyModuleDef_HEAD_INIT,
	"mandlebrot",
	"Python interface for the mandlebrot set C library", /* module documentation */
	-1, 
	MandlebrotMethods
};

PyMODINIT_FUNC
PyInit_mandlebrot(void)
{
	return PyModule_Create(&mandlebrotmodule);
}

