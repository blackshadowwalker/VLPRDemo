
// VLPRDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"

#pragma comment(lib, "TFLPRecognition.lib")
#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

char *LPlateType[10]={"δ֪����","��ͨ����","��ͨ����","�������","˫�����","��ɫ����","��ɫ�侯","��ɫ����","������"};
char *LVehicleColor[8]={"δ֪","��","��","��","��","��","��","��"};
char *Directory[3]={"δ֪","�����˶�", "��Զ�˶�"};

HANDLE handleExit = 0;
HANDLE hProcessing=0;

#define MESSAGE_RESULT  0x5001


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

	handleExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	hProcessing =  CreateEvent(NULL, FALSE, FALSE, NULL);
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



// CVLPRDemoDlg ��Ϣ�������

BOOL CVLPRDemoDlg::OnInitDialog()
{
	bWindowInited = false;

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


	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_list.SetExtendedStyle(dwStyle); //������չ���

	m_list.InsertColumn( 0, "����ͼƬ", LVCFMT_LEFT, 50 );//������
	m_list.InsertColumn( 1, "����", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 2, "���Ŷ�", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( 3, "��ʱ", LVCFMT_LEFT, 50 );
	m_list.InsertColumn( 4, "��������", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( 5, "��������ɫ", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 6, "�������ɫ", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( 7, "�˶�����", LVCFMT_LEFT, 90 );
	m_list.InsertColumn( 8, "ʱ��", LVCFMT_LEFT, 120 );
	

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
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

static long nFrames = 0;

LRESULT CVLPRDemoDlg::ProcessResult(WPARAM wParam, LPARAM lParam)
{
	debug("ProcessResult");
	return 0;
}

void PlayThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread();
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->imagesQueue.empty())
			continue;
		if(WaitForSingleObject(hProcessing,-1)==0)
		{
			unsigned char* pix = dlg->imagesQueue.front();
			dlg->PlayVideo(pix);
			//dlg->imagesQueue.pop();
		}
	}
	debug("PlayThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,�ڴ�(����2��)����,Ҫ������SetEvnetһ��,���򷵻س�ʱ
}

unsigned char *plate = new unsigned char[400*200*3];

//����ʶ�����߳�
void ProcessResultThread(void *pParam)
{
	char filename[256]={0};
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread();

	char temp[256]={0};

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->LPRQueueResult.size()<1)
			continue;

		TF_Result result = dlg->LPRQueueResult.front();

		debug("ProcessResultThread Frame=%d  Plate=%s  (%d,%d)-(%d,%d)", nFrames, result.number, \
			result.PlateRect.iLeft, result.PlateRect.iTop,
			result.PlateRect.iRight, result.PlateRect.iBottom);

		
		dlg->m_list.SetItemState(0, 0, LVIS_SELECTED|LVIS_FOCUSED); //ȡ��ѡ��

		int count = dlg->m_list.GetItemCount();
		int nRow = dlg->m_list.InsertItem(0, "");//����ͼƬ
		dlg->m_list.SetItemText(nRow, 1, result.number);//����
		sprintf(temp, "%g", result.fConfidence);
		dlg->m_list.SetItemText(nRow, 2, temp);//���Ŷ�
		sprintf(temp, "%3dms", result.takesTime);
		dlg->m_list.SetItemText(nRow, 3, temp);//��ʱ
		sprintf(temp, "%s", LPlateType[result.ePlateType]);
		dlg->m_list.SetItemText(nRow, 4, temp);//��������
		sprintf(temp, "%s", LVehicleColor[result.eVColor1]);
		dlg->m_list.SetItemText(nRow, 5, temp);//��������ɫ
		sprintf(temp, "%s", LVehicleColor[result.eVColor2]);
		dlg->m_list.SetItemText(nRow, 6, temp);//�������ɫ
		sprintf(temp, "%s", Directory[result.iMoveDir+1]);
		dlg->m_list.SetItemText(nRow, 7, temp);//�˶�����
		TF_Time *t = &result.sTime;
		sprintf(temp, "%d-%d-%d %d:%d:%d", t->iYear, t->iMonth, t->iDay, t->iHour, t->iMilliseconds, t->iSecond);
		dlg->m_list.SetItemText(nRow, 8, temp);//ʱ��

		dlg->m_list.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED); //ѡ��
		
		dlg->ShowPicture(result.pResultBits);

		//get Plate
		int w = result.PlateRect.iRight - result.PlateRect.iLeft;
		int h = result.PlateRect.iBottom - result.PlateRect.iTop;
		int linestep = WIDTHSTEP(w);
		//	unsigned char *plate = new unsigned char[w*h*3];
		memset(plate, 0 , w*h*3);
		for(int i=0; i<h; i++){
			memcpy( (plate + i * w *3),
				(result.pResultBits + result.PlateRect.iLeft*3 + (result.PlateRect.iTop+i) * dlg->m_videoplay->imageFrame->widthStep),
				linestep );
		}
		dlg->ShowPlatePicture(plate, w, h);
		//	dlg->ShowPlatePicture(result.pResultBits,dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height); 

		sprintf(filename, "E:/temp/images/%04d-%s-plate.bmp", nFrames, result.number);
		VideoUtil::write24BitBmpFile(filename, w, h,(unsigned char*)plate);//����ͼƬ
		//	delete plate;

		sprintf(filename, "E:/temp/images/%04d-%s.bmp", nFrames, result.number);

		VideoUtil::write24BitBmpFile(filename, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height,\
			(unsigned char*)result.pResultBits);//ץ����дͼ

		dlg->LPRQueueResult.pop();

	}
	debug("ProcessResultThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,��2������,Ҫ������SetEvnetһ��,���򷵻س�ʱ
}

//ʶ���߳�
void RecognitionThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread();
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("���ڷ���.....");

	TF_Rect recROI;
	recROI.iLeft = 0;
	recROI.iTop = dlg->m_videoplay->imageFrame->height/2;
	recROI.iRight = dlg->m_videoplay->imageFrame->width;
	recROI.iBottom = dlg->m_videoplay->imageFrame->height;

	nFrames = 0;
	clock_t t1,t2;
	unsigned char *p  =0;
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		while(dlg->bPause){
			if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0)
				goto end;
			Sleep(5000);
		}
		if(dlg->bStop)
			break;

		nFrames ++;
		//TFLPRecognition.h
		memset(dlg->pTF_Result->number, 0, 20);//��գ���������Ƶ�����л�����������������
		dlg->pTF_Result->fConfidence = 0.0;////��գ���������Ƶ�����л�����������������

		if(dlg->imagesQueue.size()>0)
		{
			SetEvent(hProcessing);
			p = dlg->imagesQueue.front();
		//	debug("get front image queue size=%d  @ 0x%x", dlg->imagesQueue.size(), p );
			
			t1 = clock();
			TFLPR_RecImage( p, dlg->m_videoplay->imageFrame->width, dlg->m_videoplay->imageFrame->height, dlg->pTF_Result, &recROI, dlg->pLPRInstance);
			t2 = clock();
			dlg->pTF_Result->takesTime = t2-t1;
			ResetEvent(hProcessing);
			//�ͷ�
			dlg->imagesQueue.pop();
			if(p!=NULL);
			delete p;

			//����ʶ����
			if(dlg->pTF_Result->fConfidence> 0){
				dlg->LPRQueueResult.push(*dlg->pTF_Result);
			}

		}

	}
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("�������");

end:
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("��������");
	dlg->bPlay = false;
	dlg->bStop = true;
	dlg->bPause = false;
	debug("RecognitionThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); 
}

//��Ƶ����ȡ�߳�
void VideoThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	_beginthread(ProcessResultThread, 0, pParam);
	_beginthread(PlayThread, 0, pParam);
	_beginthread(RecognitionThread, 0, pParam);

	HANDLE handleCanExit = dlg->ReginsterMyThread();
	long size = dlg->pTF_RecParma->iImageHeight * dlg->pTF_RecParma->iImageWidth * 3;
	unsigned char* pixBits  = 0;
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->imagesQueue.size()>10)
			continue;
		if(WaitForSingleObject(hProcessing,0) != 0)
		{
			if(dlg->m_videoplay->getOneFrame()!=0)
				break;
			pixBits = new unsigned char[size];
			memcpy(pixBits,  dlg->m_videoplay->pFrameBGR->data[0], size);
			dlg->imagesQueue.push(pixBits);
		//	debug("push image queue size=%d  @ 0x%x", dlg->imagesQueue.size(), pixBits );
		}
	}

	debug("VideoThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,��2������,Ҫ������SetEvnetһ��,���򷵻س�ʱ
}


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//����Ƶ������
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) 
		return ;

	SetEvent(handleExit);//��ֹͣ��һ���߳�

	mVideoPath = fileOpenDlg.GetPathName();
	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=2;
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
//��ʾͼƬ
void CVLPRDemoDlg::ShowPicture(unsigned char *pix){

	BITMAPINFO *m_bmphdr = GetBitMapInfo(m_videoplay->imageFrame->width,m_videoplay->imageFrame->height );
	CRect rc;
	//��ʾ��ЧͼƬ
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
//��ʾ����ͼƬ
void CVLPRDemoDlg::ShowPlatePicture(unsigned char *pix, int w, int h){

	BITMAPINFO *m_bmphdr = GetBitMapInfo(w, h);
	CRect rc;
	//��ʾ����ͼƬ
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
//������Ƶ
void CVLPRDemoDlg::PlayVideo(unsigned char *pix){
	if(m_videoplay==NULL || m_videoplay->imageFrame==NULL)
		return;

	BITMAPINFO *m_bmphdr = GetBitMapInfo(m_videoplay->imageFrame->width,m_videoplay->imageFrame->height );

	CRect rc;
	videoWall->GetWindowRect(&rc);

	HDC hdc = videoWall->GetDC()->m_hDC;
	SetStretchBltMode(hdc, STRETCH_HALFTONE); //�ؼӣ�StretchBlt, StretchDIBits���Զ�ͼ�����ݽ�������, ѹ����ʾʧ��

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
	SetEvent(handleExit);
	bStop = true;
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);

}

void CVLPRDemoDlg::OnClose()
{
	OnBnClickedStop();
	Sleep(1000);
	GetDlgItem(ID_STATUS)->SetWindowText("�����˳�......");
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

	debug("���������˳� @ CDialog::OnClose()");;
	CDialog::OnClose();
}


//ÿ���̶߳�Ҫע�ᣬ�˳���ʱ������һһ���
//���أ���������̵߳�CEvent
HANDLE CVLPRDemoDlg::ReginsterMyThread()
{
	// �����¼�
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}
