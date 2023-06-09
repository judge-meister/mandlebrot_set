# mandelbrot_set

Displays the Mandelbrot set with basic zoom in/out capability.

## The Story So Far

Initial attempts were in pure python and it worked, but it was a little slow. It uses
pygame as the display tool. Whether this is a good idea or not I have yet to find out.

The first improvement was to take the basic mandelbrot set calculation loop and write
it in C and create a new python module.  This improved the speed by quite a lot allowing
me to implement the first zooming feature to great effect.  It is usable now that a
640x640 image can be generated in between 0.3 secs and 0.6 secs on fairly old (circa 2012)
hardware.

The zoom level bottoms out after a while producing increasingly blocky results which I
put down to the limits of floating point precision in the C code (64 bit floats). To get further
I will have to learn more about arbitrary precision floating point arithmetic.

## Gnu MPFR library

With the help of the MPFR library (https://www.mpfr.org/) we can gain arbitrary precision in the calculations. So
I added functions to the python module to calculate the mandelbrot set using the MPFR
library.  After some teathing troubles I finally got the MPFR version to work the only
trouble being that the algorithm is much slower.  It can take as little as 1.5 seconds to
generate an image all the way up to over 90 seconds.  So far I think the factors that
influence the time are which bit of the set you are calculating (zooming in to) and how
many levels of zoom you are at.  I presume that the greater the precision calculated the
longer it will take.

## Building

#### Requirements

 * python3.9 (at least), prefereable 3.11
 * MPFR 4.2.0 - Multi Precision Floating Point Library (this requires GMP). Unfortunately you need 4.2.0 because I am using a feature only introduced in that version
 * Python PyGame - python modules for writing games, provides input event loops and rendering
 * Python PIL - Image library
 * Python setuptools - for building distributable modules
 * Python dev package - to provide Python.h

### Linux

Both the MPRF library and the python dependencies can be installed through most package managers in Linux.

Clone the repository 'cd src' and use 'make' to build the python module.

### MacOS

I used homebrew to install the MPRF library on MacOS 10.15, but I'm sure its available via MacPorts also.
Because I'm running an older version of MacOS which only comes with python2 I also installed Python3 along
with PyGame and the other python dependencies via homebrew.  You'll also require the gcc compiler from 
either homebrew or macports as I have no intention of setting up an Xcode project.

The same as for Linux, clone the repository, 'cd src' and use 'make' to build the python module.

## Running

To run the latest version. without any options it will run the threaded MPFR library version. Add -h to see options;

    python3 ./mandelbrot_set.py

To run the 64bit float version which is significantly faster but has limited zoom ranges before pixelation;

    python3 ./mandelbrot_set.py -a float


## Ideas for future development

### Oustanding ###

1. Learn about the various python math libraries to see if any of them help (numpy, numba, scipy ...)

1. May be re-write in openGL, either with Unity or from scratch. This will help with speed but maybe not
precision initially.

1. Make it multi-processor compatible (threading, processes etc...) to allow it to use more resources and
calculate faster.  The python multiprocessing module works quite nicely and I have a test program for it, but
still need to implement it in the main program.  Although, on MacOS it seems to get slower if you try to run the
multiprocessing module.

1. Could also do with learning how to run a function on module import. there is example code for this but it
doesn't seem to activate.

1. Look at swapping from pygame to something else to allow adding GUI elements.

1. Look at Sprocket project for ideas for fully OpenGL version.

1. Is it possible display the OpenGL image inside a python window? - Yes, apparently pygame can do that
see https://www.stechies.com/opengl-python/

### Achieved ###

1. Display the current centre location in the terminal as you zoom in/out.  I can show the
pixel location of the mouse, but I can only show the mandelbrot set real/imag coordinate
for floating point number calculations currently and not the MPFR. made a bodge for the MPFR
class.

1. Learn about arbitrary precision arithmetic to extend the possible zoom level. Done. Using MPFR.

1. Move all test programs to sub folder - Done, although I need a better folder name than 'test'.

1. Create python interface c file that includes the mandelbrot_set and mandelbrot_mpfr c files. Done.

1. Figure out a whether we can have a init function for the mpfr stuff to save from re-initialising everything
for every zoom level (image) calculated. Obviously we will need to also have a tidy-up/finish method to free all
the items - yes we can.  Done.

1. Learn about threads in C to take advantage of multi-processor architecture.  Done.  My initial attempt has reduced the time for the initial display from 12.5 seconds to 2.9 seconds using 16 threads.

1. Using a better color mapping.  Done. It now looks good at all zoom levels.

1. Can now provide the desired centre point, zoom level and zoom factor as input values (initially as command line options)


## Lessons Learnt ##

The mpfr library can export and import a string version of the mpfr_t value, but unfortunately the 2 strings are
not compatible with each other, or at least I cannot get them to be so.  This means you cannot import the exported
string and get the same value.  It seems to loose the decimal point. Therefore I'm going to store image coords
in static global vars in the mandelbrot.c file which then allows for the zoom calcs to work without the need
for passing the values back to the python caller.

## Desired Feature List ##

 * as MPFR calculations take a while. use either the 64bit float algorithm or values from the currently displayed image to setup an initial approximation of the next image while waiting for the true image to finish calculating.

 * have an option that either saves the current image as a file or just generates an image file instead of displaying it - could be the start of creating a video of a zoom in.

 * split the mandelbrot.c file up into a MPFR module and a 64bit float module.

 * display other fractals related to mandelbrot sets
   * budhabrot - https://en.wikipedia.org/wiki/Buddhabrot
   * julia sets

## No longer looking to do ##

 * add use of the multiprocessing module to speed up calculation. Gonna use threads in C module instead.
 * need to trial distributed multiprocessing across multiple machines. Don't think this is actually useful. It will probably spend more time sending and receiving data across the network.

## Porting to Windows

NOTE : Builds have to be Win32

Required libraries:
### MPIR
 
 Source : https://github.com/BrianGladman/mpir

Build Instructions:
* Run lib or dll_gc project first to generate the mpir files.
* Run lib or dll_mpir_cxx to generate the library or dll.

### MPFR - https://github.com/BrianGladman/mpfr

### Building Python Module

setup.py needs to include --plat-name win32 to change the architecture to Win32
