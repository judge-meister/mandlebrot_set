"""
The Mandlebrot Set

using a custom C module and pygame to display and zoom in on the mandlebrot set
"""

import pygame
import math
import time
import sys
import numpy
try:
    import mandlebrot
except ModuleNotFoundError:
    print("\nHave you built the mandlebrot C module ? try running ./build.sh\n")
    sys.exit()
from PIL import Image
    
X1=-2.0
X2=1.0
Y1=-1.5
Y2=1.5

now=time.time()
pygame.init()
window_size = 640
surface = pygame.display.set_mode((window_size, window_size))
surface.fill((255,255,255))
clock = pygame.time.Clock()
pygame.key.set_repeat(100,20)
#print("initialised in ",time.time()-now)


def scaled(x, sz, s, e):
    return ( (float(x)/float(sz)) * (e-s) ) + s


def zoom_in(xs, xe, ys, ye, pos):
    #print("pos ", pos)
    loc = (scaled(pos[0], window_size, xs, xe), scaled(pos[1], window_size, ys, ye))
    print("scaled loc ", loc)
    TL = (loc[0]-abs((xe-xs)/3), loc[1]-abs((ye-ys)/3))
    BR = (loc[0]+abs((xe-xs)/3), loc[1]+abs((ye-ys)/3))
    #print("new coords ", TL[0], BR[0], TL[1], BR[1])
    return TL[0], BR[0], TL[1], BR[1]


def zoom_out(xs, xe, ys, ye, pos): # pos(x,y) window_size(x,y)
    #print("pos ", pos)
    loc = (scaled(pos[0], window_size, xs, xe), scaled(pos[1], window_size, ys, ye))
    #print("scaled loc ", loc)
    TLx = loc[0]-abs((xe-xs)*0.75 )
    TLy = loc[1]-abs((ye-ys)*0.75 )
    BRx = loc[0]+abs((xe-xs)*0.75 )
    BRy = loc[1]+abs((ye-ys)*0.75 )
    # if we start to hit the upper bounds then adjust the centre
    if TLx < X1 or BRx > X2 or TLy < Y1 or BRy > Y2:
        TLx = X1
        BRx = X2
        TLy = Y1
        BRy = Y2

    #print("new coords ", TLx, BRx, TLy, BRy)
    return TLx, BRx, TLy, BRy


def draw_plot(xs, xe, ys, ye):
    now = time.time()

    sz = window_size
    #frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, sz, xs, xe, ys, ye, 255))
    #frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, 255, "-2.0", "1.0", "-1.5", "1.5"))
    frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, 255, "-1.7400623826", "-1.7400623824", "0.0281753398", "0.0281753395"))
    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

    print("set calculated in ", time.time()-now, " secs")

    #now = time.time()
    surface.blit(surf, (0,0))
    #asyncio.run(test4(xs, xe, ys, ye))

    pygame.display.update()

    #print("displayed in ", time.time()-now, " secs")

def zoom_in_mpfr(xs, xe, ys, ye, pos):
    """string params for mpfr"""
    TL_BR = mandlebrot.mandlebrot_zoom_in(xs, xe, ys, ye, pos[0], pos[1], window_size, window_size)
    return TL_BR

def draw_plot_mpfr(xs, xe, ys, ye):
    """params are strings representing real/imag value ranges"""
    
    now = time.time()

    sz = window_size
    #frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, sz, xs, xe, ys, ye, 255))
    #frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, 255, "-2.0", "1.0", "-1.5", "1.5"))
    #frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, 255, "-1.7400623826", "-1.7400623824", "0.0281753398", "0.0281753395"))
    frame = bytearray(mandlebrot.mandlebrot_mpfr(sz, sz, 255, xs, xe, ys, ye))
    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

    print("set calculated in ", time.time()-now, " secs")

    #now = time.time()
    surface.blit(surf, (0,0))
    #asyncio.run(test4(xs, xe, ys, ye))

    pygame.display.update()

    #print("displayed in ", time.time()-now, " secs")


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


def event_loop(xs, xe, ys, ye):
    """all params are strings"""
    zoom_level = 0
    run = False
    while not run:
        clock.tick(20)
        for event in pygame.event.get():

            mouse_pos = pygame.mouse.get_pos()
            loc = (0.0, 0.0) #(scaled(mouse_pos[0], window_size, xs, xe), scaled(mouse_pos[1], window_size, ys, ye))
            pygame.display.set_caption("Mandlebrot %s zoom=%s centre=%s" % (repr(mouse_pos), zoom_level, repr(loc)))

            if event.type == pygame.QUIT or (event.type == pygame.KEYUP and (event.key == pygame.K_q or event.key == pygame.K_ESCAPE)):
                run = True

            if event.type == pygame.KEYUP and event.key == pygame.K_c:
                zoom_level = 0
                print("zoom level ", zoom_level)
                xs,xe,ys,ye = X1,X2,Y1,Y2
                #draw_plot(xs, xe, ys, ye)
                draw_plot_mpfr(xs, xe, ys, ye)
                
            if event.type == pygame.KEYUP and event.key == pygame.K_z:
                #xs, xe, ys, ye = zoom_in(xs, xe, ys, ye, mouse_pos)
                xs, xe, ys, ye = zoom_in_mpfr(xs, xe, ys, ye, mouse_pos)
                zoom_level += 1
                print("zoom level ", zoom_level)
                #draw_plot(xs, xe, ys, ye)
                draw_plot_mpfr(xs, xe, ys, ye)

            #if event.type == pygame.KEYUP and event.key == pygame.K_x:
            #    if zoom_level > 0:
            #        xs, xe, ys, ye = zoom_out(xs, xe, ys, ye, (window_size/2, window_size/2))
            #        if xs == X1 and ys == Y1:
            #            zoom_level = 0
            #        else:
            #            zoom_level -= 1
            #            #if zoom_level == -1:
            #            #    zoom_level = 0
            #        print("zoom level ", zoom_level)
            #        draw_plot(xs, xe, ys, ye)

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
                    #xs, xe, ys, ye = zoom_in(xs, xe, ys, ye, mouse_pos)
                    zoom_level += 1
                    print("zoom level ", zoom_level)
                    #draw_plot(xs, xe, ys, ye)

                #if pressed[2]: # Button 3
                #    xs, xe, ys, ye = zoom_out(xs, xe, ys, ye, (window_size/2, window_size/2))
                #    zoom_level -= 1
                #    if zoom_level == -1:
                #        zoom_level = 0
                #    print("zoom level ", zoom_level)
                #    draw_plot(xs, xe, ys, ye)

    pygame.quit()


def main():
    display_help()
    
    xs = repr(X1)
    xe = repr(X2)
    ys = repr(Y1)
    ye = repr(Y2)

    draw_plot_mpfr(xs, xe, ys, ye)
    event_loop(xs, xe, ys, ye)


if __name__ == '__main__':
    main()
    
    
"""
Values to aim for
    
real=-1.7400623825
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
03865
    
img=0.0281753397
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