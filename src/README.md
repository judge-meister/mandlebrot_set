# Source Code

The Makefile now uses PyBind11 to generate its python module which hopefully will make building 
under Windows 11  a bit more feasible.  Check out the Makefile and also the `mandelbrot_pybind11.cpp` 
file for details.

I have included the changes we made in the windows_port branch to hide the threaded code behind 
a `#ifdef USES_THREADS` statement to make this easier to use, but also included a rule for building 
the threaded version when I need to.

## Building for Linux

Dependencies

 * pybind11
 * libGL
 * libglfw 
 * libSOIL 
 * libX11 
 * libpthread 
 * libXrandr 
 * libXi  

## Building in MacOS (Big Sur)

I use Homebrew to supply build dependencies for this project.  I recommend you do the same.

Dependencies from Homebrew

 * glew
 * glfw
 * mpfr
 * pkg-config
 * gdb

Unfortunately the Simple OpenGL Image Library used in this project is not available via Homebrew, 
so for now we have to build it from its source.

 * Download the code from https://github.com/SpartanJ/SOIL2
 * Following the build instructions for that repo recommends using premake5 which I also had to
 build from source as MacOS BigSur refused to let me run downloaded binary.  
 
## Building for Windows

TBD
