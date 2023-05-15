
# very simplictic makefile - could do better

DEBUG=-DTRACE

all: pymodule test


PKGCONFIG=$(shell pkg-config --cflags --libs gmp mpfr)

# rm needs to know which platform it is on
.PHONY: pymodule
pymodule:
	-$(RM) -fr build
	-$(RM) -f mandlebrot.cpython-3*.so
	# mandlebrot.cpython-311-x86_64-linux-gnu.so
	# call setup.py to build the c module
	python3 setup.py build_ext --inplace


.PHONY: test
test:
	-$(RM) -f test_mandlebrot_main
	gcc -g -O2 $(DEBUG) test_mandlebrot_main.c mandlebrot.c $(PKGCONFIG) -lm -lpthread \
            -o test_mandlebrot_main

coverage:
	-$(RM) -rf *.gcno *.gcda *.info lcovhtml
	gcc -g -O2 -fprofile-arcs -ftest-coverage \
	    test_mandlebrot_main.c mandlebrot.c $(PKGCONFIG) -lm -lpthread -o test_mandlebrot_main
	./test_mandlebrot_main
	lcov -c --directory . --output-file test_mandlebrot_main.info
	genhtml test_mandlebrot_main.info --output-directory lcovhtml


valgrind: test
	valgrind --leak-check=full ./test_mandlebrot_main

clean:
	-$(RM) ./test_mandlebrot_main ./mandlebrot.cpython-3*-*.so
	-$(RM) -fr build


