// ----------------------------------------------------------------------------
// C++ Library with python module interface to calculate the Mandelbrot Set
// for the area provided by a set of coordinates
//

//
// building python module with PyBind11
//
/* g++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) \
 *     $(pkg-config --cflags --libs mpfr) mandelbrot.cpp \
 *     -o mandelbrot$(python3-config --extension-suffix)
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

#include "mandelbrot.h"


// ----------------------------------------------------------------------------
// Python function interface - mandelbrot_bytearray
// mandelbrot_bytearray - return the results as a list of color values to be
//                        converted to a bytesarray by the caller
//
// Returns a PyList containing color values for all the calculated points
//
py::list float64(const unsigned int wsize,   // width of screen/display/window 
                 const unsigned int hsize,   // height of screen/display/window 
                 const unsigned int maxiter, // max iterations before escape 
                 const double Xs, // string repr of mpfr_t for X start  
                 const double Xe, // string repr of mpfr_t for X end  
                 const double Ys, // string repr of mpfr_t for Y start 
                 const double Ye  // string repr of mpfr_t for Y end 
                )
{
    py::list points;

    // create an array of integers to store the result of the mandelbrot calculation 
    char *bytearray; //[wsize * hsize * 3];
    bytearray = (char*)calloc((size_t)(wsize * hsize * 3), sizeof(char));

    // call mandelbrot_bytearray 
    mandelbrot_bytearray_c(wsize, hsize, maxiter, Xs, Xe, Ys, Ye, &bytearray);

    // transfer returned values into PyList for return 
    for(unsigned int i = 0; i < (wsize * hsize * 3); i++)
    {
        points.append(bytearray[i]);
    }
    free(bytearray);
    
    return points;
}

// ----------------------------------------------------------------------------
// Python function interface to mandelbrot_bytearray
//
// mandelbrot_bytearray - return the results as a list of color values to be
//                        converted to a bytearray by the caller
//
// Returns a PyList containing color values for all the calculated points
//
py::list mpfr( const unsigned int wsize,   // width of screen/display/window 
               const unsigned int hsize,   // height of screen/display/window 
               const unsigned int maxiter  // max iterations before escape 
             )
{
    py::list points;

    // create an array of integers to store the result of the mandelbrot calculation 
    char *bytearray; //[wsize * hsize * 3];
    bytearray = (char*)calloc((size_t)(wsize * hsize * 3), sizeof(char));

    // call mandelbrot_bytearray 
    mandelbrot_mpfr_c(wsize, hsize, maxiter, &bytearray);

    // transfer returned values into PyList for return 
    for(unsigned int i = 0; i < (wsize * hsize * 3); i++)
    {
        points.append(bytearray[i]);
    }
    free(bytearray);

    return points;
}

// ----------------------------------------------------------------------------
// python function interface to mandelbrot_mpfr_slice_c
//
// mandelbrot_mpfr_slice_c - return the results as a list of color values to be
//                           converted to a bytearray by the caller
// 
// Returns a PyList containing color values for all the calculated points
//
py::list mpfr_slice( const unsigned int wsize,   // width of screen/display/window 
                     const unsigned int hsize,   // height of screen/display/window 
                     const unsigned int nslice,  // number of slices 
                     const unsigned int slice,   // index of which slice (range 0 -> nslice-1) 
                     const unsigned int maxiter  // max iterations before escape 
                   )
{
    py::list points;

    // create an array of integers to store the result of the mandelbrot calculation 
    char *bytearray; //[wsize * hsize/nslice * 3];
    bytearray = (char*)calloc((size_t)(wsize * hsize * 3), sizeof(char));
    printf("bytearray length = %d\n",wsize * hsize/nslice * 3);

    // call mandelbrot_bytearray 
    mandelbrot_mpfr_slice_c(wsize, hsize, nslice, slice, maxiter, &bytearray);

    // transfer returned values into PyList for return 
    for(unsigned int i = 0; i < (wsize * hsize/nslice * 3); i++)
    {
        points.append(bytearray[i]);
    }
    free(bytearray);

    return points;
}

#ifdef USES_THREADS
// ----------------------------------------------------------------------------
// python function interface to mandelbrot_mpfr_thread_c
//
// Returns a PyList containing color values for all the calculated points
// 
py::list mpfr_thread( const unsigned int wsize,   // width of screen/display/window 
                      const unsigned int hsize,   // height of screen/display/window 
                      const unsigned int maxiter  // max iterations before escape 
                    )
{
    py::list points;

    // create an array of integers to store the result of the mandelbrot calculation 
    char *bytearray; //[wsize * hsize * 3];
    bytearray = (char*)calloc((size_t)(wsize * hsize * 3), sizeof(char));

    // call mandelbrot_bytearray 
    mandelbrot_mpfr_thread_c(wsize, hsize, maxiter, &bytearray);

    // transfer returned values into PyList for return 
    for(unsigned int i = 0; i < (wsize * hsize * 3); i++)
    {
        points.append(bytearray[i]);
    }
    free(bytearray);

    return points;
}
#endif



// PyBind11 Bindings 

PYBIND11_MODULE(mandelbrot, m)
{
    m.def("float64", &float64, "calculate the mandelbrot set using 64bit floats");

    m.def("mpfr", &mpfr, "calculate the mandelbrot set using the mpfr library");

#ifdef USES_THREADS
    m.def("mpfr_thread", &mpfr_thread, "calculate the mandelbrot set using the mpfr library and threads");
#else
    m.def("mpfr_thread", &mpfr, "calculate the mandelbrot set using the mpfr library and threads");
#endif

    m.def("mpfr_slice", &mpfr_slice, "calculate the mandelbrot set using the mpfr library in slices");

    m.def("zoom_in", &mpfr_zoom_in, "calculate the next mandelbrot set zoom values");

    m.def("zoom_out", &mpfr_zoom_out, "calculate the previous mandelbrot set zoom values");

    m.def("init", &initialize_c, "initialise the mpfr variables");

    m.def("setup", &setup_c, "setup the mpfr variables");

    m.def("tidyup", &free_mpfr_mem_c, "calculate the mandelbrot set using 64bit floats");
}

