# setup.py script to build mandlebrot.c as a python module
from distutils.core import setup, Extension
from subprocess import getoutput

LIB_DIRS=getoutput("pkg-config --libs-only-L gmp mpfr").replace('-L','').split(' ')
INCLUDE_DIRS=getoutput("pkg-config --cflags-only-I gmp mpfr").replace('-I','').split(' ')
#print(LIB_DIRS)
#print(INCLUDE_DIRS)

module1 = Extension("mandlebrot", 
                    sources = ["mandlebrot.c", "mandlebrot_python.c"],
                    libraries = ['mpfr'],
                    library_dirs = LIB_DIRS, #['/usr/local/lib'],
                    include_dirs = INCLUDE_DIRS) #['/usr/local/include'])

setup(name = "PackageName",
      version = '0.1',
      description = 'Mandlebrot Set calculator',
      author = 'Ian Judge',
      author_email = 'ianjudge1969@gmail.com',
      ext_modules = [module1]
)
