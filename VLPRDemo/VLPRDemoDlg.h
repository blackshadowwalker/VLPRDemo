
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
using namespace std;


// CVLPRDemoDlg �Ի���
class CVLPRDemoDlg : public CDialog
{
// ����
public:
	CVLPRDemoDlg(CWnd* pParent = NULL);	// ��׼���캯��

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

	list<HANDLE> EventList;
	HANDLE ReginsterMyThread();

	CString mVideoPath;
	CString mPicturePath;

	FFmpegVideo		*m_videoplay;
	TF_RecParma		*pTF_RecParma;
	TF_Result		*pTF_Result;
	void			*pLPRInstance;
	queue<TF_Result> LPRQueueResult;
	queue<unsigned char*> imagesQueue;
		
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
};
