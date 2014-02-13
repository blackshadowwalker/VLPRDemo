
// VLPRDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"
#include "public.h"

#pragma comment(lib, "TFLPRecognition.lib")
#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

char *LPlateType[10]={"未知类型","普通蓝牌","普通黑牌","单层黄牌","双层黄牌","白色警牌","白色武警","白色军牌","其类型"};
char *LVehicleColor[8]={"未知","白","灰","黄","红","绿","蓝","黑"};
char *Directory[3]={"未知","朝近运动", "朝远运动"};

HANDLE handleExit = 0;
HANDLE hAccessImage;//互斥，访问图像队列
HANDLE handleVideoThread=0;
HANDLE hShowVideoFrame=0;

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
	//memset(this, 0, sizeof(CVLPRDemoDlg));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_videoplay = 0;
	pTF_RecParma = NULL;
	pTF_Result = NULL;
	bPlay = false;
	bStop = true;
	bPause = false;
	pLPRInstance = NULL;
	pImageBuffer = 0;

	imageDataForShow = 0;
	imageWidth = imageHeight =0;

	handleExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	hAccessImage =  CreateEvent(NULL, FALSE, FALSE, NULL);
	handleVideoThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	hShowVideoFrame  = CreateEvent(NULL, FALSE, FALSE, NULL);

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


	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
	m_list.SetExtendedStyle(dwStyle); //设置扩展风格

	m_list.InsertColumn( 0, "车牌图片", LVCFMT_LEFT, 50 );//插入列
	m_list.InsertColumn( 1, "车牌", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 2, "置信度", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( 3, "耗时", LVCFMT_LEFT, 50 );
	m_list.InsertColumn( 4, "车牌类型", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( 5, "车身主颜色", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 6, "车身次颜色", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 7, "运动方向", LVCFMT_LEFT, 90 );
	m_list.InsertColumn( 8, "时间", LVCFMT_LEFT, 120 );


	videoWall = (CStatic*)GetDlgItem(ID_VIDEO_WALL);
	pictureWall  = (CStatic*)GetDlgItem(ID_PICTURE);
	plateWall  = (CStatic*)GetDlgItem(ID_LPR_PICTURE);


	mGroupVideo			= (CButton*)GetDlgItem(ID_GROUP_VIDEO);
	mGroupPicture		= (CButton*)GetDlgItem(ID_GROUP_PICTURE);
	mGroupLPRPicture	= (CButton*)GetDlgItem(ID_GROUP_LPR_PICTURE);
	mGroupLPR			= (CButton*)GetDlgItem(ID_GROUP_LPR);
	mGroupOperate		= (CButton*)GetDlgItem(ID_GROUP_OPERATE);

	InitMyWindows();
	ReStartThread();

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




static long nFrames = 0;

LRESULT CVLPRDemoDlg::ProcessResult(WPARAM wParam, LPARAM lParam)
{
	debug("ProcessResult");
	return 0;
}

void PlayThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("PlayThread");
	debug("PlayThread 启动  handle=0x%x", handleCanExit);

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->imagesQueuePlay.empty()){
			Sleep(10);		
			continue;
		}
	//	debug("PlayThread waiting hShowVideoFrame");
		if(WaitForSingleObject(hShowVideoFrame,0)==0)
		{
			debug("PlayThread imagesQueuePlay.size = %d", dlg->imagesQueuePlay.size());
			if(dlg->imagesQueuePlay.size() > 0){
				unsigned char* pix = dlg->imagesQueuePlay.front();
				debug("PlayThread imagesQueuePlay.front  0x%x", pix);
				dlg->imagesQueuePlay.pop();
				dlg->PlayVideo(pix);
				debug("PlayThread imagesQueuePlay pre release  0x%x", pix);
			//	delete pix;
			}
		//	SetEvent(hAccessImage);
			
		}else{
			Sleep(100);
		}
	}
	debug("PlayThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,在次(即第2次)在用,要重新在SetEvnet一次,否则返回超时

}

unsigned char *plate = new unsigned char[400*200*3];

//处理识别结果线程
void ProcessResultThread(void *pParam)
{
	char filename[256]={0};
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("ProcessResultThread");

	char temp[256]={0};
	debug("ProcessResultThread 启动  handle=0x%x", handleCanExit);

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->LPRQueueResult.size()<1){
			Sleep(10);		
			continue;
		}

		TF_Result result = dlg->LPRQueueResult.front();

		debug("ProcessResultThread Frame=%d  Plate=%s  (%d,%d)-(%d,%d)", nFrames, result.number, \
			result.PlateRect.iLeft, result.PlateRect.iTop,
			result.PlateRect.iRight, result.PlateRect.iBottom);

		dlg->m_list.SetItemState(0, 0, LVIS_SELECTED|LVIS_FOCUSED); //取消选中
		int count = dlg->m_list.GetItemCount();
		int nRow = dlg->m_list.InsertItem(0, "");//车牌图片
		dlg->m_list.SetItemText(nRow, 1, result.number);//车牌
		sprintf(temp, "%g", result.fConfidence);
		dlg->m_list.SetItemText(nRow, 2, temp);//置信度
		sprintf(temp, "%3dms", result.takesTime);
		dlg->m_list.SetItemText(nRow, 3, temp);//耗时
		sprintf(temp, "%s", LPlateType[result.ePlateType]);
		dlg->m_list.SetItemText(nRow, 4, temp);//车牌类型
		sprintf(temp, "%s", LVehicleColor[result.eVColor1]);
		dlg->m_list.SetItemText(nRow, 5, temp);//车身主颜色
		sprintf(temp, "%s", LVehicleColor[result.eVColor2]);
		dlg->m_list.SetItemText(nRow, 6, temp);//车身次颜色
		sprintf(temp, "%s", Directory[result.iMoveDir+1]);
		dlg->m_list.SetItemText(nRow, 7, temp);//运动方向
		TF_Time *t = &result.sTime;
		sprintf(temp, "%d-%d-%d %d:%d:%d", t->iYear, t->iMonth, t->iDay, t->iHour, t->iMilliseconds, t->iSecond);
		dlg->m_list.SetItemText(nRow, 8, temp);//时间

		dlg->m_list.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED); //选中

		dlg->ShowPicture(result.pResultBits);

		//get Plate
		int w = result.PlateRect.iRight - result.PlateRect.iLeft;
		int h = result.PlateRect.iBottom - result.PlateRect.iTop;
		int linestep = WIDTHSTEP(w);
		unsigned char *pBit = 0;

		pBit = result.pResultBits;//图片

		//	unsigned char *plate = new unsigned char[w*h*3];
		if(dlg->LPRQueueResult.size()>0)
			dlg->LPRQueueResult.pop();
		if(pBit!=NULL)
		{
			memset(plate, 0 , w*h*3);
			for(int i=0; i<h; i++){
				memcpy( (plate + i * linestep ),
					(pBit + result.PlateRect.iLeft*3 + (result.PlateRect.iTop+i) * WIDTHSTEP(dlg->imageWidth)),
					linestep );
			}
			dlg->ShowPlatePicture(plate, w, h );

			sprintf(filename, "E:/temp/images/%04d-%s-plate.bmp", nFrames, result.number);
			VideoUtil::write24BitBmpFile(filename, w, h,(unsigned char*)plate, linestep);//车牌图片
			//	delete plate;

			sprintf(filename, "E:/temp/images/%04d-%s.bmp", nFrames, result.number);
			VideoUtil::write24BitBmpFile(filename,dlg->imageWidth, dlg->imageHeight,(unsigned char*)pBit,  WIDTHSTEP(dlg->imageWidth));//抓拍特写图

			delete pBit;
		}
	}
	debug("ProcessResultThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,第2次再用,要重新在SetEvnet一次,否则返回超时
	
}

//识别线程
void RecognitionThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("RecognitionThread");
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("正在分析.....");

	TF_Rect recROI;
	debug("RecognitionThread 启动  handle=0x%x", handleCanExit);
	nFrames = 0;
	int imageSize = 0;
	clock_t t1,t2;
	unsigned char *p  =0;
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){

		if(dlg->imagesQueue.size()<1){
			Sleep(10);		
			continue;
		}

		recROI.iLeft = 0;
		recROI.iTop = dlg->imageHeight/2;
		recROI.iRight = dlg->imageWidth;
		recROI.iBottom = dlg->imageHeight;

		nFrames ++;
		//TFLPRecognition.h
		memset(dlg->pTF_Result->number, 0, 20);//清空，否则在视频分析中会出现连续输出的问题
		dlg->pTF_Result->fConfidence = 0.0;////清空，否则在视频分析中会出现连续输出的问题

		if(dlg->imagesQueue.size()>0)
		{
			imageSize = dlg->imageWidth * dlg->imageHeight *3;
	//		debug("RecognitionThread wating  hAccessImage");\
			WaitForSingleObject(hAccessImage, INFINITE);//一直等待可以访问
			debug("RecognitionThread getted  & imagesQueue.size=%d ", dlg->imagesQueue.size());
			if(dlg->imagesQueue.size()<1){
				SetEvent(hAccessImage);
				continue;
			}
			p = dlg->imagesQueue.front();
			debug("RecognitionThread imagesQueue.front 0x%x", p);
			memcpy(dlg->imageDataForShow, p, imageSize);
			dlg->imagesQueuePlay.push(dlg->imageDataForShow);
			SetEvent(hShowVideoFrame);
			dlg->imagesQueue.pop();
			//	debug("get front image queue size=%d  @ 0x%x", dlg->imagesQueue.size(), p );
			t1 = clock();
			TFLPR_RecImage( p, dlg->imageWidth, dlg->imageHeight, dlg->pTF_Result, &recROI, dlg->pLPRInstance);//车牌识别
			t2 = clock();
	//		SetEvent(hAccessImage);
	//		debug("RecognitionThread Release  hAccessImage");
			dlg->pTF_Result->takesTime = t2-t1;
			debug("RecognitionThread imagesQueue pre release 0x%x", p);
			delete p;

			//处理识别结果
			if(dlg->pTF_Result->fConfidence> 0){
				TF_Result r;
				r = *dlg->pTF_Result;
				if(dlg->pTF_RecParma->iRecMode!=0){
					r.pResultBits = new unsigned char[imageSize];
					if(r.pResultBits!=NULL &&  dlg->pTF_Result->pResultBits!=NULL)
						memcpy(r.pResultBits , dlg->pTF_Result->pResultBits,imageSize );
				}
				dlg->LPRQueueResult.push(r);
				debug("RecognitionThread LPRQueueResult.push  0x%x", dlg->pTF_Result);
			}
		}
	}
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("分析完成");
end:
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("分析结束");
	debug("RecognitionThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); 
	
}

//视频流获取线程
void VideoThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	
	HANDLE handleCanExit = dlg->ReginsterMyThread("VideoThread");
	debug("VideoThread 启动  handle=0x%x", handleCanExit);
	long size = dlg->imageWidth * dlg->imageHeight * 3;
	unsigned char* pixBits  = 0;
	unsigned char* pixBitsShow  = 0;
	int ret=0;
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		while(dlg->bPause && WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
			Sleep(100);
		}
		if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0)
			break;
		
		//debug("VideoThread wating  handleVideoThread");
		if(WaitForSingleObject(handleVideoThread,0)==WAIT_OBJECT_0)
		{
		//	debug("VideoThread getted  handleVideoThread");
			ret = dlg->m_videoplay->getOneFrame();
			if(ret==0)
			{
				dlg->imageWidth = dlg->m_videoplay->imageFrame->width;
				dlg->imageHeight = dlg->m_videoplay->imageFrame->height;
				size = dlg->imageWidth * dlg->imageHeight * 3;
				if(size<1)
					continue;

				if(dlg->imagesQueue.size() < 30 ){
					pixBits = new unsigned char[size];\
					memcpy(pixBits,  dlg->m_videoplay->imageFrame->imageData, size);
				//	debug("VideoThread wating  hAccessImage");\
					WaitForSingleObject(hAccessImage, -1);//一直等待可以访问
					dlg->imagesQueue.push(pixBits);\
					debug("VideoThread imagesQueue.push 0x%x   Qsize=%d", pixBits, dlg->imagesQueue.size());
				}

			//	if(dlg->imagesQueuePlay.size() < 30){
			//		pixBitsShow = new unsigned char[size];\
			//		memcpy(pixBitsShow,  dlg->m_videoplay->imageFrame->imageData, size);
			////		SetEvent(hShowVideoFrame);
			//		dlg->imagesQueuePlay.push(pixBitsShow);\
			//		debug("VideoThread imagesQueuePlay.push 0x%x   Qsize=%d ", pixBitsShow, dlg->imagesQueuePlay.size());
			//	}
				//放入视频帧队列
				
			//	SetEvent(hAccessImage);
				//	debug("push image queue size=%d  @ 0x%x", dlg->imagesQueue.size(), pixBits );
				SetEvent(handleVideoThread);
			}else{
				if(ret==-2){
					debug("视频读取完成");
					MessageBox(0, "视频读取完成","", MB_OK);
				}
				else
					debug("视频读取失败");
			}
			
		}else{
			Sleep(10);	
		}
	}
end:
	ResetEvent(handleVideoThread);
	dlg->bPlay = false;
	dlg->bStop = true;
	dlg->bPause = false;
	debug("VideoThread 正常退出");
	SetEvent(handleCanExit);//设置可以退出了
	SetEvent(handleExit); //第1次SetEvent(hEvent)设置事件有信号并WaitForSingleObject后,第2次再用,要重新在SetEvnet一次,否则返回超时
}


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//打开视频并分析
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) 
		return ;

	int ret =0;

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

	ResetEvent(handleVideoThread);
	if(m_videoplay!=0)
		m_videoplay->release();
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
		ret = m_videoplay->getOneFrame();
		if(ret==0)
		{
			imageWidth = m_videoplay->imageFrame->width;
			imageHeight = m_videoplay->imageFrame->height;
			int size = imageWidth * imageHeight * 3;
			if(imageDataForShow!=NULL)
				delete imageDataForShow;
			imageDataForShow = new unsigned char[size];

			unsigned char* temp1=0 , *temp2=0;
			long t1, t2 , sumt=0;

			if(false)
			for(int i=0; i<200; i++){
				t1 = clock();
				temp1 = new unsigned char[imageWidth * imageHeight * 3];
				temp2 = new unsigned char[imageWidth * imageHeight * 3];
				t2 = clock();
				sumt += (t2-t1);
				debug("%d -> new temp takes time : %d ms",i, t2-t1);
				t1 = clock();
				memcpy(temp1, (unsigned char*)m_videoplay->imageFrame->imageData, imageWidth * imageHeight * 3);
				memcpy(temp2, (unsigned char*)m_videoplay->imageFrame->imageData, imageWidth * imageHeight * 3);
				t2 = clock();
				sumt += (t2-t1);
				debug("%d -> memcpy temp takes time : %d ms",i, t2-t1);
				delete temp1; 
				delete temp2;
			}
			debug("takes time : %d ms", sumt);

			VideoUtil::write24BitBmpFile("E:/temp/k1.bmp",imageWidth, imageHeight,(unsigned char*)m_videoplay->imageFrame->imageData);
			
			if(pTF_RecParma->iRecMode != 0){
				pTF_RecParma->iImageHeight = imageHeight;
				pTF_RecParma->iImageWidth = imageWidth;
				pTF_Result->pResultBits = new unsigned char[imageHeight * imageWidth* 3];
			}
			pLPRInstance = TFLPR_Init(*pTF_RecParma);
		}else{
			if(ret==-2){
				debug("视频读取完成");
				MessageBox("视频读取完成","",MB_OK);
			}else{
				debug("视频读取失败");
				MessageBox("视频读取失败","错误",MB_OK);
			}
			return ;
		}
	}

	bPlay = true;
	bStop = false;
	bPause = false;

	GetDlgItem(BT_PAUSE)->EnableWindow(true);
	GetDlgItem(BT_STOP)->EnableWindow(true);

	
//	ReStartThread();
	SetEvent(handleVideoThread);
	SetEvent(hAccessImage);
	//_beginthread(VideoThread, 0 ,this);//获取视频帧
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

	BITMAPINFO *m_bmphdr = GetBitMapInfo(imageWidth, imageHeight );
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
		0, imageHeight,  
		imageWidth, -imageHeight,  
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

	BITMAPINFO *m_bmphdr = GetBitMapInfo(imageWidth, imageHeight );

	CRect rc;
	videoWall->GetWindowRect(&rc);

	HDC hdc = videoWall->GetDC()->m_hDC;
	SetStretchBltMode(hdc, STRETCH_HALFTONE); //必加，StretchBlt, StretchDIBits可以对图像数据进行拉伸, 压缩显示失真

	int nResult = ::StretchDIBits( hdc,  
		0,  
		0,  
		rc.Width(), 
		rc.Height(), 
		0, imageHeight,  
		imageWidth, -imageHeight,  
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
}

void CVLPRDemoDlg::OnBnClickedStop()
{
	//停止
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);
	ResetEvent(handleVideoThread);
}

int CVLPRDemoDlg::CloseThread()
{
	int times=0;
	while(EventList.size()>0 && times<10)
	{
		times ++;
		for(list<HANDLE>::iterator it = EventList.begin(); it != EventList.end();)
		{
			SetEvent(handleExit);
			HANDLE h = (HANDLE)*it;
			int ret =WaitForSingleObject(h, 500);
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
				debug(" WAIT_OBJECT_0 handle=0x%x  singed , so can exit", h);
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
	return EventList.size();
}

void CVLPRDemoDlg::ReStartThread()
{
	if(CloseThread() > 0)
	{
		debug("重启线程失败.");
		GetDlgItem(ID_STATUS)->SetWindowText("重启线程失败.");
		//return ;
	}
	ResetEvent(handleExit);
//	SetEvent(hAccessImage);
	_beginthread(VideoThread, 0 ,this);//获取视频帧
	_beginthread(PlayThread, 0, this);//视频播放线程
	_beginthread(RecognitionThread, 0, this);//识别线程
	_beginthread(ProcessResultThread, 0, this);//处理线程
	
}

void CVLPRDemoDlg::OnClose()
{
	OnBnClickedStop();
	Sleep(1000);
	GetDlgItem(ID_STATUS)->SetWindowText("正在退出......");
	if( CloseThread()>0)
		debug("非正常退出 @ CDialog::OnClose()  线程未全部关闭");
	else
		debug("马上正常退出 @ CDialog::OnClose()");
	Sleep(1000);
	CDialog::OnClose();
}


//每个线程都要注册，退出的时候会进行一一检测
//返回：分配给该线程的CEvent
HANDLE CVLPRDemoDlg::ReginsterMyThread(char *name)
{
	// 创建事件
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, name);
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


void CVLPRDemoDlg::OnBnClickedOpenPicture()
{
	//打开图片并分析
	//FileUtil::SelectFolder(this->m_hWnd, "选择图片");
	CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"image files (*.jpg;*.bmp) |*.jpg;*.bmp;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) return ;

	mPicturePath = fileOpenDlg.GetPathName();
	char p[256] = {0};

	Bitmap* image = KLoadBitmap(mPicturePath.GetBuffer(mPicturePath.GetLength()));
	DrawImg2Hdc(image, ID_VIDEO_WALL, this);
	DrawImg2Hdc(image, ID_PICTURE, this);
	LPRFromImage(image);
}


void CVLPRDemoDlg::LPRFromImage(Bitmap* image)
{
	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		memset(pTF_RecParma, 0, sizeof(TF_RecParma));
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=0;//
		pTF_RecParma->iMaxPlateWidth = 400;
		pTF_RecParma->iMinPlateWidth = 60;
		pTF_RecParma->iResultNum=8;
		sprintf(pTF_RecParma->pLocalChinese, "京");
		pLPRInstance = TFLPR_Init(*pTF_RecParma);
	}
	if(pTF_Result == NULL){
		pTF_Result = new TF_Result();
	}	

	long BufferLen;
	imageWidth = image->GetWidth();
	imageHeight = image->GetHeight();
	BufferLen= imageWidth * imageHeight * 3;

	if(imageDataForShow!=NULL)
		delete imageDataForShow;
	imageDataForShow = new unsigned char[BufferLen];

	BitmapData bitmapData={0};
	bitmapData.Stride = WIDTHSTEP(imageWidth);
	bitmapData.Scan0 = new BYTE[bitmapData.Stride * imageHeight];
//	if(pImageBuffer!=0)\
		delete pImageBuffer;
	pImageBuffer=new unsigned char[BufferLen];
	Rect rect(0,0, imageWidth, imageHeight);
	image->LockBits(&rect, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bitmapData); 
	
	memcpy(pImageBuffer, (BYTE*)bitmapData.Scan0, BufferLen); 
	
	delete bitmapData.Scan0;
	image->UnlockBits( &bitmapData);

	imagesQueue.push(pImageBuffer);//放入队列进行处理
	
	/*
	long t1 = clock();
	TFLPR_RecImage( pImageBuffer, imageWidth, imageHeight, pTF_Result, 0, pLPRInstance);
	long t2 = clock();
	pTF_Result->takesTime = t2-t1;

	//处理识别结果
	if(pTF_Result->fConfidence> 0){
		LPRQueueResult.push(*pTF_Result);
	}
	*/
}

