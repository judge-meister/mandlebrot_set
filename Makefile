
# very simplictic makefile - could do better

DEBUG=-DTRACE

all: pymodule test


# rm needs to know which platform it is on
pymodule:
	rm -f mandlebrot.cpython-39-*.so
	# call setup.py to build the c module
	python3 setup.py build_ext --inplace


.PHONY: test
test:
	rm -f test_mandlebrot_main
	gcc -g -O2 $(DEBUG) test_mandlebrot_main.c mandlebrot.c -lmpfr -lm -lpthread   -o test_mandlebrot_main


valgrind: test
	valgrind --leak-check=full ./test_mandlebrot_main
