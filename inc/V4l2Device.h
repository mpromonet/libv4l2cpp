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
#include <list>
#include <linux/videodev2.h>
#include <fcntl.h>

#define V4L2WRAPPER_SUPPORT_PARTIAL_WRITE   1

#ifndef V4L2_PIX_FMT_VP8
#define V4L2_PIX_FMT_VP8  v4l2_fourcc('V', 'P', '8', '0')
#endif
#ifndef V4L2_PIX_FMT_VP9
#define V4L2_PIX_FMT_VP9  v4l2_fourcc('V', 'P', '9', '0')
#endif
#ifndef V4L2_PIX_FMT_HEVC
#define V4L2_PIX_FMT_HEVC  v4l2_fourcc('H', 'E', 'V', 'C')
#endif

// ---------------------------------
// V4L2 Device parameters
// ---------------------------------
struct V4L2DeviceParameters 
{
	V4L2DeviceParameters(const char* devname, const std::list<unsigned int> & formatList, unsigned int width, unsigned int height, int fps, int verbose = 0, int openFlags = O_RDWR | O_NONBLOCK) : 
		m_devName(devname), m_formatList(formatList), m_width(width), m_height(height), m_fps(fps), m_verbose(verbose), m_openFlags(openFlags) {}

	V4L2DeviceParameters(const char* devname, unsigned int format, unsigned int width, unsigned int height, int fps, int verbose = 0, int openFlags = O_RDWR | O_NONBLOCK) : 
		m_devName(devname), m_width(width), m_height(height), m_fps(fps), m_verbose(verbose), m_openFlags(openFlags) {
			if (format) {
				m_formatList.push_back(format);
			}
	}
		
	std::string m_devName;
	std::list<unsigned int> m_formatList;
	unsigned int m_width;
	unsigned int m_height;
	int m_fps;			
	int m_verbose;
	int m_openFlags;
};

// ---------------------------------
// V4L2 Device
// ---------------------------------
class V4l2Device
{		
	friend class V4l2Capture;
	friend class V4l2Output;
	
	protected:	
		void close();	
	
		int initdevice(const char *dev_name, unsigned int mandatoryCapabilities);
		int checkCapabilities(int fd, unsigned int mandatoryCapabilities);
		int configureFormat(int fd);
		int configureFormat(int fd, unsigned int format, unsigned int width, unsigned int height);
		int configureParam(int fd);

		virtual bool init(unsigned int mandatoryCapabilities);		
		virtual size_t writeInternal(char*, size_t) { return -1; }
		virtual bool startPartialWrite(void)        { return false; }
		virtual size_t writePartialInternal(char*, size_t) { return -1; }
		virtual bool endPartialWrite(void)          { return false; }
		virtual size_t readInternal(char*, size_t)  { return -1; }
	
	public:
		V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType);		
		virtual ~V4l2Device();
	
		virtual bool isReady() { return (m_fd != -1); }
		virtual bool start()   { return true; }
		virtual bool stop()    { return true; }
	
		unsigned int getBufferSize() { return m_bufferSize; }
		unsigned int getFormat()     { return m_format;     }
		unsigned int getWidth()      { return m_width;      }
		unsigned int getHeight()     { return m_height;     }
		int getFd()         { return m_fd;         }
		void queryFormat();	

	protected:
		V4L2DeviceParameters m_params;
		int m_fd;
		v4l2_buf_type m_deviceType;	
	
		unsigned int m_bufferSize;
		unsigned int m_format;
		unsigned int m_width;
		unsigned int m_height;	

		struct v4l2_buffer m_partialWriteBuf;
		bool m_partialWriteInProgress;
};


#endif
