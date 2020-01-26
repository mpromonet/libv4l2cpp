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

#include "V4l2Access.h"

// ---------------------------------
// V4L2 Output
// ---------------------------------
class V4l2Output : public V4l2Access
{		
	protected:
		V4l2Output(V4l2Device* device);

	public:
		static V4l2Output* create(const V4L2DeviceParameters & param, IoType iotype = V4l2Access::IOTYPE_MMAP);
		~V4l2Output();
	
		size_t write(char* buffer, size_t bufferSize);
		int    isWritable(timeval* tv);
		bool   startPartialWrite(void);
		size_t writePartial(char* buffer, size_t bufferSize);
		bool   endPartialWrite(void);
};

#endif
