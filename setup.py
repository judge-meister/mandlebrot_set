# setup.py script to build mandlebrot.c as a python module
from distutils.core import setup, Extension

setup(
    ext_modules=[Extension("mandlebrot", ["mandlebrot.c"])],
)