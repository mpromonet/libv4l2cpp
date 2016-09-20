/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Capture.cpp
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/


// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"
#include "V4l2Capture.h"

// Constructor
V4l2Capture::V4l2Capture(const V4L2DeviceParameters& params) : V4l2Device(params, V4L2_BUF_TYPE_VIDEO_CAPTURE)
{
}



				
