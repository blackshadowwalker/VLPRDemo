#pragma once


// CLoadingDlg �Ի���

class CLoadingDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoadingDlg)

public:
	CLoadingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLoadingDlg();

// �Ի�������
	enum { IDD = IDD_LOADING_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_TextShow;
	virtual INT_PTR DoModal();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
