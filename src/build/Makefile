
# a single makefile to rule them all !


# -----------------------------------------------------------------------------
# common pkconfig
MPFR_LIBS=$(shell pkg-config --libs mpfr)
MPFR_INCL=$(shell pkg-config --cflags gmp mpfr)

# -----------------------------------------------------------------------------
# common variables
CXXFLAGS= -g -O3 -Wall -fPIC -std=c++17
CCFLAGS= -g -O3 -Wall -fPIC -std=c17
CC = gcc

LDFLAGS = -Wl,-rpath=./,-rpath-link=./ -Wl,--no-undefined
LD_SHARED = -shared $(LDFLAGS)

#DEBUG=-DTRACE
DEBUG=
USES_THREADS=-DUSES_THREADS


# -----------------------------------------------------------------------------
# top level rules
all :  libmandelbrot.so libglad.so mandelbrot_main mandelbrot_image pybind11
.PHONY : all

clean: 
	-$(RM) $(CLEAN_LIST)
	-$(RM) -r $(CLEAN_DIRS)



# -----------------------------------------------------------------------------
# libmandelbrot.so
LIB_MAND_SRCS = ../mandelbrot_mpfr.cpp
LIB_MAND_OBJS = $(patsubst %.cpp,%.o,$(notdir $(LIB_MAND_SRCS)))
#$(warning MAND_OBJS $(LIB_MAND_OBJS))
LIB_MAND_SO = libmandelbrot.so
MAND_LIBRARIES = $(MPFR_LIBS)
INCLUDES = -I.. $(MPFR_INCL)
CLEAN_LIST += $(LIB_MAND_SO) $(LIB_MAND_OBJS)

$(LIB_MAND_SO): $(LIB_MAND_OBJS)
	g++ $(LD_SHARED) $(LIB_MAND_OBJS) $(MAND_LIBRARIES) -o $@ 

# -----------------------------------------------------------------------------
# libglad.so
LIB_GLAD_SRCS = ../glad.c
LIB_GLAD_OBJS = $(patsubst %.c,%.o,$(notdir $(LIB_GLAD_SRCS)))
LIB_GLAD_SO = libglad.so
INCLUDES += -I../include
CLEAN_LIST += $(LIB_GLAD_SO) $(LIB_GLAD_OBJS)

$(LIB_GLAD_SO): $(LIB_GLAD_OBJS)
	g++ $(LD_SHARED) $(LIB_GLAD_OBJS) -o $@

# -----------------------------------------------------------------------------
# pybind11
PYBIND11_INCL=$(shell python3 -m pybind11 --includes)
PYEMBED_LIBS=$(shell pkg-config --libs python3-embed)
PYEMBED_SUFFIX=$(shell python3-config --extension-suffix)
PYBIND_SRCS=../mandelbrot_pybind11.cpp
PYMODULE=mandelbrot$(PYEMBED_SUFFIX)
CLEAN_LIST += $(LIB_MAND_SO) mandelbrot$(PYEMBED_SUFFIX)

.PHONY: pybind11
pybind11: $(LIB_MAND_SO)
	$(CXX) $(CXXFLAGS) $(LD_SHARED) $(USES_THREADS) $(PYBIND11_INCL) $(MPFR_INCL) $(MPFR_LIBS) \
	$(LIB_MAND_SO) $(PYEMBED_LIBS) $(PYBIND_SRCS) -o $(PYMODULE)

# -----------------------------------------------------------------------------
# prog
PROG_SRCS = ../MandelbrotAdapter.cpp \
            ../MandelbrotWindow.cpp \
			../MandelbrotOpenGL.cpp \
            ../Shader.cpp \
            ../Window.cpp \
			../Texture.cpp \
			../ImageFile.cpp \
			../ImageData.cpp \
			../CmdOptions.cpp \
			../KeyboardMouseHandler.cpp \
            ../mandelbrot_main.cpp

PROG_OBJS = $(patsubst %.cpp,%.o,$(notdir $(PROG_SRCS)))
PROG_NAME = mandelbrot_main
PROG_LIBS = -L. -lmandelbrot -lglad -lglfw -lGL -lm -lpng
CLEAN_LIST += $(PROG_NAME) $(PROG_OBJS)

$(PROG_NAME): $(PROG_OBJS) $(LIB_GLAD_SO) $(LIB_MAND_SO)
	g++ $(LDFLAGS) $(PROG_OBJS) $(PROG_LIBS) -o $@ 

# -----------------------------------------------------------------------------
IMAGE_SRCS = ../mandelbrot_image.cpp \
             ../MandelbrotAdapter.cpp \
			 ../CmdOptions.cpp \
			 ../ImageFile.cpp \
			 ../ImageData.cpp

IMAGE_OBJS = $(patsubst %.cpp,%.o,$(notdir $(IMAGE_SRCS)))
IMAGE_NAME = mandelbrot_image
IMAGE_LIBS = -L. -lmandelbrot -lm -lpng
CLEAN_LIST += $(IMAGE_NAME) $(IMAGE_OBJS)

$(IMAGE_NAME): $(IMAGE_OBJS) $(LIB_MAND_SO)
	g++ $(LDFLAGS) $(IMAGE_OBJS) $(IMAGE_LIBS) -o $@ 

# -----------------------------------------------------------------------------
# mandelbrot test
TEST_SRCS = ../test/Mandelbrot_Test.cpp
TEST_OBJS = $(patsubst %.cpp,%.o,$(notdir $(TEST_SRCS)))
TEST_NAME = Mandelbrot_Test
TEST_LIBS = -L. -lmandelbrot 
CLEAN_LIST += $(TEST_NAME) $(TEST_OBJS)

$(TEST_NAME): $(TEST_OBJS) $(LIB_MAND_SO)
	g++ $(LDFLAGS) $(TEST_OBJS) $(TEST_LIBS) -o $@

# -----------------------------------------------------------------------------
# shader test
SHADER_TEST_SRCS = ../Shader.cpp ../test/Shader_Test.cpp
SHADER_TEST_OBJS = $(patsubst %.cpp,%.o,$(notdir $(SHADER_TEST_SRCS)))
SHADER_TEST_NAME = Shader_Test
SHADER_TEST_LIBS = -L. -lglad -lglfw
CLEAN_LIST += $(SHADER_TEST_NAME) $(SHADER_TEST_OBJS)

$(SHADER_TEST_NAME): $(SHADER_TEST_OBJS) $(LIB_GLAD_SO)
	g++ $(LDFLAGS) $(SHADER_TEST_OBJS) $(SHADER_TEST_LIBS) -o $@

# -----------------------------------------------------------------------------
# window test
WINDOW_TEST_SRCS = ../Window.cpp ../test/Window_Test.cpp
WINDOW_TEST_OBJS = $(patsubst %.cpp,%.o,$(notdir $(WINDOW_TEST_SRCS)))
WINDOW_TEST_NAME = Window_Test
WINDOW_TEST_LIBS = -L. -lglad -lglfw
INCLUDES += -I../test
CLEAN_LIST += $(WINDOW_TEST_NAME) $(WINDOW_TEST_OBJS)

$(WINDOW_TEST_NAME): $(WINDOW_TEST_OBJS) $(LIB_GLAD_SO)
	g++ $(LDFLAGS) $(WINDOW_TEST_OBJS) $(WINDOW_TEST_LIBS) -o $@

# -----------------------------------------------------------------------------
# coverage
.PHONY: coverage
CLEAN_LIST += *.gcda *.gcno $(TEST_NAME).info
CLEAN_DIRS += lcovhtml
coverage: CXXFLAGS += --coverage -DUSES_THREADS
coverage: CCFLAGS += --coverage -DUSES_THREADS
coverage: LDFLAGS += --coverage
coverage: LD_SHARED += -lgcov
coverage: clean $(TEST_NAME) $(SHADER_TEST_NAME) $(WINDOW_TEST_NAME)
	./$(TEST_NAME)
	./$(SHADER_TEST_NAME)
	./$(WINDOW_TEST_NAME)
	lcov -c --directory . --output-file $(TEST_NAME).info
	genhtml $(TEST_NAME).info --output-directory lcovhtml

# -----------------------------------------------------------------------------
# valgrind
.PHONY: valgrind
valgrind: clean $(TEST_NAME)
	valgrind --leak-check=full ./$(TEST_NAME)



# -----------------------------------------------------------------------------
# src to obj rules
%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) $(USES_THREADS) -c $(INCLUDES) $< -o $@

%.o: ../test/%.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) $(USES_THREADS) -c $(INCLUDES) $< -o $@

%.o: ../%.c
	$(CC) $(CCFLAGS) $(DEBUG) $(USES_THREADS) -c $(INCLUDES) $< -o $@


# -----------------------------------------------------------------------------

#	-$(RM) $(LIB_OBJS) $(PROG_OBJS) $(PROG) $(LIB_SO) 
#$(LIB_SO): $(LIB_OBJS)
#	g++ $(LD_SHARED) $(LIB_OBJS) $(LIBS) -o $@ 

#$(PROG): $(LIB_SO) $(PROG_OBJS)
#	g++ $(PROG_LIBS) $(PROG_OBJS) -o $@ 

        
#LIBS= -lfreetype -lGL -lGLEW -lglfw $(MPFR_LIBS)

#PROG_LIBS= -L. -lsprocket -lGL -Wl,--no-undefined -lm -lpthread \
           -Wl,-rpath=./,-rpath-link=./ 

#INCLUDES = $(addprefix -I,$(shell find .. -type d)) \
           -I/usr/include/freetype2 $(MPFR_INCL)
#$(warning INC $(INCLUDES))


#LIB_SRC_DIRS = ../

#LIB_SRCS = $(shell find $(LIB_SRC_DIRS) -name "*.cpp")
#LIB_SRCS = ../mandelbrot_mpfr.cpp
#$(warning SRCS = $(LIB_SRCS))

#LIB_OBJS = $(patsubst %.cpp,%.o,$(notdir $(LIB_SRCS)))
#$(warning OBJS = $(LIB_OBJS))

#PROG_SRC_DIR = ../examples
#PROG_SRCS = #$(shell find $(PROG_SRC_DIR) -name "*.cpp")
#PROG_OBJS = $(patsubst %.cpp,%.o,$(notdir $(PROG_SRCS)))

#PROG = WindowsExample
#LIB_SO = libmandelbrot.so

# -----------------------------------------------------------------------------
#%.o: ../%.c
#	$(CC) $(CCFLAGS) $(DEBUG)  -c $(INCLUDES) $< -o $@

#%.o: ../%.cpp
#	$(CXX) $(CXXFLAGS) $(DEBUG)  -c $(INCLUDES) $< -o $@

#%.o: ../*/%.cpp
#	$(CXX) $(CXXFLAGS) $(DEBUG)  -c $(INCLUDES) $< -o $@

#%.o: ../*/*/%.cpp
#	$(CXX) $(CXXFLAGS) $(DEBUG)  -c $(INCLUDES) $< -o $@

