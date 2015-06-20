/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2vp8.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include <fstream>

#include "vpx/vpx_encoder.h"
#include "vpx/vp8cx.h"

#include "libyuv.h"

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
						fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VP8;
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

	int                  flags=0;
	int                  frame_cnt=0;

	vpx_image_t          raw;
	if(!vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1))
		LOG(WARN) << "vpx_img_alloc"; 

	const vpx_codec_iface_t* algo = vpx_codec_vp8_cx();
	vpx_codec_enc_cfg_t  cfg;
	if (vpx_codec_enc_config_default(algo, &cfg, 0) != VPX_CODEC_OK)
		LOG(WARN) << "vpx_codec_enc_config_default"; 

    
	vpx_codec_ctx_t      codec;
	if(vpx_codec_enc_init(&codec, algo, &cfg, 0))    
		LOG(WARN) << "vpx_codec_enc_init"; 
		
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
		int outputFd = createOutput(out_devname, videoCapture->getFd());		
		fd_set fdset;
		FD_ZERO(&fdset);
		timeval tv;
		tv.tv_sec=1;
		tv.tv_usec=0;
		LOG(NOTICE) << "Start Compressing " << in_devname << " to " << out_devname; 
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
				
				ConvertToI420(buffer, rsize,
					raw.planes[0], width,
					raw.planes[1], width/2,
					raw.planes[2], width/2,
					0, 0,
					width, height,
					width, height,
					libyuv::kRotate0, libyuv::FOURCC_YUY2);
												
				if(vpx_codec_encode(&codec, &raw, frame_cnt++, 1, flags, VPX_DL_REALTIME))    
				{					
					LOG(WARN) << "vpx_codec_encode: " << vpx_codec_error(&codec) << "(" << vpx_codec_error_detail(&codec) << ")";
				}
				
				vpx_codec_iter_t iter = NULL;
				const vpx_codec_cx_pkt_t *pkt;
				while( (pkt = vpx_codec_get_cx_data(&codec, &iter)) ) 
				{
					if (pkt->kind==VPX_CODEC_CX_FRAME_PKT)
					{
						int wsize = write(outputFd, pkt->data.frame.buf, pkt->data.frame.sz);
						LOG(DEBUG) << "Copied " << rsize << " " << wsize; 
					}
					else
					{
						break;
					}
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
