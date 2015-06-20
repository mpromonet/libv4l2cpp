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

// -----------------------------------------
//    create output
// -----------------------------------------
int createOutput(const std::string & outputFile, int inputFd)
{
	int outputFd = -1;
	struct stat sb;		
	if ( (stat(outputFile.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR) ) 
	{
		// open & initialize a V4L2 output
		outputFd = open(outputFile.c_str(), O_WRONLY);
		if (outputFd != -1)
		{
			struct v4l2_capability cap;
			memset(&(cap), 0, sizeof(cap));
			if (0 == ioctl(outputFd, VIDIOC_QUERYCAP, &cap)) 
			{			
				LOG(NOTICE) << "Output device name:" << cap.driver << " cap:" <<  std::hex << cap.capabilities;
				if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) 
				{				
					struct v4l2_format   fmt;			
					memset(&(fmt), 0, sizeof(fmt));
					fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					if (ioctl(inputFd, VIDIOC_G_FMT, &fmt) == -1)
					{
						LOG(ERROR) << "Cannot get input format "<< strerror(errno);
					}		
					else 
					{
						fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
						if (ioctl(outputFd, VIDIOC_S_FMT, &fmt) == -1)
						{
							LOG(ERROR) << "Cannot set output format "<< strerror(errno);
						}		
					}
				}			
			}
		}
		else
		{
			LOG(ERROR) << "Cannot open " << outputFile << " " << strerror(errno);
		}
	}
	
	if (outputFd == -1)		
	{
		LOG(NOTICE) << "Error openning " << outputFile << " " << strerror(errno);
	}
	
	return outputFd;
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
		int outputFd = createOutput(out_devname, videoCapture->getFd());		
		fd_set fdset;
		FD_ZERO(&fdset);
		timeval tv;
		tv.tv_sec=1;
		tv.tv_usec=0;
		LOG(NOTICE) << "Start Copying " << in_devname << " to " << out_devname; 
		videoCapture->captureStart();
		
		int stop=0;
		while (!stop) 
		{
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
					int wsize = write(outputFd, buffer, rsize);
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
