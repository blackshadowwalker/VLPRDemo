// Config.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "Config.h"

#include "FileUtil.h"


#define MAX_THEME_LEN  2
char *themeName[MAX_THEME_LEN]={"", "skin/AquaOS.ssk"};


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
	DDX_Control(pDX, IDC_COMBO1, m_combox);
	DDX_Control(pDX, IDC_COMB_LOCAL, m_localCH);
}


BEGIN_MESSAGE_MAP(CConfig, CDialog)
	ON_BN_CLICKED(IDOK, &CConfig::OnBnClickedOk)
	ON_BN_CLICKED(BT_BROWSER_FOLDER, &CConfig::OnBnClickedBrowserFolder)
	ON_BN_CLICKED(IDC_BUTTON2, &CConfig::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CConfig::OnCbnSelchangeCombo1)
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

	m_localCH.GetWindowText(pLocalChinese, 3);

	OnOK();
}

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

#include "VLPRDemoDlg.h"
void CConfig::OnCbnSelchangeCombo1()
{
	UpdateData(true);
	char	skinPath[256] = {0};
	int index = m_combox.GetCurSel();
	if(index<0 || index> (MAX_THEME_LEN-1))
		return ;
	sprintf(skinPath, "%s/%s",CVLPRDemoApp::m_appPath, themeName[index]); 
	if(index==0)
	{
		skinppExitSkin();
	}else{
		skinppLoadSkin( themeName[index] );
	}
	Invalidate();
}



BOOL CConfig::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_combox.AddString("Ĭ������");
	m_combox.AddString("AquaOS");
	m_combox.SetCurSel(0);

	int index = 0;
	for(int i=0; i<31; i++){
		m_localCH.AddString(character[i]);
		if(strcmp(pLocalChinese, character[i])==0)
			index =i;
	}
	m_localCH.SetCurSel(index);

	return TRUE;  
}
