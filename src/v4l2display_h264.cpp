/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2display_h264.cpp
** 
** read from a V4L2 capture device in H264 format -> uncompress using OMX -> display using OMX
** 
** -------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>

extern "C"
{
#include "bcm_host.h"
#include "ilclient.h"
}

#include "V4l2Device.h"
#include "V4l2Capture.h"
#include "V4l2Output.h"

bool encode_config_clock(COMPONENT_T* handle)
{
	OMX_ERRORTYPE omx_return;
	OMX_TIME_CONFIG_CLOCKSTATETYPE cstate;
	memset(&cstate, 0, sizeof(cstate));
	cstate.nSize = sizeof(cstate);
	cstate.nVersion.nVersion = OMX_VERSION;
	cstate.eState = OMX_TIME_ClockStateWaitingForStartTime;
	cstate.nWaitMask = 1;

	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexConfigTimeClockState, &cstate) ;
	if(omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for clock failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;	   
	}
	return true;   
}

bool encode_config_decoder(COMPONENT_T* handle)
{
	OMX_ERRORTYPE omx_return;
	OMX_VIDEO_PARAM_PORTFORMATTYPE format;
	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 130;
	format.eCompressionFormat = OMX_VIDEO_CodingAVC;

	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamVideoPortFormat, &format) ;
	if(omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for bitrate for video_decoder failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;	   
	}
	return true;   
}   

ILCLIENT_T * encode_init(COMPONENT_T **video_decode, COMPONENT_T **video_render, COMPONENT_T **clock, COMPONENT_T ** video_scheduler)
{
	ILCLIENT_T *client = ilclient_init();
	if (client == NULL)
	{
		return NULL;
	}

	if (OMX_Init() != OMX_ErrorNone)
	{
		ilclient_destroy(client);
		return NULL;
	}

	// create video_decode
	int omx_return = ilclient_create_component(client, video_decode, "video_decode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS));
	if (omx_return != 0)
	{
		fprintf(stderr, "ilclient_create_component() for video_decode failed with %x!\n", omx_return);
		ilclient_destroy(client);
		return NULL;
	}
	
	omx_return = ilclient_create_component(client, video_render, "video_render", ILCLIENT_DISABLE_ALL_PORTS);
	if (omx_return != 0)
	{
		fprintf(stderr, "ilclient_create_component() for video_render failed with %x!\n", omx_return);
		ilclient_destroy(client);
		return NULL;
	}

	omx_return = ilclient_create_component(client, clock, "clock", ILCLIENT_DISABLE_ALL_PORTS);
	if (omx_return != 0)
	{
		fprintf(stderr, "ilclient_create_component() for clock failed with %x!\n", omx_return);
		ilclient_destroy(client);
		return NULL;
	}
	
	omx_return = ilclient_create_component(client, video_scheduler, "video_scheduler", ILCLIENT_DISABLE_ALL_PORTS);
	if (omx_return != 0)
	{
		fprintf(stderr, "ilclient_create_component() for video_scheduler failed with %x!\n", omx_return);
		ilclient_destroy(client);
		return NULL;
	}
	

	return client;
}

void encode_deinit( COMPONENT_T **list, TUNNEL_T* tunnel, ILCLIENT_T *client)
{	
	ilclient_disable_tunnel(tunnel);
	ilclient_disable_tunnel(tunnel+1);
	ilclient_disable_tunnel(tunnel+2);
	ilclient_teardown_tunnels(tunnel);

	ilclient_state_transition(list, OMX_StateIdle);
	ilclient_state_transition(list, OMX_StateLoaded);

	ilclient_cleanup_components(list);

	OMX_Deinit();

	ilclient_destroy(client);	
}

bool encode_config_activate_decode_clock(COMPONENT_T* video_render, COMPONENT_T* clock, COMPONENT_T* video_scheduler, COMPONENT_T*video_decode, TUNNEL_T* tunnel)
{
	set_tunnel(&tunnel[0], video_decode, 131, video_scheduler, 10);
	set_tunnel(&tunnel[1], video_scheduler, 11, video_render, 90);
	set_tunnel(&tunnel[2], clock, 80, video_scheduler, 12);

	// setup clock tunnel first
	if(ilclient_setup_tunnel(&tunnel[2], 0, 0) != 0)
	{
		fprintf(stderr, "cannot setup tunnel\n");
		return false;
	}
	ilclient_change_component_state(clock, OMX_StateExecuting);
	
	ilclient_change_component_state(video_decode, OMX_StateIdle);	
	if (ilclient_enable_port_buffers(video_decode, 130, NULL, NULL, NULL) != 0)
	{
		fprintf(stderr, "cannot enable video_decode port\n");
		return false;
	}
	ilclient_change_component_state(video_decode, OMX_StateExecuting);

	return true;
}

bool encode_config_activate_scheduler_render(COMPONENT_T* video_render, COMPONENT_T* video_scheduler, TUNNEL_T* tunnel)
{
	if(ilclient_setup_tunnel(&tunnel[0], 0, 0) != 0)
	{
		fprintf(stderr, "cannot setup tunnel\n");
		return false;
	}
	ilclient_change_component_state(video_scheduler, OMX_StateExecuting);
	
	// now setup tunnel to video_render
	if(ilclient_setup_tunnel(&tunnel[1], 0, 1000) != 0)
	{
		fprintf(stderr, "cannot setup tunnel\n");
		return false;
	}
	ilclient_change_component_state(video_render, OMX_StateExecuting);
	
	return true;
}

void encode_deactivate(COMPONENT_T* video_render, COMPONENT_T*video_decode, TUNNEL_T* tunnel)
{
	fprintf(stderr, "wait for EOS from render\n");
	ilclient_wait_for_event(video_render, OMX_EventBufferFlag, 90, 0, OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS, 10000);

	fprintf(stderr, "flush tunnels\n");
	ilclient_flush_tunnels(tunnel, 0);

	fprintf(stderr, "disabling port buffers for 130...\n");
	ilclient_disable_port_buffers(video_decode, 130, NULL, NULL, NULL);	
}


int main (int argc, char **argv)
{
	if (argc < 2) 
	{
		printf("Usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	bcm_host_init();

	COMPONENT_T *video_decode = NULL, *video_scheduler = NULL, *video_render = NULL, *clock = NULL;
	TUNNEL_T tunnel[4];
	int status = 0;
	memset(tunnel, 0, sizeof(tunnel));

	ILCLIENT_T *client = encode_init(&video_decode, &video_render, &clock, &video_scheduler);
	if(client != NULL)
	{
		encode_config_clock(clock);
		encode_config_decoder(video_decode);
		encode_config_activate_decode_clock(video_render, clock, video_scheduler, video_decode, tunnel);

		OMX_BUFFERHEADERTYPE *buf = NULL;
		int port_settings_changed = 0;
		int first_packet = 1;
		
		V4L2DeviceParameters param(argv[1],V4L2_PIX_FMT_H264,640,480,0,true);
		V4l2Capture* videoCapture = V4l2DeviceFactory::CreateVideoCapure(param, true);
		fd_set fdset;
		FD_ZERO(&fdset);
		videoCapture->captureStart();
		timeval tv;	
		
		while((buf = ilclient_get_input_buffer(video_decode, 130, 1)) != NULL)
		{
			tv.tv_sec=1;
			tv.tv_usec=0;
			FD_SET(videoCapture->getFd(), &fdset);
			int ret = select(videoCapture->getFd()+1, &fdset, NULL, NULL, &tv);
			if (ret == 1)
			{
				buf->nFilledLen = videoCapture->read( (char*)buf->pBuffer, buf->nAllocLen);		

				if (port_settings_changed == 0) 
				{
					if (ilclient_remove_event(video_decode, OMX_EventPortSettingsChanged, 131, 0, 0, 1) == 0) 
					{
						printf("port_settings_changed\n");
						port_settings_changed = 1;
						encode_config_activate_scheduler_render(video_render, video_scheduler, tunnel);
					}
					else
					{
						printf("waiting...\n");
					}
				}

				if(!buf->nFilledLen)
				{
					printf("no more data\n");
					break;
				}

				buf->nOffset = 0;				
				if(first_packet)
				{
					printf("first packet\n");
					buf->nFlags = OMX_BUFFERFLAG_STARTTIME;
					first_packet = 0;
				}
				else
				{
					buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN;
				}

				if(OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_decode), buf) != OMX_ErrorNone)
				{
					status = -6;
					break;
				}
			}
		}

		buf->nFilledLen = 0;
		buf->nFlags = OMX_BUFFERFLAG_TIME_UNKNOWN | OMX_BUFFERFLAG_EOS;

		encode_deactivate(video_render, video_decode, tunnel);
		COMPONENT_T *list[] = {video_decode, video_render, clock, video_scheduler, NULL};      
		encode_deinit(list, tunnel, client);
	}

	return status;
}


