# mandlebrot_set

Displays the Mandlebrot set with basic zooming

## Running

First the mandlebrot python modules needs compiling;

    ./build.sh

Then to run the latest version;

    python3 ./test.py

Other versions;

 * `mandlebrot_pygame.py` - first attempts and working out, is runnable though. you will need to read the code to see what keys can be used to do what.  I still need to write a help doc.

## The Story So Far

Initial attempts were in pure python and it worked, but it was a little slow. It uses 
pygame as the display tool. Whether this is a good idea or not I have yet to find out.

The first improvement was to take the basic mandlebrot set calculation loop and write 
it in C and create a new python module.  This improved the speed by quite a lot allowing
me to implement the first zooming feature to great effect.  It is usable now that a
640x640 image can be generated in between 0.3 secs and 0.6 secs on fairly old (circa 2021)
hardware.

The zoom level bottoms out after a while producing increasingly blocky results which I
put down to the limits of floating point precision in either the C code or Python code
or both.  I have yet to discover which, but then I haven't really looked yet.

## Ideas for future development

1. display the current centre location in the terminal as you zoom in/out.
1. learn about arbitrary precision arithmatic to extend the possible zoom level.
1. learn about the various python math libraries to see if any of them help (numpy, numba, scipy ...)
1. may be re-write in openGL, either with Unity or from scratch. This will help with speed but maybe not precision initially.
1. make it multi-processor compatible (threading, processes etc...) to allow it to use more resources and calculate faster.
