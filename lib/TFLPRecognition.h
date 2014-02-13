//TFLPRecognition.h

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

typedef struct 
{
	int iImageMode;           //0��Ϊ֡ģʽ 1��Ϊ��ģʽ
	int iRecMode;             //ʶ��ģʽ��0������ͼƬʶ��1:��Ƶ���+ʶ��2����Ƶ��֡�ں�ʶ��(ע��1ģʽ��2ģʽ�죬��ʶ���ʺͲ����ʱ�2��)
	int iResultNum;           //���������������֧�����4�����
	int iMinPlateWidth;       //ʶ����С���ƿ��
	int iMaxPlateWidth;       //ʶ������ƿ��
	int iImageWidth;          //ͼƬ��� ֻ�����Ƶʶ��ģʽ����(ģʽ1 ģʽ2)
	int iImageHeight;         //ͼƬ�߶� ֻ�����Ƶʶ��ģʽ����(ģʽ1 ģʽ2)
	char pLocalChinese[3];    //���غ����ַ���������ַ�����Ϊ�ջ���31��ʡ��֮����֣���ʹ���׺���
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

typedef struct  
{
	char number[20];            //ʶ����
	TF_Rect PlateRect;         //����λ��
	float fConfidence;          //�������Ŷ�
	TF_LPlateType ePlateType;  //��������
	int iMoveDir;               //�����˶����� 0�������˶���1����Զ�˶���-1��δ֪
	TF_LVehicleColor eVColor1; //��������ɫ
	TF_LVehicleColor eVColor2; //�������ɫ
	unsigned char *pResultBits; //��Ƶʶ��ģʽ��ץ�ĵ�ͼƬ,��Ƶģʽʶ������Ч����Ҫ����
	TF_Time sTime;             //��Ƶʶ��ģʽ��ץ�Ĵ���ͼ��ʱ��       
}TF_Result;

extern "C"  __declspec (dllexport) int   TFLPR_ThreadInit();  //�̳߳�ʼ����ֻ����һ��
extern "C"  __declspec (dllexport) void* TFLPR_Init(TF_RecParma RecParma);  //��ʼ��,ÿ���̶߳��������һ��
extern "C"  __declspec (dllexport) int   TFLPR_RecImage(const unsigned char *pBits, int iWidth, int iHeight ,TF_Result* pLprResult,TF_Rect* recROI,void* pInstance);//ʶ��
extern "C"  __declspec (dllexport) void  TFLPR_Uninit(void* pInstance);       //�ͷ�


#endif