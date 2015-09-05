/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Output.h
** 
** V4L2 Output wrapper 
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_OUTPUT
#define V4L2_OUTPUT

#include "V4l2Device.h"

// ---------------------------------
// V4L2 Output
// ---------------------------------
class V4l2Output
{		
	public:
		V4l2Output(const V4L2DeviceParameters&  params);
		virtual ~V4l2Output();
	
	public:
		int getFd() { return m_fd; };	
		virtual size_t write(char* buffer, size_t bufferSize);
		
	protected:
		int m_fd;
};

#endif
