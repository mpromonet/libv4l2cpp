/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Output.cpp
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/

#include <string.h>

// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"
#include "V4l2Output.h"

// Constructor
V4l2Output::V4l2Output(const V4L2DeviceParameters& params) : V4l2Device(params, V4L2_BUF_TYPE_VIDEO_OUTPUT)
{
}

int V4l2Output::isWritable(timeval* tv)
{
	fd_set fdset;
	FD_ZERO(&fdset);	
	FD_SET(m_fd, &fdset);
	return select(m_fd+1, NULL, &fdset, NULL, tv);
}
