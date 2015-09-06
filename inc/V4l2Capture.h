/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Capture.h
** 
** V4L2 Capture wrapper 
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_CAPTURE
#define V4L2_CAPTURE

#include <string>
#include <list> 
#include <iostream>

#include "V4l2Device.h"

// ---------------------------------
// V4L2 Capture
// ---------------------------------
class V4l2Capture : public virtual V4l2Device
{		
	protected:
		V4l2Capture(const V4L2DeviceParameters&  params);
	
	public:
		virtual ~V4l2Capture();
	
	public:
		int getBufferSize() { return m_bufferSize; };
		int getFormat() { return m_format; } ;
		int getWidth() { return m_width; };
		int getHeight() { return m_height; };
		void queryFormat();

	protected:
		bool init(unsigned int mandatoryCapabilities);
	
		int initdevice(const char *dev_name, unsigned int mandatoryCapabilities);
		int checkCapabilities(int fd, unsigned int mandatoryCapabilities);
		int configureFormat(int fd);
		int configureParam(int fd);		
				
	public:
		virtual bool captureStart() = 0;
		virtual size_t read(char* buffer, size_t bufferSize) = 0;
		virtual bool captureStop() = 0;
		virtual bool isReady() = 0;
		
	protected:
		int m_bufferSize;
		int m_format;
		int m_width;
		int m_height;
};


#endif
