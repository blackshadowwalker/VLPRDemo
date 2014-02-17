// Config.cpp : 实现文件
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "Config.h"

#include "FileUtil.h"


#define MAX_THEME_LEN  2
char *themeName[MAX_THEME_LEN]={"", "skin/AquaOS.ssk"};


// CConfig 对话框

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


// CConfig 消息处理程序


void CConfig::OnBnClickedOk()
{
	UpdateData(true);

	if(m_imageMaxWidth>400 || m_imageMaxWidth<90){
		MessageBox("车牌最大宽度范围是(90 - 400)");
		return ;
	}
	if(m_imageMinWidth > 120 || m_imageMinWidth < 60){
		MessageBox("车牌最小宽度范围是(60 - 120)");
		return ;
	}

	m_localCH.GetWindowText(pLocalChinese, 3);

	OnOK();
}

void CConfig::OnBnClickedBrowserFolder()
{
	UpdateData(true);
	char *path = FileUtil::SelectFolder(this->m_hWnd, "选择输出文件夹");
	if(path!=0)
		m_imageDir.Format("%s",path);
	UpdateData(false);
}

void CConfig::OnBnClickedButton2()
{
	//恢复默认配置
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

	m_combox.AddString("默认主题");
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
