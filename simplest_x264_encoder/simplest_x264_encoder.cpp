/**
 * 最简单的基于X264的视频编码器
 * Simplest X264 Encoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序可以YUV格式的像素数据编码为H.264码流，是最简单的
 * 基于libx264的视频编码器
 *
 * This software encode YUV data to H.264 bitstream.
 * It's the simplest encoder example based on libx264.
 */
#include <stdio.h>
#include <stdlib.h>

#include "stdint.h"

#if defined ( __cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif


int main(int argc, char** argv)
{

	int ret;
	int y_size;
	int i,j;

	//FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv444p.yuv", "rb");
	FILE* fp_src  = fopen("../cuc_ieschool_640x360_yuv420p.yuv", "rb");

	FILE* fp_dst = fopen("cuc_ieschool.h264", "wb");
	
	//Encode 50 frame
	//if set 0, encode all frame
	int frame_num=50;
	int csp=X264_CSP_I420;
	int width=640,height=360;

	int iNal   = 0;
	x264_nal_t* pNals = NULL;
	x264_t* pHandle   = NULL;
	x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));
	
	//Check
	if(fp_src==NULL||fp_dst==NULL){
		printf("Error open files.\n");
		return -1;
	}

	x264_param_default(pParam);
	pParam->i_width   = width; 
	pParam->i_height  = height;
	/*
	//Param
	pParam->i_log_level  = X264_LOG_DEBUG;
	pParam->i_threads  = X264_SYNC_LOOKAHEAD_AUTO;
	pParam->i_frame_total = 0;
	pParam->i_keyint_max = 10;
	pParam->i_bframe  = 5;
	pParam->b_open_gop  = 0;
	pParam->i_bframe_pyramid = 0;
	pParam->rc.i_qp_constant=0;
	pParam->rc.i_qp_max=0;
	pParam->rc.i_qp_min=0;
	pParam->i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	pParam->i_fps_den  = 1; 
	pParam->i_fps_num  = 25;
	pParam->i_timebase_den = pParam->i_fps_num;
	pParam->i_timebase_num = pParam->i_fps_den;
	*/
	pParam->i_csp=csp;
	x264_param_apply_profile(pParam, x264_profile_names[5]);
	
	pHandle = x264_encoder_open(pParam);
    
	x264_picture_init(pPic_out);
	x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

	//ret = x264_encoder_headers(pHandle, &pNals, &iNal);

	y_size = pParam->i_width * pParam->i_height;
	//detect frame number
	if(frame_num==0){
		fseek(fp_src,0,SEEK_END);
		switch(csp){
		case X264_CSP_I444:frame_num=ftell(fp_src)/(y_size*3);break;
		case X264_CSP_I420:frame_num=ftell(fp_src)/(y_size*3/2);break;
		default:printf("Colorspace Not Support.\n");return -1;
		}
		fseek(fp_src,0,SEEK_SET);
	}
	
	//Loop to Encode
	for( i=0;i<frame_num;i++){
		switch(csp){
		case X264_CSP_I444:{
			fread(pPic_in->img.plane[0],y_size,1,fp_src);	//Y
			fread(pPic_in->img.plane[1],y_size,1,fp_src);	//U
			fread(pPic_in->img.plane[2],y_size,1,fp_src);	//V
			break;}
		case X264_CSP_I420:{
			fread(pPic_in->img.plane[0],y_size,1,fp_src);	//Y
			fread(pPic_in->img.plane[1],y_size/4,1,fp_src);	//U
			fread(pPic_in->img.plane[2],y_size/4,1,fp_src);	//V
			break;}
		default:{
			printf("Colorspace Not Support.\n");
			return -1;}
		}
		pPic_in->i_pts = i;

		ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
		if (ret< 0){
			printf("Error.\n");
			return -1;
		}

		printf("Succeed encode frame: %5d\n",i);

		for ( j = 0; j < iNal; ++j){
			 fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
	}
	i=0;
	//flush encoder
	while(1){
		ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
		if(ret==0){
			break;
		}
		printf("Flush 1 frame.\n");
		for (j = 0; j < iNal; ++j){
			fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
		}
		i++;
	}
	x264_picture_clean(pPic_in);
	x264_encoder_close(pHandle);
	pHandle = NULL;

	free(pPic_in);
	free(pPic_out);
	free(pParam);

	fclose(fp_src);
	fclose(fp_dst);

	return 0;
}
