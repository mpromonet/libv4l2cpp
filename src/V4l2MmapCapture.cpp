/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapCapture.cpp
** 
** V4L2 source using mmap API
**
** -------------------------------------------------------------------------*/

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

// libv4l2
#include <linux/videodev2.h>

// project
#include "logger.h"
#include "V4l2MmapCapture.h"

V4l2MmapCapture* V4l2MmapCapture::createNew(V4L2DeviceParameters params) 
{ 
	V4l2MmapCapture* device = new V4l2MmapCapture(params); 
	if (device && !device->init(V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_STREAMING))
	{
		delete device;
		device=NULL;
	}
	if (device)
	{
		device->captureStart();
	}	
	return device;
}

V4l2MmapCapture::V4l2MmapCapture(V4L2DeviceParameters params) : V4l2Device(params, V4L2_BUF_TYPE_VIDEO_CAPTURE), V4l2MmapDevice(params, V4L2_BUF_TYPE_VIDEO_CAPTURE), V4l2Capture(params)
{
}

size_t V4l2MmapCapture::read(char* buffer, size_t bufferSize)
{
	size_t size = 0;
	if (n_buffers > 0)
	{
		struct v4l2_buffer buf;	
		memset (&buf, 0, sizeof(buf));
		buf.type = m_deviceType;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(m_fd, VIDIOC_DQBUF, &buf)) 
		{
			perror("VIDIOC_DQBUF");
			size = -1;
		}
		else if (buf.index < n_buffers)
		{
			size = buf.bytesused;
			if (size > bufferSize)
			{
				size = bufferSize;
				LOG(WARN) << "Device " << m_params.m_devName << " buffer truncated available:" << bufferSize << " needed:" << buf.bytesused;
			}
			memcpy(buffer, m_buffer[buf.index].start, size);

			if (-1 == xioctl(m_fd, VIDIOC_QBUF, &buf))
			{
				perror("VIDIOC_QBUF");
				size = -1;
			}
		}
	}
	return size;
}

