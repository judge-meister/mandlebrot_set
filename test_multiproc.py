
import time
import math
from numpy import arange
from multiprocessing import Pool

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
  xl,xh,xs,yl,yh,ys = params
  s = []
  for y in arange(yl,yh,ys):
    for x in arange(xl,xh,xs):
      s.append(math.sqrt(mand(complex(x,y))/MAX))
  # return s
  
def test2():
  # multiprocessing
  cpus = 128
  yslice = (yh-yl)/cpus
  y8s = ys * cpus
  params = []
  for n in range(cpus):
    params.append([xl,xh,xs, yl+(n*yslice),y1+((n+1)*yslice),y8s])
    
  pool = Pool(processes=cpus)
  pool.map(test1,params)
  
def main():
  now = time.time()
  test1()
  print("test1 ",time.time() - now)
  now = time.time()
  test2()
  print("test2 ",time.time() - now)
  
if __name__ == '__main__':
  main()
