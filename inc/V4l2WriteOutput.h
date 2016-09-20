/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2WriteOutput.h
** 
** V4L2 Output wrapper 
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_WRITE_OUTPUT
#define V4L2_WRITE_OUTPUT

#include "V4l2Output.h"

// ---------------------------------
// V4L2 Output
// ---------------------------------
class V4l2WriteOutput : public V4l2Output
{		
	public:
		static V4l2WriteOutput* createNew(const V4L2DeviceParameters& params) ;

	protected:
		V4l2WriteOutput(const V4L2DeviceParameters&  params);
	
	public:
		virtual size_t write(char* buffer, size_t bufferSize);
		
};

#endif
