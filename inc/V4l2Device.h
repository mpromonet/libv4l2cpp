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
	friend class V4l2Capture;
	friend class V4l2Output;
	
	protected:	
		void close();	
	
		int initdevice(const char *dev_name, unsigned int mandatoryCapabilities);
		int checkCapabilities(int fd, unsigned int mandatoryCapabilities);
		int configureFormat(int fd);
		int configureParam(int fd);

		virtual bool init(unsigned int mandatoryCapabilities);		
		virtual size_t writeInternal(char*, size_t) { return -1; };
		virtual size_t readInternal(char*, size_t)  { return -1; };		
	
	public:
		V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType);		
		virtual ~V4l2Device();
	
		virtual bool isReady() { return (m_fd != -1); }
		virtual bool start()   { return true; }
		virtual bool stop()    { return true; }
	
		int getBufferSize() { return m_bufferSize; }
		int getFormat()     { return m_format;     }
		int getWidth()      { return m_width;      }
		int getHeight()     { return m_height;     }
		int getFd()         { return m_fd;         }
		void queryFormat();	

	protected:
		V4L2DeviceParameters m_params;
		int m_fd;
		v4l2_buf_type m_deviceType;	
	
		int m_bufferSize;
		int m_format;
		int m_width;
		int m_height;	
};

class V4l2Access
{
	public:
		V4l2Access(V4l2Device* device) : m_device(device) {}
		virtual ~V4l2Access() { delete m_device; }
		
		int getFd()         { return m_device->getFd();         }
		int getBufferSize() { return m_device->getBufferSize(); }
		int getFormat()     { return m_device->getFormat();     }
		int getWidth()      { return m_device->getWidth();      }
		int getHeight()     { return m_device->getHeight();     }
		void queryFormat()  { m_device->queryFormat();          }

		int isReady()       { return m_device->isReady();       }
		int start()         { return m_device->start();         }
		int stop()          { return m_device->stop();          }

	private:
		V4l2Access(const V4l2Access&);
		V4l2Access & operator=(const V4l2Access&);
	
	protected:
		V4l2Device* m_device;		
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
			IOTYPE_READWRITE,
			IOTYPE_MMAP,
		};
		static V4l2Capture* CreateVideoCapture(const V4L2DeviceParameters & param, IoType iotype);
		static V4l2Output*  CreateVideoOutput(const V4L2DeviceParameters & param, IoType iotype);
};

#endif
