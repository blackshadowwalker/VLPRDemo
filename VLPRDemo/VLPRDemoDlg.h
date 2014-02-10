
// VLPRDemoDlg.h : ͷ�ļ�
//

#pragma once

#include "FFmpegVideo.h"
#include "TFLPRecognition.h"


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

	CString mVideoPath;
	CString mPicturePath;

	FFmpegVideo		*m_videoplay;
	TF_RecParma		*pTF_RecParma;
	TF_Result		*pTF_Result;
	void			*pLPRInstance;
		
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedStop();
	void ShowPicture();

	bool  bPlay;
	bool  bStop;
	bool  bPause;
	afx_msg void OnClose();

	CStatic *videoWall;
	CStatic *pictureWall;
	CStatic *plateWall;
};
