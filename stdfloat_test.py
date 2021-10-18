
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

class Mandlebrot_stdfloat:

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

                    color = self.iterate_location(x0, y0)
                bar.next()

    def iterate_location(self, x0, y0):
        """"""
        x = 0.0
        y = 0.0
        iteration = 0
        maxiter = self.breakout

        sq_x_plus_sq_y = 0.0
        while (sq_x_plus_sq_y <= 4) and iteration < maxiter:
            sq_x_plus_sq_y = x*x + y*y
            xtemp = sq_x_plus_sq_y + x0
            y = 2*x*y + y0
            x = xtemp
            iteration += 1

        color = grayscale(iteration, maxiter)



if __name__ == '__main__':
    print("Testing std float module")

    now = time.time()

    mand = Mandlebrot_stdfloat(255)
    mand.generate(320, 320)

    print("Took ", time.time() - now, " secs")

