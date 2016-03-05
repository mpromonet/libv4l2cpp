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
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <sys/ioctl.h>

// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"
#include "V4l2Output.h"

V4l2Output* V4l2Output::createNew(const V4L2DeviceParameters& params)
{
	V4l2Output* device = new V4l2Output(params);
	if (device && !device->init(V4L2_CAP_VIDEO_OUTPUT))
	{
		delete device;
		device=NULL;
	}
	return device;
}

// Constructor
V4l2Output::V4l2Output(const V4L2DeviceParameters& params) : V4l2Device(params, V4L2_BUF_TYPE_VIDEO_OUTPUT)
{
}

// write
size_t V4l2Output::write(char* buffer, size_t bufferSize)
{
	return ::write(m_fd, buffer,  bufferSize);
}

// Destructor
V4l2Output::~V4l2Output()
{
} 

