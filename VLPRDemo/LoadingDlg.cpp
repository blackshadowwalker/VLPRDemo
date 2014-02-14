// LoadingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "LoadingDlg.h"


// CLoadingDlg �Ի���

IMPLEMENT_DYNCREATE(CLoadingDlg, CDHtmlDialog)

CLoadingDlg::CLoadingDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CLoadingDlg::IDD, CLoadingDlg::IDH, pParent)
{

}

CLoadingDlg::~CLoadingDlg()
{
}

void CLoadingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CLoadingDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

BEGIN_MESSAGE_MAP(CLoadingDlg, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CLoadingDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// CLoadingDlg ��Ϣ�������

HRESULT CLoadingDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT CLoadingDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
