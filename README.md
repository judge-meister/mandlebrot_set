# mandlebrot_set

Displays the Mandlebrot set with basic zoom in/out capability.

## The Story So Far

Initial attempts were in pure python and it worked, but it was a little slow. It uses
pygame as the display tool. Whether this is a good idea or not I have yet to find out.

The first improvement was to take the basic mandlebrot set calculation loop and write
it in C and create a new python module.  This improved the speed by quite a lot allowing
me to implement the first zooming feature to great effect.  It is usable now that a
640x640 image can be generated in between 0.3 secs and 0.6 secs on fairly old (circa 2012)
hardware.

The zoom level bottoms out after a while producing increasingly blocky results which I
put down to the limits of floating point precision in the C code (64 bit floats). To get further
I will have to learn more about arbitrary precision floating point arithmetic.  

## Gnu MPFR library

With the help of the MPFR library we can gain arbitrary precision in the calculations. So
I added functions to the python module to calculate the mandlebrot set using the MPFR
library.  After some teathing troubles I finally got the MPFR version to work the only
trouble being that the algorithm is much slower.  It can take as little as 1.5 seconds to
generate an image all the way up to over 90 seconds.  So far I think the factors that
influence the time are which bit of the set you are calculating (zooming in to) and how
many levels of zoom you are at.  I presume that the greater the precision calculated the
longer it will take.

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
pixel location of the mouse, but I can only show the mandlebrot set real/imag coordinate
for floating point number calculations currently and not the MPFR. made a bodge for the MPFR
class.

1. Learn about arbitrary precision arithmatic to extend the possible zoom level. Done. Using MPFR.

1. Move all test programs to sub folder - Done, although I need a better folder name than 'test'.

1. Create python interface c file that includes the mandlebrot_set and mandlebrot_mpfr c files. Done.

1. Figure out a whether we can have a init function for the mpfr stuff to save from re-initialising everything
for every zoom level (image) calculated. Obviously we will need to also have a tidy-up/finish method to free all
the items - yes we can.  Done.


## Lessons Learnt ##

The mpfr library can export and import a string version of the mpfr_t value, but unfortunately the 2 strings are
not compatible with each other, or at least I cannot get them to be so.  This means you cannot import the exported
string and get the same value.  It seems to loose the decimal point. Therefore I'm going to store image coords
in static global vars in the mandlebrot.c file which then allows for the zoom calcs to work without the need
for passing the values back to the python caller.

## Running

Firstly, the mandlebrot python module needs compiling;

    make

Then to run the latest version. without any options it will runthe MPFR library version. Add -h to see options;

    python3 ./mandlebrot_set.py

To run the 64bit float version which is significantly faster but has limited zoom ranges before pixelation;

    python3 ./mandlebrot_set.py -a float

## Desired Feature List ##

 * get the window displayed while initial image calculation is happening. this will also stop the window hanging while the calculation happens. 
   * threading maybe
 * as MPFR calculations take a while use either the 64bit float algorithm or values from the currently displayed image to setup an initial approximation of the next image while waiting for the true image to finish calculating.
 * have an option that either saves the current image as a file or just generates an image file instead of displaying it 
   * could be the start of creating a video of a zoom in.
 * provide the desired centre point, zoom level and zoom factor as input values (initially as command line options)
   * partially done, still need to apply it to the MPFR library code.
 * split the mandlebrot.c file up into a MPFR module and a 64bit float module.
 * add use of the multiprocessing module to speed up calculation.
   * need to trial distributed multiprocessing across multiple machines.
 * display other fractals related to mandlebrot sets
   * budhabrot - https://en.wikipedia.org/wiki/Buddhabrot
   * julia sets
 * use a color gradient image file instead of calculating the color values. might be faster once the file is loaded.
 
