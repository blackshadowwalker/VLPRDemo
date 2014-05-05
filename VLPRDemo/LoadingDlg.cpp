// LoadingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "LoadingDlg.h"


// CLoadingDlg �Ի���

IMPLEMENT_DYNAMIC(CLoadingDlg, CDialog)

CLoadingDlg::CLoadingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadingDlg::IDD, pParent)
	, m_TextShow(_T(""))
{

}

CLoadingDlg::~CLoadingDlg()
{
}

void CLoadingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, ID_TEXT_SHOW, m_TextShow);
}


BEGIN_MESSAGE_MAP(CLoadingDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLoadingDlg ��Ϣ�������

INT_PTR CLoadingDlg::DoModal()
{
	return CDialog::DoModal();
}

BOOL CLoadingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(1, 1000, NULL);

	return TRUE;  
}

void CLoadingDlg::OnTimer(UINT_PTR nIDEvent)
{
	OnOK();
	CDialog::OnTimer(nIDEvent);
}
