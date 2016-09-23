/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2DeviceFactory.cpp
** 
** -------------------------------------------------------------------------*/


#include "logger.h"

#include "V4l2Capture.h"

// -----------------------------------------
//    create video capture interface
// -----------------------------------------
V4l2Capture* V4l2DeviceFactory::CreateVideoCapture(const V4L2DeviceParameters & param, IoType iotype)
{
	return V4l2Capture::create(param, iotype);
}

#include "V4l2Output.h"

// -----------------------------------------
//    create video output interface
// -----------------------------------------
V4l2Output* V4l2DeviceFactory::CreateVideoOutput(const V4L2DeviceParameters & param, IoType iotype)
{
	return V4l2Output::create(param, iotype);
}
