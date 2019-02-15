CXXFLAGS = -W -Wall -pthread -g -pipe $(EXTRA_CXXFLAGS)
CXXFLAGS += -I inc
RM = rm -rf
CXX ?= $(CROSS)g++
AR ?= $(CROSS)ar
PREFIX?=/usr

ifneq ($(wildcard $(SYSROOT)$(PREFIX)/include/log4cpp/Category.hh),)
CXXFLAGS += -DHAVE_LOG4CPP -I $(SYSROOT)$(PREFIX)/include
endif

V4L2WRAPPER_CPP:=$(wildcard src/*.cpp)
V4L2WRAPPER_OBJ:=$(V4L2WRAPPER_CPP:%.cpp=%.o)

.DEFAULT_GOAL := all

all: libv4l2wrapper.a

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

libv4l2wrapper.a: $(V4L2WRAPPER_OBJ)
	$(AR) rcs $@ $^


clean:
	-@$(RM) *.a $(V4L2WRAPPER_OBJ)

install:
	mkdir -p $(PREFIX)/include/libv4l2cpp/
	install -D -m 0755 inc/*.h $(PREFIX)/include/libv4l2cpp/
	install -D -m 0755 *.a $(PREFIX)/lib

