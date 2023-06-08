
# very simplictic makefile - could do better

OS := $(shell uname)
ifeq ($(OS),Darwin)
  # Run MacOS commands
  SO := .dylib
endif
ifeq ($(OS),Linux)
  # check for Linux and run other commands
  SO := .so
endif

DEBUG=-DTRACE
USES_THREADS=-DUSES_THREADS

MPFR_LIBS=$(shell pkg-config --libs mpfr)
MPFR_INCL=$(shell pkg-config --cflags gmp mpfr)
PYBIND11_INCL=$(shell python3 -m pybind11 --includes)
PYEMBED_SUFFIX=$(shell python3-config --extension-suffix)
PYEMBED_LIBS=$(shell pkg-config --libs python3-embed)
PYEMBED_INCL=$(shell pkg-config --cflags python3-embed)

NAME=mandelbrot

TEST_EXE=test_$(NAME)_main
TEST_SRC=test_$(NAME)_main.c 

PYBIND_SRCS=$(NAME)_pybind11.cpp

PYMODULE=$(NAME)$(PYEMBED_SUFFIX)

SHARED_LIB_SRCS=$(NAME).c
SHARED_LIB=lib$(NAME)$(SO)
MANDELBROT_LIBS=-L./ -l$(NAME)

LD_SHARED=-shared -Wl,-rpath=./,-rpath-link=./ -Wl,--no-undefined
SONAME=-Wl,-soname=$(SHARED_LIB)
#-Wl,--unresolved-symbols=report-all
#ifeq ($(OS),Darwin)
#LD_SHARED=-dylib
#endif

# -----------------------------------------------------------------------------

all: pybind11 test

.PHONY: pymodule
pymodule: clean_pymodule
	# call setup.py to build the c module
	python3 setup.py build_ext --inplace


.PHONY: test
test: $(TEST_EXE)
	./$(TEST_EXE)

$(TEST_EXE): $(SHARED_LIB)
	gcc -g -O3 $(DEBUG) $(USES_THREADS) $(MANDELBROT_LIBS) $(MPFR_INCL) $(MPFR_LIBS) \
		-Wl,-rpath=./ $(TEST_SRC) -o $(TEST_EXE)

.PHONY: coverage
coverage: $(SHARED_LIB)
	gcc -g -O3 $(DEBUG) $(USES_THREADS) $(MANDELBROT_LIBS) $(MPFR_INCL) $(MPFR_LIBS) \
		-fprofile-arcs -ftest-coverage $(TEST_SRC) -o $(TEST_EXE)
	./$(TEST_EXE)
	lcov -c --directory . --output-file $(TEST_EXE).info
	genhtml $(TEST_EXE).info --output-directory lcovhtml


.PHONY: valgrind
valgrind: $(TEST_EXE)
	valgrind --leak-check=full ./$(TEST_EXE)

$(NAME).o:
	gcc -O3 -g $(USES_THREADS) -fPIC -c $(NAME).c -o $@ 

$(SHARED_LIB): $(NAME).o
	gcc -o $@ $^ -O3 -g -Wall $(LD_SHARED) $(SONAME) $(MPFR_INCL) $(MPFR_LIBS) -lm -lpthread
	

.PHONY: pybind11
pybind11: $(SHARED_LIB)
	g++ -O3 -Wall $(LD_SHARED) -std=c++11 -fPIC $(USES_THREADS) $(PYBIND11_INCL) $(MPFR_INCL) $(MPFR_LIBS) \
	$(MANDELBROT_LIBS) $(PYEMBED_LIBS) $(PYBIND_SRCS) -o $(PYMODULE)


clean_lib:
	-$(RM) $(NAME).o $(SHARED_LIB)

clean_test:
	-$(RM) $(TEST_EXE)
ifeq ($(OS),Darwin)
	-$(RM) -r $(TEST_EXE).dSYM
endif

clean_pybind11:
	-$(RM) $(PYMODULE)

clean_pymodule:
	-$(RM) -r build 
	-$(RM) $(PYMODULE)

clean_coverage:
	-$(RM) *.gcno *.gcda *.info
	-$(RM) -r lcovhtml

clean: clean_test clean_pymodule clean_pybind11 clean_coverage clean_lib
	

