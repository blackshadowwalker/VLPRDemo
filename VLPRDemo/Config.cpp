// Config.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "Config.h"


// CConfig �Ի���

IMPLEMENT_DYNAMIC(CConfig, CDialog)

CConfig::CConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CConfig::IDD, pParent)
	, m_imageMaxWidth(400)
	, m_imageMinWidth(60)
	, m_imageDir(_T("E:/temp/image"))
{

}

CConfig::~CConfig()
{
}

void CConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, EDIT_IMAGE_MAX_WIDTH, m_imageMaxWidth);
	DDX_Text(pDX, EDIT_IMAGE_MAX_HEIGHT, m_imageMinWidth);
	DDX_Text(pDX, EDIT_DIR, m_imageDir);
}


BEGIN_MESSAGE_MAP(CConfig, CDialog)
	ON_BN_CLICKED(IDOK, &CConfig::OnBnClickedOk)
	ON_BN_CLICKED(BT_BROWSER_FOLDER, &CConfig::OnBnClickedBrowserFolder)
	ON_BN_CLICKED(IDC_BUTTON2, &CConfig::OnBnClickedButton2)
END_MESSAGE_MAP()


// CConfig ��Ϣ�������

void CConfig::OnBnClickedOk()
{
	UpdateData(true);
	if(m_imageMaxWidth>400 || m_imageMaxWidth<90){
		MessageBox("��������ȷ�Χ��(90 - 400)");
		return ;
	}
	if(m_imageMinWidth > 120 || m_imageMinWidth < 60){
		MessageBox("������С��ȷ�Χ��(60 - 120)");
		return ;
	}

	OnOK();
}
#include "FileUtil.h"
void CConfig::OnBnClickedBrowserFolder()
{
	UpdateData(true);
	char *path = FileUtil::SelectFolder(this->m_hWnd, "ѡ������ļ���");
	if(path!=0)
		m_imageDir.Format("%s",path);
	UpdateData(false);
}

void CConfig::OnBnClickedButton2()
{
	//�ָ�Ĭ������
	m_imageMaxWidth = 400;
	m_imageMinWidth = 60;
	m_imageDir = CVLPRDemoApp::m_appPath + "/images";
	UpdateData(false);
}
