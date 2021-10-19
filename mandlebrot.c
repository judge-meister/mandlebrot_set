/*
 *  C library with python module interface to calculate the Mandlebrot set for
 *  the area provided by a set of coordinates
 */
 
/*
from wikipedia (https://en.wikipedia.org/wiki/Mandelbrot_set) 
pseudo code to generate the mandlebrot set

for each pixel (Px, Py) on the screen do
    x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
    y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
    x := 0.0
    y := 0.0
    iteration := 0
    max_iteration := 1000
    while (x*x + y*y ≤ 2*2 AND iteration < max_iteration) do
        xtemp := x*x - y*y + x0
        y := 2*x*y + y0
        x := xtemp
        iteration := iteration + 1
    
    color := palette[iteration]
    plot(Px, Py, color)
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

/* ----------------------------------------------------------------------------
 * run the z = z^2 + c algorithm for the point provided to assertain if the 
 * point is inside or outside of the mandlebrot set
 *
 * Params
 * x0, y0 (double) location to calculate for
 * Returns
 * number of iterations (int) to achieve the answer
 */
static int calculate_point(double x0, double y0)
{
	double x = 0.0;
	double y = 0.0;
	int iteration = 0;
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

	if (it < MAXITER)
	{
		/*int idx = (int)ceil( sqrt( sqrt( (double)it / (double)MAXITER ) ) * 255.0 );*/
		int idx = (int)ceil( sqrt( (double)it / (double)MAXITER ) * 255.0 );
		c.r = idx;
		c.g = idx;
		c.b = idx;
	}
	return c;
}

/* ----------------------------------------------------------------------------
 * gradient1 - first attempt at a procedural color gradient
 *
 * Params
 * it (int) - the iteration value calculated at a point
 * Returns
 * color (struct if 3 ints) - RGB color value
 */
static struct Color gradient1(int it)
{
	struct Color c;
	if (it < MAXITER)
	{
		double m = sqrt( (double)it / (double)MAXITER );
		/*printf("sqrt(it/max) = %5.2f | ", m);*/
		c.r = (int)floor((( sin(0.65 * m * 85.0) *0.5)+0.5) *255);
		c.g = (int)floor((( sin(0.45 * m * 85.0) *0.5)+0.5) *255);
		c.b = (int)floor((( sin(0.25 * m * 85.0) *0.5)+0.5) *255);
		if (c.r>255 || c.g>255 || c.b>255 || c.r<0 || c.g<0 || c.b<0)
			printf("col(%5.6f,%5.6f,%5.6f) | ",sin(0.30 * m * 20.0),sin(0.45 * m * 20.0),sin(0.65 * m * 20.0));
	}
	else 
	{
		c.r = 0; c.g = 0; c.b = 0;
	}
	return c;
}
/* ----------------------------------------------------------------------------
 * scaled - return a value in the range Xe to Xs given a location in a larger
 *          integer range
 * Params
 * x1 (int) - position in large integer range
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
 * Python function interface - mandlebrot
 *
 * Params - (contained in PyObject *args)
 * ssize (int) - size of display screen/window (assumed square)
 * Xs, Xe, Ys, Ye (double) bounds of mandlebrot set to calculate
 *
 * Returns a PyList of PyTuple containing iteration, color and screen coord values 
 *         for all the calculated points
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
 * Python function interface - mandlebrot_bytearray
 * mandlebrot_bytearray - return the results as a list of color values to be 
 *                        converted to a bytesarray by the caller
 *
 * Params - (contained in PyObject *args)
 * wsize (int) - width of display screen/window
 * hsize (int) - height of display screen/window
 * Xs, Xe, Ys, Ye (double) bounds of mandlebrot set to calculate
 *
 * Returns a PyList containing color values for all the calculated points
 */
static PyObject * mandlebrot_bytearray(PyObject *self, PyObject *args)
{
	int wsize = 0;
	int hsize = 0;
	double Xs, Xe, Ys, Ye;
	
	if (!PyArg_ParseTuple(args, "iidddd", &wsize, &hsize, &Xs, &Xe, &Ys, &Ye))
		return NULL;

	printf("params, %d,%d %1.20lf,%1.20lf %1.20lf,%1.20lf\n", wsize,hsize, Xs,Xe, Ys,Ye);
	PyObject *points = PyList_New(0);
	
	for(int Dy = 0; Dy < hsize; Dy++)
	{
		for(int Dx = 0; Dx < wsize; Dx++)
		{
			double x0 = scaled(Dx, wsize, Xs, Xe);
			double y0 = scaled(Dy, hsize, Ys, Ye);
			struct Color rgb;
			int iter;

			iter = calculate_point(x0, y0);
			/*if (iter == MAXITER)
				iter = 0;*/
			/*rgb = grayscale(iter);*/
			rgb = gradient1(iter);
			/*sh = shade(iter);*/
			
			/* just add the rgb values to the list */
			PyList_Append(points, PyLong_FromLong((long) rgb.r));
			PyList_Append(points, PyLong_FromLong((long) rgb.g));
			PyList_Append(points, PyLong_FromLong((long) rgb.b));

			/*PyList_Append(points, PyLong_FromLong((long) iter));
			PyList_Append(points, PyLong_FromLong((long) iter));
			PyList_Append(points, PyLong_FromLong((long) iter));*/
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

