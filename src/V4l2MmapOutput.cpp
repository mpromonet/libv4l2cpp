/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapOutput.cpp
** 
** V4L2 output using mmap API
**
** -------------------------------------------------------------------------*/

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

// libv4l2
#include <linux/videodev2.h>
#include <libv4l2.h>

// project
#include "logger.h"
#include "V4l2MmapOutput.h"

V4l2MmapOutput* V4l2MmapOutput::createNew(V4L2DeviceParameters params) 
{ 
	V4l2MmapOutput* device = new V4l2MmapOutput(params); 
		
/*	if (device &&  !device->init(V4L2_CAP_STREAMING))
	{
		delete device;
		device=NULL; 
	}*/
	if (device)
	{
		device->captureStart();
	}
	return device;
}

V4l2MmapOutput::V4l2MmapOutput(V4L2DeviceParameters params) : V4l2Device(params), V4l2MmapDevice(params, V4L2_BUF_TYPE_VIDEO_OUTPUT), V4l2Output(params) 
{
}


size_t V4l2MmapOutput::write(char* buffer, size_t bufferSize)
{
	size_t size = 0;
	if (n_buffers > 0)
	{
		struct v4l2_buffer buf;	
		memset (&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(m_fd, VIDIOC_DQBUF, &buf)) 
		{
			perror("VIDIOC_DQBUF");
			size = -1;
		}
		else if (buf.index < n_buffers)
		{
			size = bufferSize;
			if (size > buf.length)
			{
				size = buf.length;
				LOG(WARN) << "Device " << m_params.m_devName << " buffer truncated available:" << buf.length << " needed:" << size;
			}
			memcpy(m_buffer[buf.index].start, buffer, size);
			buf.bytesused = size;

			if (-1 == xioctl(m_fd, VIDIOC_QBUF, &buf))
			{
				perror("VIDIOC_QBUF");
				size = -1;
			}
		}
	}
	return size;
}

