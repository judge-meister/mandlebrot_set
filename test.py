
from PIL import Image
import pygame
import math
import time
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
    sz = 640
    frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, xs, xe, ys, ye))
    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

    print("set calculated in ", time.time()-now, " secs")
    return surf,sz

def scaled(x, sz, s, e):
    return ( (float(x)/float(sz)) * (e-s) ) + s

def zoom_in(xs, xe, ys, ye, pos):
    #print("pos ", pos)
    loc = (scaled(pos[0], 640, xs, xe), scaled(pos[1], 640, ys, ye))
    #print("scaled loc ", loc)
    TL = (loc[0]-abs((xe-xs)/3), loc[1]-abs((ye-ys)/3))
    BR = (loc[0]+abs((xe-xs)/3), loc[1]+abs((ye-ys)/3))
    #print("new coords ", TL[0], BR[0], TL[1], BR[1])
    return TL[0], BR[0], TL[1], BR[1]

def zoom_out(xs, xe, ys, ye, pos):
    #print("pos ", pos)
    loc = (scaled(pos[0], 640, xs, xe), scaled(pos[1], 640, ys, ye))
    #print("scaled loc ", loc)
    TLx = loc[0]-abs((xe-xs)*0.75 )
    TLy = loc[1]-abs((ye-ys)*0.75 )
    BRx = loc[0]+abs((xe-xs)*0.75 )
    BRy = loc[1]+abs((ye-ys)*0.75 )
    # if we start to hit the upper bounds then adjust the centre
    if TLx < X1 or BRx > X2:
        TLx = X1
        BRx = X2
    if TLy < Y1 or BRy > Y2:
        TLy = Y1
        BRy = Y2

    print("new coords ", TLx, BRx, TLy, BRy)
    return TLx, BRx, TLy, BRy

def draw_plot(xs, xe, ys, ye):
    surf,sz = test3(xs, xe, ys, ye)

    now = time.time()
    surface.blit(surf, (0,0))
    pygame.display.update()

    #print("displayed in ", time.time()-now, " secs")


if __name__ == '__main__':
    xs=X1
    xe=X2
    ys=Y1
    ye=Y2

    draw_plot(xs, xe, ys, ye)

    zoom_level = 0
    run = False
    while not run:
        clock.tick(20)
        for event in pygame.event.get():
            if event.type == pygame.QUIT or (event.type == pygame.KEYUP and (event.key == pygame.K_q or event.key == pygame.K_ESCAPE)):
                run = True

            if event.type == pygame.KEYUP and event.key == pygame.K_c:
                zoom_level = 0
                print("zoom level ", zoom_level)
                xs,xe,ys,ye = X1,X2,Y1,Y2
                draw_plot(xs, xe, ys, ye)
                
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
                    print("zoom level ", zoom_level)
                    draw_plot(xs, xe, ys, ye)

    pygame.quit()
    exit()

