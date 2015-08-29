/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** v4l2grab_h264.cpp
** 
** Grab raspberry screen -> compress in H264 using OMX -> write to a V4L2 output device
** 
** -------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

extern "C"
{
#include "bcm_host.h"
#include "ilclient.h"
}

#include "V4l2Output.h"

#define NUMFRAMES 300000

int take_snapshot(void *buffer, DISPMANX_DISPLAY_HANDLE_T display, DISPMANX_RESOURCE_HANDLE_T resource, DISPMANX_MODEINFO_T info, VC_RECT_T *rect)
{
	DISPMANX_TRANSFORM_T transform = DISPMANX_NO_ROTATE;
	int ret = 0;

	ret = vc_dispmanx_snapshot(display, resource, transform);
	assert(ret == 0);

	ret = vc_dispmanx_resource_read_data(resource, rect, buffer, info.width * 3);
	assert(ret == 0);

	return info.width * info.height * 3;
}

static void print_def(OMX_PARAM_PORTDEFINITIONTYPE def)
{
	fprintf(stderr, "Port %u: %s %u/%u %u %u %s,%s,%s %ux%u %ux%u @%u %u\n",
	  def.nPortIndex,
	  def.eDir == OMX_DirInput ? "in" : "out",
	  def.nBufferCountActual,
	  def.nBufferCountMin,
	  def.nBufferSize,
	  def.nBufferAlignment,
	  def.bEnabled ? "enabled" : "disabled",
	  def.bPopulated ? "populated" : "not pop.",
	  def.bBuffersContiguous ? "contig." : "not cont.",
	  def.format.video.nFrameWidth,
	  def.format.video.nFrameHeight,
	  def.format.video.nStride,
	  def.format.video.nSliceHeight,
	  def.format.video.xFramerate, def.format.video.eColorFormat);
}

ILCLIENT_T * encode_init(COMPONENT_T **video_encode)
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

	// create video_encode
	int omx_return = ilclient_create_component(client, video_encode, "video_encode",
				(ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS |
				ILCLIENT_ENABLE_INPUT_BUFFERS |
				ILCLIENT_ENABLE_OUTPUT_BUFFERS));
	if (omx_return != 0)
	{
		fprintf(stderr, "ilclient_create_component() for video_encode failed with %x!\n", omx_return);
		ilclient_destroy(client);
		return NULL;
	}

	return client;
}

bool encode_config_input(COMPONENT_T* handle, int32_t width, int32_t height, int32_t framerate)
{
	OMX_ERRORTYPE omx_return;

	// get current settings of video_encode component from port 200
	OMX_PARAM_PORTDEFINITIONTYPE def;
	memset(&def, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	def.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	def.nVersion.nVersion = OMX_VERSION;
	def.nPortIndex = 200;

	if (OMX_GetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamPortDefinition, &def) != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_GetParameter() for video_encode port 200 failed!\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	print_def(def);
	
	def.format.video.nFrameWidth = width;
	def.format.video.nFrameHeight = height;
	def.format.video.xFramerate = framerate << 16;
	def.format.video.nSliceHeight = def.format.video.nFrameHeight;
	def.format.video.nStride = def.format.video.nFrameWidth;
	def.format.video.eColorFormat = OMX_COLOR_Format24bitBGR888;

	print_def(def);

	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamPortDefinition, &def);
	if (omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;
	}
	return true;
}

bool encode_config_output(COMPONENT_T* handle, OMX_VIDEO_CODINGTYPE codec, uint32_t bitrate)
{
	OMX_ERRORTYPE omx_return;
	
	// set format
	OMX_VIDEO_PARAM_PORTFORMATTYPE format;
	memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
	format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
	format.nVersion.nVersion = OMX_VERSION;
	format.nPortIndex = 201;
	format.eCompressionFormat = codec;

	fprintf(stderr, "OMX_SetParameter(OMX_IndexParamVideoPortFormat) for video_encode:201...\n");
	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamVideoPortFormat, &format);
	if (omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;
	}

	// ask to repeat SPS/PPS
	OMX_CONFIG_PORTBOOLEANTYPE config;
	config.nSize = sizeof(config);
	config.nVersion.nVersion = OMX_VERSION;
	config.nPortIndex = 201;
	config.bEnabled = OMX_TRUE;
	fprintf(stderr, "OMX_SetParameter(OMX_IndexParamBrcmVideoAVCInlineHeaderEnable) for video_encode:201...\n");
	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamBrcmVideoAVCInlineHeaderEnable, &config);
	if (omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;
	}	
	
	// set current bitrate 
	OMX_VIDEO_PARAM_BITRATETYPE bitrateType;
	memset(&bitrateType, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
	bitrateType.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
	bitrateType.nVersion.nVersion = OMX_VERSION;
	bitrateType.eControlRate = OMX_Video_ControlRateVariable;
	bitrateType.nTargetBitrate = bitrate;
	bitrateType.nPortIndex = 201;

	omx_return = OMX_SetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamVideoBitrate, &bitrateType);
	if (omx_return != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_SetParameter() for bitrate for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__, omx_return);
		return false;
	}
   
	// get current bitrate
	memset(&bitrateType, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
	bitrateType.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
	bitrateType.nVersion.nVersion = OMX_VERSION;
	bitrateType.nPortIndex = 201;

	if (OMX_GetParameter(ILC_GET_HANDLE(handle), OMX_IndexParamVideoBitrate, &bitrateType) != OMX_ErrorNone)
	{
		fprintf(stderr, "%s:%d: OMX_GetParameter() for video_encode for bitrate port 201 failed!\n", __FUNCTION__, __LINE__);
		return false;
	}

	fprintf(stderr, "Current Bitrate=%u\n",bitrateType.nTargetBitrate);

	return true;
}

bool encode_config_activate(COMPONENT_T* handle)
{
	fprintf(stderr, "encode to idle...\n");
	if (ilclient_change_component_state(handle, OMX_StateIdle) == -1)
	{
		fprintf(stderr, "%s:%d: ilclient_change_component_state(video_encode, OMX_StateIdle) failed", __FUNCTION__, __LINE__);
		return false;
	}

	fprintf(stderr, "enabling port buffers for 200...\n");
	if (ilclient_enable_port_buffers(handle, 200, NULL, NULL, NULL) != 0)
	{
		fprintf(stderr, "enabling port buffers for 200 failed!\n");
		return false;
	}

	fprintf(stderr, "enabling port buffers for 201...\n");
	if (ilclient_enable_port_buffers(handle, 201, NULL, NULL, NULL) != 0)
	{
		fprintf(stderr, "enabling port buffers for 201 failed!\n");
		return false;
	}

	fprintf(stderr, "encode to executing...\n");
	ilclient_change_component_state(handle, OMX_StateExecuting);

	return true;
}

void encode_deactivate(COMPONENT_T* handle)
{
	fprintf(stderr, "disabling port buffers for 200 and 201...\n");
	ilclient_disable_port_buffers(handle, 200, NULL, NULL, NULL);
	ilclient_disable_port_buffers(handle, 201, NULL, NULL, NULL);
}

void encode_deinit(COMPONENT_T **list, ILCLIENT_T *client)
{
	ilclient_state_transition(list, OMX_StateIdle);
	ilclient_state_transition(list, OMX_StateLoaded);

	ilclient_cleanup_components(list);

	OMX_Deinit();

	ilclient_destroy(client);
}

int main(int argc, char **argv) 
{
	if (argc < 2) 
	{
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "Record to file: %s <filename>\n", argv[0]);
		exit(1);
	}

	bcm_host_init();
	
	DISPMANX_DISPLAY_HANDLE_T   display;
	DISPMANX_RESOURCE_HANDLE_T  resource;
	DISPMANX_MODEINFO_T         info;
	VC_RECT_T                   rect;
	uint32_t                    screen = 0;
	uint32_t                    vc_image_ptr;
	OMX_BUFFERHEADERTYPE *      buf = NULL;
	OMX_BUFFERHEADERTYPE *      out = NULL;
	COMPONENT_T *               video_encode = NULL;

	int status = 0;
	int framenumber = 0;

	int ret = 0;

	fprintf(stderr, "Open display[%i]...\n", screen );
	display = vc_dispmanx_display_open( screen );

	ret = vc_dispmanx_display_get_info(display, &info);
	assert(ret == 0);
	fprintf(stderr, "Display is %d x %d\n", info.width, info.height );
	resource = vc_dispmanx_resource_create( VC_IMAGE_BGR888,
		   info.width,
		   info.height,
		   &vc_image_ptr );

	fprintf(stderr, "VC image ptr: 0x%X\n", vc_image_ptr);

	ret = vc_dispmanx_rect_set(&rect, 0, 0, info.width, info.height);
	assert(ret == 0);
	
	
	V4L2DeviceParameters outparam(argv[1], V4L2_PIX_FMT_H264,  info.width, info.height, 0, true);
	V4l2Output outDev(outparam);	

	ILCLIENT_T *client = encode_init(&video_encode);
	if (client)
	{
		encode_config_input(video_encode, info.width, info.height, 30);
		encode_config_output(video_encode, OMX_VIDEO_CodingAVC, 10000000);

		encode_config_activate(video_encode);

		fprintf(stderr, "looping for buffers...\n");
		do
		{
			buf = ilclient_get_input_buffer(video_encode, 200, 0);
			if (buf != NULL)
			{
				/* fill it */
				buf->nFilledLen = take_snapshot(buf->pBuffer, display, resource, info, &rect);
				framenumber++;

				if (OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_encode), buf) != OMX_ErrorNone)
				{
					fprintf(stderr, "Error emptying buffer!\n");
				}
			}
				
			out = ilclient_get_output_buffer(video_encode, 201, 0);
			if (out != NULL)
			{
				OMX_ERRORTYPE r = OMX_FillThisBuffer(ILC_GET_HANDLE(video_encode), out);
				if (r != OMX_ErrorNone)
				{
					fprintf(stderr, "Error filling buffer: %x\n", r);
				}
				if (out->nFilledLen > 0)
				{
					size_t sz = write(outDev.getFd(), out->pBuffer, out->nFilledLen);
					if (sz != out->nFilledLen)
					{
						fprintf(stderr, "fwrite: Error emptying buffer: %d!\n", sz);
					}
					else
					{
//						fprintf(stderr, "Writing frame size:%d %d/%d\n", sz ,framenumber, NUMFRAMES);
					}
				}
				
				out->nFilledLen = 0;
			}

		} while (framenumber < NUMFRAMES);
		
		fprintf(stderr, "Exit\n");
		
		encode_deactivate(video_encode);
		COMPONENT_T *list[] = {video_encode , NULL};
		encode_deinit(list, client);		
	}	
	
	return status;
}
