
// VLPRDemoDlg.h : ͷ�ļ�
//

#pragma once

#include "FFmpegVideo.h"
#include "TFLPRecognition.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <queue>
#include <iostream>
#include <list>
#include "Config.h"
#include "LoadingDlg.h"
using namespace std;

enum RecognitionMode{ VIDEO,  PICTURE};

// CVLPRDemoDlg �Ի���
class CVLPRDemoDlg : public CDialog
{
// ����
public:
	CVLPRDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

	CLoadingDlg loading;

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
	int imageWidth;
	int imageHeight;
	bool fastProcess;//�Ƿ���п��ٴ���//��������ģʽ��ȱ��ᶪ֡
	clock_t timeStart, timeNow;


	FFmpegVideo		*m_videoplay;
	TF_RecParma		*pTF_RecParma;
	TF_Result		*pTF_Result;
	unsigned char   *pImageBuffer;
	void			*pLPRInstance;
	queue<TF_Result*> LPRQueueResult;
	queue<unsigned char*> imagesQueue;
	queue<unsigned char*> imagesQueuePlay;
	unsigned char*  imageDataForShow;
		
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	void	PlayVideo(unsigned char*);
	void	ShowPicture(unsigned char*);
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
};
