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
		int height; //ͼ��߶�
		int width; //ͼ����
		long size;
		long widthStep;
		uint8_t *imageData; //����ͼ��֡
	}ImageFrame;

public:
	char chVidName[1024]; //�����ļ���
	ImageFrame *imageFrame;
	float fRate; //���ű���
	UINT32 iTotalFrameNum; //��֡��
	UINT32 iNowFrameNum; //�Ѳ���֡��
	float fFrmRat; //֡��
	int iProcessOrder;//�̺߳�
	bool bIfSuccess; //�Ƿ�ɹ�

	bool bIfUseHD; //�Ƿ������

public:

	FFmpegVideo(void);
	FFmpegVideo(char* chVidName1, int iProcessOrder1, float fRate1=0);
	~FFmpegVideo(void);

	void gotoCertSeg(float fRate1=0); //��fRate1��ʼ����
	int getOneFrame(); //�õ�һ֡ͼ��
	void restart();//���¿�ʼ
	void release(); //�ͷ�

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
