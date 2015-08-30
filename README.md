
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
 
To create a V4L2 Capture interface :
------------------------------------
{{{
	V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
	V4l2Capture* videoCapture = V4l2DeviceFactory::CreateVideoCapure(param, useMmap);
}}}


To create a V4L2 Output interface :
------------------------------------
{{{
        V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
        V4l2Output videoOutput(param);
}}}

