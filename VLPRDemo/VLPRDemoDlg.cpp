
// VLPRDemoDlg.cpp : ʵ���ļ�
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
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


// CVLPRDemoDlg �Ի���




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


// CVLPRDemoDlg ��Ϣ�������

BOOL CVLPRDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	videoWall = (CStatic*)GetDlgItem(ID_VIDEO_WALL);
	

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CVLPRDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	ShowPicture();
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CVLPRDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVLPRDemoDlg::OnBnClickedOpenPicture()
{
	//��ͼƬ������
	//FileUtil::SelectFolder(this->m_hWnd, "ѡ��ͼƬ");
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
	//����Ƶ������
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
		sprintf(pTF_RecParma->pLocalChinese, "��");
	}
	if(pTF_Result == NULL){
		pTF_Result = new TF_Result();
	}

	char cBuff[256]={0};

	m_videoplay = new FFmpegVideo(mVideoPath.GetBuffer(mVideoPath.GetLength()), 0);
	if(m_videoplay->bIfSuccess == false)
	{
		sprintf(cBuff,"����Ƶ�ļ�[%s]ʧ��!\n",mVideoPath);
		MessageBox(cBuff,"����",MB_OK);
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

	SetStretchBltMode(videoWall->GetDC()->m_hDC, STRETCH_HALFTONE); //�ؼӣ�StretchBlt, StretchDIBits���Զ�ͼ�����ݽ�������, ѹ����ʾʧ��

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
	//��ͣ/�ָ�
	if(bPause){
		bPause = false;
		GetDlgItem(BT_PAUSE)->SetWindowText("��ͣ");
	}else{
		bPause = true;
		GetDlgItem(BT_PAUSE)->SetWindowText("�ָ�");
	}
}

void CVLPRDemoDlg::OnBnClickedStop()
{
	//ֹͣ
	bStop = true;
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);
	SetEvent(eventExit);
}

void CVLPRDemoDlg::OnClose()
{
	SetEvent(eventExit);
	//OnBnClickedStop();
	GetDlgItem(ID_STATUS)->SetWindowText("�����˳�......");
	if(WaitForSingleObject(eventCanExit, 5*1000)==WAIT_OBJECT_0)
		debug("���������˳�");;
	CDialog::OnClose();
}
