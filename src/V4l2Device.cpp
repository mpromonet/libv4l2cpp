/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Device.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <errno.h>

// libv4l2
#include <linux/videodev2.h>
#include <libv4l2.h>

#include "V4l2Device.h"
#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"


// -----------------------------------------
//    V4L2Device
// -----------------------------------------
V4l2Device::V4l2Device(const V4L2DeviceParameters&  params) : m_params(params), m_fd(-1)
{
}

V4l2Device::~V4l2Device() 
{
	this->close();
}

void V4l2Device::close() 
{
	if (m_fd != -1) 		
		::close(m_fd);
	
	m_fd = -1;
}

// ioctl encapsulation
int V4l2Device::xioctl(int fd, int request, void *arg)
{
	int ret = -1;
	errno=0;
	do 
	{
		ret = v4l2_ioctl(fd, request, arg);
	} while ((ret == -1) && ((errno == EINTR) || (errno == EAGAIN)));

	return ret;
}

// ----------------------------------------- 
//    create video capture interface
// -----------------------------------------
V4l2Capture* V4l2DeviceFactory::CreateVideoCapure(const V4L2DeviceParameters & param, bool useMmap)
{
	V4l2Capture* videoCapture = NULL;
	if (useMmap)
	{
		videoCapture = V4l2MmapCapture::createNew(param);
	}
	else
	{
		videoCapture = V4l2ReadCapture::createNew(param);
	}
	return videoCapture;
}

#include "V4l2Output.h"
#include "V4l2MmapOutput.h"
// -----------------------------------------
//    create video output interface
// -----------------------------------------
V4l2Output* V4l2DeviceFactory::CreateVideoOutput(const V4L2DeviceParameters & param, bool useMmap)
{
	V4l2Output* videoOutput = NULL;
	if (useMmap)
	{
		videoOutput = V4l2MmapOutput::createNew(param);
	}
	else
	{
		videoOutput = new V4l2Output(param);
	}
	return videoOutput;	
}
