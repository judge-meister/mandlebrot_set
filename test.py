
from PIL import Image
import pygame
import math
import time
import mandlebrot

now=time.time()
pygame.init()
window_size = 640
surface = pygame.display.set_mode((window_size, window_size))
surface.fill((255,255,255))
clock = pygame.time.Clock()
print("initialised in ",time.time()-now)

def test():
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


def test3():
    now = time.time()
    print("start test3")
    sz = 640
    frame = bytearray(mandlebrot.mandlebrot_bytearray(sz, -2.0, 1.0, -1.5, 1.5))
    surf = pygame.image.frombuffer(frame, (sz,sz), 'RGB')

    print("end test3 in ",time.time()-now)
    return surf,sz


if __name__ == '__main__':

    surf,sz = test3()

    now = time.time()
    surface.blit(surf, (0,0))

    pygame.display.update()
    #pygame.display.flip()

    print("displayed in ",time.time()-now)

    run = False
    while not run:
        clock.tick(20)
        for event in pygame.event.get():
            if event.type == pygame.QUIT or (event.type == pygame.KEYUP and (event.key == pygame.K_q or event.key == pygame.K_ESCAPE)):
                run = True

        #window.fill(0)
        #pygame.Surface.blit(window, surf, (sz,sz))
        #pygame.display.update()


    pygame.quit()
    exit()

