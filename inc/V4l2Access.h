/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Access.h
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/


#ifndef V4L2_ACCESS
#define V4L2_ACCESS

#include "V4l2Device.h"

class V4l2Access
{
	public:		
		V4l2Access(V4l2Device* device);
		virtual ~V4l2Access();
		
		int getFd()         { return m_device->getFd();         }
		unsigned int getBufferSize() { return m_device->getBufferSize(); }
		unsigned int getFormat()     { return m_device->getFormat();     }
		unsigned int getWidth()      { return m_device->getWidth();      }
		unsigned int getHeight()     { return m_device->getHeight();     }
		
		void queryFormat()  { m_device->queryFormat();          }
		int setFormat(unsigned int format, unsigned int width, unsigned int height)  { return m_device->setFormat(format, width, height); }

		int isReady()       { return m_device->isReady();       }
		int start()         { return m_device->start();         }
		int stop()          { return m_device->stop();          }

	private:
		V4l2Access(const V4l2Access&);
		V4l2Access & operator=(const V4l2Access&);
	
	protected:
		V4l2Device* m_device;		
};


#endif
