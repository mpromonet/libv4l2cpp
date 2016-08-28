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
#include <sys/ioctl.h>
#include <string.h>

// libv4l2
#include <linux/videodev2.h>

#include "logger.h"

#include "V4l2Device.h"
#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"


// -----------------------------------------
//    V4L2Device
// -----------------------------------------
V4l2Device::V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType) : m_params(params), m_fd(-1), m_deviceType(deviceType), m_bufferSize(0), m_format(0)
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

// query current format
void V4l2Device::queryFormat()
{
	struct v4l2_format     fmt;
	memset(&fmt,0,sizeof(fmt));
	fmt.type  = m_deviceType;
	if (0 == ioctl(m_fd,VIDIOC_G_FMT,&fmt)) 
	{
		m_format     = fmt.fmt.pix.pixelformat;
		m_width      = fmt.fmt.pix.width;
		m_height     = fmt.fmt.pix.height;
		m_bufferSize = fmt.fmt.pix.sizeimage;
	}
}

// intialize the V4L2 connection
bool V4l2Device::init(unsigned int mandatoryCapabilities)
{
        struct stat sb;
        if ( (stat(m_params.m_devName.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR) )
        {
		if (initdevice(m_params.m_devName.c_str(), mandatoryCapabilities) == -1)
		{
			LOG(ERROR) << "Cannot init device:" << m_params.m_devName;
		}
	}
	else
	{
                // open a normal file
                m_fd = open(m_params.m_devName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
	}
	return (m_fd!=-1);
}

// intialize the V4L2 device
int V4l2Device::initdevice(const char *dev_name, unsigned int mandatoryCapabilities)
{
	m_fd = open(dev_name,  O_RDWR | O_NONBLOCK);
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
int V4l2Device::checkCapabilities(int fd, unsigned int mandatoryCapabilities)
{
	struct v4l2_capability cap;
	memset(&(cap), 0, sizeof(cap));
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) 
	{
		LOG(ERROR) << "Cannot get capabilities for device:" << m_params.m_devName << " " << strerror(errno);
		return -1;
	}
	LOG(NOTICE) << "driver:" << cap.driver << " " << std::hex << cap.capabilities;
		
	if ((cap.capabilities & V4L2_CAP_READWRITE))    LOG(NOTICE) << m_params.m_devName << " support read/write";
	if ((cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) LOG(NOTICE) << m_params.m_devName << " support output";
	if ((cap.capabilities & V4L2_CAP_STREAMING))    LOG(NOTICE) << m_params.m_devName << " support streaming";
	if ((cap.capabilities & V4L2_CAP_TIMEPERFRAME)) LOG(NOTICE) << m_params.m_devName << " support timeperframe"; 
	
	if ( (cap.capabilities & mandatoryCapabilities) != mandatoryCapabilities )
	{
		LOG(ERROR) << "Mandatory capability not available for device:" << m_params.m_devName;
		return -1;
	}
	
	return 0;
}

std::string fourcc(unsigned int format)
{
	char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff), (char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
	return std::string(formatArray, strlen(formatArray));
}

// configure capture format 
int V4l2Device::configureFormat(int fd)
{
	
	if (m_params.m_format==0) 
	{
		this->queryFormat();
		m_params.m_format = m_format;
		m_params.m_width  = m_width;
		m_params.m_height = m_height;
	}		
		
	struct v4l2_format   fmt;			
	memset(&(fmt), 0, sizeof(fmt));
	fmt.type                = m_deviceType;
	fmt.fmt.pix.width       = m_params.m_width;
	fmt.fmt.pix.height      = m_params.m_height;
	fmt.fmt.pix.pixelformat = m_params.m_format;
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;
	
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
	{
		LOG(ERROR) << "Cannot set format for device:" << m_params.m_devName << " " << strerror(errno);
		return -1;
	}			
	if (fmt.fmt.pix.pixelformat != m_params.m_format) 
	{
		LOG(ERROR) << "Cannot set pixelformat to:" << fourcc(m_params.m_format) << " format is:" << fourcc(fmt.fmt.pix.pixelformat);
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
	
	LOG(NOTICE) << m_params.m_devName << ":" << fourcc(m_format) << " size:" << m_params.m_width << "x" << m_params.m_height << " bufferSize:" << m_bufferSize;
	
	return 0;
}

// configure capture FPS 
int V4l2Device::configureParam(int fd)
{
	if (m_params.m_fps!=0)
	{
		struct v4l2_streamparm   param;			
		memset(&(param), 0, sizeof(param));
		param.type = m_deviceType;
		param.parm.capture.timeperframe.numerator = 1;
		param.parm.capture.timeperframe.denominator = m_params.m_fps;

		if (ioctl(fd, VIDIOC_S_PARM, &param) == -1)
		{
			LOG(WARN) << "Cannot set param for device:" << m_params.m_devName << " " << strerror(errno);
		}
	
		LOG(NOTICE) << "fps:" << param.parm.capture.timeperframe.numerator << "/" << param.parm.capture.timeperframe.denominator;
		LOG(NOTICE) << "nbBuffer:" << param.parm.capture.readbuffers;
	}
	
	return 0;
}

int V4l2Device::isReadable(timeval* tv)
{
	fd_set fdset;
	FD_ZERO(&fdset);	
	FD_SET(m_fd, &fdset);
	return select(m_fd+1, &fdset, NULL, NULL, tv);
}

int V4l2Device::isWritable(timeval* tv)
{
	fd_set fdset;
	FD_ZERO(&fdset);	
	FD_SET(m_fd, &fdset);
	return select(m_fd+1, NULL, &fdset, NULL, tv);
}


// ----------------------------------------- 
//    create video capture interface
// -----------------------------------------
V4l2Capture* V4l2DeviceFactory::CreateVideoCapture(const V4L2DeviceParameters & param, IoType iotype)
{
	V4l2Capture* videoCapture = NULL;
	switch (iotype)
	{
		case IOTYPE_MMAP: videoCapture = V4l2MmapCapture::createNew(param); break;
		default:          videoCapture = V4l2ReadCapture::createNew(param); break;
	}
	return videoCapture;
}

#include "V4l2Output.h"
#include "V4l2MmapOutput.h"
// -----------------------------------------
//    create video output interface
// -----------------------------------------
V4l2Output* V4l2DeviceFactory::CreateVideoOutput(const V4L2DeviceParameters & param, IoType iotype)
{
	V4l2Output* videoOutput = NULL;
	switch (iotype)
	{
		case IOTYPE_MMAP: videoOutput = V4l2MmapOutput::createNew(param); break;
		default:          videoOutput = V4l2Output::createNew(param);     break;
	}	
	return videoOutput;	
}
