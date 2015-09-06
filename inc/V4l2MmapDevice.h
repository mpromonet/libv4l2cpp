/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapDevice.h
** 
** V4L2 source using mmap API
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_MMAP_DEVICE
#define V4L2_MMAP_DEVICE
 
#include "V4l2Device.h"

#define V4L2MMAP_NBBUFFER 10
class V4l2MmapDevice : public virtual V4l2Device
{	
	protected:
		V4l2MmapDevice(V4L2DeviceParameters params, v4l2_buf_type deviceType);
			
	public:
		virtual bool captureStart();
		virtual bool captureStop();
		virtual bool isReady() { return  ((m_fd != -1)&& (n_buffers != 0)); };
	
	protected:
		v4l2_buf_type m_deviceType;
		unsigned int  n_buffers;
	
		struct buffer 
		{
			void *                  start;
			size_t                  length;
		};
		buffer m_buffer[V4L2MMAP_NBBUFFER];
};

#endif

