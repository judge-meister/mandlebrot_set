/*
 *  C library with python module interface to calculate the Mandlemrot set for
 *  the area provided by a set of coordinates
 */
 
/* ----------------------------------------------------------------------------
 * calculate_point
 *
 * Params 
 * x0 (double) - the scaled x position in the rectangle ( values between -2.0 and 1.0)
 * y0 (double) - the scaled y position in the rectangle ( values between -1.5 and 1.5)
 * Returns
 * iteration - count of how many iterations it took to head to infinity
 */
#include <stdio.h>
#include <math.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#define MAXITER 1000

struct Color { int r, g, b; };

static int calculate_point(double x0, double y0)
{
	double x = 0.0;
	double y = 0.0;
	int iteration = 0;
	/*int maxiter = 1000;*/
	double xtemp = 0.0;
	
	while (((x*x + y*y) <= 2*2) && iteration < MAXITER)
	{
		xtemp = x*x - y*y + x0;
		y = 2.0*x*y + y0;
		x = xtemp;
		iteration++;
	}
	return iteration;
}

/* ----------------------------------------------------------------------------
 * grayscale - return a gray scale rgb value representing the iteration count
 *             value obtained by calculate_point
 *
 * Params
 * iteration (int) - interation value calculated at a point
 * Return
 * color (struct of 3 int vals) - RGB color value
 */
static struct Color grayscale(int it)
{
	struct Color c; /*  = { 0, 0, 0 } */
	c.r = 0;
	c.g = 0;
	c.b = 0;

	/*if (it >= MAXITER)
	{
		struct color c = { 0, 0, 0 };
	}*/
	if (it < MAXITER)
	{
		int idx = (int)ceil( sqrt( sqrt( (double)it / (double)MAXITER ) ) * 255.0 );
		c.r = idx;
		c.g = idx;
		c.b = idx;
	}
	return c;
}

/* ----------------------------------------------------------------------------
 * scaled - return a value in the range Xe to Xs given a location in a larger
 *          integer range
 * Params
 * x1 (int) - position in  large integer range
 * sz (int) - size of integer range
 * Xs (double) - start of smaller range
 * Xe (double) - end of smaller range
 * Returns
 * (double) - the calculated scaled value
 */
static double scaled(int x1, int sz, double Xs, double Xe)
{
	return ( ((double)x1 / (double)sz) * (Xe-Xs) ) + Xs;
}
/* ----------------------------------------------------------------------------
 *
 * Params - (contained in PyObject *args)
 * ssize (int) - size of display screen/window (assumed square)
 * Xs (double)
 * Xe (double)
 * Ys (double)
 * Ye (double)
 */
static PyObject * mandlebrot(PyObject *self, PyObject *args)
{
	int ssize = 0;
	double Xs, Xe, Ys, Ye;
	
	if (!PyArg_ParseTuple(args, "idddd", &ssize, &Xs, &Xe, &Ys, &Ye))
		return NULL;

	PyObject *points = PyList_New(0);
	
	for(int Dy = 0; Dy < ssize; Dy++)
	{
		for(int Dx = 0; Dx < ssize; Dx++)
		{
			double x0 = scaled(Dx, ssize, Xs, Xe);
			double y0 = scaled(Dy, ssize, Ys, Ye);
			struct Color rgb;
			int iter;

			iter = calculate_point(x0, y0);
			rgb = grayscale(iter);
			
			/* added color to PyList, but still need x,y coords*/
			PyObject *pycolor = PyTuple_New(6);
			PyTuple_SetItem(pycolor, 0, PyLong_FromLong((long) iter));
			PyTuple_SetItem(pycolor, 1, PyLong_FromLong((long) rgb.r));
			PyTuple_SetItem(pycolor, 2, PyLong_FromLong((long) rgb.g));
			PyTuple_SetItem(pycolor, 3, PyLong_FromLong((long) rgb.b));
			PyTuple_SetItem(pycolor, 4, PyLong_FromLong((long) Dx));
			PyTuple_SetItem(pycolor, 5, PyLong_FromLong((long) Dy));
			/*PyTuple_SetItem(pycolor, 0, PyLong_FromLong((long) iter));
			PyTuple_SetItem(pycolor, 1, PyFloat_FromDouble((double) x0));
			PyTuple_SetItem(pycolor, 2, PyFloat_FromDouble((double) y0));*/
			PyList_Append(points, pycolor);
			Py_DECREF(pycolor);
		}
	}
	return points;
}

/* ----------------------------------------------------------------------------
 * mandlebrot_bytearray - return the results as a list of color values to be 
 *                        converted to a bytesarray by the caller
 *
 * Params - (contained in PyObject *args)
 * ssize (int) - size of display screen/window (assumed square)
 * Xs (double)
 * Xe (double)
 * Ys (double)
 * Ye (double)
 */
static PyObject * mandlebrot_bytearray(PyObject *self, PyObject *args)
{
	int ssize = 0;
	double Xs, Xe, Ys, Ye;
	
	if (!PyArg_ParseTuple(args, "idddd", &ssize, &Xs, &Xe, &Ys, &Ye))
		return NULL;

	PyObject *points = PyList_New(0);
	
	for(int Dy = 0; Dy < ssize; Dy++)
	{
		for(int Dx = 0; Dx < ssize; Dx++)
		{
			double x0 = scaled(Dx, ssize, Xs, Xe);
			double y0 = scaled(Dy, ssize, Ys, Ye);
			struct Color rgb;
			int iter;

			iter = calculate_point(x0, y0);
			rgb = grayscale(iter);
			
            /* just add the rgb values to the list */
			PyList_Append(points, PyLong_FromLong((long) rgb.r));
			PyList_Append(points, PyLong_FromLong((long) rgb.g));
			PyList_Append(points, PyLong_FromLong((long) rgb.b));
		}
	}
	return points;
}

/* ----------------------------------------------------------------------------
 * Python Module Stuff  - from docs.python.org/3/extending/extending.html
 */
static PyMethodDef MandlebrotMethods[] = { 
	{"mandlebrot",           mandlebrot,           METH_VARARGS, "calculate mandlebrot set" },
	{"mandlebrot_bytearray", mandlebrot_bytearray, METH_VARARGS, "calculate mandlebrot set into a bytearray" },
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

int
main(int argc, char *argv[])
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
    PyObject *pmodule = PyImport_ImportModule("mandlebrot");
    if (!pmodule) {
        PyErr_Print();
        fprintf(stderr, "Error: could not import module 'mandlebrot'\n");
    }

    /* ... */

    PyMem_RawFree(program);
    return 0;
}
