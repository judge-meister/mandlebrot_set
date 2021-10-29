
# very simplictic makefile - could do better

all: pymodule test


# rm needs to know which platform it is on
pymodule:
	rm -f mandlebrot.cpython-39-darwin.so
	# call setup.py to build the c module
	python3 setup.py build_ext --inplace


test:
	rm -f test_mandlebrot_main
	gcc  test_mandlebrot_main.c mandlebrot.c -lmpfr -lm   -o test_mandlebrot_main

