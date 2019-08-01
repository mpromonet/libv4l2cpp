/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2ReadWriteDevice.cpp
** 
** V4L2 source using read/write API
**
** -------------------------------------------------------------------------*/

#include <unistd.h>

#include "V4l2ReadWriteDevice.h"

V4l2ReadWriteDevice::V4l2ReadWriteDevice(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType) : V4l2Device(params, deviceType) {
}


size_t V4l2ReadWriteDevice::writeInternal(char* buffer, size_t bufferSize) { 
	return ::write(m_fd, buffer,  bufferSize); 
}

size_t V4l2ReadWriteDevice::readInternal(char* buffer, size_t bufferSize)  { 
	return ::read(m_fd, buffer,  bufferSize); 
}
		
	

