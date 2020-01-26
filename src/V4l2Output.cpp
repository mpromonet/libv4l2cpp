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

// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"

#include "V4l2MmapDevice.h"
#include "V4l2Output.h"
#include "V4l2ReadWriteDevice.h"

// -----------------------------------------
//    create video output interface
// -----------------------------------------
V4l2Output* V4l2Output::create(const V4L2DeviceParameters & param, IoType iotype)
{
	V4l2Output* videoOutput = NULL;
	V4l2Device* videoDevice = NULL; 
	int caps = V4L2_CAP_VIDEO_OUTPUT;
	switch (iotype)
	{
		case IOTYPE_MMAP: 
			videoDevice = new V4l2MmapDevice(param, V4L2_BUF_TYPE_VIDEO_OUTPUT); 
			caps |= V4L2_CAP_STREAMING;
		break;
		case IOTYPE_READWRITE:
			videoDevice = new V4l2ReadWriteDevice(param, V4L2_BUF_TYPE_VIDEO_OUTPUT); 
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
		videoOutput = new V4l2Output(videoDevice);
	}	
	return videoOutput;
}

// -----------------------------------------
//    constructor
// -----------------------------------------
V4l2Output::V4l2Output(V4l2Device* device) : V4l2Access(device)
{
}

// -----------------------------------------
//    destructor
// -----------------------------------------
V4l2Output::~V4l2Output() 
{
}

// -----------------------------------------
//    check writability
// -----------------------------------------
int V4l2Output::isWritable(timeval* tv)
{
	int fd = m_device->getFd();
	fd_set fdset;
	FD_ZERO(&fdset);	
	FD_SET(fd, &fdset);
	return select(fd+1, NULL, &fdset, NULL, tv);
}

// -----------------------------------------
//    write to V4l2Device
// -----------------------------------------
size_t V4l2Output::write(char* buffer, size_t bufferSize)
{
	return m_device->writeInternal(buffer, bufferSize);
}


bool V4l2Output::startPartialWrite(void)
{
	return m_device->startPartialWrite();
}

size_t V4l2Output::writePartial(char* buffer, size_t bufferSize)
{
	return m_device->writePartialInternal(buffer, bufferSize);
}

bool V4l2Output::endPartialWrite(void)
{
	return m_device->endPartialWrite();
}

