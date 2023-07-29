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
    unsigned char *bytearray; //[wsize * hsize * 3];
    bytearray = (unsigned char*)calloc((size_t)(wsize * hsize * 3), sizeof(unsigned char));

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
    unsigned char *bytearray; //[wsize * hsize * 3];
    bytearray = (unsigned char*)calloc((size_t)(wsize * hsize * 3), sizeof(unsigned char));

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
    m.def("mpfr", &mpfr, "calculate the mandelbrot set using the mpfr library");

#ifdef USES_THREADS
    m.def("mpfr_thread", &mpfr_thread, "calculate the mandelbrot set using the mpfr library and threads");
#else
    m.def("mpfr_thread", &mpfr, "calculate the mandelbrot set using the mpfr library and threads");
#endif

    m.def("zoom_in_via_mouse", &mpfr_zoom_in_via_mouse, "calculate the next mandelbrot set zoom values");
    
    m.def("zoom_in", &mpfr_zoom_in, "calculate the next mandelbrot set zoom values");

    m.def("zoom_out", &mpfr_zoom_out, "calculate the previous mandelbrot set zoom values");

    m.def("init", &initialize_c, "initialise the mpfr variables");

    m.def("setup", &setup_c, "setup the mpfr variables");

    m.def("tidyup", &free_mpfr_mem_c, "calculate the mandelbrot set using 64bit floats");
}

