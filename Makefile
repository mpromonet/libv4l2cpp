ALL_PROGS = v4l2copy v4l2compress_vp8 v4l2compress_h264
CFLAGS = -W -Wall -pthread -g -pipe $(CFLAGS_EXTRA)
CFLAGS += -I inc
RM = rm -rf
CC = g++

# log4cpp
LDFLAGS += -llog4cpp 
# v4l2
LDFLAGS += -lv4l2 

V4L2WRAPPER=src/V4l2Device.cpp src/V4l2Output.cpp src/V4l2Capture.cpp src/V4l2MmapCapture.cpp src/V4l2ReadCapture.cpp

all: $(ALL_PROGS)

v4l2copy: src/v4l2copy.cpp $(V4L2WRAPPER)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

libyuv/source/*.cc:
	git submodule init libyuv
	git submodule update libyuv

v4l2compress_vp8: src/v4l2compress_vp8.cpp libyuv/source/*.cc $(V4L2WRAPPER)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lvpx -I libyuv/include

v4l2compress_h264: src/v4l2compress_h264.cpp libyuv/source/*.cc $(V4L2WRAPPER) 
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lx264 -I libyuv/include

clean:
	-@$(RM) $(ALL_PROGS) .*o 
