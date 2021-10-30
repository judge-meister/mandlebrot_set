# mandlebrot_set

Displays the Mandlebrot set with basic zooming

## Running

First the mandlebrot python modules needs compiling;

    ./build.sh

Then to run the latest version;

    python3 ./test.py

Other versions;

 * `mandlebrot_pygame.py` - first attempts and working out, is runnable though. you will need to 
 read the code to see what keys can be used to do what.  I still need to write a help doc.

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
I will have to learn more about arbitrary precision floating point arithmetic.  (We can use the
Gnu Mpfr library.)


## Ideas for future development

1. display the current centre location in the terminal as you zoom in/out.

1. learn about arbitrary precision arithmatic to extend the possible zoom level.

1. learn about the various python math libraries to see if any of them help (numpy, numba, scipy ...)

1. may be re-write in openGL, either with Unity or from scratch. This will help with speed but maybe not precision initially.

1. make it multi-processor compatible (threading, processes etc...) to allow it to use more resources and calculate faster.

1. test other languages, like Go - turns out Go is comparible in speed to C, the same order of magnitude at least.

1. move all test programs to sub folder

1. create python interface c file that includes the mandlebrot_set and mandlebrot_mpfr c files

1. figure out a whether we can have a init function for the mpfr stuff to save from re-initialising everything for every zoom level (image) calculated. obviously we will need to also have a tidy-up/finish method to free all the items - yes we can.  could do with learning how to run a function on module import.

1. look at swapping from pygame to something else to allow adding GUI elements. 

1. look at Sprocket project for ideas for fully OpenGL version. 

1. is it possible display the OpenGL image inside a python window? - Yes, apparently pygame can do that see https://www.stechies.com/opengl-python/

## Lessons Learnt ##

the mpfr library can export and import a string version of the mpfr_t value, but unfortunately the 2 strings are not compatible, or at least I cannot get them to be so.  Therefore going to store image coords in static global vars in the mandlebrot.c which then allows for the zoom calcs to work without the need for passing the values back to the python caller.
