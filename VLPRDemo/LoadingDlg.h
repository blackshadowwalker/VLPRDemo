#pragma once

#ifdef _WIN32_WCE
#error "Windows CE ��֧�� CDHtmlDialog��"
#endif 

// CLoadingDlg �Ի���

class CLoadingDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CLoadingDlg)

public:
	CLoadingDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CLoadingDlg();
// ��д
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// �Ի�������
	enum { IDD = IDD_LOADING_DLG, IDH = IDR_HTML_LOADINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
