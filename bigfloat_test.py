
import time
import math
from bigfloat import *
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

class Mandlebrot_bigfloat:

    def __init__(self, breakout=1000):
        """"""
        self.my_precision = half_precision
        self.Xs = BigFloat('-2.0', self.my_precision)
        self.Xe = BigFloat( '1.0', self.my_precision)
        self.Ys = BigFloat('-1.5', self.my_precision)
        self.Ye = BigFloat( '1.5', self.my_precision)
        self.breakout = breakout

    def generate(self, w, h):
        """"""
        with Bar('row', max=h) as bar:
            for Ph in range(h):
                for Pw in range(w):

                    x0 = ((Pw/w)*(self.Xe-self.Xs))+self.Xs
                    y0 = ((Ph/h)*(self.Ye-self.Ys))+self.Ys

                    color = self.iterate_location(x0, y0)
                bar.next()

    def iterate_location(self, x0, y0):
        """"""
        x = BigFloat('0.0', self.my_precision)
        y = BigFloat('0.0', self.my_precision)
        iteration = 0
        maxiter = self.breakout

        while (x*x + y*y <= 4) and iteration < maxiter:
            xtemp = x*x + y*y + x0
            y = 2*x*y + y0
            x = xtemp
            iteration += 1

        color = grayscale(iteration, maxiter)



if __name__ == '__main__':
    print("Testing bigfloat module")

    now = time.time()

    mand = Mandlebrot_bigfloat(255)
    mand.generate(80, 80)

    print("Took ", time.time() - now, " secs")

