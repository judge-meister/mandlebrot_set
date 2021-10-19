
from PIL import Image
import pygame
import math
import time
import numpy
import asyncio
import mandlebrot

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
print("initialised in ",time.time()-now)

def test1():
    buf = Image.open('/home/judge/clones/dotfiles/.local/share/icons/battery_low_dark.png').tobytes()
    print("is list? ", isinstance(buf, list), dir(buf) )
    print(buf[int(len(buf)/2):int(len(buf)/2)+32])
    #for b in buf:
    #    print(b)


def test2():
    print("start test2")
    frame = bytearray()
    for r in range(64,128):
        for g in range(64,128):
            for b in range(64,128):
                frame.append(r)
                frame.append(g)
                frame.append(b)

    sz = int(math.sqrt(len(frame)/3))
    print(sz)

    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')
    print("end test2")
    return surf,sz


def test3(xs, xe, ys, ye):
    now = time.time()
    #print("start test3")
    sz = window_size
    frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, sz, xs, xe, ys, ye))
    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

    print("set calculated in ", time.time()-now, " secs")
    return surf,sz

async def mandlebrot_bytearray_async(sx, sy, xs, xe, ys, ye):
    return mandlebrot.mandlebrot_bytearray(sx, sy, xs,xe, ys,ye)

async def test4(xs, xe, ys, ye):
    """split square into 4 horizontal strips and use asyncio to call 
    mandlebrot module 4 times, will need to combine the returned
    bytearrays in to a single surface.
    """
    now = time.time()

    sz = 640
    y4 = numpy.linspace(ys, ye, 5)
    sz4 = int(sz/4) #numpy.linspace(1, 640, 5)
    print(sz4, y4)

    chunks = await asyncio.gather(
        mandlebrot_bytearray_async(sz, sz4, xs,xe, y4[0],y4[1]),
        mandlebrot_bytearray_async(sz, sz4, xs,xe, y4[1],y4[2]),
        mandlebrot_bytearray_async(sz, sz4, xs,xe, y4[2],y4[3]),
        mandlebrot_bytearray_async(sz, sz4, xs,xe, y4[3],y4[4])
    )

    new_y = 0
    for frame in chunks:
        surf = pygame.image.frombuffer(bytearray(frame), (sz,sz4), 'RGB')
        surface.blit(surf, (0,new_y))
        new_y += sz4

    #surface.blit(surf, (0,0))
    pygame.display.update()

    print("set calculated in ", time.time()-now, " secs")
    return surf, sz

def scaled(x, sz, s, e):
    return ( (float(x)/float(sz)) * (e-s) ) + s

def zoom_in(xs, xe, ys, ye, pos):
    #print("pos ", pos)
    loc = (scaled(pos[0], window_size, xs, xe), scaled(pos[1], window_size, ys, ye))
    #print("scaled loc ", loc)
    TL = (loc[0]-abs((xe-xs)/3), loc[1]-abs((ye-ys)/3))
    BR = (loc[0]+abs((xe-xs)/3), loc[1]+abs((ye-ys)/3))
    #print("new coords ", TL[0], BR[0], TL[1], BR[1])
    return TL[0], BR[0], TL[1], BR[1]

def zoom_out(xs, xe, ys, ye, pos):
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
    surf,sz = test3(xs, xe, ys, ye)

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

if __name__ == '__main__':
    xs=X1
    xe=X2
    ys=Y1
    ye=Y2

    display_help()
    
    draw_plot(xs, xe, ys, ye)

    pygame.key.set_repeat(100,20)
    
    zoom_level = 0
    run = False
    while not run:
        clock.tick(20)
        for event in pygame.event.get():

            mouse_pos = pygame.mouse.get_pos()
            pygame.display.set_caption("Mandlebrot (%s)" % repr(mouse_pos))

            if event.type == pygame.QUIT or (event.type == pygame.KEYUP and (event.key == pygame.K_q or event.key == pygame.K_ESCAPE)):
                run = True

            if event.type == pygame.KEYUP and event.key == pygame.K_c:
                zoom_level = 0
                print("zoom level ", zoom_level)
                xs,xe,ys,ye = X1,X2,Y1,Y2
                draw_plot(xs, xe, ys, ye)
                
            if event.type == pygame.KEYUP and event.key == pygame.K_z:
                xs, xe, ys, ye = zoom_in(xs, xe, ys, ye, mouse_pos)
                zoom_level += 1
                print("zoom level ", zoom_level)
                draw_plot(xs, xe, ys, ye)

            if event.type == pygame.KEYUP and event.key == pygame.K_x:
                if zoom_level > 0:
                    xs, xe, ys, ye = zoom_out(xs, xe, ys, ye, (window_size/2, window_size/2))
                    if xs == X1 and ys == Y1:
                        zoom_level = 0
                    else:
                        zoom_level -= 1
                        #if zoom_level == -1:
                        #    zoom_level = 0
                    print("zoom level ", zoom_level)
                    draw_plot(xs, xe, ys, ye)

            if event.type == pygame.KEYDOWN and event.key == pygame.K_UP:
                if mouse_pos[1] > 1:
                    pygame.mouse.set_pos(mouse_pos[0], mouse_pos[1]-1)

            if event.type == pygame.KEYDOWN and event.key == pygame.K_DOWN:
                if mouse_pos[1] < window_size-1:
                    pygame.mouse.set_pos(mouse_pos[0], mouse_pos[1]+1)

            if event.type == pygame.KEYDOWN and event.key == pygame.K_LEFT:
                if mouse_pos[0] > 1:
                    pygame.mouse.set_pos(mouse_pos[0]-1, mouse_pos[1])

            if event.type == pygame.KEYDOWN and event.key == pygame.K_RIGHT:
                if mouse_pos[0] < window_size-1:
                    pygame.mouse.set_pos(mouse_pos[0]+1, mouse_pos[1])
                
            if event.type == pygame.MOUSEBUTTONDOWN:
                mouse_pos = pygame.mouse.get_pos()
                pressed = pygame.mouse.get_pressed()
                if pressed[0]: # Button 1
                    xs, xe, ys, ye = zoom_in(xs, xe, ys, ye, mouse_pos)
                    zoom_level += 1
                    print("zoom level ", zoom_level)
                    draw_plot(xs, xe, ys, ye)

                if pressed[2]: # Button 3
                    xs, xe, ys, ye = zoom_out(xs, xe, ys, ye, (window_size/2, window_size/2))
                    zoom_level -= 1
                    if zoom_level == -1:
                        zoom_level = 0
                    print("zoom level ", zoom_level)
                    draw_plot(xs, xe, ys, ye)

    pygame.quit()
    exit()

