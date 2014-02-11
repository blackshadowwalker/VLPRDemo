#include "StdAfx.h"
#include "FFmpegVideo.h"

#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

FFmpegVideo::FFmpegVideo(void)
{
}



FFmpegVideo::FFmpegVideo(char* chVidName1, int iProcessOrder1, float fRate1)
{
	this->iProcessOrder = iProcessOrder1;
	strcpy(chVidName, chVidName1);
	this->fRate = fRate1;
	iTotalFrameNum = 0;
	iNowFrameNum = 0;
	frameFinished = 0;
	nFps = 0;
	buffer = NULL;
	pFrameBGR = NULL;
	pFrameRGB = NULL;
	pFrameOri = NULL;
	pCodecCtx = NULL;
	imageFrame = new ImageFrame();
	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(av_open_input_file(&pFormatCtx, chVidName, NULL, 0, NULL)!=0)
	{
		bIfSuccess = false;
		return; // Couldn't open file
	}

	// Retrieve stream information
	if(av_find_stream_info(pFormatCtx)<0)
	{
		bIfSuccess = false;
		return; // Couldn't find stream information
	}

	// Dump information about file onto standard error
	dump_format(pFormatCtx, 0, chVidName, 0);

	this->iTotalFrameNum = pFormatCtx->streams[0]->duration;
	this->fFrmRat = pFormatCtx->streams[0]->r_frame_rate.num/(float)(pFormatCtx->streams[0]->r_frame_rate.den);

	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	}
	if(videoStream==-1)
	{
		bIfSuccess = false;
		return; // Didn't find a video stream
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	printf("%d-%d\n", pCodecCtx->height, pCodecCtx->width);




	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		bIfSuccess = false;
		fprintf(stderr, "Unsupported codec!\n");
		return; // Codec not found
	}
	// Open codec
	//while(bIfLockCloseCodec)
	//{
	//	Sleep(10);
	//}
	//bIfLockCloseCodec = true;
	//if(avcodec_open(pCodecCtx, pCodec)<0)
	//	return -1; // Could not open codec
	//bIfLockCloseCodec = false;

	while (avcodec_open(pCodecCtx, pCodec) < 0)/*这个函数总是返回-1*/ {
		//fprintf(stderr, "could not open codec\n");
		Sleep(this->iProcessOrder);
		//exit(1);
	}

	// Allocate video frame
	pFrameOri=avcodec_alloc_frame();

	// Allocate an AVFrame structure
	pFrameBGR=avcodec_alloc_frame();
	if(pFrameBGR==NULL)
	{
		bIfSuccess = false;
		return;
	}
	pFrameRGB=avcodec_alloc_frame();
	if(pFrameRGB==NULL)
	{
		bIfSuccess = false;
		return;
	}

	// Determine required buffer size and allocate buffer
	imageFrame->size = numBytes=avpicture_get_size(PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
	imageFrame->widthStep = WIDTHSTEP(pCodecCtx->width);
	imageFrame->height = pCodecCtx->height;
	imageFrame->width = pCodecCtx->width;
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	imageFrame->imageData = new uint8_t[numBytes*sizeof(uint8_t)];
	memset(imageFrame->imageData, 0, numBytes*sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameBGR, buffer, PIX_FMT_BGR24,
		pCodecCtx->width, pCodecCtx->height);
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height);
	//注意，这里是PIX_FMT_RGB24，它决定了图片的格式
	ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
		PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

	this->getOneFrame();

	bIfSuccess = true;
}

void FFmpegVideo::restart()
{
	char chVidName1[200];
	int iProcessOrder1 = this->iProcessOrder;
	strcpy(chVidName1, this->chVidName);

	//结束
	// Free the RGB image
	av_free(buffer);
	av_free(pFrameBGR);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrameOri);

	// Close the codec
	avcodec_close(pCodecCtx);

	av_close_input_file(pFormatCtx);

	if(imageFrame)
		delete imageFrame;

	//重新开始
	this->iProcessOrder = iProcessOrder1;
	strcpy(chVidName, chVidName1);
	this->fRate = 0;
	iTotalFrameNum = 0;
	iNowFrameNum = 0;
	frameFinished = 0;
	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(av_open_input_file(&pFormatCtx, chVidName, NULL, 0, NULL)!=0)
	{
		bIfSuccess = false;
		return; // Couldn't open file
	}

	// Retrieve stream information
	if(av_find_stream_info(pFormatCtx)<0)
	{
		bIfSuccess = false;
		return; // Couldn't find stream information
	}

	// Dump information about file onto standard error
	dump_format(pFormatCtx, 0, chVidName, 0);

	this->iTotalFrameNum = pFormatCtx->streams[0]->duration;
	this->fFrmRat = pFormatCtx->streams[0]->r_frame_rate.num/(float)(pFormatCtx->streams[0]->r_frame_rate.den);

	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
	{
		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	}
	if(videoStream==-1)
	{
		bIfSuccess = false;
		return; // Didn't find a video stream
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL) {
		bIfSuccess = false;
		fprintf(stderr, "Unsupported codec!\n");
		return; // Codec not found
	}
	// Open codec
	while (avcodec_open(pCodecCtx, pCodec) < 0)/*这个函数总是返回-1*/ {
		Sleep(this->iProcessOrder);
	}

	// Allocate video frame
	pFrameOri=avcodec_alloc_frame();

	// Allocate an AVFrame structure
	pFrameBGR=avcodec_alloc_frame();
	if(pFrameBGR==NULL)
	{
		bIfSuccess = false;
		return;
	}
	pFrameRGB=avcodec_alloc_frame();
	if(pFrameRGB==NULL)
	{
		bIfSuccess = false;
		return;
	}

	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_BGR24, pCodecCtx->width,pCodecCtx->height);
	imageFrame->height = pCodecCtx->height;
	imageFrame->width = pCodecCtx->width;
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
	imageFrame->imageData = new uint8_t[numBytes*sizeof(uint8_t)];
	

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameBGR, buffer, PIX_FMT_BGR24,
		pCodecCtx->width, pCodecCtx->height);
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		pCodecCtx->width, pCodecCtx->height);
	
	//注意，这里是PIX_FMT_RGB24，它决定了图片的格式
	if(this->bIfUseHD == false)
		ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
		PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);

	this->getOneFrame();

	bIfSuccess = true;

}

FFmpegVideo::~FFmpegVideo()
{
	// Free the RGB image
	av_free(buffer);
	av_free(pFrameBGR);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrameOri);

	// Close the codec
	
	if(pCodecCtx)
		avcodec_close(pCodecCtx);
	

	// Close the video file
	//for(i = 0; i < pFormatCtx->nb_streams; i++) 
	//{            
	//	av_freep(&pFormatCtx->streams[i]->codec);                       
	//	av_freep(&pFormatCtx->streams[i]);                           
	//}
	if(pFormatCtx)
		av_close_input_file(pFormatCtx);

	if(imageFrame)
		delete imageFrame;


}

void FFmpegVideo::gotoCertSeg(float fRate1/* =0 */)
{

		if(fRate1 < 0)
			fRate1 = 0;
		if(fRate1 >= 1)
			fRate1 = 1;
		int iIfRead;
		while(this->fRate < fRate1)
		{
			av_read_frame(pFormatCtx, &packet);
			iNowFrameNum ++;
			this->fRate = iNowFrameNum/(float)iTotalFrameNum;
		}

		this->getOneFrame();

}

void FFmpegVideo::release()
{
	delete this;
}


int FFmpegVideo::getOneFrame()
{
	bool  bHaveReadVideoFrame = false;

	if(nFps == 0)
		nFps = pFormatCtx->streams[0]->time_base.den;


	{
		//TIMEBEGIN(t_pr, "decode") //测试时间
		int iIfRead = 0;
		int y;
		while(bHaveReadVideoFrame == false)
		{
			iIfRead = av_read_frame(pFormatCtx, &packet);
			iNowFrameNum ++;

			this->fRate = iNowFrameNum/(float)iTotalFrameNum;
			if(iIfRead < 0)
				return -1;
			//if(frameFinished)//判断视频祯是否读完
			//	return -1;

			// Is this a packet from the video stream?
			if(packet.stream_index==videoStream) {

				bHaveReadVideoFrame = true;
				// Decode video frame
				avcodec_decode_video(pCodecCtx, pFrameOri, &frameFinished, packet.data, packet.size);

				// Did we get a video frame?
				if(frameFinished) {
					// Convert the image from its native format to RGB
					// img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24,
					//                   (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width,
					//                  pCodecCtx->height);

					//Get BGR24 pixs
					sws_scale(ctx, pFrameOri->data, pFrameOri->linesize,
						0, pCodecCtx->height,pFrameBGR->data,
						pFrameBGR->linesize);

					//Get RGB24 pixs
					sws_scale(ctx, pFrameOri->data, pFrameOri->linesize,
						0, pCodecCtx->height,pFrameRGB->data,
						pFrameRGB->linesize);
					imageFrame->widthStep = pFrameRGB->linesize[0];

					//    sws_scale(ctx, pFrame->data, pFrame->linesize,
					//     0, pCodecCtx->height,pict.data, pict.linesize);

					// Save the frame to disk
					memcpy(imageFrame->imageData, pFrameBGR->data[0], pFrameBGR->linesize[0]*imageFrame->height);

					//if(++i<=5)
					for(y=0; y<imageFrame->height; y++)
					{
						//fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
				//		memcpy(pFrame->imageData + pFrameBuffer->widthStep*y, pFrameBGR->data[0]+y*pFrameBGR->linesize[0], sizeof(unsigned char)*width*3);

					}
				}
				//else
				//	return -1;
			}
		}

	

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
		//TIMEEND(t_pr, "decode") //测试时间
	}

	return 0;
}



