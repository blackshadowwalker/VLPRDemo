#pragma once

#ifdef _WIN32_WCE
#error "Windows CE 不支持 CDHtmlDialog。"
#endif 

// CLoadingDlg 对话框

class CLoadingDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CLoadingDlg)

public:
	CLoadingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLoadingDlg();
// 重写
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// 对话框数据
	enum { IDD = IDD_LOADING_DLG, IDH = IDR_HTML_LOADINGDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
