
// VLPRDemoDlg.h : ͷ�ļ�
//

#pragma once

#include "FFmpegVideo.h"
#include "TFLPRecognition.h"
#include "afxcmn.h"
#include "afxwin.h"

#include "Config.h"
#include "LoadingDlg.h"
#include "TH_PlateID.h"

//=====  STL ==============
#include <queue>
#include <iostream>
#include <list>
using namespace std;
//=========================

typedef struct LPR_Image
{
	unsigned char* buffer;
	int  imageWidth;
	int  imageHeight;
	int  imageSize;
	LPR_Image(){
		memset(this, 0, sizeof(LPR_Image));
	}
}LPR_Image;


enum RecognitionMode{ VIDEO,  PICTURE};
enum COMPANY {NONE, TELEFRAME, WENTONG};

// CVLPRDemoDlg �Ի���
class CVLPRDemoDlg : public CDialog
{
// ����
public:
	CVLPRDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

	CLoadingDlg loading;
	void StartProcessVideo(CString fileName);
	COMPANY company;

	bool  TF_LPR_canRun;
	bool  TH_LPR_canRun;

// �Ի�������
	enum { IDD = IDD_VLPRDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpenPicture();
	afx_msg void OnBnClickedOpenVideo();
	void LPRFromImage(Bitmap* image);

	RecognitionMode recognitionMode;

	void CRect2TF_Rect(CRect &pt , TF_Rect &tfPt, int imageWidth, int imageHeight, bool b2=true);
	CRect m_ROIRect;
	int pointPtr;
	bool startDraw;

	TH_PlateIDCfg plateConfigTh;
	bool TH_InitDll(int bMovingImage);

	list<HANDLE> EventList;
	HANDLE ReginsterMyThread(char *);

	CString mVideoPath;
	CString mPicturePath;
	list<char *> mListPicturesPath;
	void listFiles(CString firstFile);
	
	bool ReadConfig();
	bool SaveConfig();
	CConfig configDlg;//
	CString m_imageDir;
	int plateMaxWidth;
	int plateMinWidth;
	bool fastProcess;//�Ƿ���п��ٴ���//��������ģʽ��ȱ��ᶪ֡
	clock_t timeStart, timeNow;

	char pLocalChinese[3];    //���غ����ַ���������ַ�����Ϊ�ջ���31��ʡ��֮����֣���ʹ���׺���
	FFmpegVideo		*m_videoplay;
	TF_RecParma		*pTF_RecParma;
	TF_Result		*pTF_Result;
	TF_Rect			ROIRect;
	unsigned char   *pImageBuffer;
	void			*pLPRInstance;
	queue<LPR_Result*> LPRQueueResult;
	queue<LPR_Image*> imagesQueue;
	queue<LPR_Image*> imagesQueuePlay;

	int imageWidth;
	int imageHeight;
		
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	void	PlayVideo(unsigned char*, int imageWidht, int imageHeight);
	void	ShowPicture(unsigned char *pix, int imageWidht, int imageHeight);
	void	ShowPlatePicture(unsigned char*, int w, int h);

	bool  bPlay;
	bool  bStop;
	bool  bPause;
	afx_msg void OnClose();

	CStatic *videoWall;
	CStatic *pictureWall;
	CStatic *plateWall;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT ProcessResult(WPARAM wParam, LPARAM lParam); 

	CButton *mGroupVideo;
	CButton *mGroupPicture;
	CButton *mGroupLPRPicture;
	CButton *mGroupLPR;
	CButton *mGroupOperate;

	CButton *bt;
	void InitMyWindows();
	void InitOpearteWindows(UINT id, int &dtx, int &dty, bool bNewCol=false);
	bool bWindowInited;
	CListCtrl m_list;
	afx_msg void OnBnClickedGroupOperate();
	CStatic m_plate;

	int CloseThread();
	void ReStartThread();
	afx_msg void OnBnClickedNextPicture();
	
	afx_msg void OnEnChangeImageDir();
	afx_msg void OnEnKillfocusImageDir();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedSetConfig();

	afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedStartAnnay();
	afx_msg void OnStnClickedVideoWall();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedSetArea();
	afx_msg void OnBnClickedCarlogoDetect();
};
