/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2copy.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include <fstream>

#include "logger.h"

#include "V4l2MmapCapture.h"
#include "V4l2ReadCapture.h"

// -----------------------------------------
//    create video capture interface
// -----------------------------------------
V4l2Capture* createVideoCapure(const V4L2DeviceParameters & param, bool useMmap)
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

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{	
	int verbose=0;
	const char *in_devname = "/dev/video0";	
	const char *out_devname = "/dev/video1";	
	int c = 0;
	bool useMmap = true;
	
	while ((c = getopt (argc, argv, "hP:F:v::r")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;
			case 'r':	useMmap = false; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-W width] [-H height] source_device dest_device" << std::endl;
				std::cout << "\t -v            : verbose " << std::endl;
				std::cout << "\t -vv           : very verbose " << std::endl;
				std::cout << "\t -r            : V4L2 capture using read interface (default use memory mapped buffers)" << std::endl;
				std::cout << "\t source_device : V4L2 capture device (default "<< in_devname << ")" << std::endl;
				std::cout << "\t dest_device   : V4L2 capture device (default "<< out_devname << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		in_devname = argv[optind];
		optind++;
	}	
	if (optind<argc)
	{
		out_devname = argv[optind];
		optind++;
	}	

	// initialize log4cpp
	initLogger(verbose);

	// init V4L2 capture interface
	V4L2DeviceParameters param(in_devname, 0, 0, 0, 0,verbose);
	V4l2Capture* videoCapture = createVideoCapure(param, useMmap);
	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << in_devname; 
	}
	else
	{
		V4L2DeviceParameters outparam(out_devname, videoCapture->getFormat(), videoCapture->getWidth(), videoCapture->getHeight(), 0,verbose);
		V4l2Output out(outparam);
		
		fd_set fdset;
		FD_ZERO(&fdset);
		timeval tv;
		LOG(NOTICE) << "Start Copying " << in_devname << " to " << out_devname; 
		videoCapture->captureStart();
		
		int stop=0;
		while (!stop) 
		{
			tv.tv_sec=1;
			tv.tv_usec=0;
			FD_SET(videoCapture->getFd(), &fdset);
			int ret = select(videoCapture->getFd()+1, &fdset, NULL, NULL, &tv);
			if (ret == 1)
			{
				char buffer[videoCapture->getBufferSize()];
				int rsize = videoCapture->read(buffer, sizeof(buffer));
				if (rsize == -1)
				{
					LOG(NOTICE) << "stop " << strerror(errno); 
					stop=1;					
				}
				else
				{
					int wsize = write(out.getFd(), buffer, rsize);
					LOG(DEBUG) << "Copied " << rsize << " " << wsize; 
				}
			}
			else if (ret == -1)
			{
				LOG(NOTICE) << "stop " << strerror(errno); 
				stop=1;
			}
		}
		videoCapture->captureStop();
		delete videoCapture;
	}
	
	return 0;
}
