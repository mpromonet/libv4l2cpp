/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapCapture.h
** 
** V4L2 source using mmap API
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_MMAP_CAPTURE
#define V4L2_MMAP_CAPTURE

// project
#include "V4l2Capture.h"
#include "V4l2MmapDevice.h"

class V4l2MmapCapture : public V4l2MmapDevice, public V4l2Capture
{
	public:
		static V4l2MmapCapture* createNew(V4L2DeviceParameters params);
	
	protected:
		V4l2MmapCapture(V4L2DeviceParameters params);
			
	public:
		virtual size_t read(char* buffer, size_t bufferSize);
		virtual bool isReady() { return  ((m_fd != -1)&& (n_buffers != 0)); };		
};

#endif

