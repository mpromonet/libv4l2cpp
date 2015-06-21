CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
RM = rm -rf
ALL_PROGS = v4l2copy v4l2compress_vp8 v4l2compress_h264
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

libyuv/source/*.cc:
	git submodule init libyuv
	git submodule update libyuv

v4l2compress_vp8: src/v4l2compress_vp8.cpp libyuv/source/*.cc src/V4l2Capture.cpp src/V4l2MmapCapture.cpp src/V4l2ReadCapture.cpp
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lvpx -I libyuv/include

v4l2compress_h264: src/v4l2compress_h264.cpp libyuv/source/*.cc src/V4l2Capture.cpp src/V4l2MmapCapture.cpp src/V4l2ReadCapture.cpp 
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lx264 -I libyuv/include

clean:
	-@$(RM) $(ALL_PROGS) .*o 
