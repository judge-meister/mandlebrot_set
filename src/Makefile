
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

DEBUG=
USES_THREADS=

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

INCLUDES = ../include $(MPFR_INCL)
LOCAL_LIBS = 

CC = gcc
CCFLAGS = -g -O3 -Wall -fPIC 

CXXFLAGS = -g -O3 -Wall -fPIC 

%.o: %.c
	$(CC) $(CCFLAGS) $(DEBUG) $(USES_THREADS) -c -I../include $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) $(USES_THREADS) -c -I../include $< -o $@

# -----------------------------------------------------------------------------

all: clean test mandelbrot_main


.PHONY: test
test: $(TEST_EXE)
	./$(TEST_EXE)

testlib.a: test_$(NAME)_main.o
	ar -r testlib.a test_$(NAME)_main.o

$(TEST_EXE): testlib.a $(SHARED_LIB)
	$(CXX) $(DEBUG) $(USES_THREADS) testlib.a $(MANDELBROT_LIBS) $(MPFR_INCL) $(MPFR_LIBS) \
		$(LDFLAGS)  -Wl,-rpath=./  -o $(TEST_EXE)

test_$(NAME)_main.o: test_$(NAME)_main.c
	$(CC) $(CCFLAGS) $(DEBUG) $(USES_THREADS) -c -I../include $< -o $@

.PHONY: coverage
coverage: CXXFLAGS += --coverage -DUSES_THREADS
coverage: CCFLAGS += --coverage -DUSES_THREADS
coverage: LDFLAGS += --coverage
coverage: $(TEST_EXE)
	./$(TEST_EXE)
	lcov -c --directory . --output-file $(TEST_EXE).info
	genhtml $(TEST_EXE).info --output-directory lcovhtml

debug: DEBUG=-DTRACE
debug: $(TEST_EXE) mandelbrot_main

threaded: USES_THREADS=-DUSES_THREADS -DSTD_THREADS
threaded: DEBUG=-DTRACE
threaded: clean $(TEST_EXE) mandelbrot_main

.PHONY: valgrind
valgrind: $(TEST_EXE)
	valgrind --leak-check=full ./$(TEST_EXE)

$(SHARED_LIB): $(NAME)_mpfr.o
	$(CXX) -o $@ $^ $(DEBUG) $(LD_SHARED) $(SONAME) $(MPFR_INCL) $(MPFR_LIBS) -lgcov -lm -lpthread
	

.PHONY: pybind11
pybind11: $(SHARED_LIB)
	$(CXX) $(CXXFLAGS) $(LD_SHARED) -std=c++11 $(USES_THREADS) $(PYBIND11_INCL) $(MPFR_INCL) $(MPFR_LIBS) \
	$(MANDELBROT_LIBS) $(PYEMBED_LIBS) $(PYBIND_SRCS) -o $(PYMODULE)


# -----------------------------------------------------------------------------
OBJS = Shader.o Window.o MandelbrotAdapter.o mandelbrot_main.o glad.o
main.a: $(OBJS)
	ar -r main.a $^

mandelbrot_main: main.a $(SHARED_LIB)
	$(CXX) $(DEBUG) $(USES_THREADS) -o $@ main.a $(MANDELBROT_LIBS) $(MPFR_LIBS) \
	    -lglfw -lGL -lSOIL -lX11 -lpthread -lXrandr -lXi -ldl -Wl,-rpath=./

clean_mandelbrot_main:
	-$(RM) $(OBJS) mandelbrot_main main.a


# -----------------------------------------------------------------------------
clean_lib:
	-$(RM) $(NAME).o $(SHARED_LIB)

clean_test:
	-$(RM) $(TEST_EXE) testlib.a test_$(NAME)_main.o
ifeq ($(OS),Darwin)
	-$(RM) -r $(TEST_EXE).dSYM
endif

clean_pybind11:
	-$(RM) $(PYMODULE)

clean_coverage:
	-$(RM) *.gcno *.gcda *.info
	-$(RM) -r lcovhtml

clean: clean_test clean_pybind11 clean_coverage clean_lib clean_mandelbrot_main
	

