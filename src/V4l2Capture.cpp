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
#include "V4l2MmapDevice.h"
#include "V4l2ReadWriteDevice.h"


// -----------------------------------------
//    create video capture interface
// -----------------------------------------
V4l2Capture* V4l2Capture::create(const V4L2DeviceParameters & param)
{
	V4l2Capture* videoCapture = NULL;
	V4l2Device* videoDevice = NULL; 
	int caps = V4L2_CAP_VIDEO_CAPTURE;
	switch (param.m_iotype)
	{
		case IOTYPE_MMAP: 
			videoDevice = new V4l2MmapDevice(param, V4L2_BUF_TYPE_VIDEO_CAPTURE); 
			caps |= V4L2_CAP_STREAMING;
		break;
		case IOTYPE_READWRITE:
			videoDevice = new V4l2ReadWriteDevice(param, V4L2_BUF_TYPE_VIDEO_CAPTURE); 
			caps |= V4L2_CAP_READWRITE;
		break;
	}
	
	if (videoDevice &&  !videoDevice->init(caps))
	{
		delete videoDevice;
		videoDevice=NULL; 
	}
	
	if (videoDevice)
	{
		videoCapture = new V4l2Capture(videoDevice);
	}	
	return videoCapture;
}

// -----------------------------------------
//    constructor
// -----------------------------------------
V4l2Capture::V4l2Capture(V4l2Device* device) : V4l2Access(device)
{
}

// -----------------------------------------
//    destructor
// -----------------------------------------
V4l2Capture::~V4l2Capture() 
{
}

// -----------------------------------------
//    check readability
// -----------------------------------------
bool V4l2Capture::isReadable(timeval* tv)
{
	return poll(tv, V4l2Access::READ);
}

// -----------------------------------------
//    read from V4l2Device
// -----------------------------------------
size_t V4l2Capture::read(char* buffer, size_t bufferSize)
{
	return m_device->readInternal(buffer, bufferSize);
}

				
