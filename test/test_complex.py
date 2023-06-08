
import time
import math
from progress.bar import Bar

def scaled(x, sz, e, s):
    return ((x/sz)*(e-s))+s

def grayscale(it, itmax):
    """"""
    color = (0,0,0)
    if it<itmax:
        idx = math.sqrt( it/itmax) * 255
        color = (idx, idx, idx)
    return color

class Mandelbrot_stdfloat:

    def __init__(self, breakout=1000):
        """"""
        self.Xs = -2.0
        self.Xe = 1.0
        self.Ys = -1.5
        self.Ye = 1.5
        self.breakout = breakout

    def generate(self, w, h):
        """"""
        with Bar('row', max=h) as bar:
            for Ph in range(h):
                for Pw in range(w):

                    x0 = ((Pw/w)*(self.Xe-self.Xs))+self.Xs
                    y0 = ((Ph/h)*(self.Ye-self.Ys))+self.Ys

                    it = self.iterate_location(complex(x0, y0))
                    color = grayscale(it, self.breakout)
                bar.next()


    def iterate_location(self, c):
        """"""
        z = 0
        for n in range(self.breakout):
            z = z*z + c
            if(abs(z) > 2):
                return n
        return n


if __name__ == '__main__':
    print("Testing std float module")

    now = time.time()

    mand = Mandelbrot_stdfloat(1000)
    mand.generate(640, 640)

    print("Took ", time.time() - now, " secs")

