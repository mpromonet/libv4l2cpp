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
#include "V4l2Capture.h"

// Constructor
V4l2Capture::V4l2Capture(const V4L2DeviceParameters& params) : m_params(params), m_fd(-1), m_bufferSize(0), m_format(0)
{
}

// Destructor
V4l2Capture::~V4l2Capture()
{
	if (m_fd !=-1) v4l2_close(m_fd);
}

// intialize the V4L2 connection
bool V4l2Capture::init(unsigned int mandatoryCapabilities)
{
	if (initdevice(m_params.m_devName.c_str(), mandatoryCapabilities) == -1)
	{
		LOG(ERROR) << "Cannot init device:" << m_params.m_devName;
	}
	return (m_fd!=-1);
}

// close the V4L2 connection
void V4l2Capture::close()
{
	if (m_fd != -1) v4l2_close(m_fd);
	m_fd = -1;
}

// intialize the V4L2 device
int V4l2Capture::initdevice(const char *dev_name, unsigned int mandatoryCapabilities)
{
	m_fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (m_fd < 0) 
	{
		LOG(ERROR) << "Cannot open device:" << m_params.m_devName << " " << strerror(errno);
		this->close();
		return -1;
	}
	if (checkCapabilities(m_fd,mandatoryCapabilities) !=0)
	{
		this->close();
		return -1;
	}	
	if (configureFormat(m_fd) !=0) 
	{
		this->close();
		return -1;
	}
	if (configureParam(m_fd) !=0)
	{
		this->close();
		return -1;
	}
	
	return m_fd;
}

// check needed V4L2 capabilities
int V4l2Capture::checkCapabilities(int fd, unsigned int mandatoryCapabilities)
{
	struct v4l2_capability cap;
	memset(&(cap), 0, sizeof(cap));
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) 
	{
		LOG(ERROR) << "Cannot get capabilities for device:" << m_params.m_devName << " " << strerror(errno);
		return -1;
	}
	LOG(NOTICE) << "driver:" << cap.driver << " " << std::hex << cap.capabilities;
	
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
	{
		LOG(ERROR) << "No capture support for device:" << m_params.m_devName << " " << strerror(errno);
		return -1;
	}
	
	if ((cap.capabilities & V4L2_CAP_READWRITE))    LOG(NOTICE) << m_params.m_devName << " support read/write";
	if ((cap.capabilities & V4L2_CAP_STREAMING))    LOG(NOTICE) << m_params.m_devName << " support streaming";
	if ((cap.capabilities & V4L2_CAP_TIMEPERFRAME)) LOG(NOTICE) << m_params.m_devName << " support timeperframe"; 
	
	if ( (cap.capabilities & mandatoryCapabilities) != mandatoryCapabilities )
	{
		LOG(ERROR) << "Mandatory capability not available for device:" << m_params.m_devName;
		return -1;
	}
	
	return 0;
}

// configure capture format 
int V4l2Capture::configureFormat(int fd)
{
	if (m_params.m_format!=0) 
	{
		struct v4l2_format   fmt;			
		memset(&(fmt), 0, sizeof(fmt));
		fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width       = m_params.m_width;
		fmt.fmt.pix.height      = m_params.m_height;
		fmt.fmt.pix.pixelformat = m_params.m_format;
		fmt.fmt.pix.field       = V4L2_FIELD_ANY;
		
		if (xioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
		{
			LOG(ERROR) << "Cannot set format for device:" << m_params.m_devName << " " << strerror(errno);
			return -1;
		}			
		if (fmt.fmt.pix.pixelformat != m_params.m_format) 
		{
			char formatArray[] = { (fmt.fmt.pix.pixelformat&0xff), ((fmt.fmt.pix.pixelformat>>8)&0xff), ((fmt.fmt.pix.pixelformat>>16)&0xff), ((fmt.fmt.pix.pixelformat>>24)&0xff), 0 };
			LOG(ERROR) << "Cannot set pixelformat to:" << formatArray;
			return -1;
		}
		if ((fmt.fmt.pix.width != m_params.m_width) || (fmt.fmt.pix.height != m_params.m_height))
		{
			LOG(WARN) << "Cannot set size width:" << fmt.fmt.pix.width << " height:" << fmt.fmt.pix.height;
		}
		
		m_format     = fmt.fmt.pix.pixelformat;
		m_width      = fmt.fmt.pix.width;
		m_height     = fmt.fmt.pix.height;		
		m_bufferSize = fmt.fmt.pix.sizeimage;
	}
	else
	{
		this->queryFormat();
	}
	
	char formatArray[] = { (m_format&0xff), ((m_format>>8)&0xff), ((m_format>>16)&0xff), ((m_format>>24)&0xff), 0 };
	LOG(NOTICE) << m_params.m_devName << ":" << formatArray << " size:" << m_params.m_width << "x" << m_params.m_height << " bufferSize:" << m_bufferSize;
	
	return 0;
}

// configure capture FPS 
int V4l2Capture::configureParam(int fd)
{
	struct v4l2_streamparm   param;			
	memset(&(param), 0, sizeof(param));
	param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	param.parm.capture.timeperframe.numerator = 1;
	param.parm.capture.timeperframe.denominator = m_params.m_fps;

	if (xioctl(fd, VIDIOC_S_PARM, &param) == -1)
	{
		LOG(WARN) << "Cannot set param for device:" << m_params.m_devName << " " << strerror(errno);
	}
	
	LOG(NOTICE) << "fps:" << param.parm.capture.timeperframe.numerator << "/" << param.parm.capture.timeperframe.denominator;
	LOG(NOTICE) << "nbBuffer:" << param.parm.capture.readbuffers;
	
	return 0;
}


// query current format
void V4l2Capture::queryFormat()
{
	struct v4l2_format     fmt;
	memset(&fmt,0,sizeof(fmt));
	fmt.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == ioctl(m_fd,VIDIOC_G_FMT,&fmt)) // don't understand why xioctl give a different result
	{
		m_format     = fmt.fmt.pix.pixelformat;
		m_width      = fmt.fmt.pix.width;
		m_height     = fmt.fmt.pix.height;
		m_bufferSize = fmt.fmt.pix.sizeimage;
	}
}

// ioctl encapsulation
int V4l2Capture::xioctl(int fd, int request, void *arg)
{
	int ret = -1;
	errno=0;
	do 
	{
		ret = v4l2_ioctl(fd, request, arg);
	} while ((ret == -1) && ((errno == EINTR) || (errno == EAGAIN)));

	return ret;
}
				

V4l2Output::V4l2Output(const V4L2DeviceParameters& params) : m_fd(-1)
{
	struct stat sb;		
	if ( (stat(params.m_devName.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR) ) 
	{
		// open & initialize a V4L2 output
		m_fd = open(params.m_devName.c_str(), O_WRONLY);
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

V4l2Output::~V4l2Output()
{
	if (m_fd !=-1) v4l2_close(m_fd);
}

