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
#include <libv4l2.h>

// project
#include "logger.h"
#include "V4l2Output.h"

// Constructor
V4l2Output::V4l2Output(const V4L2DeviceParameters& params) : V4l2Device(params)
{
	struct stat sb;		
	if ( (stat(params.m_devName.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR) ) 
	{
		// open & initialize a V4L2 output
		m_fd = open(params.m_devName.c_str(), O_RDWR | O_NONBLOCK);
		if (m_fd != -1)
		{
			struct v4l2_capability cap;
			memset(&(cap), 0, sizeof(cap));
			if (0 == ioctl(m_fd, VIDIOC_QUERYCAP, &cap)) 
			{			
				LOG(NOTICE) << "Output device name:" << cap.driver << " cap:" <<  std::hex << cap.capabilities;
				if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) 
				{				
					struct v4l2_format   fmt;			
					memset(&(fmt), 0, sizeof(fmt));
					fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
					fmt.fmt.pix.width  = params.m_width;
					fmt.fmt.pix.height = params.m_height;
					fmt.fmt.pix.pixelformat = params.m_format;
					if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) == -1)
					{
						LOG(ERROR) << "Cannot set output format "<< strerror(errno);
					}		
				}			
			}
		}
		else
		{
			LOG(ERROR) << "Cannot open " << params.m_devName << " " << strerror(errno);
		}
	}
	else
	{
		// open a normal file
		m_fd = open(params.m_devName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	}
	
	if (m_fd == -1)		
	{
		LOG(NOTICE) << "Error openning " << params.m_devName << " " << strerror(errno);
	}
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

