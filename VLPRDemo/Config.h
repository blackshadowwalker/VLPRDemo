#pragma once
#include "afxwin.h"


// CConfig 对话框

class CConfig : public CDialog
{
	DECLARE_DYNAMIC(CConfig)

public:
	CConfig(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CConfig();

// 对话框数据
	enum { IDD = IDD_DIALOG_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBrowserFolder();

	int m_imageMaxWidth;
	int m_imageMinWidth;
	CString m_imageDir;
	afx_msg void OnBnClickedButton2();
	CComboBox m_combox;
	afx_msg void OnCbnSelchangeCombo1();
	virtual BOOL OnInitDialog();
};
