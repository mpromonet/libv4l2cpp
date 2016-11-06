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
		enum IoType
		{
			IOTYPE_READWRITE,
			IOTYPE_MMAP,
		};
		
		V4l2Access(V4l2Device* device) : m_device(device) {}
		virtual ~V4l2Access() { delete m_device; }
		
		int getFd()         { return m_device->getFd();         }
		int getBufferSize() { return m_device->getBufferSize(); }
		int getFormat()     { return m_device->getFormat();     }
		int getWidth()      { return m_device->getWidth();      }
		int getHeight()     { return m_device->getHeight();     }
		void queryFormat()  { m_device->queryFormat();          }

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
