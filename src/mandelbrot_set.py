"""
The Mandelbrot Set

using a custom C module and pygame to display and zoom in on the mandelbrot set
"""

import pygame
import math
import time
import os
import sys
import getopt
import functools
import threading

try:
    import mandelbrot
except ModuleNotFoundError:
    print("\nHave you built the mandelbrot cpp module ? try running 'make'\n")
    sys.exit()
from PIL import Image


X1=-2.0
X2=1.0
Y1=-1.5
Y2=1.5


def timer(func):
    """decorator function to display the time taken to run a func"""
    @functools.wraps(func)
    def wrapper_func(*args, **kwargs):
        start_time = time.perf_counter()
        value = func(*args, **kwargs)
        end_time = time.perf_counter()
        run_time = end_time - start_time
        print(f"Finished {func.__name__!r} in {run_time:.4f} secs")
        return value
    return wrapper_func


class PyGameWindow:
    """PyGame Window Class"""
    def __init__(self, size):
        """"""
        pygame.init()
        scr_inf = pygame.display.Info()
        #os.environ['SDL_VIDEO_WINDOW_POS'] = '{}, {}'.format(scr_inf.current_w, # // 2 - WINSIZE[0] // 2,
        #                                                     scr_inf.current_h)# // 2 - WINSIZE[1] // 2)
        self.pygameSurface = pygame.display.set_mode((size, size))
        self.pygameSurface.fill((255,255,255))
        self.pygameClock = pygame.time.Clock()
        pygame.key.set_repeat(100,20)
        self.windowsize = size

    def surface(self):
        """return the pygame surface"""
        return self.pygameSurface

    def clock(self):
        """return the pygame clock"""
        return self.pygameClock

    def winsize(self):
        """return the window size"""
        return self.windowsize


class mpfr_c:
    """Interface class for accessing the C code module methods that calculate the mandelbrot set"""
    def __init__(self, Cx=None, Cy=None):
        """initialise class data, and MPFR C module"""
        self.Xs = repr(X1)
        self.Xe = repr(X2)
        self.Ys = repr(Y1)
        self.Ye = repr(Y2)
        mandelbrot.setup()
        print("call setup(%s, %s, %s, %s)" % (self.Xs, self.Xe, self.Ys, self.Ye))
        if Cx is not None and Cy is not None:
            mandelbrot.init(self.Xs, self.Xe, self.Ys, self.Ye, Cx, Cy)
        else:
            mandelbrot.init(self.Xs, self.Xe, self.Ys, self.Ye, "99.9", "99.9")

    def zoom_in_via_mouse(self, px, py, sz, sz1, factor):
        """call the zoom in method of the C module"""
        mandelbrot.zoom_in_via_mouse(px, py, sz, sz1, factor)

    def zoom_in(self, px, py, sz, sz1, factor):
        """call the zoom in method of the C module"""
        mandelbrot.zoom_in(sz, sz1, factor)

    def zoom_out(self, factor):
        """call the zoom out method of the C module"""
        mandelbrot.zoom_out(factor)

    @timer
    def slice_set(self, sz, slices, slice, maxiter):
        """call the mpfr slice method of the C module"""
        #mandelbrot.initialize("-2.0", "1.0", "-1.5", "1.5")
        print("slice the set ",sz, slices, slice, maxiter)
        frame = bytearray(mandelbrot.mpfr_slice(sz, sz, slices, slice, maxiter))
        return (slice, frame)

    @timer
    def mpfr(self, sz, maxiter):
        """call the mpfr method of the C module"""
        frame = bytearray(mandelbrot.mpfr(sz, sz, maxiter))
        return frame

    @timer
    def mpfr_thread(self, sz, maxiter):
        """call the threaded mpfr method of the C module"""
        frame = bytearray(mandelbrot.mpfr_thread(sz, sz, maxiter))
        return frame


class mandelbrot_c_mpfr:
    """use the C module to implement a mandelbrot class"""
    def __init__(self, pgwin, Cx=None, Cy=None):
        """initialise some data"""
        self.Xs = repr(X1)
        self.Xe = repr(X2)
        self.Ys = repr(Y1)
        self.Ye = repr(Y2)
        self.factor = 80 #5 #10
        self.zoom = 1
        self.maxiter = 1000
        self.pgwin = pgwin
        self.centre = {'r': -0.5, 'i': 0.0}
        self.sz = self.pgwin.winsize()

        self.mpfr = mpfr_c(Cx, Cy) if Cx is not None else mpfr_c()


    def _scaledX(self, x):
        """for display only"""
        return ( (float(x)/float( self.sz )) * (float(self.Xe) - float(self.Xs)) ) + float(self.Xs)


    def _scaledY(self, y):
        """for display only"""
        return ( (float(y)/float( self.sz )) * (float(self.Ye) - float(self.Ys)) ) + float(self.Ys)


    def scaled_pos(self, pos):
        """for display only"""
        loc = (self._scaledX(pos[0]), self._scaledY(pos[1]))
        return {'r': loc[0], 'i': loc[1]}


    def reset(self):
        """reset the mandelbrot set data bounds"""
        self.Xs = repr(X1)
        self.Xe = repr(X2)
        self.Ys = repr(Y1)
        self.Ye = repr(Y2)


    def zoom_in(self, pos):
        """zoom in the data set"""
        self.zoom +=1
        #mandelbrot.mandelbrot_zoom_in(pos[0], pos[1], self.sz, self.sz, self.factor)
        self.mpfr.zoom_in_via_mouse(pos[0], pos[1], self.sz, self.sz, self.factor)


    def zoom_out(self, pos):
        """zoom out the data set"""
        self.zoom -=1
        #mandelbrot.mandelbrot_zoom_out(int(pos[0]), int(pos[1]), self.sz, self.sz, self.factor)
        self.mpfr.zoom_out(self.factor)


    def draw_plot(self):
        """call the mpfr generator and display the result"""
        #slices = 1
        #slice, frame = self.mpfr.slice_set(self.sz, slices, 0, self.maxiter)
        frame = self.mpfr.mpfr(self.sz, self.maxiter)
        surf = pygame.image.frombuffer(frame, (self.sz, self.sz), 'RGB')
        self.pgwin.surface().blit(surf, (0,0))
        pygame.display.update()


    def mp_draw_plot(self):
        """multiprocessing

        THIS DOES NOT WORK AS THE MANDELBROT MODULE IS RESET FOR EACH PROCESS WHICH CLEARS THE
        CORNER VALUES AND PRODUCES A SOLID BLACK IMAGE.

        NEED TO LOOK INTO THREADING IN THE C CODE !!

        """


class mandelbrot_c_mpfr_thread(mandelbrot_c_mpfr):
    """class based in the mandelbrot_c_mpfr class that uses the threaded module"""
    def __init__(self, pgwin, Cx=None, Cy=None):
        """"""
        super().__init__(pgwin, Cx, Cy)

    def draw_plot(self):
        """call the mpfr_thread method and display the result"""
        #slices = 1
        #slice, frame = self.mpfr.slice_set(self.sz, slices, 0, self.maxiter)
        frame = self.mpfr.mpfr_thread(self.sz, self.maxiter)
        surf = pygame.image.frombuffer(frame, (self.sz, self.sz), 'RGB')
        self.pgwin.surface().blit(surf, (0,0))
        pygame.display.update()



def display_help():
    """display some help at the beginning"""
    help = """
    How to control the Mandelbrot Set viewer.

    Initially move the mouse to somewhere on the edge of the black region and left click, you will zoom in.  Now Repeat.
    To zoom out again use the right mouse button.

    Alternatively once the mouse pointer is within the window you can use the keyboard.
      cursor keys 'up', 'down', 'left', 'right' will move the mouse pointer.
      'z' to zoom in and 'x' to zoom out again.

    Use 'c' to reset back to the start.

    To exit use 'q' or 'esc'
    """
    print(help)


def event_loop(mand, pgwin):
    """takes a class instance that supports draw_plot, zoom_in, zoom_out methods"""

    fft = True
    ev_count = 0
    zoom_level = 0
    pygame.mouse.set_cursor(pygame.cursors.broken_x)
    window_size = pgwin.winsize()

    run = False
    while not run:
        pgwin.clock().tick(20)

        if fft and ev_count == 25:
            fft = False
            mand.draw_plot()
        ev_count +=1

        for event in pygame.event.get():

            mouse_pos = pygame.mouse.get_pos()
            centre_pos = mand.scaled_pos(mouse_pos)
            pygame.display.set_caption("Mandelbrot %s zoom=%s centre=%s" % (repr(mouse_pos), mand.zoom, repr(centre_pos)))

            if event.type == pygame.QUIT or (event.type == pygame.KEYUP and (event.key == pygame.K_q or event.key == pygame.K_ESCAPE)):
                run = True

            if event.type == pygame.KEYUP and event.key == pygame.K_c:
                print("zoom level ", mand.zoom)
                mand.reset()
                mand.draw_plot()

            if event.type == pygame.KEYUP and event.key == pygame.K_z:
                mand.zoom_in(mouse_pos)
                print("zoom level ", mand.zoom)
                mand.draw_plot()

            if event.type == pygame.KEYUP and event.key == pygame.K_x:
                mand.zoom_out((window_size/2, window_size/2))
                print("zoom level ", mand.zoom)
                mand.draw_plot()

            if event.type == pygame.KEYUP and event.key == pygame.K_UP:
                if mouse_pos[1] > 1:
                    pygame.mouse.set_pos(mouse_pos[0], mouse_pos[1]-1)

            if event.type == pygame.KEYUP and event.key == pygame.K_DOWN:
                if mouse_pos[1] < window_size-1:
                    pygame.mouse.set_pos(mouse_pos[0], mouse_pos[1]+1)

            if event.type == pygame.KEYUP and event.key == pygame.K_LEFT:
                if mouse_pos[0] > 1:
                    pygame.mouse.set_pos(mouse_pos[0]-1, mouse_pos[1])

            if event.type == pygame.KEYUP and event.key == pygame.K_RIGHT:
                if mouse_pos[0] < window_size-1:
                    pygame.mouse.set_pos(mouse_pos[0]+1, mouse_pos[1])

            if event.type == pygame.MOUSEBUTTONDOWN:
                mouse_pos = pygame.mouse.get_pos()
                pressed = pygame.mouse.get_pressed()

                if pressed[0]: # Button 1
                    mand.zoom_in(mouse_pos)
                    print("zoom level ", mand.zoom)
                    mand.draw_plot()

                if pressed[2]: # Button 3
                    mand.zoom_out((window_size/2, window_size/2))
                    print("zoom level ", mand.zoom)
                    mand.draw_plot()


def main_mpfr(pgwin, options):
    """run using the mandelbrot_c_mpfr class"""
    mand = mandelbrot_c_mpfr(pgwin)
    #mand.draw_plot()
    event_loop(mand, pgwin)
    mandelbrot.tidyup()


def main_mpfr_thread(pgwin, options):
    """run using the mandelbrot_c_mpfr_thread class"""
    Cx = options['real']
    Cy = options['imag']
    mand = mandelbrot_c_mpfr_thread(pgwin, Cx, Cy)
    event_loop(mand, pgwin)
    mandelbrot.tidyup()


def usage():
    """display some usage info"""
    print("mandelbrot_set.py")
    print("   -h  this help")
    print("   -z  zoom level to start with")
    print("   -r  real value of point to zoom in to")
    print("   -i  imaginary value of point to zoom in to")
    print("   -d  display size (assumes a square)")
    print("   -a  which algorithm (mpfr, thread)")
    print("   -f  zoom factor")


def getOptions(options):
    """parse the command line options"""
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hz:r:i:a:d:f:", ["help", "zoom=", "real=", "imag=", "algo=", "disp=", "factor="])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(2)

    options['algo'] = 'thread'
    options['disp'] = 640
    options['real'] = None
    options['imag'] = None

    for o, a in opts:
        if o == "-h" or o == "-help":
            usage()
            sys.exit()

        elif o == "-z" or o == "-zoom":
            try:
                options['zoom'] = int(a)
                print("WARNING: option -zoom is still to be implemented")
            except ValueError:
                print("\nERROR: -zoom must be a positive integer\n")
                usage()
                sys.exit(2)

        elif o == "-r" or o == "-real":
            try:
                r = float(a)
                options['real'] = a # value is stored as a string
            except ValueError:
                print("\nERROR: -real must be a floating point number\n")
            #print("WARNING: option -real is still to be implemented")

        elif o == "-i" or o == "-imag":
            try:
                r = float(a)
                options['imag'] = a # value is stored as a string
            except ValueError:
                print("\nERROR: -imag must be a floating point number\n")
            #print("WARNING: option -imag is still to be implemented")

        elif o == "-a" or o == "-algo":
            if a in [ 'mpfr', 'thread' ]:
                options['algo'] = a
            else:
                print("\nERROR: invalid value for the -algo option\n")
                usage()
                sys.exit(2)

        elif o == "-d" or o == "-disp":
            try:
                options['disp'] = int(a)
            except ValueError:
                print("\nERROR: -disp must be a positive integer\n")
                usage()
                sys.exit(2)

        elif o == "-f" or o == "-factor":
            try:
                options['factor'] = float(a)
            except ValueError:
                print("\nERROR: -factor must be a number\n")
                usage()
                sys.exit(2)


def main(options):
    """main method to initiate the program"""

    display_help()

    pygwin = PyGameWindow(options['disp'])

    if options['algo'] == 'mpfr':
        main_mpfr(pygwin, options)

    elif options['algo'] == 'thread':
        main_mpfr_thread(pygwin, options)

    pygame.quit()


if __name__ == '__main__':
    options = {}
    getOptions(options)
    main(options)


"""
Values to aim for
"""

real = """-1.7400623825
7933990522
0844167065
8256382966
4172043617
1866879862
4184611829
1964415305
6054840718
3394832257
4345000825
9172138785
4929836778
9336650341
7299549623
7388383033
4646546129
0768441055
4861368707
1985055926
9507357211
7902436669
4013479375
3068611574
7459438207
1288525822
2629105433
6486959460
03865"""

imag = """0.0281753397
7921104899
2411521144
3195096875
3907674299
0608570401
3095958801
7432409201
8638540081
4658560553
6156950844
8677407700
0669037710
1916653380
6041899932
4320867147
0287689837
0483131652
7873719459
2645920846
0043315033
3362859318
1020170329
5807479996
6721030307
0821501719
9479847808
9798638258
639934"""
