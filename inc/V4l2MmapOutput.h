/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2MmapOutput.h
** 
** V4L2 source using mmap API
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_MMAP_OUTPUT
#define V4L2_MMAP_OUTPUT

// project
#include "V4l2Output.h"
#include "V4l2MmapDevice.h"

class V4l2MmapOutput : public V4l2MmapDevice, public V4l2Output
{
	public:
		static V4l2MmapOutput* createNew(V4L2DeviceParameters params);
	
	protected:
		V4l2MmapOutput(V4L2DeviceParameters params);
			
	public:
		virtual bool captureStart() { return V4l2MmapDevice::captureStart(); };
		virtual size_t write(char* buffer, size_t bufferSize);
		virtual bool captureStop() { return V4l2MmapDevice::captureStop(); };;
};

#endif

