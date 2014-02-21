//TFLPRecognition.h

#pragma once

#ifndef TF_LPRECOGNITION_H
#define TF_LPRECOGNITION_H
#include <windows.h>

#define  TF_OK               0   //��ʼ����ȷ
#define  TF_KEY_ERROR       -1   //δ�ҵ����ܹ�
#define  TF_THREAD_ERROR    -2   //�����̳߳�����Ŀ
#define  TF_PROCESS_ERROR   -3   //��ǰ���̲�����ʶ��

//��������
typedef enum
{
	E_TF_NOTKNOWN,     //δ֪����
	E_TF_NORMAL_BLUE,  //��ͨ����
	E_TF_NORMAL_BLACK, //��ͨ����
	E_TF_NORMAL_YELLOW,//�������
	E_TF_DOUBLE_YELLOW,//˫�����
	E_TF_POLICE,       //��ɫ����
	E_TF_WJ,           //��ɫ�侯
	E_TF_ARMY,         //��ɫ����
	E_TF_OTHER,        //������
}TF_LPlateType;


typedef enum
{
	E_TF_BGR,
	E_TF_RGB,
}TF_ImageType;

typedef struct
{
	char cVersion[64];     //�汾��
	char cComplieDate[64]; //����ʱ��
	char cCorpName[256];   //��˾����
}TF_SDK_Details;


//������ɫ
typedef enum
{
	E_TF_VC_NOTKNOWN, //δ֪
	E_TF_VC_WHITE,    //��
	E_TF_VC_SILVER,   //��
	E_TF_VC_YELLOW,   //��
	E_TF_VC_RED,	  //��
	E_TF_VC_GREEN,    //��
	E_TF_VC_BLUE,     //��
	E_TF_VC_BLACK,    //��
}TF_LVehicleColor;

typedef struct  
{
	int iLeft;
	int iTop;
	int iRight;
	int iBottom;
}TF_Rect;

typedef struct TF_RecParma
{
	TF_ImageType eImageType;  //ͼ������
	int iImageMode;           //0��Ϊ֡ģʽ 1��Ϊ��ģʽ
	int iRecMode;             //ʶ��ģʽ��0������ͼƬʶ��1:��Ƶ���+ʶ��2����Ƶ��֡�ں�ʶ��(ע��1ģʽ��2ģʽ�죬��ʶ���ʺͲ����ʱ�2��)
	int iResultNum;           //���������������֧�����8�����
	int iMinPlateWidth;       //ʶ����С���ƿ��60
	int iMaxPlateWidth;       //ʶ������ƿ��400
	int iImageWidth;          //ͼƬ��� ֻ�����Ƶʶ��ģʽ����(ģʽ1 ģʽ2)
	int iImageHeight;         //ͼƬ�߶� ֻ�����Ƶʶ��ģʽ����(ģʽ1 ģʽ2)
	char pLocalChinese[3];    //���غ����ַ���������ַ�����Ϊ�ջ���31��ʡ��֮����֣���ʹ���׺���

	TF_RecParma(){
		eImageType  = E_TF_BGR;
	}
}TF_RecParma;

typedef struct
{
	int iYear;
	int iMonth;
	int iDay;
	int iHour;
	int iMinute;
	int iSecond;
	int iMilliseconds;
}TF_Time;

typedef struct  TF_Result
{
	char number[20];            //ʶ����
	TF_Rect PlateRect;         //����λ��
	float fConfidence;          //�������Ŷ�
	TF_LPlateType ePlateType;  //��������
	int iMoveDir;               //�����˶����� -1��δ֪, 0�������˶���1����Զ�˶���
	TF_LVehicleColor eVColor1; //��������ɫ
	TF_LVehicleColor eVColor2; //�������ɫ
	unsigned char *pResultBits; //��plateColorƵʶ��ģʽ��ץ�ĵ�ͼƬ,��Ƶģʽʶ������Ч����Ҫ����
	TF_Time sTime;             //��Ƶʶ��ģʽ��ץ�Ĵ���ͼ��ʱ��     
	long	takesTime; //��ʱ
	char	nCarLogo[20]; //����

    TF_Result(){
		memset(this, 0, sizeof(TF_Result));
	}
}TF_Result;


typedef struct LPR_Result
{
	char plate[32];//����
	float confidence;//���Ŷ�
	CRect plateRect;//��������
	char  plateType[32];//��������
//	char  plateColor[32];//������ɫ
	
	char  carLogo[32];//����
	char  carColor1[32];//������ɫ
	char  carColor2[32];//��������ɫ
	char  direct[32];//�˶�����

	unsigned char * pResultBits;
	int imageWidth;
	int imageHeight;

	char  takesTime;//��ʱ
	
	LPR_Result(){
		memset(this, 0, sizeof(LPR_Result));
	}

}LPR_Result;

extern "C"  __declspec (dllexport) int   TFLPR_ThreadInit();  //�̳߳�ʼ����ֻ����һ��
extern "C"  __declspec (dllexport) void* TFLPR_Init(TF_RecParma RecParma);  //��ʼ��,ÿ���̶߳��������һ��
extern "C"  __declspec (dllexport) int   TFLPR_RecImage(const unsigned char *pBits, int iWidth, int iHeight ,TF_Result* pLprResult,TF_Rect* recROI,void* pInstance);//ʶ��
extern "C"  __declspec (dllexport) void  TFLPR_Uninit(void* pInstance);       //�ͷ�
extern "C"  __declspec (dllexport) void  TFLPR_GetSDKdetails(TF_SDK_Details* SDK_Details);      //��ð汾��Ϣ


#endif