# Using PyBind11

The Makefile now uses PyBind11 to generate its python module which hopefully will make building under Windows 11  a bit more feasible.  Check out the Makefile and also the mandelbrot_pybind11.cpp file for details.

I have included the changes we made in the windows_port branch to hide the threaded code behind a `#ifdef USES_THREADS` statement to make this easier to use, but also included a rule for building the threaded version when I need to.

