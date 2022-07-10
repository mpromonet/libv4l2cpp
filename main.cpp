/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
** 
** test V4L2 capture device 
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "logger.h"
#include "V4l2Capture.h"

int stop=0;

/* ---------------------------------------------------------------------------
**  SIGINT handler
** -------------------------------------------------------------------------*/
void sighandler(int)
{ 
       printf("SIGINT\n");
       stop =1;
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{	
	int verbose = 0;
	const char *in_devname = "/dev/video0";	
	V4l2IoType ioTypeIn  = IOTYPE_MMAP;
	int format = 0;
	int width = 0;
	int height = 0;
	int fps = 0;
	
	int c = 0;
	while ((c = getopt (argc, argv, "hv:" "G:f:r")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose   = 1; if (optarg && *optarg=='v') verbose++; break;
			case 'r':	ioTypeIn  = IOTYPE_READWRITE                        ; break;			
            case 'G':   sscanf(optarg,"%dx%dx%d", &width, &height, &fps)    ; break;
			case 'f':	format    = V4l2Device::fourcc(optarg)              ; break;
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-G <width>x<height>x<fps>] [-f format] [device] [-r]" << std::endl;
				std::cout << "\t -v            : verbose " << std::endl;
				std::cout << "\t -vv           : very verbose " << std::endl;
				std::cout << "\t -r            : V4L2 capture using read interface (default use memory mapped buffers)" << std::endl;
				std::cout << "\t device        : V4L2 capture device (default "<< in_devname << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		in_devname = argv[optind];
		optind++;
	}	

	// initialize log4cpp
	initLogger(verbose);

	// init V4L2 capture interface
	V4L2DeviceParameters param(in_devname, format, width, height, fps, ioTypeIn, verbose);
	V4l2Capture* videoCapture = V4l2Capture::create(param);
	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot reading from V4L2 capture interface for device:" << in_devname; 
	}
	else
	{
		timeval tv;
		
		LOG(NOTICE) << "Start reading from " << in_devname;
		signal(SIGINT,sighandler);				
		while (!stop) 
		{
			tv.tv_sec=1;
			tv.tv_usec=0;
			int ret = videoCapture->isReadable(&tv);
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
					LOG(NOTICE) << "size:" << rsize;
				}
			}
			else if (ret == -1)
			{
				LOG(NOTICE) << "stop " << strerror(errno); 
				stop=1;
			}
		}
		

		delete videoCapture;
	}
	
	return 0;
}
