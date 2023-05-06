# setup.py script to build mandlebrot.c as a python module
from distutils.core import setup, Extension

module1 = Extension("mandlebrot", 
                    sources = ["mandlebrot.c", "mandlebrot_python.c"],
                    libraries = ['mpfr'],
                    library_dirs = ['/usr/local/lib'],
                    include_dirs = ['/usr/local/include'])

setup(name = "PackageName",
      version = '0.1',
      description = 'Mandlebrot Set calculator',
      author = 'Ian Judge',
      author_email = 'ianjudge1969@gmail.com',
      ext_modules = [module1]
)
