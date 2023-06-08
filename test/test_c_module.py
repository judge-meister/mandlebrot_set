
import time
import mandelbrot
from numpy import arange
from multiprocessing import Pool, cpu_count


class Mandelbrot_c_module:

    def __init__(self):
        """"""
        self.Xs = -2.0
        self.Xe = 1.0
        self.Ys = -1.5
        self.Ye = 1.5

    def generate(self, w, h):
        """"""
        frame = bytearray(mandelbrot.mandelbrot_bytearray(w, h, self.Xs, self.Xe, self.Ys, self.Ye, 1000))

    def mp_generate(self, w, h):
        # multiprocessing
        cpus = int(cpu_count()) # should be 2* num cpu cores, any more gives negligible benefit
        #print(cpus, " CPUs")
        yslice = (self.Ye-self.Ys)/cpus
        params = []
        for n in range(cpus):
            params.append((w,int(h/cpus), self.Xs, self.Xe, self.Ys+(n*yslice), self.Ys+((n+1)*yslice), 1000))

        pool = Pool(processes=cpus)
        d = pool.starmap(mandelbrot.mandelbrot_bytearray, params)
        #for a in d:
        #    print(a[0], a[1])


if __name__ == '__main__':
    sz = 1280
    print("Testing c module")
    mand = Mandelbrot_c_module()

    for a in range(100):
        #now = time.time()
        #mand.generate(sz, sz)
        #print("Took ", time.time() - now, " secs")

        now = time.time()
        mand.mp_generate(sz, sz)
        print("Took ", time.time() - now, " secs")

