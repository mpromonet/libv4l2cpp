CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
CFLAGS += -I inc
RM = rm -rf
CC = $(CROSS)gcc
AR = $(CROSS)ar
PREFIX?=/usr

ifneq ($(wildcard $(SYSROOT)$(PREFIX)/include/log4cpp/Category.hh),)
CFLAGS += -DHAVE_LOG4CPP -I $(SYSROOT)$(PREFIX)/include
endif

V4L2WRAPPER_CPP:=$(wildcard src/*.cpp)
V4L2WRAPPER_OBJ:=$(V4L2WRAPPER_CPP:%.cpp=%.o)

.DEFAULT_GOAL := all

all: libv4l2wrapper.a

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

libv4l2wrapper.a: $(V4L2WRAPPER_OBJ)
	$(AR) rcs $@ $^


clean:
	-@$(RM) *.a $(V4L2WRAPPER_OBJ)

install:
	mkdir $(PREFIX)/include/libv4l2cpp/
	install -D -m 0755 inc/*.h $(PREFIX)/include/libv4l2cpp/
	install -D -m 0755 *.a $(PREFIX)/lib

