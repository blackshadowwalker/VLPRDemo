
// VLPRDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"

#pragma comment(lib, "TFLPRecognition.lib")

CEvent eventCanExit;
CEvent eventExit;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVLPRDemoDlg 对话框




CVLPRDemoDlg::CVLPRDemoDlg(CWnd* pParent /*=NULL*/)
: CDialog(CVLPRDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_videoplay = 0;
	pTF_RecParma = NULL;
	pTF_Result = NULL;
	bPlay = false;
	bStop = true;
	bPause = false;
	pLPRInstance = NULL;
}

void CVLPRDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVLPRDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(BT_OPEN_PICTURE, &CVLPRDemoDlg::OnBnClickedOpenPicture)
	ON_BN_CLICKED(BT_OPEN_VIDEO, &CVLPRDemoDlg::OnBnClickedOpenVideo)
	ON_BN_CLICKED(BT_PAUSE, &CVLPRDemoDlg::OnBnClickedPause)
	ON_BN_CLICKED(BT_STOP, &CVLPRDemoDlg::OnBnClickedStop)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVLPRDemoDlg 消息处理程序

BOOL CVLPRDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	videoWall = (CStatic*)GetDlgItem(ID_VIDEO_WALL);
	

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVLPRDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVLPRDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	ShowPicture();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVLPRDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVLPRDemoDlg::OnBnClickedOpenPicture()
{
	//打开图片并分析
	//FileUtil::SelectFolder(this->m_hWnd, "选择图片");
	CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"image files (*.jpg;*.bmp) |*.jpg;*.bmp;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) return ;

	mPicturePath = fileOpenDlg.GetPathName();

}

void VideoThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	char filename[256]={0};
	long nFrames = 0;

	while(dlg->m_videoplay->getOneFrame()==0){

		if(WaitForSingleObject(eventExit,1)==WAIT_OBJECT_0)
			break;

		while(dlg->bPause){
			if(dlg->bStop)
				break;
			Sleep(200);
		}
		//if(dlg->bStop)
		//	break;

		//	GetFileSnapPicPath(m_cMovieFile,m_nChannel,m_BackPicPath);
		nFrames ++;
		sprintf(filename, "E:/temp/%04d.bmp", nFrames);
//		VideoUtil::write24BitBmpFile(filename, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height,\
			(unsigned char*)dlg->m_videoplay->imageFrame->imageData);

		dlg->ShowPicture();

		//TFLPRecognition.h
		TFLPR_RecImage(dlg->m_videoplay->pFrameBGR->data[0], dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height, 
			dlg->pTF_Result, 0, dlg->pLPRInstance);
		//	if(dlg->pTF_Result->fConfidence > 0 )
		debug(" Frame=%d  Plate=%s",nFrames, dlg->pTF_Result->number);

	}

	SetEvent(eventCanExit);
	debug("VideoThread Exit and SetEvent(eventCanExit)");
}


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//打开视频并分析
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) return ;

	mVideoPath = fileOpenDlg.GetPathName();
	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=1;
		pTF_RecParma->iMaxPlateWidth = 400;
		pTF_RecParma->iMinPlateWidth = 60;
		pTF_RecParma->iResultNum=8;
		sprintf(pTF_RecParma->pLocalChinese, "京");
	}
	if(pTF_Result == NULL){
		pTF_Result = new TF_Result();
	}

	char cBuff[256]={0};

	m_videoplay = new FFmpegVideo(mVideoPath.GetBuffer(mVideoPath.GetLength()), 0);
	if(m_videoplay->bIfSuccess == false)
	{
		sprintf(cBuff,"打开视频文件[%s]失败!\n",mVideoPath);
		MessageBox(cBuff,"错误",MB_OK);
		delete m_videoplay;
		m_videoplay = NULL;
		return ;
	}
	else
	{
		m_videoplay->getOneFrame();
		//	GetFileSnapPicPath(m_cMovieFile,m_nChannel,m_BackPicPath);
		VideoUtil::write24BitBmpFile("E:/temp/k1.bmp",m_videoplay->imageFrame->width,m_videoplay->imageFrame->height,
			(unsigned char*)m_videoplay->imageFrame->imageData);
		if(pTF_RecParma->iImageMode > 0){
			pTF_RecParma->iImageHeight = m_videoplay->imageFrame->height;
			pTF_RecParma->iImageWidth = m_videoplay->imageFrame->width;
			pTF_Result->pResultBits = new unsigned char[pTF_RecParma->iImageHeight * pTF_RecParma->iImageWidth * 3];
		}
		pLPRInstance = TFLPR_Init(*pTF_RecParma);

	}

	bPlay = true;
	bStop = false;
	bPause = false;
	ResetEvent(eventExit);
	GetDlgItem(BT_PAUSE)->EnableWindow(true);
	GetDlgItem(BT_STOP)->EnableWindow(true);
	_beginthread(VideoThread, 0 ,this); 
}

void CVLPRDemoDlg::ShowPicture(){
	if(m_videoplay==NULL || m_videoplay->imageFrame==NULL)
		return;

	BITMAPINFO *m_bmphdr;  
	DWORD dwBmpHdr = sizeof(BITMAPINFO);  
	m_bmphdr = new BITMAPINFO[dwBmpHdr];  
	m_bmphdr->bmiHeader.biBitCount = 24;  
	m_bmphdr->bmiHeader.biClrImportant = 0;  
	m_bmphdr->bmiHeader.biSize = dwBmpHdr;  
	m_bmphdr->bmiHeader.biSizeImage = 0;  
	m_bmphdr->bmiHeader.biWidth = m_videoplay->imageFrame->width;  
	m_bmphdr->bmiHeader.biHeight = m_videoplay->imageFrame->height;  
	m_bmphdr->bmiHeader.biXPelsPerMeter = 0;  
	m_bmphdr->bmiHeader.biYPelsPerMeter = 0;  
	m_bmphdr->bmiHeader.biClrUsed = 0;  
	m_bmphdr->bmiHeader.biPlanes = 1;  
	m_bmphdr->bmiHeader.biCompression = BI_RGB;  

	CRect rc;
	videoWall->GetWindowRect(&rc);

	SetStretchBltMode(videoWall->GetDC()->m_hDC, STRETCH_HALFTONE); //必加，StretchBlt, StretchDIBits可以对图像数据进行拉伸, 压缩显示失真

	int nResult = ::StretchDIBits( videoWall->GetDC()->m_hDC,  
		0,  
		0,  
		rc.Width(),//rc.right - rc.left,  
		rc.Height(),//rc.top,  
		0, m_videoplay->imageFrame->height,  
		m_videoplay->imageFrame->width, -m_videoplay->imageFrame->height,  
		m_videoplay->pFrameRGB->data[0],  
		m_bmphdr,  
		DIB_RGB_COLORS,  
		SRCCOPY);  
}

void CVLPRDemoDlg::OnBnClickedPause()
{
	//暂停/恢复
	if(bPause){
		bPause = false;
		GetDlgItem(BT_PAUSE)->SetWindowText("暂停");
	}else{
		bPause = true;
		GetDlgItem(BT_PAUSE)->SetWindowText("恢复");
	}
}

void CVLPRDemoDlg::OnBnClickedStop()
{
	//停止
	bStop = true;
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);
	SetEvent(eventExit);
}

void CVLPRDemoDlg::OnClose()
{
	SetEvent(eventExit);
	//OnBnClickedStop();
	GetDlgItem(ID_STATUS)->SetWindowText("正在退出......");
	if(WaitForSingleObject(eventCanExit, 5*1000)==WAIT_OBJECT_0)
		debug("马上正常退出");;
	CDialog::OnClose();
}
