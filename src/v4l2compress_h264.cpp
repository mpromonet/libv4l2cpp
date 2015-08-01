/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2compress_x264.cpp
** 
** Read YUYV from a V4L2 capture -> compress in H264 -> write to a V4L2 output device
**
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <signal.h>

#include <fstream>

extern "C" 
{
	#include "x264.h"
}

#include "libyuv.h"

#include "logger.h"

#include "V4l2Device.h"
#include "V4l2Capture.h"
#include "V4l2Output.h"

int stop=0;
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
	int verbose=0;
	const char *in_devname = "/dev/video0";	
	const char *out_devname = "/dev/video1";	
	int width = 640;
	int height = 480;	
	int fps = 10;	
	int c = 0;
	bool useMmap = true;
	
	while ((c = getopt (argc, argv, "hW:H:P:F:v::r")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose = 1; if (optarg && *optarg=='v') verbose++;  break;
			case 'W':	width = atoi(optarg); break;
			case 'H':	height = atoi(optarg); break;
			case 'F':	fps = atoi(optarg); break;
			case 'r':	useMmap = false; break;			
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-W width] [-H height] source_device dest_device" << std::endl;
				std::cout << "\t -v            : verbose " << std::endl;
				std::cout << "\t -vv           : very verbose " << std::endl;
				std::cout << "\t -W width      : V4L2 capture width (default "<< width << ")" << std::endl;
				std::cout << "\t -H height     : V4L2 capture height (default "<< height << ")" << std::endl;
				std::cout << "\t -F fps        : V4L2 capture framerate (default "<< fps << ")" << std::endl;
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
	int format = V4L2_PIX_FMT_YUYV;
	V4L2DeviceParameters param(in_devname,format,width,height,fps,verbose);
	V4l2Capture* videoCapture = V4l2DeviceFactory::CreateVideoCapure(param, useMmap);
	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot create V4L2 capture interface for device:" << in_devname; 
	}
	else
	{
		x264_param_t param;
		x264_param_default_preset(&param, "ultrafast", "zerolatency");
		if (verbose>1)
		{
			param.i_log_level = X264_LOG_DEBUG;
		}
		param.i_threads = 1;
		param.i_width = width;
		param.i_height = height;
		param.i_fps_num = fps;
		param.i_fps_den = 1;
		param.i_keyint_min = fps;
		param.i_keyint_max = fps;
#if CQP
		param.rc.i_rc_method = X264_RC_CQP;
		param.rc.i_qp_constant = 20;
#endif		
#ifdef CRF
		param.rc.i_rc_method = X264_RC_CRF;
		param.rc.f_rf_constant = 10;
		param.rc.f_rf_constant_max = 10;
#endif		
#ifdef CBR
		param.rc.i_rc_method = X264_RC_ABR;
		param.rc.i_vbv_buffer_size = 100;
		param.rc.i_bitrate = param.rc.i_vbv_buffer_size * param.i_fps_num / param.i_fps_den;
		param.rc.i_vbv_max_bitrate = param.rc.i_bitrate;
#endif		
		
		x264_t* encoder = x264_encoder_open(&param);
		if (!encoder)
		{
			LOG(WARN) << "Cannot create X264 encoder for device:" << in_devname; 
		}
		else
		{		
			x264_picture_t pic_in;
			x264_picture_init( &pic_in );
			x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);
			
			x264_picture_t pic_out;
			
			V4L2DeviceParameters outparam(out_devname, V4L2_PIX_FMT_H264, videoCapture->getWidth(), videoCapture->getHeight(), 0,verbose);
			V4l2Output out(outparam);
			fd_set fdset;
			FD_ZERO(&fdset);
			timeval tv;
			timeval refTime;
			timeval curTime;
			LOG(NOTICE) << "Start Compressing " << in_devname << " to " << out_devname; 
			videoCapture->captureStart();
			
			signal(SIGINT,sighandler);
			while (!stop) 
			{
				FD_SET(videoCapture->getFd(), &fdset);
				tv.tv_sec=1;
				tv.tv_usec=0;
				int ret = select(videoCapture->getFd()+1, &fdset, NULL, NULL, &tv);
				if (ret == 1)
				{
					gettimeofday(&refTime, NULL);	
					char buffer[videoCapture->getBufferSize()];
					int rsize = videoCapture->read(buffer, sizeof(buffer));
					
					gettimeofday(&curTime, NULL);												
					timeval captureTime;
					timersub(&curTime,&refTime,&captureTime);
					refTime = curTime;
					
					ConvertToI420((const uint8*)buffer, rsize,
						pic_in.img.plane[0], width,
						pic_in.img.plane[1], width/2,
						pic_in.img.plane[2], width/2,
						0, 0,
						width, height,
						width, height,
						libyuv::kRotate0, libyuv::FOURCC_YUY2);

					gettimeofday(&curTime, NULL);												
					timeval convertTime;
					timersub(&curTime,&refTime,&convertTime);
					refTime = curTime;
					
					x264_nal_t* nals = NULL;
					int i_nals = 0;
					int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
					
					gettimeofday(&curTime, NULL);												
					timeval endodeTime;
					timersub(&curTime,&refTime,&endodeTime);
					refTime = curTime;
					
					if (frame_size >= 0)
					{
						for (int i=0; i < i_nals; ++i)
						{
							int wsize = write(out.getFd(), nals[i].p_payload, nals[i].i_payload);
							LOG(INFO) << "Copied " << i << "/" << i_nals << " size:" << wsize; 					
						}
					}
					
					gettimeofday(&curTime, NULL);												
					timeval writeTime;
					timersub(&curTime,&refTime,&writeTime);
					refTime = curTime;

					LOG(DEBUG) << "dts:" << pic_out.i_dts << " captureTime:" << (captureTime.tv_sec*1000+captureTime.tv_usec/1000) 
							<< " convertTime:" << (convertTime.tv_sec*1000+convertTime.tv_usec/1000)					
							<< " endodeTime:" << (endodeTime.tv_sec*1000+endodeTime.tv_usec/1000)
							<< " writeTime:" << (writeTime.tv_sec*1000+writeTime.tv_usec/1000); 					
					
				}
				else if (ret == -1)
				{
					LOG(NOTICE) << "stop error:" << strerror(errno); 
					stop=1;
				}
			}
			
			x264_picture_clean(&pic_in);
			x264_encoder_close(encoder);
		}
		
		videoCapture->captureStop();
		delete videoCapture;
	}
	
	return 0;
}
