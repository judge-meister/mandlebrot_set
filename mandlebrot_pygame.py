
import os
import sys
import time
import math
import asyncio
import pygame
from pygame.locals import *
import mandlebrot as mandlebrot_c

"""
pseudo code from wikipedia to generate the mandlebrot set

for each pixel (Px, Py) on the screen do
    x0 := scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.00, 0.47))
    y0 := scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1.12, 1.12))
    x := 0.0
    y := 0.0
    iteration := 0
    max_iteration := 1000
    while (x*x + y*y ≤ 2*2 AND iteration < max_iteration) do
        xtemp := x*x - y*y + x0
        y := 2*x*y + y0
        x := xtemp
        iteration := iteration + 1
    
    color := palette[iteration]
    plot(Px, Py, color)
"""

WHITE = (255, 255, 255)
BLACK = (0,0,0)


X1 = -2.0
X2 = 0.6 #0.47
def scaled_x(x1, sz, Xs, Xe):
    x0 = ( (x1/sz) * (Xe-Xs) ) + Xs
    return x0

Y1 = -1.3 #-1.12
Y2 = 1.3 #1.12
def scaled_y(y1, sz, Ys, Ye):
    y0 = ( (y1/sz) * (Ye-Ys) ) + Ys
    return y0

def warp_red(it, itmax):
    if it >= itmax:
        color = BLACK
        
    if it >= 0 and it < itmax/2:
        idx = it / (itmax/2) * 255
        color = (idx, 0, 0)
        
    if it > itmax/2 and it < itmax:
        idx = (it - itmax/2) / (itmax/2) * 255
        color = (255, idx, idx)
    return color

def grayscale(it, itmax):
    if it >= itmax:
        color = BLACK
    if it < itmax:
        idx = math.sqrt( math.sqrt(it/itmax) ) * 255
        color = (idx, idx, idx)
    return color

async def iterate_location_coro(Px, Py, screen, clock, ssize, Xs,Xe, Ys,Ye):
    screen_x, screen_y = ssize
    x0 = scaled_x(Px, screen_x, Xs, Xe)
    y0 = scaled_y(Py, screen_y, Ys, Ye)
    x = 0.0
    y = 0.0
    iteration = 0
    maxiter = 1000
        
    while (x*x + y+y <= 2*2) and iteration < maxiter:
        xtemp = x*x - y*y + x0
        y = 2*x*y + y0
        x = xtemp
        iteration += 1

    color = grayscale(iteration, maxiter)
    #pygame.draw.line(screen, color, [Px,Py], [Px,Py], 1)
    #if iteration > 1 and iteration < 25:
    #    pygame.display.update()
    #    clock.tick()
    return Px, Py, color

async def mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye):
    screen_x, screen_y = ssize
    coro = [asyncio.create_task(iterate_location_coro(Px, Py, screen, clock, ssize, Xs,Xe, Ys,Ye)) for Px in range(screen_x) for Py in range(screen_y)]
    st=time.time()
    print("waiting for coroutines")
    completed, pending = await asyncio.wait(coro)
    #completed = await asyncio.gather(coro)
    print("await coro to complete in %2.2f secs" % (time.time()-st))
    c=0
    for item in completed:
        c+=1
        x, y, color = item.result()
        pygame.draw.line(screen, color, [x,y], [x,y], 1)
        if c % screen_x == 0:
            pygame.display.update()
            #clock.tick()
    pygame.display.update()
    #clock.tick()

def iterate_location(x0, y0):

    x = 0.0
    y = 0.0
    iteration = 0
    maxiter = 1000
        
    while (x*x + y+y <= 2*2) and iteration < maxiter:
        xtemp = x*x - y*y + x0
        y = 2*x*y + y0
        x = xtemp
        iteration += 1

    color = grayscale(iteration, maxiter)

    return color

def mandlebrot(ssize, screen, clock, Xs,Xe,Ys,Ye):
    """
    would using numpy improve performance and/or precision ?
    """
    screen_x, screen_y = ssize

    print("start mandlebrot")
    
    for Py in range(screen_y):
        for Px in range(screen_x):
            x0 = scaled_x(Px, screen_x, Xs, Xe)
            y0 = scaled_y(Py, screen_y, Ys, Ye)

            color = iterate_location(x0, y0)
            
            #color = warp_red(iteration, maxiter)
            #color = grayscale(iteration, maxiter)
                
            pygame.draw.line(screen, color, [Px,Py], [Px,Py], 1)
            #print(color, Px, Py)
        if Py % 25 == 0:
            pygame.display.update()
            #clock.tick()

    pygame.display.update()
    #clock.tick()
    print("finished")


def draw(screen, clock, data):
    c = 0
    for point in data:
        #print(point)
        c+=1
        iter, r, g, b, x, y = point
        pygame.draw.line(screen, (r, g, b), [x,y], [x,y], 1)

        
    pygame.display.update()
    clock.tick()
    print("finished")
    
    
def topleft(xs, xe, ys, ye):
    xe = xe - abs((xe-xs)/2)
    ye = ye - abs((ye-ys)/2)
    print("TL",xs,xe,ys,ye)
    return xs,xe,ys,ye

def topright(xs, xe, ys, ye):
    xs = xe - abs((xe-xs)/2)
    ye = ye - abs((ye-ys)/2)
    print("TR",xs,xe,ys,ye)
    return xs,xe,ys,ye

def bottomleft(xs, xe, ys, ye):
    xe = xe - abs((xe-xs)/2)
    ys = ys + abs((ye-ys)/2)
    print("BL",xs,xe,ys,ye)
    return xs,xe,ys,ye

def bottomright(xs, xe, ys, ye):
    xs = xe - abs((xe-xs)/2)
    ys = ys + abs((ye-ys)/2)
    print("BR",xs,xe,ys,ye)
    return xs,xe,ys,ye

def profile_mandlebrot_c(ssize, Xs, Xe, Ys, Ye):
    import cProfile
    import pstats
    
    with cProfile.Profile() as pr:
        plot = mandlebrot_c.mandlebrot(ssize, Xs, Xe, Ys, Ye)

    stats = pstats.Stats(pr)
    stats.sort_stats(pstats.SortKey.TIME)
    stats.print_stats()
    return plot

def profile_mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye):
    import cProfile
    import pstats
    
    with cProfile.Profile() as pr:
        mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)

    stats = pstats.Stats(pr)
    stats.sort_stats(pstats.SortKey.TIME)
    stats.print_stats()

def main(ssize, screen, clock):

    sx, sy = ssize
    print("start loop", sx)
    done = 0
    Xs = X1
    Xe = X2
    Ys = Y1
    Ye = Y2
    while not done:
        for e in pygame.event.get():
            if e.type == KEYUP and e.key == K_ESCAPE:
                done = 1
                
            if e.type == KEYDOWN and e.key == K_g:
                mandlebrot(ssize, screen, clock, X1, X2, Y1, Y2)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, X1, X2, Y1, Y2))
                #draw(screen, clock, mandlebrot_c.mandlebrot(sx, X1, X2, Y1, Y2))
                Xs = X1
                Xe = X2
                Ys = Y1
                Ye = Y2

            if e.type == KEYDOWN and e.key == K_c:
                #mandlebrot(ssize, screen, clock, X1, X2, Y1, Y2)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, X1, X2, Y1, Y2))
                #draw(screen, clock, mandlebrot_c.mandlebrot(sx, X1, X2, Y1, Y2))
                draw(screen, clock, profile_mandlebrot_c(sx, X1, X2, Y1, Y2))
                Xs = X1
                Xe = X2
                Ys = Y1
                Ye = Y2
                
            if e.type == KEYDOWN and e.key == K_q:
                Xsp, Xep, Ysp, Yep = Xs, Xe, Ys, Ye
                Xs, Xe, Ys, Ye = topleft(Xs, Xe, Ys, Ye)
                #mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye))
                draw(screen, clock, mandlebrot_c.mandlebrot(sx, Xs, Xe, Ys, Ye))
                
            if e.type == KEYDOWN and e.key == K_w:
                Xsp, Xep, Ysp, Yep = Xs, Xe, Ys, Ye
                Xs, Xe, Ys, Ye = topright(Xs, Xe, Ys, Ye)
                #mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye))
                draw(screen, clock, mandlebrot_c.mandlebrot(sx, Xs, Xe, Ys, Ye))
                
            if e.type == KEYDOWN and e.key == K_a:
                Xsp, Xep, Ysp, Yep = Xs, Xe, Ys, Ye
                Xs, Xe, Ys, Ye = bottomleft(Xs, Xe, Ys, Ye)
                #mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye))
                draw(screen, clock, mandlebrot_c.mandlebrot(sx, Xs, Xe, Ys, Ye))
                
            if e.type == KEYDOWN and e.key == K_s:
                Xsp, Xep, Ysp, Yep = Xs, Xe, Ys, Ye
                Xs, Xe, Ys, Ye = bottomright(Xs, Xe, Ys, Ye)
                #mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye))
                draw(screen, clock, mandlebrot_c.mandlebrot(sx, Xs, Xe, Ys, Ye))
                
            if e.type == KEYDOWN and e.key == K_z:
                Xs, Xe, Ys, Ye = Xsp, Xep, Ysp, Yep
                #mandlebrot(ssize, screen, clock, Xs, Xe, Ys, Ye)
                #asyncio.run(mandlebrot_coro(ssize, screen, clock, Xs,Xe,Ys,Ye))
                draw(screen, clock, mandlebrot_c.mandlebrot(sx, Xs, Xe, Ys, Ye))
                
        pygame.display.update()
        clock.tick(10)
    print("Quiting")
        
def setup(ssize):
    pygame.init()
    scr_inf = pygame.display.Info()
    os.environ['SDL_VIDEO_WINDOW_POS'] = '{}, {}'.format(scr_inf.current_w, # // 2 - WINSIZE[0] // 2,
                                                         scr_inf.current_h)# // 2 - WINSIZE[1] // 2)
    screen = pygame.display.set_mode(ssize)
    screen.fill(WHITE)
    clock = pygame.time.Clock()
    
    #pygame.draw.rect(screen, BLACK, [1,1,200,200], 1)
    pygame.display.set_caption("Mandlebrot")
    pygame.display.flip()
    pygame.display.update()
    clock.tick()

    return screen, clock

def profile_func():
    import cProfile
    import pstats
    
    with cProfile.Profile() as pr:
        x, y = -2.0, -1.5
        while x <= 1.0:
            x += 0.1
            while y <= 1.5:
                y += 0.1
                iterate_location(x,y)

    stats = pstats.Stats(pr)
    stats.sort_stats(pstats.SortKey.TIME)
    stats.print_stats()

   

if __name__ == '__main__':
   
    #profile_func()
    #sys.exit()

    ssize = (640, 640)
    
    screen, clock = setup(ssize)
    
    main(ssize, screen, clock)
    
