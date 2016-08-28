/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Device.h
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_DEVICE
#define V4L2_DEVICE

#include <string>
#include <linux/videodev2.h>

// ---------------------------------
// V4L2 Device parameters
// ---------------------------------
struct V4L2DeviceParameters 
{
	V4L2DeviceParameters(const char* devname, unsigned int format, unsigned int width, unsigned int height, int fps, int verbose) : 
		m_devName(devname), m_format(format), m_width(width), m_height(height), m_fps(fps), m_verbose(verbose) {};
		
	std::string m_devName;
	unsigned int m_format;
	unsigned int m_width;
	unsigned int m_height;
	int m_fps;			
	int m_verbose;
};

// ---------------------------------
// V4L2 Device
// ---------------------------------
class V4l2Device
{		
	protected:
		V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType);
	
		int xioctl(int fd, int request, void *arg); 	
		int xioctlSelect(int fd, int request, void *arg);
		bool init(unsigned int mandatoryCapabilities);
		void close();	
	
		int initdevice(const char *dev_name, unsigned int mandatoryCapabilities);
		int checkCapabilities(int fd, unsigned int mandatoryCapabilities);
		int configureFormat(int fd);
		int configureParam(int fd);		
	
	public:
		virtual ~V4l2Device();
		int getBufferSize() { return m_bufferSize; };
		int getFormat() { return m_format; } ;
		int getWidth() { return m_width; };
		int getHeight() { return m_height; };
		void queryFormat();	
		int getFd() { return m_fd; };		

	protected:
		V4L2DeviceParameters m_params;
		int m_fd;
		v4l2_buf_type m_deviceType;	
	
		int m_bufferSize;
		int m_format;
		int m_width;
		int m_height;	
};

// ---------------------------------
// V4L2 Device factory
// ---------------------------------
class V4l2Capture;
class V4l2Output;
class V4l2DeviceFactory
{
	public:
		enum IoType
		{
			IOTYPE_READ,
			IOTYPE_MMAP,
		};
		static V4l2Capture* CreateVideoCapture(const V4L2DeviceParameters & param, IoType iotype);
		static V4l2Output*  CreateVideoOutput(const V4L2DeviceParameters & param, IoType iotype);
};

#endif
