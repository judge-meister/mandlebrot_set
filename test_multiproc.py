
import time
import math
from numpy import arange
from multiprocessing import Pool, cpu_count

sz = 640

yl,yh = -1.25, 1.25
ys = (yh-yl) / float(sz)

xl,xh = -2.0, 0.5
xs = (xh-xl) / float(sz)

MAX=1000

def mand(c):
    z = 0
    for n in arange(MAX):
        z = z*z +c
        if abs(z) > 2:
            return n
    return MAX

def test1(params):
    n, xl,xh,xs, yl,yh,ys = params
    s = []
    for y in arange(yl,yh,ys):
        for x in arange(xl,xh,xs):
            s.append(math.sqrt(mand(complex(x,y))/MAX))
    return n, s
  
def test2():
    # multiprocessing
    print(cpu_count(), " CPUs")
    cpus = cpu_count() # should be 2* num cpu cores, any more gives negligible benefit
    yslice = (yh-yl)/cpus
    params = []
    for n in range(cpus):
        params.append([n, xl,xh,xs, yl+(n*yslice),yl+((n+1)*yslice),ys])

    pool = Pool(processes=cpus)
    d = pool.map(test1,params)
    for a in d:
        print(a[0], len(a[1]))
  
def main():
    #now = time.time()
    #test1([0, xl,xh,xs, yl,yh,ys])
    #print("test1 ",time.time() - now)
    now = time.time()
    test2()
    print("test2 ",time.time() - now)
  
if __name__ == '__main__':
    main()
