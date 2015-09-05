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

class V4l2Capture;
class V4l2Output;

class V4l2DeviceFactory
{
	public:
		static V4l2Capture * CreateVideoCapure(const V4L2DeviceParameters & param, bool useMmap);
		static V4l2Output *  CreateVideoOutput(const V4L2DeviceParameters & param, bool useMmap);
};

#endif
