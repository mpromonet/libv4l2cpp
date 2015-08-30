CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
CFLAGS += -I inc
RM = rm -rf
CC = gcc
AR = ar

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
