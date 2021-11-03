"""
The Mandlebrot Set

using a custom C module and pygame to display and zoom in on the mandlebrot set
"""

import pygame
import math
import time
import sys
import getopt
import functools

try:
    import mandlebrot
except ModuleNotFoundError:
    print("\nHave you built the mandlebrot C module ? try running 'make'\n")
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
    """"""
    def __init__(self, size):
        """"""
        pygame.init()
        self.pygameSurface = pygame.display.set_mode((size, size))
        self.pygameSurface.fill((255,255,255))
        self.pygameClock = pygame.time.Clock()
        pygame.key.set_repeat(100,20)
        self.windowsize = size

    def surface(self):
        """"""
        return self.pygameSurface

    def clock(self):
        """"""
        return self.pygameClock

    def winsize(self):
        """"""
        return self.windowsize


class mandlebrot_python_float:
    """"""
    def __init__(self, pgwin):
        """"""
        self.Xs = X1 #-2.0
        self.Xe = X2 #1.0
        self.Ys = Y1 #-1.5
        self.Ye = Y2 #1.5
        self.factor = 10
        self.zoom = 1
        self.maxiter = 1000
        self.pgwin = pgwin
        self.centre = {'r': -0.5, 'i': 0.0} # not used in this version


    @classmethod
    def _scaled(cls, x, sz, s, e):
        """"""
        return ( (float(x)/float(sz)) * (e-s) ) + s


    def scaled_pos(self, pos):
        """"""
        sz = self.pgwin.winsize()
        loc = (self._scaled(pos[0], sz, self.Xs, self.Xe), self._scaled(pos[1], sz, self.Ys, self.Ye))
        return {'r': loc[0], 'i': loc[1]}


    def reset(self):
        """"""
        self.Xs = X1 #-2.0
        self.Xe = X2 #1.0
        self.Ys = Y1 #-1.5
        self.Ye = Y2 #1.5


    def zoom_in(self, pos):
        """"""
        self.zoom +=1
        sz = self.pgwin.winsize()
        loc = (self._scaled(pos[0], sz, self.Xs, self.Xe), self._scaled(pos[1], sz, self.Ys, self.Ye))
        self.centre = {'r': loc[0], 'i': loc[1]}
        
        print("scaled loc ", loc)
        TLx = loc[0]-abs((self.Xe-self.Xs)/self.factor)
        TLy = loc[1]-abs((self.Ye-self.Ys)/self.factor)
        BRx = loc[0]+abs((self.Xe-self.Xs)/self.factor)
        BRy = loc[1]+abs((self.Ye-self.Ys)/self.factor)
        print("new coords %2.9f %2.9f %2.9f %2.9f" % ( TLx, BRx, TLy, BRy))

        self.Xs, self.Xe, self.Ys, self.Ye = TLx, BRx, TLy, BRy


    def zoom_out(self, pos):
        """"""
        self.zoom -=1
        sz = self.pgwin.winsize()
        loc = (self._scaled(pos[0], sz, self.Xs, self.Xe), self._scaled(pos[1], sz, self.Ys, self.Ye))
        self.centre = {'r': loc[0], 'i': loc[1]}

        TLx = loc[0]-abs((self.Xe-self.Xs)*self.factor )
        TLy = loc[1]-abs((self.Ye-self.Ys)*self.factor )
        BRx = loc[0]+abs((self.Xe-self.Xs)*self.factor )
        BRy = loc[1]+abs((self.Ye-self.Ys)*self.factor )

        # if we start to hit the upper bounds then adjust the centre
        if TLx < X1 and BRx > X2 and TLy < Y1 and BRy > Y2:
            #TLx, BRx, TLy, BRy = X1, X2, Y1, Y2
            self.reset()
        else:
            self.Xs, self.Xe, self.Ys, self.Ye = TLx, BRx, TLy, BRy


    @timer
    def draw_plot(self):
        """"""
        sz = self.pgwin.winsize()
        frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, sz, self.maxiter, self.Xs, self.Xe, self.Ys, self.Ye))

        surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

        self.pgwin.surface().blit(surf, (0,0))

        pygame.display.update()


class mandlebrot_python_float_centre:
    """"""
    def __init__(self, pgwin):
        """"""
        self.Xs = X1 #-2.0
        self.Xe = X2 #1.0
        self.Ys = Y1 #-1.5
        self.Ye = Y2 #1.5
        self.factor = 6
        self._zoom = -1
        self.maxiter = 1000
        self.pgwin = pgwin
        self._centre = {'r': -0.5, 'i': 0.0}

    @property
    def zoom(self):
        """"""
        return self._zoom

    @zoom.setter
    def zoom(self, value):
        """"""
        self._zoom = value
        
    @property
    def centre(self):
        """"""
        return self._centre

    @centre.setter
    def centre(self, value):
        self._centre = value


    def _scaledX(self, x):
        """"""
        return ( (float(x)/float( self.pgwin.winsize() )) * (self.Xe - self.Xs) ) + self.Xs


    def _scaledY(self, y):
        """"""
        return ( (float(y)/float( self.pgwin.winsize() )) * (self.Ye - self.Ys) ) + self.Ys


    def scaled_pos(self, pos):
        """"""
        loc = (self._scaledX(pos[0]), self._scaledY(pos[1]))
        return {'r': loc[0], 'i': loc[1]}


    def create_corners(self):
        """"""
        offset = 1.0 / (math.pow(self.factor, self._zoom))
        self.Xs = self._centre['r'] - offset
        self.Xe = self._centre['r'] + offset
        self.Ys = self._centre['i'] - offset
        self.Ye = self._centre['i'] + offset
        print("centre ",self._centre, "offset ",offset)
        print("corners ",self.Xs, self.Xe, self.Ys, self.Ye)


    def reset(self):
        """"""
        self._centre = {'r': -0.5, 'i': 0.0}
        self._zoom = 0
        self.Xs, self.Xe, self.Ys, self.Ye = X1, X2, Y1, Y2
        

    def zoom_in(self, pos):
        """"""
        self._centre = {'r': self._scaledX(pos[0]), 'i': self._scaledY(pos[1])}
        self._zoom +=1
        print("zoom in ", self._zoom)
        self.create_corners()


    def zoom_out(self, pos):
        """"""
        self._zoom -=1
        print("zoom out ", self._zoom)
        self.create_corners()
        #if self._zoom <= 0:
        #    self.reset()
        #    print("zoom out ", self._zoom)
        #else:
        #    print("zoom out ", self._zoom)
        #    self.create_corners()


    @timer
    def draw_plot(self):
        """"""
        sz = self.pgwin.winsize()
        frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, sz, self.maxiter, self.Xs, self.Xe, self.Ys, self.Ye))

        surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

        self.pgwin.surface().blit(surf, (0,0))

        pygame.display.update()


class mandlebrot_c_mpfr:
    """"""
    def __init__(self, pgwin):
        """"""
        self.Xs = repr(X1)
        self.Xe = repr(X2)
        self.Ys = repr(Y1)
        self.Ye = repr(Y2)
        self.factor = 10
        self.zoom = 1
        self.maxiter = 1000
        self.pgwin = pgwin
        self.centre = {'r': -0.5, 'i': 0.0}
        
        mandlebrot.setup()
        mandlebrot.initialize(self.Xs, self.Xe, self.Ys, self.Ye)


    def _scaledX(self, x):
        """"""
        return ( (float(x)/float( self.pgwin.winsize() )) * (float(self.Xe) - float(self.Xs)) ) + float(self.Xs)


    def _scaledY(self, y):
        """"""
        return ( (float(y)/float( self.pgwin.winsize() )) * (float(self.Ye) - float(self.Ys)) ) + float(self.Ys)


    def scaled_pos(self, pos):
        """"""
        loc = (self._scaledX(pos[0]), self._scaledY(pos[1]))
        return {'r': loc[0], 'i': loc[1]}


    def reset(self):
        """"""
        xs = repr(X1)
        xe = repr(X2)
        ys = repr(Y1)
        ye = repr(Y2)
        mandlebrot.initialize(self.Xs, self.Xe, self.Ys, self.Ye)


    def zoom_in(self, pos):
        """"""
        sz = self.pgwin.winsize()
        mandlebrot.mandlebrot_zoom_in(pos[0], pos[1], sz, sz, self.factor)


    def zoom_out(self, pos):
        """"""
        sz = self.pgwin.winsize()
        mandlebrot.mandlebrot_zoom_out(int(pos[0]), int(pos[1]), sz, sz, self.factor)


    @timer
    def draw_plot(self):
        """params are strings representing real/imag value ranges"""
        sz = self.pgwin.winsize()
        frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, self.maxiter))

        surf = pygame.image.frombuffer(frame, (sz, sz), 'RGB')

        self.pgwin.surface().blit(surf, (0,0))

        pygame.display.update()


def display_help():
    help = """
    How to control the Mandlebrot Set viewer.

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

    zoom_level = 0
    pygame.mouse.set_cursor(pygame.cursors.broken_x)
    window_size = pgwin.winsize()
    
    run = False
    while not run:
        pgwin.clock().tick(20)

        for event in pygame.event.get():

            mouse_pos = pygame.mouse.get_pos()
            centre_pos = mand.scaled_pos(mouse_pos)
            pygame.display.set_caption("Mandlebrot %s zoom=%s centre=%s" % (repr(mouse_pos), mand.zoom, repr(centre_pos)))

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
    """"""
    mand = mandlebrot_c_mpfr(pgwin)
    mand.draw_plot()
    event_loop(mand, pgwin)
    mandlebrot.free_mpfr_mem()


def main_python(pgwin, options):
    """"""
    mand = mandlebrot_python_float(pgwin)
    mand.draw_plot()
    event_loop(mand, pgwin)


def main_python_centre(pgwin, options):
    """"""
    mand = mandlebrot_python_float_centre(pgwin)
    if 'real' in options and 'imag' in options:
        mand.centre = {'r': float(options['real']), 'i': float(options['imag'])}
        mand.zoom = options['zoom']
        mand.create_corners()
    mand.draw_plot()
    event_loop(mand, pgwin)


def usage():
    """"""
    print("mandlebrot_set.py")
    print("   -h  this help")
    print("   -z  zoom level to start with")
    print("   -r  real value of point to zoom in to")
    print("   -i  imaginary value of point to zoom in to")
    print("   -d  display size (assumes a square)")
    print("   -a  which algorithm (float, mpfr)")
    print("   -f  zoom factor")


def getOptions(options):
    """"""
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hz:r:i:a:d:f:", ["help", "zoom=", "real=", "imag=", "algo=", "disp=", "factor="])
    except getopt.GetoptError as err:
        print(err)
        usage()
        sys.exit(2)

    options['algo'] = 'mpfr'
    options['disp'] = 640

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
            if a in ['float', 'mpfr', 'centre']:
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
                options['factor'] = int(a)
            except ValueError:
                print("\nERROR: -factor must be a positive integer\n")
                usage()
                sys.exit(2)


def main(options):
    """"""
    display_help()

    pygwin = PyGameWindow(options['disp'])

    if options['algo'] == "float":
        main_python(pygwin, options)

    if options['algo'] == "centre":
        main_python_centre(pygwin, options)

    elif options['algo'] == 'mpfr':
        main_mpfr(pygwin, options)

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
