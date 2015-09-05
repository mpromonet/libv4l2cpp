/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Device.cpp
** 
** -------------------------------------------------------------------------*/


#include "V4l2Device.h"
#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"

// -----------------------------------------
//    create video capture interface
// -----------------------------------------
V4l2Capture* V4l2DeviceFactory::CreateVideoCapure(const V4L2DeviceParameters & param, bool useMmap)
{
	V4l2Capture* videoCapture = NULL;
	if (useMmap)
	{
		videoCapture = V4l2MmapCapture::createNew(param);
	}
	else
	{
		videoCapture = V4l2ReadCapture::createNew(param);
	}
	return videoCapture;
}

#include "V4l2Output.h"
// -----------------------------------------
//    create video output interface
// -----------------------------------------
V4l2Output* V4l2DeviceFactory::CreateVideoOutput(const V4L2DeviceParameters & param, bool useMmap)
{
	return new V4l2Output(param);
}
