/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2x264.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include <fstream>

extern "C" 
{
	#include "x264.h"
}


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
						fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
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
	else
	{
		outputFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
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
	int width = 320;
	int height = 240;	
	int fps = 10;	
	int c = 0;
	bool useMmap = false;
	
	while ((c = getopt (argc, argv, "hW:H:P:F:v::rM")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;
			case 'W':	width = atoi(optarg); break;
			case 'H':	height = atoi(optarg); break;
			case 'F':	fps = atoi(optarg); break;
			case 'M':	useMmap = true; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-W width] [-H height] source_device dest_device" << std::endl;
				std::cout << "\t -v            : verbose " << std::endl;
				std::cout << "\t -vv           : very verbose " << std::endl;
				std::cout << "\t -W width      : V4L2 capture width (default "<< width << ")" << std::endl;
				std::cout << "\t -H height     : V4L2 capture height (default "<< height << ")" << std::endl;
				std::cout << "\t -F fps        : V4L2 capture framerate (default "<< fps << ")" << std::endl;
				std::cout << "\t -M            : V4L2 capture using memory mapped buffers (default use read interface)" << std::endl;				
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
	int format = V4L2_PIX_FMT_YUYV;
	V4L2DeviceParameters param(in_devname,format,width,height,fps,verbose);
	V4l2Capture* videoCapture = createVideoCapure(param, useMmap);
	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << in_devname; 
	}
	else
	{
		x264_param_t param;
		x264_param_default_preset(&param, "veryfast", "zerolatency");
		param.i_threads = 1;
		param.i_width = width;
		param.i_height = height;
		param.i_fps_num = fps;
		param.i_fps_den = 1;
		// Intra refres:
		param.i_keyint_max = fps;
		param.b_intra_refresh = 1;
		//Rate control:
		param.rc.i_rc_method = X264_RC_CRF;
		param.rc.f_rf_constant = 25;
		param.rc.f_rf_constant_max = 35;
		//For streaming:
		param.b_repeat_headers = 1;
		param.b_annexb = 1;
		
		x264_t* encoder = x264_encoder_open(&param);
		
		x264_picture_t pic_in;
		x264_picture_alloc(&pic_in, X264_CSP_YV12, width, height);
		x264_picture_t pic_out;
		
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
				
				int widthXheight = width * height;
				memcpy(pic_in.img.plane[0], buffer, widthXheight);
				memcpy(pic_in.img.plane[1], buffer + widthXheight, widthXheight >> 2);
				memcpy(pic_in.img.plane[2], buffer + widthXheight + (widthXheight >> 2), widthXheight >> 2);
				
				
				x264_nal_t* nals = NULL;
				int i_nals = 0;
				int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
				if (frame_size >= 0)
				{
					int wsize = write(outputFd, nals[0].p_payload, frame_size);
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
