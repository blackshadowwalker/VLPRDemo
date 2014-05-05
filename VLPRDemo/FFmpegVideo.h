#pragma once

//FFmpeg
#ifdef __cplusplus
extern "C"
{
	#include "libavutil/avutil.h"
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}
#endif
#pragma comment(lib, "avcore.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")


class FFmpegVideo
{	
	typedef struct ImageFrame{
		int height; //图像高度
		int width; //图像宽度
		long size;
		long widthStep;
		uint8_t *imageData; //读出图像帧
	}ImageFrame;

public:
	char chVidName[1024]; //播放文件名
	ImageFrame *imageFrame;
	float fRate; //播放比率
	UINT32 iTotalFrameNum; //总帧数
	UINT32 iNowFrameNum; //已播放帧数
	float fFrmRat; //帧率
	int iProcessOrder;//线程号
	bool bIfSuccess; //是否成功

	bool bIfUseHD; //是否处理高清

public:

	FFmpegVideo(void);
	FFmpegVideo(char* chVidName1, int iProcessOrder1, float fRate1=0);
	~FFmpegVideo(void);

	void gotoCertSeg(float fRate1=0); //从fRate1开始播放
	int getOneFrame(); //得到一帧图像
	void restart();//重新开始
	void release(); //释放

	long Save2jpeg (uint8_t *buffer, int width, int height, char *fileName);
	long SaveFrame2jpeg (AVCodecContext *pCodecCtxIn, AVFrame *pFrame, char *fileName);

	int  nFps;

public:
	AVFrame         *pFrameBGR;
	AVFrame         *pFrameRGB;
private:
	AVFormatContext *pFormatCtx;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame         *pFrameOri;
	
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer;
	struct SwsContext      *ctx;

};
