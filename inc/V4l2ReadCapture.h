/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2ReadCapture.h
** 
** V4L2 source using read API
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_READ_CAPTURE
#define V4L2_READ_CAPTURE

// project
#include "V4l2Capture.h"

class V4l2ReadCapture : public V4l2Capture
{
	public:
		static V4l2ReadCapture* createNew(V4L2DeviceParameters params);
	
	protected:
		V4l2ReadCapture(V4L2DeviceParameters params) : V4l2Device(params,V4L2_BUF_TYPE_VIDEO_CAPTURE), V4l2Capture(params), m_counter(0) {};
			
	public:
		virtual size_t read(char* buffer, size_t bufferSize);
		virtual bool isReady() { return (m_fd != -1); };
	
	protected:
		int m_counter;
};

#endif