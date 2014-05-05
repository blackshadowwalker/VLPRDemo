
// VLPRDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"
#include "public.h"

#pragma comment(lib, "TFLPRecognition.lib")
#pragma comment(lib, "TH_PLATEID.lib");
#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

char *TF_ERROR[5] = {"��ǰ���̲�����ʶ��", "�����̳߳�����Ŀ", "δ�ҵ����ܹ�", "��ʼ����ȷ",""};

char *LPlateType[10]={"δ֪����","��ͨ����","��ͨ����","�������","˫�����","��ɫ����","��ɫ�侯","��ɫ����","������"};
char *LVehicleColor[8]={"δ֪","��","��","��","��","��","��","��"};
char *Directory[3]={"δ֪","�����˶�", "��Զ�˶�"};

HANDLE handleExit = 0;
HANDLE hAccessImage;//���⣬����ͼ�����
HANDLE handleVideoThread=0;
HANDLE handleVideoThreadStoped=0;
HANDLE handleLPRThreadStoped=0;
HANDLE hShowVideoFrame=0;

#define MESSAGE_RESULT  0x5001

#define MAX_WIDTH_OF_PLATE  400
#define MAX_HEIGHT_OF_PLATE  400
#define MAX_SIZE_OF_PLATE	(MAX_WIDTH_OF_PLATE * MAX_HEIGHT_OF_PLATE * 3)
unsigned char *plate = new unsigned char[MAX_SIZE_OF_PLATE*10];

#define PointColor		RGB(255,0,0) // �����ɫ
#define LineColor		RGB(0,255,0) // ��ǰѡ�����/��������ʾ��ɫ

#define penWidth	5	//���ʴ�ϸ
#define lineSpan	10	//
#define radius		5	//Բ��뾶
#define radiueSpan	50	//
#define point2LineSpan	penWidth*2+radius	//������Բ�ĵ�MAX����


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
public:
	CString m_version;
	CString m_dateTime;
	TF_SDK_Details sdk;
	CString m_sdkInfo;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
, m_version(_T("Version:1.2.3"))
, m_dateTime(_T(__DATE__"  "__TIME__))
, m_sdkInfo(_T(""))
{

	TFLPR_GetSDKdetails(&sdk);
	m_sdkInfo.Format("SDK Version:%s \r\n \r\n����ʱ��:%s ", sdk.cVersion, sdk.cComplieDate);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, ID_VERSION, m_version);
	DDX_Text(pDX, ID_DATE_TIME, m_dateTime);
	DDX_Text(pDX, IDC_SDK_INFO, m_sdkInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)

END_MESSAGE_MAP()


// CVLPRDemoDlg �Ի���




CVLPRDemoDlg::CVLPRDemoDlg(CWnd* pParent /*=NULL*/)
: CDialog(CVLPRDemoDlg::IDD, pParent)
, m_imageDir(_T("E:/temp/images"))
{
	//memset(this, 0, sizeof(CVLPRDemoDlg));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_videoplay = 0;
	pTF_RecParma = NULL;
	pTF_Result = NULL;
	bPlay = false;
	bStop = true;
	bPause = false;
	fastProcess = false;
	pLPRInstance = NULL;

	startDraw = false;

	handleExit = CreateEvent(NULL, FALSE, FALSE, NULL);
	hAccessImage =  CreateEvent(NULL, FALSE, FALSE, NULL);
	handleVideoThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	handleVideoThreadStoped = CreateEvent(NULL, FALSE, FALSE, NULL);
	hShowVideoFrame  = CreateEvent(NULL, FALSE, FALSE, NULL);
	handleLPRThreadStoped = CreateEvent(NULL, FALSE, FALSE, NULL);

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
	ON_BN_CLICKED(BT_NEXT_PICTURE, &CVLPRDemoDlg::OnBnClickedNextPicture)
	ON_EN_CHANGE(ID_IMAGE_DIR, &CVLPRDemoDlg::OnEnChangeImageDir)
	ON_BN_CLICKED(IDC_CHECK1, &CVLPRDemoDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(BT_SET_CONFIG, &CVLPRDemoDlg::OnBnClickedSetConfig)
	ON_NOTIFY(NM_CLICK, IDC_LIST, &CVLPRDemoDlg::OnNMClickList)
	ON_BN_CLICKED(BT_START_ANNAY, &CVLPRDemoDlg::OnBnClickedStartAnnay)
	ON_STN_CLICKED(ID_VIDEO_WALL, &CVLPRDemoDlg::OnStnClickedVideoWall)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(BT_SET_AREA, &CVLPRDemoDlg::OnBnClickedSetArea)
	ON_BN_CLICKED(IDC_CARLOGO_DETECT, &CVLPRDemoDlg::OnBnClickedCarlogoDetect)
END_MESSAGE_MAP()



// CVLPRDemoDlg ��Ϣ�������

BOOL CVLPRDemoDlg::OnInitDialog()
{
	bWindowInited = false;

	CDialog::OnInitDialog();

	//skinppLoadSkin( "skin/AquaOS.ssk");

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

	this->SetWindowText("��֡����ʶ��");

	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
	//	dwStyle |= LVS_EX_CHECKBOXES;//itemǰ����checkbox�ؼ�
	m_list.SetExtendedStyle(dwStyle); //������չ���

	int cols = 0;
	m_list.InsertColumn( cols++, "����ͼƬ", LVCFMT_LEFT, 1 );//������
	m_list.InsertColumn( cols++, "����", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( cols++, "���Ŷ�", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( cols++, "��ʱ", LVCFMT_LEFT, 50 );
	m_list.InsertColumn( cols++, "��������", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( cols++, "����", LVCFMT_LEFT, 70 );
	m_list.InsertColumn( cols++, "��������ɫ", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( cols++, "�������ɫ", LVCFMT_LEFT, 80 );
	m_list.InsertColumn( cols++, "�˶�����", LVCFMT_LEFT, 90 );
	m_list.InsertColumn( cols++, "ʱ��", LVCFMT_LEFT, 120 );
	m_list.InsertColumn( cols++, "����", LVCFMT_LEFT, 120 );
	m_list.InsertColumn( cols++, "��ͼ", LVCFMT_LEFT, 120 );

	//int nRow = m_list.InsertItem(0, "");//
	//m_list.SetItemText(nRow, 0, "000");//
	//m_list.SetItemText(nRow, 9, "009");//
	//m_list.SetItemText(nRow, 10, "100");//


	videoWall = (CStatic*)GetDlgItem(ID_VIDEO_WALL);
	pictureWall  = (CStatic*)GetDlgItem(ID_PICTURE);
	plateWall  = (CStatic*)GetDlgItem(ID_LPR_PICTURE);


	mGroupVideo			= (CButton*)GetDlgItem(ID_GROUP_VIDEO);
	mGroupPicture		= (CButton*)GetDlgItem(ID_GROUP_PICTURE);
	mGroupLPRPicture	= (CButton*)GetDlgItem(ID_GROUP_LPR_PICTURE);
	mGroupLPR			= (CButton*)GetDlgItem(ID_GROUP_LPR);
	mGroupOperate		= (CButton*)GetDlgItem(ID_GROUP_OPERATE);


	if(!ReadConfig()){
		plateMaxWidth = 400;
		plateMinWidth = 60;
		m_imageDir = CVLPRDemoApp::m_appPath +"/image";
	}
	FileUtil::CreateFolders(m_imageDir.GetBuffer(m_imageDir.GetLength()));

	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=2;
		pTF_RecParma->iMaxPlateWidth = plateMaxWidth;
		pTF_RecParma->iMinPlateWidth = plateMinWidth;
		pTF_RecParma->iResultNum=8;
		sprintf(pTF_RecParma->pLocalChinese, pLocalChinese);
	}
	if(pTF_Result == NULL){
		pTF_Result = new TF_Result();
	}

	company = TELEFRAME;

	//	InitMyWindows();
	ReStartThread();

	int ret = TFLPR_ThreadInit();
	if(ret<0){
		TF_LPR_canRun = false;
		MessageBox(TF_ERROR[ret+3], "", MB_OK);
		//exit(0);
	}else
		TF_LPR_canRun = true;

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


static long nFrames = 0;

LRESULT CVLPRDemoDlg::ProcessResult(WPARAM wParam, LPARAM lParam)
{
	debug("ProcessResult");
	return 0;
}



//����ʶ�����߳�
void ProcessResultThread(void *pParam)
{
	char filename[256]={0};
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("ProcessResultThread");

	char temp[256]={0};
	debug("ProcessResultThread ����  handle=0x%x", handleCanExit);

	CxImage image;
	CxImage imagePlate;

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->LPRQueueResult.size()<1){
			Sleep(10);		
			continue;
		}
		if(dlg->LPRQueueResult.empty())
			continue;
		LPR_Result *result = dlg->LPRQueueResult.front();

		sprintf(temp, "�������ʣ��: %d ", dlg->LPRQueueResult.size());
		release(temp);
		dlg->GetDlgItem(ID_STATUS2)->SetWindowText(temp);

		debug("ProcessResultThread Frame=%d  Plate=%s  (%d,%d)-(%d,%d)", nFrames, result->plate, \
			result->plateRect.left, result->plateRect.top,
			result->plateRect.right, result->plateRect.bottom);

		dlg->m_list.SetItemState(0, 0, LVIS_SELECTED|LVIS_FOCUSED); //ȡ��ѡ��
		int column = 1;
		dlg->GetDlgItem(ID_LPR)->SetWindowText(result->plate);
		int count = dlg->m_list.GetItemCount();
		int nRow = dlg->m_list.InsertItem(0, "");//����ͼƬ
		dlg->m_list.SetItemText(nRow, column++, result->plate);//����
		sprintf(temp, "%g", result->confidence);
		dlg->m_list.SetItemText(nRow, column++, temp);//���Ŷ�
		sprintf(temp, "%3dms", result->takesTime);
		dlg->m_list.SetItemText(nRow, column++, temp);//��ʱ
		sprintf(temp, "%s", result->plateType);
		dlg->m_list.SetItemText(nRow, column++, temp);//��������

		sprintf(temp, "%s", result->carLogo);
		dlg->m_list.SetItemText(nRow, column++, temp);//����

		sprintf(temp, "%s", result->carColor1 );
		dlg->m_list.SetItemText(nRow, column++, temp);//��������ɫ
		sprintf(temp, "%s", result->carColor2 );
		dlg->m_list.SetItemText(nRow, column++, temp);//�������ɫ
		sprintf(temp, "%s", result->direct);
		dlg->m_list.SetItemText(nRow, column++, temp);//�˶�����

		time_t timer;
		timer = time(NULL);
		struct tm *tblock;
		tblock = localtime(&timer);
		sprintf(temp,"%d-%d-%d %d:%d:%d", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec );
		dlg->m_list.SetItemText(nRow, column++, temp);//ʱ��

		dlg->m_list.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED); //ѡ��

		dlg->ShowPicture(result->pResultBits, result->imageWidth, result->imageHeight);

		//get Plate
		int w = result->plateRect.right - result->plateRect.left;
		int h = result->plateRect.bottom - result->plateRect.top;
		int linestep = WIDTHSTEP(w);
		unsigned char *pBit = 0;

		pBit = result->pResultBits;//ͼƬ

		if(dlg->LPRQueueResult.size()>0)
			dlg->LPRQueueResult.pop();
		if(pBit!=NULL)
		{
			memset(plate, 0 , w*h*3);
			for(int i=0; i<h; i++){
				memcpy( (plate + i * linestep ),
					(pBit + result->plateRect.left*3 + (result->plateRect.top+i) * WIDTHSTEP(result->imageWidth)),
					linestep );
			}
			dlg->ShowPlatePicture(plate, w, h );

			{
				time_t timer;
				timer = time(NULL);
				struct tm *tblock;
				tblock = localtime(&timer);
				sprintf(temp,"%04d-%02d-%02d_%02d.%02d.%02d", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec );
			}
			//������ͼƬ������Ϊ�� ʱ��_����_��������_������ɫ_�����ٶ�.bmp \
				��:	 \
					2014-02-24_15.29.24 ��W56Y2 С������  ��ɫ 0 \
					2014-02-24_15.29.24_��W56Y22_02_3_0.bmp

			// ������дͼ
			sprintf(filename, "%s\\%s_%s_02_2_0.jpg", dlg->m_imageDir, temp, result->plate );
			image.CreateFromArray( pBit, result->imageWidth, result->imageHeight, 24, WIDTHSTEP(result->imageWidth), true);
			image.Save( filename, CXIMAGE_FORMAT_JPG, true );
			image.Destroy();
		//	dlg->m_videoplay->Save2jpeg( (unsigned char*)pBit,  result->imageWidth, result->imageHeight,filename);
		//	VideoUtil::write24BitBmpFile(filename, result->imageWidth, result->imageHeight,(unsigned char*)pBit,  WIDTHSTEP(result->imageWidth));//ץ����дͼ
			dlg->m_list.SetItemText(nRow, column++, filename);//��дͼλ��  column = 11
			debug(filename);

			//����ͼƬ
			sprintf(filename, "%s\\%s_%s_02_2_0.jpg_plate.jpg", dlg->m_imageDir, temp, result->plate ); 
			debug(filename);
			imagePlate.CreateFromArray( plate, w, h, 24, WIDTHSTEP(w), true);
			imagePlate.Save( filename, CXIMAGE_FORMAT_JPG );//
			imagePlate.Destroy();
		//	VideoUtil::write24BitBmpFile(filename, w, h,(unsigned char*)plate, true, linestep);//����ͼƬ
			dlg->m_list.SetItemText(nRow, column++, filename);//����λ��  column = 10

			

			delete pBit;
		}
		debug("ProcessResultThread LPRQueueResult.front 0x%x", result);
		delete result;

	}
	debug("ProcessResultThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,��2������,Ҫ������SetEvnetһ��,���򷵻س�ʱ

}

void PlayThread(void *pParam)
{
	try{

		CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
		HANDLE handleCanExit = dlg->ReginsterMyThread("PlayThread");
		debug("PlayThread ����  handle=0x%x", handleCanExit);
		char temp[256]={0};

		while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){

			if(dlg->imagesQueuePlay.empty()){
				Sleep(10);
				continue;
			}
			sprintf(temp, "PlaySize: %d ", dlg->LPRQueueResult.size());
			release(temp);
			dlg->GetDlgItem(ID_STATUS3)->SetWindowText(temp);

			if(true)
	//		if(WaitForSingleObject(hShowVideoFrame,0)==0)
			{
				if(dlg->imagesQueuePlay.empty()==false){
					LPR_Image* lpr = dlg->imagesQueuePlay.front();
					dlg->imagesQueuePlay.pop();
					debug("PlayThread imagesQueuePlay.front  0x%x  imagesQueuePlaysize = %d", lpr, dlg->imagesQueuePlay.size()+1);

					dlg->PlayVideo(lpr->buffer, lpr->imageWidth, lpr->imageHeight);

					delete lpr->buffer;
					delete lpr;
				}
			}
			/*while(dlg->imagesQueuePlay.size() > 1){
				LPR_Image* lpr = dlg->imagesQueuePlay.front();
				debug("PlayThread while imagesQueuePlay.front  0x%x  imagesQueuePlaysize = %d", lpr, dlg->imagesQueuePlay.size()+1);
				dlg->imagesQueuePlay.pop();
				delete lpr->buffer;
				delete lpr;
			}*/
		}
		debug("PlayThread �����˳�");
		SetEvent(handleCanExit);//���ÿ����˳���
		SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,�ڴ�(����2��)����,Ҫ������SetEvnetһ��,���򷵻س�ʱ


	}
	catch(...)
	{
		release("PlayThread TH_RecogImage Error");
		Sleep(100);
		//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
	}
}

//ʶ���߳�
void RecognitionThread(void *pParam)
{
	try
	{
		CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
		HANDLE handleCanExit = dlg->ReginsterMyThread("RecognitionThread");
		//dlg->GetDlgItem(ID_STATUS)->SetWindowText("���ڷ���.....");

		TF_Rect recROI;
		debug("RecognitionThread ����  handle=0x%x", handleCanExit);
		nFrames = 0;
		int ret=0;
		int imageSize = 0;
		clock_t t1,t2;
		LPR_Image   *p2=0;
		LPR_Image *pLprImage=0;
		char temp[256]={0};
		SetEvent(handleLPRThreadStoped);
		unsigned char *imageTemp = 0;
		while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){

			if(dlg->imagesQueue.size()<1){
				SetEvent(handleLPRThreadStoped);
				Sleep(10);		
				continue;
			}
			ResetEvent(handleLPRThreadStoped);

			nFrames ++;	

			if(dlg->imagesQueue.size()>0)
			{
				pLprImage = dlg->imagesQueue.front();
				dlg->imagesQueue.pop();
				imageSize = pLprImage->imageWidth * pLprImage->imageHeight * 3;

				debug("RecognitionThread imagesQueue.front ed 0x%x   imagesQueue size=%d ", pLprImage, dlg->imagesQueue.size());

				LPR_Image *lpr = new LPR_Image();
				memcpy(lpr, pLprImage, sizeof(LPR_Image));
				lpr->buffer = new unsigned char[pLprImage->imageSize];
				memcpy(lpr->buffer, pLprImage->buffer, pLprImage->imageSize);
				dlg->imagesQueuePlay.push(lpr);//For Play
				debug("imagesQueuePlay.push( 0x%x )  size=%d ", lpr, dlg->imagesQueuePlay.size());
				SetEvent(hShowVideoFrame);// start play

				ret = 0;

				if(dlg->company == TELEFRAME && dlg->TF_LPR_canRun){

					if(dlg->ROIRect.iLeft >= 0 && dlg->ROIRect.iTop >= 0 && dlg->ROIRect.iRight <= pLprImage->imageWidth && dlg->ROIRect.iBottom <= pLprImage->imageHeight)
					{
						recROI = dlg->ROIRect;
					}else{
						recROI.iLeft =  recROI.iTop =0;
						recROI.iRight = pLprImage->imageWidth;
						recROI.iBottom = pLprImage->imageHeight;
					}
					debug("================== recROI(%d,%d)--(%d,%d)", recROI.iLeft, recROI.iTop, recROI.iRight, recROI.iBottom);

					memset(dlg->pTF_Result->number, 0, 20);//��գ���������Ƶ�����л�����������������
					dlg->pTF_Result->fConfidence = 0.0;////��գ���������Ƶ�����л�����������������

					try
					{
						t1 = clock();
						if(dlg->recognitionMode==PICTURE) // ����ͼƬ
							ret = TFLPR_RecImage( pLprImage->buffer, pLprImage->imageWidth, pLprImage->imageHeight, dlg->pTF_Result, 0, dlg->pLPRInstance);//����ʶ��
						else
							ret = TFLPR_RecImage( pLprImage->buffer, pLprImage->imageWidth, pLprImage->imageHeight, dlg->pTF_Result, &recROI, dlg->pLPRInstance);//����ʶ��
						t2 = clock();
					}
					catch(...)
					{
						release("TFLPR_RecImage Error");
						Sleep(100);
						continue;
						//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
					}
					dlg->pTF_Result->takesTime = t2-t1;

					if(ret>0){

						LPR_Result *r = new LPR_Result();
						sprintf(r->plate, "%s", dlg->pTF_Result->number);
						sprintf(r->plateType, "%s",  LPlateType[dlg->pTF_Result->ePlateType]);
						r->confidence = dlg->pTF_Result->fConfidence;
						r->plateRect.left =  dlg->pTF_Result->PlateRect.iLeft;
						r->plateRect.top =  dlg->pTF_Result->PlateRect.iTop;
						r->plateRect.right =  dlg->pTF_Result->PlateRect.iRight;
						r->plateRect.bottom =  dlg->pTF_Result->PlateRect.iBottom;
						sprintf(r->direct, "%s",Directory[dlg->pTF_Result->iMoveDir+1]);
						sprintf(r->carColor1, "%s", LVehicleColor[dlg->pTF_Result->eVColor1] );
						sprintf(r->carColor2, "%s", LVehicleColor[dlg->pTF_Result->eVColor2] );
						r->takesTime = dlg->pTF_Result->takesTime;

						r->imageWidth = pLprImage->imageWidth;
						r->imageHeight = pLprImage->imageHeight;

						r->pResultBits = new unsigned char[imageSize];
						if(dlg->pTF_RecParma->iRecMode!=0) //������Ƶ
						{
							if(r->pResultBits!=NULL &&  dlg->pTF_Result->pResultBits!=NULL)
								memcpy(r->pResultBits , dlg->pTF_Result->pResultBits, imageSize );
						}else{ 
							if(dlg->recognitionMode==PICTURE) // ����ͼƬ
								memcpy(r->pResultBits , pLprImage->buffer, imageSize );
						}
						dlg->GetDlgItem(ID_LPR)->SetWindowText("");
						dlg->LPRQueueResult.push(r);
						debug("RecognitionThread LPRQueueResult.push  0x%x", r);

					}
					else {
						debug("-------------- δʶ�� ERROR[%d]: %s ------------- " , ret, TF_ERROR[ret+3]);
						if(!dlg->fastProcess && dlg->imagesQueue.size()>0){//
							p2 = dlg->imagesQueue.front();
							dlg->imagesQueue.pop();
							delete p2->buffer;
							delete p2;
						}
						if(dlg->recognitionMode==PICTURE){
							dlg->GetDlgItem(ID_LPR)->SetWindowText("δʶ��");
						}
					}
				}
				else if(dlg->company == WENTONG ) { //if(dlg->company == WENTONG && dlg->TH_LPR_canRun) 
					TH_PlateIDResult result[6];        //���붨�������� 
					memset(&result, 0, sizeof(result)); 
					int nResultNum = 1; 
					TH_RECT rcDetect={0, 0, pLprImage->imageWidth, pLprImage->imageHeight}; 
					if(dlg->ROIRect.iLeft >= 0 && dlg->ROIRect.iTop >= 0 && 
						dlg->ROIRect.iRight <= pLprImage->imageWidth && dlg->ROIRect.iBottom <= pLprImage->imageHeight)
					{
						rcDetect.left = dlg->ROIRect.iLeft;
						rcDetect.top = dlg->ROIRect.iTop;
						rcDetect.right = dlg->ROIRect.iRight;
						rcDetect.bottom = dlg->ROIRect.iBottom;
					}

					try
					{
						t1 = clock();
						ret = -1;
						ret  =  TH_RecogImage( pLprImage->buffer,  pLprImage->imageWidth, pLprImage->imageHeight,  result, &nResultNum, &rcDetect, &dlg->plateConfigTh); 
						t2 = clock();
					}
					catch(...)
					{
						release("TH_RecogImage Error");
						Sleep(100);
						continue;
						//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
					}

					if(ret!=0 || nResultNum<=0)
						ret = -1;
					else{

						ret = 1;
						LPR_Result *r = new LPR_Result();
						r->takesTime = t2-t1;
						sprintf(r->plate, "%s", result[0].license);
						r->confidence = result[0].nConfidence*1.0/100;
						sprintf(r->carLogo, "%s", CarLogo[ result[0].nCarLogo] );
						sprintf(r->plateType, "%s", CarType[result[0].nType]);
						sprintf(r->direct, "%s", TH_Dirction[result[0].nDirection]);
						sprintf(r->carColor1, "%s", CarColor[result[0].nCarColor]);
						r->plateRect.left = result[0].rcLocation.left;
						r->plateRect.top = result[0].rcLocation.top;
						r->plateRect.right = result[0].rcLocation.right;
						r->plateRect.bottom = result[0].rcLocation.bottom;

						r->imageWidth = pLprImage->imageWidth;
						r->imageHeight = pLprImage->imageHeight;
						r->pResultBits = new unsigned char[imageSize];
						memcpy(r->pResultBits , pLprImage->buffer, imageSize );

						debug("����: %s  ����: %s", result[0].license, CarLogo[ result[0].nCarLogo] );

						dlg->GetDlgItem(ID_LPR)->SetWindowText("");
						dlg->LPRQueueResult.push(r);
						debug("RecognitionThread LPRQueueResult.push  0x%x", r);

					}
				}
				delete pLprImage->buffer;
				delete pLprImage;


				dlg->timeNow = clock();
				sprintf(temp, "ʶ������: %d  Rate:%d fps",dlg->imagesQueue.size(), nFrames*1000/(dlg->timeNow - dlg->timeStart));
				release(temp);
				debug("RecognitionThread %s",temp );
				dlg->GetDlgItem(ID_STATUS)->SetWindowText(temp);

				//	SetEvent(handleLPRThreadStoped);
			}
		}
		dlg->GetDlgItem(ID_STATUS)->SetWindowText("�������");
end:
		dlg->GetDlgItem(ID_STATUS)->SetWindowText("��������");
		debug("RecognitionThread �����˳�");
		SetEvent(handleCanExit);//���ÿ����˳���
		SetEvent(handleExit); 

	}
	catch(...)
	{
		release("RecognitionThread TH_RecogImage Error");
		Sleep(100);
		//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
	}

}

//��Ƶ����ȡ�߳�
void VideoThread(void* pParam)
{
	try{
		CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;

		HANDLE handleCanExit = dlg->ReginsterMyThread("VideoThread");
		debug("VideoThread ����  handle=0x%x", handleCanExit);
		unsigned char* pixBits  = 0;
		int ret=0;

		SetEvent(handleVideoThreadStoped);

		while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0 ){
			if(dlg->bStop == true){
				debug("VideoThread dlg->bStop == true.");
				SetEvent(handleVideoThreadStoped);
				debug("VideoThread WaitForSingleObject(handleVideoThread,-1)");
				WaitForSingleObject(handleVideoThread,-1);
			}
			while(dlg->bPause && WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
				SetEvent(handleVideoThreadStoped);
				Sleep(100);
				nFrames = 0;
			}
			if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0){
				SetEvent(handleVideoThreadStoped);
				break;
			}
			//	if(true)
			{
				ResetEvent(handleVideoThreadStoped);
				//if(dlg->fastProcess) //��������ģʽ��ȱ��:�ᶪ֡
				//	ret = dlg->m_videoplay->getOneFrame();
				if(dlg->imagesQueue.size() < 10 )
				{
					debug("VideoThread dlg->m_videoplay->getOneFrame();");
				//	if(!dlg->fastProcess)
						ret = dlg->m_videoplay->getOneFrame();
					if(ret==0)
					{
						LPR_Image *pLprImage  = new LPR_Image();
						pLprImage->imageWidth = dlg->m_videoplay->imageFrame->width;
						pLprImage->imageHeight = dlg->m_videoplay->imageFrame->height;
						pLprImage->imageSize = pLprImage->imageWidth * pLprImage->imageHeight * 3;
						if(pLprImage->imageSize<1)
							continue;
						dlg->recognitionMode = VIDEO;
						pLprImage->buffer = new unsigned char[pLprImage->imageSize];
						memcpy(pLprImage->buffer,  dlg->m_videoplay->imageFrame->imageData, pLprImage->imageSize);

						dlg->imagesQueue.push(pLprImage);
						debug("VideoThread imagesQueue.push 0x%x   imagesQueue=%d  nFrame=%d / %d ", pLprImage, dlg->imagesQueue.size(), 
								dlg->m_videoplay->iNowFrameNum,  dlg->m_videoplay->iTotalFrameNum);
					}else{
						if(ret==-2){
							ResetEvent(handleVideoThread);
							SetEvent(handleVideoThreadStoped);
							dlg->OnBnClickedStop();
							debug("��Ƶ��ȡ���");
							MessageBox(0, "��Ƶ��ȡ���","", MB_OK);
						}
						else{
							debug("��Ƶ��ȡʧ��");
						}
					}
				}else{//if(dlg->imagesQueue.size() < 30 )

				}				
			}
		}
end:
		ResetEvent(handleVideoThread);
		dlg->bPlay = false;
		dlg->bStop = true;
		dlg->bPause = false;
		debug("VideoThread �����˳�");
		SetEvent(handleCanExit);//���ÿ����˳���
		SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,��2������,Ҫ������SetEvnetһ��,���򷵻س�ʱ

	}
	catch(...)
	{
		release("VideoThread TH_RecogImage Error");
		Sleep(100);
		//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
	}
}

void CVLPRDemoDlg::StartProcessVideo(CString fileName)
{
	try
	{
		int ret =0;
		recognitionMode = VIDEO;
		nFrames = 0;

		if(	((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck())
			pTF_RecParma->iRecMode =1;
		else
			pTF_RecParma->iRecMode=2;//��Ƶʶ��ģʽ

		char cBuff[256]={0};

		ResetEvent(handleVideoThread);

		if(m_videoplay!=0){
			m_videoplay->release();
			m_videoplay = 0;
		}
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
			ret = m_videoplay->getOneFrame();
			if(ret==0)
			{
				imageWidth = m_videoplay->imageFrame->width;
				imageHeight = m_videoplay->imageFrame->height;
				int size = imageWidth * imageHeight * 3;

				unsigned char *imageDataForShow = new unsigned char[size];
				memcpy(imageDataForShow, (unsigned char*)m_videoplay->imageFrame->imageData, size);
				//	VideoUtil::write24BitBmpFile("E:/temp/k1.bmp",imageWidth, imageHeight,);
				PlayVideo(imageDataForShow, imageWidth, imageHeight);
				delete imageDataForShow;

				if(pTF_RecParma->iRecMode != 0){
					pTF_RecParma->iImageHeight = imageHeight;
					pTF_RecParma->iImageWidth = imageWidth;
					pTF_Result->pResultBits = new unsigned char[imageHeight * imageWidth* 3];
				}
				if(pLPRInstance)
					TFLPR_Uninit(pLPRInstance);
				pLPRInstance = TFLPR_Init(*pTF_RecParma);
			}else{
				debug("��Ƶ��ȡʧ��[%d]", ret);
				MessageBox("��Ƶ��ȡʧ��","����",MB_OK);
				return ;
			}
		}

	}
	catch(...)
	{
		release("StartProcessVideo TH_RecogImage Error");
		Sleep(100);
		//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
	}
}


void PreStartThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	// ��ʼ����
	if(dlg->mVideoPath.IsEmpty())
		return ;

	dlg->GetDlgItem(ID_STATUS)->SetWindowText("������������...");
	debug("OnBnClickedStartAnnay  waiting  handleVideoThreadStoped  ");
	WaitForSingleObject(handleVideoThreadStoped, -1);
	debug("OnBnClickedStartAnnay  waiting  handleLPRThreadStoped  ");
	WaitForSingleObject(handleLPRThreadStoped, -1);
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("...");

	if(dlg->company == WENTONG )
	{
		if(dlg->TH_InitDll(1)==false)  //��Ƶ��ʽ��Ϊ 1 
			return ;
	}

	dlg->StartProcessVideo(dlg->mVideoPath);

	dlg->bPlay = true;
	dlg->bStop = false;
	dlg->bPause = false;

	dlg->GetDlgItem(BT_PAUSE)->EnableWindow(true);
	dlg->GetDlgItem(BT_STOP)->EnableWindow(true);
	dlg->GetDlgItem(BT_START_ANNAY)->EnableWindow(false);
	dlg->GetDlgItem(BT_OPEN_VIDEO)->EnableWindow(false);
	dlg->GetDlgItem(BT_OPEN_PICTURE)->EnableWindow(false);
	dlg->GetDlgItem(IDC_CHECK1)->EnableWindow(false);
	dlg->GetDlgItem(BT_SET_AREA)->EnableWindow(false);

	//	ReStartThread();
	nFrames = 0;
	dlg->timeStart = clock();
	SetEvent(handleVideoThread);
	//SetEvent(hAccessImage);
	//_beginthread(VideoThread, 0 ,this);//��ȡ��Ƶ֡
}

void CVLPRDemoDlg::OnBnClickedStartAnnay()
{
	_beginthread(PreStartThread, 0 ,this);
}


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//����Ƶ������
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) 
		return ;

	mVideoPath = fileOpenDlg.GetPathName();

	if(mVideoPath.IsEmpty())
		return ;

	GetDlgItem(BT_START_ANNAY)->EnableWindow(true);
	GetDlgItem(BT_SET_AREA)->EnableWindow(true);

	StartProcessVideo(mVideoPath);

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
void CVLPRDemoDlg::ShowPicture(unsigned char *pix, int imageWidth, int imageHeight)
{

	BITMAPINFO *m_bmphdr = GetBitMapInfo(imageWidth, imageHeight );
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
		0, imageHeight,  
		imageWidth, -imageHeight,  
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
void CVLPRDemoDlg::PlayVideo(unsigned char *pix, int imageWidth, int imageHeight){

	BITMAPINFO *m_bmphdr = GetBitMapInfo(imageWidth, imageHeight );

	CRect rc;
	videoWall->GetWindowRect(&rc);

	HDC hdc = videoWall->GetDC()->m_hDC;
	SetStretchBltMode(hdc, STRETCH_HALFTONE); //�ؼӣ�StretchBlt, StretchDIBits���Զ�ͼ�����ݽ�������, ѹ����ʾʧ��

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
	//��ͣ/�ָ�
	if(bPause){
		bPause = false;
		GetDlgItem(BT_PAUSE)->SetWindowText("��ͣ");
		timeStart = clock();
		GetDlgItem(BT_SET_AREA)->EnableWindow(false);
	}else{
		bPause = true;
		GetDlgItem(BT_PAUSE)->SetWindowText("�ָ�");
		GetDlgItem(BT_SET_AREA)->EnableWindow(true);
	}
}

void CVLPRDemoDlg::OnBnClickedStop()
{
	//ֹͣ
	bStop = true;
	ResetEvent(handleVideoThread);

	WaitForSingleObject(handleVideoThreadStoped, -1);
	SetEvent(handleVideoThreadStoped);

	if(m_videoplay!=0){
		debug("OnBnClickedStop()  release m_videoplay");
		m_videoplay->release();
		m_videoplay = 0;
	}

	GetDlgItem(BT_SET_AREA)->EnableWindow(true);
	GetDlgItem(BT_PAUSE)->EnableWindow(false);
	GetDlgItem(BT_STOP)->EnableWindow(false);
	GetDlgItem(IDC_CHECK1)->EnableWindow(true);
	GetDlgItem(BT_START_ANNAY)->EnableWindow(true);
	GetDlgItem(BT_OPEN_VIDEO)->EnableWindow(true);
	GetDlgItem(BT_OPEN_PICTURE)->EnableWindow(true);

}

int CVLPRDemoDlg::CloseThread()
{
	try
	{
		int times=0;
		while(EventList.size()>0 && times<10)
		{
			times ++;
			for(list<HANDLE>::iterator it = EventList.begin(); it != EventList.end();)
			{
				SetEvent(handleExit);
				HANDLE h = (HANDLE)*it;
				int ret =WaitForSingleObject(h, 10);
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
	}
	catch(...)
	{
		release("PlayThread TH_RecogImage Error");
		Sleep(100);
		//	MessageBox(0, "RecognitionThread Error", "", MB_OK);
	}
	return EventList.size();
}

void CVLPRDemoDlg::ReStartThread()
{
	if(CloseThread() > 0)
	{
		debug("�����߳�ʧ��.");
		GetDlgItem(ID_STATUS)->SetWindowText("�����߳�ʧ��.");
		//return ;
	}
	ResetEvent(handleExit);
	//	SetEvent(hAccessImage);
	_beginthread(VideoThread, 0 ,this);//��ȡ��Ƶ֡
	_beginthread(PlayThread, 0, this);//��Ƶ�����߳�
	_beginthread(RecognitionThread, 0, this);//ʶ���߳�
	_beginthread(ProcessResultThread, 0, this);//�����߳�

}

void LoadingThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	dlg->loading.m_TextShow = "�����˳�......";
	dlg->loading.DoModal();
	//	MessageBox(0, "�����˳�...", "", 0);
}

void CVLPRDemoDlg::OnClose()
{
	_beginthread(LoadingThread, 0, this);

	this->ShowWindow(SW_HIDE);

	OnBnClickedStop();
	Sleep(1000);

	GetDlgItem(ID_STATUS)->SetWindowText("�����˳�......");
	if( CloseThread()>0)
		debug("�������˳� @ CDialog::OnClose()  �߳�δȫ���ر�");
	else
		debug("���������˳� @ CDialog::OnClose()");
	Sleep(1000);
	
	::PostQuitMessage(0);

}


//ÿ���̶߳�Ҫע�ᣬ�˳���ʱ������һһ���
//���أ���������̵߳�CEvent
HANDLE CVLPRDemoDlg::ReginsterMyThread(char *name)
{
	// �����¼�
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, name);
	SetEvent(hEvent);
	EventList.push_back(hEvent);
	debug("EventList.size = %d", EventList.size());
	return hEvent;

}

void CVLPRDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	/*
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
	*/

}

void CVLPRDemoDlg::OnBnClickedGroupOperate()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CVLPRDemoDlg::listFiles(CString firstFile)
{
	int index = firstFile.ReverseFind('\\');
	if(index<1)
		index = firstFile.ReverseFind('/');
	if(index<1)
		return;

	while(mListPicturesPath.size() > 0 )
	{
		char *p = mListPicturesPath.front();
		mListPicturesPath.pop_front();
		delete p;
	}

	CString path = firstFile.Left(index);
	FileUtil::ListFiles(path.GetBuffer(path.GetLength()), mListPicturesPath, "*.jpg", true);
	FileUtil::ListFiles(path.GetBuffer(path.GetLength()), mListPicturesPath, "*.bmp", true);
	FileUtil::ListFiles(path.GetBuffer(path.GetLength()), mListPicturesPath, "*.png", true);

	if(mListPicturesPath.size() > 0)
		GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(true);
	else
		GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(false);
}

void CVLPRDemoDlg::OnEnChangeImageDir()
{

}

void CVLPRDemoDlg::OnBnClickedCheck1()
{
	//��������ģʽ
	UpdateData(true);
	//fastProcess = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	//StartProcessVideo(mVideoPath);
}

void CVLPRDemoDlg::OnBnClickedSetConfig()
{
	//
	CString temp;
	if(ReadConfig())
	{
		configDlg.m_imageMaxWidth = plateMaxWidth;
		configDlg.m_imageMinWidth = plateMinWidth;
		configDlg.m_imageDir = m_imageDir;
		memcpy(configDlg.pLocalChinese, pLocalChinese, 3);
	}
	if(configDlg.DoModal()==IDOK){
		plateMaxWidth = configDlg.m_imageMaxWidth;
		plateMinWidth = configDlg.m_imageMinWidth;
		m_imageDir = configDlg.m_imageDir;
		FileUtil::CreateFolders(m_imageDir.GetBuffer(m_imageDir.GetLength()));

		pTF_RecParma->iMaxPlateWidth = plateMaxWidth;
		pTF_RecParma->iMinPlateWidth = plateMinWidth;
		sprintf(pLocalChinese, "%s", configDlg.pLocalChinese );

		SaveConfig();
	}

}
bool CVLPRDemoDlg::ReadConfig()
{
	char lpFileName[512]={0};
	char retString[512]={0};
	char temp[256]={0};
	sprintf(lpFileName,"%s\\VLPRConfig.ini",CVLPRDemoApp::m_appPath);
	bool ret = 0;
	ret = GetPrivateProfileString("config","imagedir", "", temp, 256, lpFileName);	
	m_imageDir.Format("%s", temp);
	plateMaxWidth = GetPrivateProfileInt("config","plateMaxWidth", 0, lpFileName);	
	plateMinWidth = GetPrivateProfileInt("config","plateMinWidth", 0, lpFileName);
	ret = GetPrivateProfileString("config","LocalChinese", "��", pLocalChinese, 256, lpFileName);
	int i=0;
	for( ;i<31; i++){
		if(strcmp(pLocalChinese, character[i])==0)
			break;
	}
	if(i>=31)
		sprintf(pLocalChinese, "��");

	ret = ret & (plateMaxWidth>0) & (plateMinWidth>0);
	return ret;

}
bool CVLPRDemoDlg::SaveConfig()
{
	char lpFileName[512]={0};
	char retString[512]={0};
	char temp[256]={0};
	sprintf(lpFileName,"%s\\VLPRConfig.ini",CVLPRDemoApp::m_appPath);
	bool ret=0;
	ret =WritePrivateProfileString("config","imagedir", m_imageDir, lpFileName);	
	sprintf(temp, "%d", plateMaxWidth);
	ret =WritePrivateProfileString("config","plateMaxWidth", temp, lpFileName);	
	sprintf(temp, "%d", plateMinWidth);
	ret = WritePrivateProfileString("config","plateMinWidth", temp, lpFileName);	
	ret = WritePrivateProfileString("config","LocalChinese", pLocalChinese, lpFileName);

	return ret;
}
void CVLPRDemoDlg::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	int nItem = m_list.GetNextSelectedItem(pos);
	CString temp;
	if(nItem>=0){
		temp = m_list.GetItemText(nItem, 1 );
		GetDlgItem(ID_LPR)->SetWindowText(temp);

		temp = m_list.GetItemText(nItem, 10 );
		Bitmap* imagePlate = KLoadBitmap(temp.GetBuffer(temp.GetLength()));
		DrawImg2Hdc(imagePlate, ID_LPR_PICTURE, this);

		temp = m_list.GetItemText(nItem, 11 );
		Bitmap* imageScreen = KLoadBitmap(temp.GetBuffer(temp.GetLength()));
		DrawImg2Hdc(imageScreen, ID_PICTURE, this);
	}
	*pResult = 0;
}



void CVLPRDemoDlg::OnStnClickedVideoWall()
{

}



void CVLPRDemoDlg::OnLButtonDown(UINT nFlags, CPoint pt)
{
	CPoint point;
	CRect r;
	GetDlgItem(ID_VIDEO_WALL)->GetWindowRect(&r);
	ScreenToClient(&r);

	if(r.PtInRect(pt) && startDraw)
	{
		CPoint cpt(r.left,r.top);
		point = pt - cpt;

		debug("onclick ID_VIDEO_WALL  at Point(%d, %d)", point.x, point.y);
		int dt=5;
		CDC *dc = GetDlgItem(ID_VIDEO_WALL)->GetDC();
		CPen penPoint(PS_SOLID, penWidth, PointColor);
		CPen penLine(PS_SOLID, penWidth, LineColor);
		dc->SelectObject(&penPoint);

		dc->Ellipse(point.x-dt,point.y-dt, point.x+dt,point.y+dt);
		pointPtr ++;
		dc->SelectObject(&penLine);

		switch(pointPtr)
		{	
		case 1:
			m_ROIRect.left = point.x;
			m_ROIRect.top = point.y;
			break;
		case 2:
			m_ROIRect.right = point.x;
			m_ROIRect.bottom = point.y;

			dc->MoveTo( m_ROIRect.left, m_ROIRect.top );
			dc->LineTo( m_ROIRect.right, m_ROIRect.top );

			dc->MoveTo( m_ROIRect.right, m_ROIRect.top );
			dc->LineTo( m_ROIRect.right, m_ROIRect.bottom );

			dc->MoveTo( m_ROIRect.right, m_ROIRect.bottom );
			dc->LineTo( m_ROIRect.left, m_ROIRect.bottom );

			dc->MoveTo( m_ROIRect.left, m_ROIRect.bottom );
			dc->LineTo( m_ROIRect.left, m_ROIRect.top );
			break;

		}

		if(pointPtr >= 2){
			startDraw = false;
			GetDlgItem(BT_SET_AREA)->SetWindowText("��������");
		}

	}

	CDialog::OnLButtonDown(nFlags, point);
}
/*
* bCRect2TF_Rect  true : CRect 2 TF_Rect
*                 false: TF_Rect 2 CRect
*/       
void CVLPRDemoDlg::CRect2TF_Rect(CRect &pt , TF_Rect &tfPt, int imageWidth, int imageHeight, bool bCRect2TF_Rect)
{
	CRect r;
	GetDlgItem(ID_VIDEO_WALL)->GetWindowRect(&r);
	ScreenToClient(&r);
	float dtW = imageWidth*1.0 / r.Width();
	float dtH = imageHeight*1.0 / r.Height();

	if(bCRect2TF_Rect){// CRect2TF_Rect
		tfPt.iLeft = pt.left * dtW ;
		tfPt.iTop = pt.top * dtH;
		tfPt.iRight = pt.right * dtW ;
		tfPt.iBottom = pt.bottom * dtH;
	}else{//TF_Rect2CRect
		pt.left = tfPt.iLeft / dtW;
		pt.top = tfPt.iTop / dtH;
		pt.right = tfPt.iRight / dtW;
		pt.bottom = tfPt.iBottom / dtH;
	}
}
//m_ROIRect
void CVLPRDemoDlg::OnBnClickedSetArea()
{
	if( !startDraw ){
		if(pointPtr==2){

			CRect2TF_Rect( m_ROIRect, ROIRect, imageWidth, imageHeight);

			MessageBox("����ɹ�");
			GetDlgItem(BT_SET_AREA)->SetWindowText("��������");
			pointPtr = 0;
			startDraw = false;

		}else{
			GetDlgItem(BT_SET_AREA)->SetWindowText("ȡ��");
			pointPtr = 0;
			startDraw = true;
		}
	}else if(startDraw){
		GetDlgItem(BT_SET_AREA)->SetWindowText("��������");
		pointPtr = 0;
		startDraw = false;
	}
}



Bitmap* image = 0;
void CVLPRDemoDlg::OnBnClickedNextPicture()
{
	// ������һ��ͼƬ
	if(mListPicturesPath.size()<1){
		GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(false);
		MessageBox("�ѵ������һ��","", MB_OK);
		return ;
	}

	if(mListPicturesPath.size()<1)
		return;

	char *filename =0;
	filename= mListPicturesPath.front();
	mListPicturesPath.pop_front();
	if( mPicturePath.Find(filename) != -1 ){
		filename = mListPicturesPath.front();
		mListPicturesPath.pop_front();
	}


	if(image!=0){
		delete image;
		image =0 ;
	}
	image = KLoadBitmap(filename);
	if(filename!=0)
		delete filename;

	if(image==0)
		return ;
	DrawImg2Hdc(image, ID_VIDEO_WALL, this);
	//	DrawImg2Hdc(image, ID_PICTURE, this);
	LPRFromImage(image);

	GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(false);
	//	Sleep(500);
	GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(true);
}

void CVLPRDemoDlg::LPRFromImage(Bitmap* image)
{
	LPR_Image *lpr = new LPR_Image();
	lpr->imageWidth = imageWidth = image->GetWidth();
	lpr->imageHeight = imageHeight = image->GetHeight();
	lpr->imageSize = lpr->imageWidth * lpr->imageHeight * 3;
	lpr->buffer = new unsigned char[lpr->imageSize];

	BitmapData bitmapData={0};
	bitmapData.Stride = WIDTHSTEP(lpr->imageWidth);
	bitmapData.Scan0 = new BYTE[bitmapData.Stride * lpr->imageHeight];


	Rect rect(0,0, lpr->imageWidth, lpr->imageHeight);
	image->LockBits(&rect, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bitmapData); 

	memcpy(lpr->buffer, (BYTE*)bitmapData.Scan0, lpr->imageSize); 

	delete bitmapData.Scan0;
	image->UnlockBits( &bitmapData);

	imagesQueue.push(lpr);//������н��д���

}


//�����һ·ʶ�������mem1 �� mem2 �ڴ�����С������һ�ڸ����ο�ֵ�� 
static unsigned char mem1[0x4000]; 
static unsigned char mem2[100<<20];//60M 

bool CVLPRDemoDlg::TH_InitDll(int bMovingImage)
{	
	int r = TH_UninitPlateIDSDK(&plateConfigTh);
	//plateConfigTh = c_defConfig;

	//���õ�һ·ʶ����� 
	plateConfigTh.nMinPlateWidth = 60; 
	plateConfigTh.nMaxPlateWidth = 400; 
	plateConfigTh.nMaxImageWidth = 1920; 
	plateConfigTh.nMaxImageHeight = 1920; 
	plateConfigTh.bVertCompress = 0; 
	plateConfigTh.bIsFieldImage = 0; 
	plateConfigTh.bOutputSingleFrame = 1; 
	plateConfigTh.bMovingImage = bMovingImage;   //��Ƶ��ʽ��Ϊ 1 
	plateConfigTh.pFastMemory=mem1; 
	plateConfigTh.nFastMemorySize=0x4000; 
	plateConfigTh.pMemory=mem2; 
	plateConfigTh.nMemorySize = 100<<20 ; 

	int n = TH_InitPlateIDSDK( &plateConfigTh ); 
	if(n!=0)
		MessageBox( pErrorInfo[n]);

	int m =TH_SetEnabledPlateFormat(PARAM_TWOROWYELLOW_ON, &plateConfigTh); 
	int k = TH_SetImageFormat(ImageFormatBGR, FALSE, FALSE, &plateConfigTh); 
	char m_LocalProvince[10] = "��"; 
	sprintf(m_LocalProvince, "%s", pLocalChinese);
	int l = TH_SetProvinceOrder(m_LocalProvince, &plateConfigTh); 
	int logo =TH_SetEnableCarLogo(true, &plateConfigTh); 

	if (n!=0||m!=0||k!=0||l!=0 | logo!=0) 
		return false; 

	return true;
}


void CVLPRDemoDlg::OnBnClickedOpenPicture()
{
	//��ͼƬ������
	//FileUtil::SelectFolder(this->m_hWnd, "ѡ��ͼƬ");
	CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"image files (*.jpg;*.bmp;*.png) |*.jpg;*.bmp;*.png|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) return ;

	mPicturePath = fileOpenDlg.GetPathName();
	char p[256] = {0};

	if(pLPRInstance)
		TFLPR_Uninit(pLPRInstance);

	pTF_RecParma->iRecMode=0;//ͼƬʶ��ģʽ
	if(TH_InitDll(0)==false)   //��Ƶ��ʽ��Ϊ 1 
		return ;

	pLPRInstance = TFLPR_Init(*pTF_RecParma);

	recognitionMode = PICTURE;

	listFiles(mPicturePath);

	Bitmap* image = KLoadBitmap(mPicturePath.GetBuffer(mPicturePath.GetLength()));
	DrawImg2Hdc(image, ID_VIDEO_WALL, this);
	DrawImg2Hdc(image, ID_PICTURE, this);
	LPRFromImage(image);


}
void CVLPRDemoDlg::OnBnClickedCarlogoDetect()
{
	UpdateData(true);
	bool check = ((CButton*)GetDlgItem(IDC_CARLOGO_DETECT))->GetCheck();
	company = NONE;
	if(check){
		company = WENTONG;
		//	MessageBox("δ�ҵ�������");
	}
	else {
		if(TF_LPR_canRun)
			company = TELEFRAME;
		else
			MessageBox("δ�ҵ����ܹ�");
	}

}


