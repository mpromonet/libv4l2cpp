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
V4l2Output::V4l2Output(const V4L2DeviceParameters& params) : V4l2Device(params, V4L2_BUF_TYPE_VIDEO_OUTPUT)
{
	struct stat sb;		
	if ( (stat(params.m_devName.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR) ) 
	{
		// open & initialize a V4L2 output
		if (!this->init(V4L2_CAP_VIDEO_OUTPUT))
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

