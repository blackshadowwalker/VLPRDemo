
// VLPRDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"

#pragma comment(lib, "TFLPRecognition.lib")
#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

HANDLE handleExit = 0;

#define MESSAGE_RESULT  0x5001


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

	handleExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(handleExit);

}

void CVLPRDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, ID_LPR, m_plate);
}

BEGIN_MESSAGE_MAP(CVLPRDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MESSAGE_RESULT,  &CVLPRDemoDlg::ProcessResult)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(BT_OPEN_PICTURE, &CVLPRDemoDlg::OnBnClickedOpenPicture)
	ON_BN_CLICKED(BT_OPEN_VIDEO, &CVLPRDemoDlg::OnBnClickedOpenVideo)
	ON_BN_CLICKED(BT_PAUSE, &CVLPRDemoDlg::OnBnClickedPause)
	ON_BN_CLICKED(BT_STOP, &CVLPRDemoDlg::OnBnClickedStop)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_GROUP_OPERATE, &CVLPRDemoDlg::OnBnClickedGroupOperate)
END_MESSAGE_MAP()



// CVLPRDemoDlg 消息处理程序

BOOL CVLPRDemoDlg::OnInitDialog()
{
	bWindowInited = false;

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
	pictureWall  = (CStatic*)GetDlgItem(ID_PICTURE);
	plateWall  = (CStatic*)GetDlgItem(ID_LPR_PICTURE);


	mGroupVideo			= (CButton*)GetDlgItem(ID_GROUP_VIDEO);
	mGroupPicture		= (CButton*)GetDlgItem(ID_GROUP_PICTURE);
	mGroupLPRPicture	= (CButton*)GetDlgItem(ID_GROUP_LPR_PICTURE);
	mGroupLPR			= (CButton*)GetDlgItem(ID_GROUP_LPR);
	mGroupOperate		= (CButton*)GetDlgItem(ID_GROUP_OPERATE);

	InitMyWindows();

	bWindowInited = true;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

#define BASE_WIDTH 15

void CVLPRDemoDlg:: InitOpearteWindows(UINT id, int &dtx, int &dty, bool bNewCol){
	CRect rc, rg;
	bt = (CButton*)GetDlgItem(id);
	mGroupOperate->GetWindowRect(&rg);
	bt->GetWindowRect(&rc);
	ScreenToClient(&rc);
	//	bt->SetParent((CButton*)mGroupOperate);
	int w = rc.Width();
	int h = rc.Height();
	rc.left = rg.left + dtx;
	rc.top = rg.top + dty;
	rc.right = w  + rc.left;
	rc.bottom = h + rc.top;
	bt->MoveWindow(rc);
	dty += BASE_WIDTH + rc.Height();
	if(bNewCol){
		dtx += BASE_WIDTH + rc.Width();
		dty = BASE_WIDTH;
	}
}

void CVLPRDemoDlg::InitMyWindows(){

	int dtx = BASE_WIDTH, dty=BASE_WIDTH;
	InitOpearteWindows(BT_OPEN_VIDEO, dtx, dty);
	InitOpearteWindows(BT_PAUSE, dtx, dty);
	InitOpearteWindows(BT_STOP, dtx, dty, true);

	InitOpearteWindows(BT_OPEN_PICTURE, dtx, dty);

	//GetDlgItem(BT_PAUSE)->SetParent((CButton*)mGroupOperate);
	//GetDlgItem(BT_PAUSE)->SetParent((CButton*)mGroupOperate);

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

static long nFrames = 0;

LRESULT CVLPRDemoDlg::ProcessResult(WPARAM wParam, LPARAM lParam)
{
	debug("ProcessResult");
	return 0;

	/*
	TF_Result *result = (TF_Result*)wParam;


	debug("ProcessResult  : %s ", result->number );

	m_plate.SetWindowText(pTF_Result->number);
	//	PlayVideo();


	TF_Result *re = new TF_Result();
	memcpy(re, pTF_Result, sizeof(TF_Result));
	if(pTF_RecParma->iRecMode > 0){
	pTF_RecParma->iImageHeight = m_videoplay->imageFrame->height;
	pTF_RecParma->iImageWidth = m_videoplay->imageFrame->width;
	pTF_Result->pResultBits = new unsigned char[pTF_RecParma->iImageHeight * pTF_RecParma->iImageWidth * 3];
	}


	sprintf(filename, "E:/temp/images/%04d.bmp", nFrames);

	VideoUtil::write24BitBmpFile(filename, m_videoplay->imageFrame->width, m_videoplay->imageFrame->height,\
	(unsigned char*)m_videoplay->pFrameBGR->data[0]);

	sprintf(filename, "E:/temp/images/%04d-%s.bmp", nFrames, pTF_Result->number);

	//	VideoUtil::write24BitBmpFile(filename, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height,\
	(unsigned char*)dlg->pTF_Result->pResultBits);

	*/
	return 0L;
}

void PlayThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread();
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->imagesQueue.empty())
			continue;
		unsigned char* pix = dlg->imagesQueue.front();
		dlg->PlayVideo(pix);
		if(!dlg->imagesQueue.empty())
			dlg->imagesQueue.pop();
	}
	debug("PlayThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,在次(即第2次)在用,要重新在SetEvnet一次,否则返回超时
}

unsigned char *plate = new unsigned char[400*200*3];

void ProcessResultThread(void *pParam)
{
	char filename[256]={0};
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread();

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->LPRQueueResult.size()<1)
			continue;

		TF_Result result = dlg->LPRQueueResult.front();

		debug("ProcessResultThread Frame=%d  Plate=%s  (%d,%d)-(%d,%d)", nFrames, dlg->pTF_Result->number, \
			dlg->pTF_Result->PlateRect.iLeft, dlg->pTF_Result->PlateRect.iTop,
			dlg->pTF_Result->PlateRect.iRight, dlg->pTF_Result->PlateRect.iBottom);

		dlg->ShowPicture(result.pResultBits);

		//get Plate
		int w = result.PlateRect.iRight - result.PlateRect.iLeft;
		int h = result.PlateRect.iBottom - result.PlateRect.iTop;
		int linestep = WIDTHSTEP(w);
	//	unsigned char *plate = new unsigned char[w*h*3];
		memset(plate, 0 , w*h*3);
		for(int i=0; i<h; i++){
			memcpy( (plate + i*w*3),
				(result.pResultBits + result.PlateRect.iLeft*3 + (result.PlateRect.iTop+i) * dlg->m_videoplay->imageFrame->widthStep),
					w*3);
		}
		dlg->ShowPlatePicture(plate, w, h);
	//	dlg->ShowPlatePicture(result.pResultBits,dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height); 

		sprintf(filename, "E:/temp/images/%04d-%s-plate.bmp", nFrames, result.number);
		VideoUtil::write24BitBmpFile(filename, w, h,(unsigned char*)plate);//车牌图片
	//	delete plate;

		sprintf(filename, "E:/temp/images/%04d-%s.bmp", nFrames, result.number);

		VideoUtil::write24BitBmpFile(filename, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height,\
			(unsigned char*)result.pResultBits);//抓拍特写图

		dlg->LPRQueueResult.pop();

	}
	debug("ProcessResultThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,在次(即第2次)在用,要重新在SetEvnet一次,否则返回超时
}

void VideoThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	_beginthread(ProcessResultThread, 0, pParam);
	_beginthread(PlayThread, 0, pParam);

	HANDLE handleCanExit = dlg->ReginsterMyThread();

	long size = dlg->pTF_RecParma->iImageHeight * dlg->pTF_RecParma->iImageWidth * 3;
	unsigned char* pixBits = new unsigned char[size];
	nFrames = 0;
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("正在分析.....");

	TF_Rect recROI;
		recROI.iLeft = 0;
		recROI.iTop = dlg->m_videoplay->imageFrame->height/2;
		recROI.iRight = dlg->m_videoplay->imageFrame->width;
		recROI.iBottom = dlg->m_videoplay->imageFrame->height;

	while(dlg->m_videoplay->getOneFrame()==0){

		if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0)
			break;

		while(dlg->bPause){
			if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0)
				goto end;
			Sleep(200);
		}
		if(dlg->bStop)
			break;

		//	ResetEvent(eventCanExit);//若此时收到退出消息，需要等待线程本次处理完成才退出
		nFrames ++;
		//TFLPRecognition.h
		memset(dlg->pTF_Result->number, 0, 20);
		dlg->pTF_Result->fConfidence = 0.0;
		memcpy(pixBits,  dlg->m_videoplay->pFrameBGR->data[0], size);
		dlg->imagesQueue.push(pixBits);

		TFLPR_RecImage( pixBits, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height, dlg->pTF_Result, &recROI, dlg->pLPRInstance);

		int ret = 0;
		//		ret = dlg->PostMessage( MESSAGE_RESULT,  (WPARAM)( dlg->pTF_Result), 1L);//无响应?
		if(ret != ERROR_SUCCESS)
		{
			long error = GetLastError();
			debug("error : [%d][%d]",  ret,error);
		}
		if(dlg->pTF_Result->fConfidence> 0){
			dlg->LPRQueueResult.push(*dlg->pTF_Result);
		}
	}
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("分析完成");

end:
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("分析结束");
	dlg->bPlay = false;
	dlg->bStop = true;
	dlg->bPause = false;
	SetEvent(handleCanExit);////失去信号,设置可以退出了
	debug("VideoThread 正常退出");
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,在次(即第2次)在用,要重新在SetEvnet一次,否则返回超时
}


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//打开视频并分析
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) 
		return ;

	SetEvent(handleExit);//先停止上一个线程

	mVideoPath = fileOpenDlg.GetPathName();
	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=2;
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
		if(pTF_RecParma->iRecMode > 0){
			pTF_RecParma->iImageHeight = m_videoplay->imageFrame->height;
			pTF_RecParma->iImageWidth = m_videoplay->imageFrame->width;
			pTF_Result->pResultBits = new unsigned char[pTF_RecParma->iImageHeight * pTF_RecParma->iImageWidth * 3];
		}
		pLPRInstance = TFLPR_Init(*pTF_RecParma);

	}

	bPlay = true;
	bStop = false;
	bPause = false;

	ResetEvent(handleExit);
	GetDlgItem(BT_PAUSE)->EnableWindow(true);
	GetDlgItem(BT_STOP)->EnableWindow(true);
	_beginthread(VideoThread, 0 ,this); 
}

BITMAPINFO *GetBitMapInfo(int width, int height, int bitCount=24){
	BITMAPINFO *m_bmphdr=0;  
	DWORD dwBmpHdr = sizeof(BITMAPINFO);  
	m_bmphdr = new BITMAPINFO[dwBmpHdr];  
	m_bmphdr->bmiHeader.biBitCount = 24;  
	m_bmphdr->bmiHeader.biClrImportant = 0;  
	m_bmphdr->bmiHeader.biSize = dwBmpHdr;  
	m_bmphdr->bmiHeader.biSizeImage = 0;  
	m_bmphdr->bmiHeader.biWidth = width;  
	m_bmphdr->bmiHeader.biHeight = height;  
	m_bmphdr->bmiHeader.biXPelsPerMeter = 0;  
	m_bmphdr->bmiHeader.biYPelsPerMeter = 0;  
	m_bmphdr->bmiHeader.biClrUsed = 0;  
	m_bmphdr->bmiHeader.biPlanes = 1;  
	m_bmphdr->bmiHeader.biCompression = BI_RGB;  
	return m_bmphdr;
}
//显示图片
void CVLPRDemoDlg::ShowPicture(unsigned char *pix){

	BITMAPINFO *m_bmphdr = GetBitMapInfo(m_videoplay->imageFrame->width,m_videoplay->imageFrame->height );
	CRect rc;
	//显示特效图片
	HDC hdc = pictureWall->GetDC()->m_hDC;
	SetStretchBltMode(hdc, STRETCH_DELETESCANS);
	pictureWall->GetWindowRect(&rc);
	::StretchDIBits(hdc,  
		0,  
		0,  
		rc.Width(),//rc.right - rc.left,  
		rc.Height(),//rc.top,  
		0, m_videoplay->imageFrame->height,  
		m_videoplay->imageFrame->width, -m_videoplay->imageFrame->height,  
		pix,  
		m_bmphdr,  
		DIB_RGB_COLORS,  
		SRCCOPY);  
}
//显示车牌图片
void CVLPRDemoDlg::ShowPlatePicture(unsigned char *pix, int w, int h){

	BITMAPINFO *m_bmphdr = GetBitMapInfo(w, h);
	CRect rc;
	//显示车牌图片
	HDC	hdc = plateWall->GetDC()->m_hDC;
	plateWall->GetWindowRect(&rc);
	//int w = pTF_Result->PlateRect.iRight - pTF_Result->PlateRect.iLeft;
	//int h = pTF_Result->PlateRect.iBottom - pTF_Result->PlateRect.iTop;

	SetStretchBltMode(hdc, STRETCH_HALFTONE);
	::StretchDIBits( hdc,  
		0,  
		0,  
		rc.Width(),//rc.right - rc.left,  
		rc.Height(),//rc.top,  
		0, h,
		w, -h,  
		pix,  
		m_bmphdr,  
		DIB_RGB_COLORS,  
		SRCCOPY); 

}
//播放视频
void CVLPRDemoDlg::PlayVideo(unsigned char *pix){
	if(m_videoplay==NULL || m_videoplay->imageFrame==NULL)
		return;

	BITMAPINFO *m_bmphdr = GetBitMapInfo(m_videoplay->imageFrame->width,m_videoplay->imageFrame->height );

	CRect rc;
	videoWall->GetWindowRect(&rc);

	HDC hdc = videoWall->GetDC()->m_hDC;
	SetStretchBltMode(hdc, STRETCH_HALFTONE); //必加，StretchBlt, StretchDIBits可以对图像数据进行拉伸, 压缩显示失真

	int nResult = ::StretchDIBits( hdc,  
		0,  
		0,  
		rc.Width(), 
		rc.Height(), 
		0, m_videoplay->imageFrame->height,  
		m_videoplay->imageFrame->width, -m_videoplay->imageFrame->height,  
		pix, //m_videoplay->pFrameRGB->data[0],  
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

	int times=0;
	for(list<HANDLE>::iterator it = EventList.begin(); it != EventList.end();)
	{
		HANDLE h = (HANDLE)*it;
		int ret =WaitForSingleObject(h, 1000);
		switch(ret){
		case WAIT_FAILED:
			debug(" WAIT_FAILED");
			break;
		case WAIT_TIMEOUT:
			debug("times=%d WAIT_TIMEOUT", times);
			break;
		case WAIT_OBJECT_0:
			debug(" WAIT_OBJECT_0  singed , so can exit");
			EventList.erase(it);
			break;
		default :
			debug("ret=%d, 0x%x  !=WAIT_OBJECT_0  LastError=",ret, h, GetLastError());
			break;
		}
		it++;
	}
}

void CVLPRDemoDlg::OnBnClickedStop()
{
	//停止
	SetEvent(handleExit);
	bStop = true;
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);

}

void CVLPRDemoDlg::OnClose()
{
	OnBnClickedStop();

	GetDlgItem(ID_STATUS)->SetWindowText("正在退出......");
	int times=0;
	while(EventList.size()>0 && times<10)
	{
		times ++;
		SetEvent(handleExit);
		for(list<HANDLE>::iterator it = EventList.begin(); it != EventList.end();)
		{
			HANDLE h = (HANDLE)*it;
			int ret =WaitForSingleObject(h, 1000);
			bool bSigned=false;
			switch(ret)
			{
			case WAIT_FAILED:
				debug(" WAIT_FAILED");
				break;
			case WAIT_TIMEOUT:
				debug("times=%d WAIT_TIMEOUT", times);
				break;
			case WAIT_OBJECT_0:
				debug(" WAIT_OBJECT_0  singed , so can exit");
				it = EventList.erase(it);
				bSigned = true;
				break;
			default :
				debug("ret=%d, 0x%x  !=WAIT_OBJECT_0  LastError=",ret, h, GetLastError());
				break;
			}
			if(bSigned==false)
				it++;

		}
	}

	debug("马上正常退出 @ CDialog::OnClose()");;
	CDialog::OnClose();
}


//每个线程都要注册，退出的时候会进行一一检测
//返回：分配给该线程的CEvent
HANDLE CVLPRDemoDlg::ReginsterMyThread()
{
	// 创建事件
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	SetEvent(hEvent);
	EventList.push_back(hEvent);
	debug("EventList.size = %d", EventList.size());
	return hEvent;

}

void CVLPRDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if(m_list.m_hWnd == NULL)
		return;
	CRect mRect, rg ,rc;
	int dt = 20;
	this->GetWindowRect(&mRect);
	ScreenToClient(&mRect);
	mGroupOperate->GetWindowRect(&rg);
	ScreenToClient(&rg);
	rc.SetRect(mRect.right - rg.Width() -dt, rg.top, mRect.right - dt, mRect.bottom - dt);
	mGroupOperate->MoveWindow(rc);
	InitMyWindows();

}

void CVLPRDemoDlg::OnBnClickedGroupOperate()
{
	// TODO: 在此添加控件通知处理程序代码
}
