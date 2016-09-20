/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2WriteOutput.cpp
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/

#include <string.h>

// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"
#include "V4l2WriteOutput.h"

V4l2WriteOutput* V4l2WriteOutput::createNew(const V4L2DeviceParameters& params)
{
	V4l2WriteOutput* device = new V4l2WriteOutput(params);
	if (device && !device->init(V4L2_CAP_VIDEO_OUTPUT|V4L2_CAP_READWRITE))
	{
		delete device;
		device=NULL;
	}
	return device;
}

// Constructor
V4l2WriteOutput::V4l2WriteOutput(const V4L2DeviceParameters& params) : V4l2Device(params,m_deviceType), V4l2Output(params)
{
}

// write
size_t V4l2WriteOutput::write(char* buffer, size_t bufferSize)
{
	return ::write(m_fd, buffer,  bufferSize);
}

