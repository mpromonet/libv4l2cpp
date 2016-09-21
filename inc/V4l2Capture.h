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

#include "V4l2Device.h"

// ---------------------------------
// V4L2 Capture
// ---------------------------------
class V4l2Capture : public virtual V4l2Device
{		
	protected:
		V4l2Capture(const V4L2DeviceParameters&  params);
					
	public:
		virtual size_t read(char* buffer, size_t bufferSize) = 0;
		int            isReadable(timeval* tv);	
};


#endif
