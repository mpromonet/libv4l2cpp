/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Access.cpp
**
** V4L2 wrapper
**
** -------------------------------------------------------------------------*/

#include "V4l2Access.h"

V4l2Access::V4l2Access(V4l2Device* device) : m_device(device) {
}

V4l2Access::~V4l2Access() { 
	delete m_device; 
}

