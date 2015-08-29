
v4l2wrapper
====================

It is C++ wrapper for V4L2

It is based on :
- libv4l-dev 

License
------------
Domain public 

Dependencies
------------
 - libv4l-dev
 - liblog4cpp5-dev
 - libvpx-dev      (for v4l2compress_vp8)
 - libx264-dev     (for v4l2compress_h264)
 
Samples
-------
 - v4l2copy          : 

>	read from a V4L2 capture device and write to a V4L2 output device

 - v4l2compress_vp8  : 

>	read YUYV from a V4L2 capture device, compress in VP8 format using libvpx and write to a V4L2 output device
 - v4l2compress_h264 : 

>	read YUYV from a V4L2 capture device, compress in H264 format using libx264 and write to a V4L2 output device

 - v4l2grab_h264     : 

>	grab raspberry pi screen, compress in H264 format using OMX and write to a V4L2 output device

 - v4l2diplay_h264     : 

>	read H264 from V4L2 capture device, uncompress and display using OMX
