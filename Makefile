CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2copy
PREFIX=/usr

CC = g++
# log4cpp
LDFLAGS += -llog4cpp 
#
CFLAGS += -g -fpermissive
CFLAGS += -I inc
LDFLAGS += -lv4l2 

all: $(ALL_PROGS)

v4l2copy: src/v4l2copy.cpp src/V4l2Capture.cpp src/V4l2MmapCapture.cpp src/V4l2ReadCapture.cpp
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	-@$(RM) $(ALL_PROGS) .*o 
