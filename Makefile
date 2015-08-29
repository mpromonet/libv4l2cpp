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

.DEFAULT_GOAL := all

# raspberry grab -> compress H264 -> write V4L2 output
ILCLIENTDIR=/opt/vc/src/hello_pi/libs/ilclient
ifneq ($(wildcard $(ILCLIENTDIR)),)
CFLAGS  +=-I /opt/vc/include/ -I /opt/vc/include/interface/vcos/ -I /opt/vc/include/interface/vcos/pthreads/ -I /opt/vc/include/interface/vmcs_host/linux/ -I $(ILCLIENTDIR)
LDFLAGS +=-L /opt/vc/lib -L $(ILCLIENTDIR) -lpthread -lopenmaxil -lbcm_host -lvcos -lvchiq_arm

v4l2grab_h264: src/v4l2grab_h264.cpp src/V4l2Output.cpp $(ILCLIENTDIR)/libilclient.a
	$(CC) -o $@ $^ -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi $(CFLAGS) $(LDFLAGS) 

v4l2display_h264: src/v4l2display_h264.cpp $(V4L2WRAPPER) $(ILCLIENTDIR)/libilclient.a
	$(CC) -o $@ $^ -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi $(CFLAGS) $(LDFLAGS) 

$(ILCLIENTDIR)/libilclient.a:
	make -C $(ILCLIENTDIR)
	
ALL_PROGS+=v4l2grab_h264
ALL_PROGS+=v4l2display_h264
endif

all: $(ALL_PROGS)

libyuv/source/*.cc:
	git submodule init libyuv
	git submodule update libyuv

# read V4L2 capture -> write V4L2 output
v4l2copy: src/v4l2copy.cpp $(V4L2WRAPPER)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

# read V4L2 capture -> compress using libvpx -> write V4L2 output
v4l2compress_vp8: src/v4l2compress_vp8.cpp libyuv/source/*.cc $(V4L2WRAPPER)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lvpx -I libyuv/include

# read V4L2 capture -> compress using x264 -> write V4L2 output
v4l2compress_h264: src/v4l2compress_h264.cpp libyuv/source/*.cc $(V4L2WRAPPER) 
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) -lx264 -I libyuv/include

clean:
	-@$(RM) $(ALL_PROGS) .*o 
