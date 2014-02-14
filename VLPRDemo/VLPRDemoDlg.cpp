
// VLPRDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#include "VideoUtil.h"
#include "FileUtil.h"
#include "public.h"

#pragma comment(lib, "TFLPRecognition.lib")
#define WIDTHSTEP(pixels_width)  (((pixels_width) * 24/8 +3) / 4 *4)

char *TF_ERROR[5] = {"��ǰ���̲�����ʶ��", "�����̳߳�����Ŀ", "δ�ҵ����ܹ�", "��ʼ����ȷ",""};

char *LPlateType[10]={"δ֪����","��ͨ����","��ͨ����","�������","˫�����","��ɫ����","��ɫ�侯","��ɫ����","������"};
char *LVehicleColor[8]={"δ֪","��","��","��","��","��","��","��"};
char *Directory[3]={"δ֪","�����˶�", "��Զ�˶�"};

HANDLE handleExit = 0;
HANDLE hAccessImage;//���⣬����ͼ�����
HANDLE handleVideoThread=0;
HANDLE hShowVideoFrame=0;

#define MESSAGE_RESULT  0x5001

#define MAX_WIDTH_OF_PLATE  400
#define MAX_HEIGHT_OF_PLATE  400
#define MAX_SIZE_OF_PLATE	(MAX_WIDTH_OF_PLATE * MAX_HEIGHT_OF_PLATE * 3)
unsigned char *plate = new unsigned char[MAX_SIZE_OF_PLATE];


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
	ON_BN_CLICKED(BT_NEXT_PICTURE, &CVLPRDemoDlg::OnBnClickedNextPicture)
	ON_EN_CHANGE(ID_IMAGE_DIR, &CVLPRDemoDlg::OnEnChangeImageDir)
	ON_BN_CLICKED(IDC_CHECK1, &CVLPRDemoDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(BT_SET_CONFIG, &CVLPRDemoDlg::OnBnClickedSetConfig)
	ON_NOTIFY(NM_CLICK, IDC_LIST, &CVLPRDemoDlg::OnNMClickList)
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
	m_list.InsertColumn( 9, "����", LVCFMT_LEFT, 120 );
	m_list.InsertColumn( 10, "��ͼ", LVCFMT_LEFT, 120 );

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

	if(pTF_RecParma==NULL){
		pTF_RecParma = new TF_RecParma();
		pTF_RecParma->iImageMode = 0;
		pTF_RecParma->iRecMode=2;
		pTF_RecParma->iMaxPlateWidth = plateMaxWidth;
		pTF_RecParma->iMinPlateWidth = plateMinWidth;
		pTF_RecParma->iResultNum=8;
		sprintf(pTF_RecParma->pLocalChinese, "��");
	}
	if(pTF_Result == NULL){
		pTF_Result = new TF_Result();
	}

//	InitMyWindows();
	ReStartThread();

	int ret = TFLPR_ThreadInit();
	if(ret<0){
		MessageBox(TF_ERROR[ret+3], "", MB_OK);
		exit(0);
	}

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

void PlayThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("PlayThread");
	debug("PlayThread ����  handle=0x%x", handleCanExit);

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->imagesQueuePlay.empty()){
			Sleep(10);		
			continue;
		}
		if(WaitForSingleObject(hShowVideoFrame,0)==0)
		{
			debug("PlayThread imagesQueuePlay.size = %d", dlg->imagesQueuePlay.size());
			if(dlg->imagesQueuePlay.size() > 0){
				unsigned char* pix = dlg->imagesQueuePlay.front();
				debug("PlayThread imagesQueuePlay.front  0x%x", pix);
				dlg->imagesQueuePlay.pop();
				dlg->PlayVideo(pix);
			//	debug("PlayThread imagesQueuePlay pre release  0x%x", pix);\
				delete pix;
			}
		//	SetEvent(hAccessImage);
			
		}else{
			Sleep(100);
		}
	}
	debug("PlayThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,�ڴ�(����2��)����,Ҫ������SetEvnetһ��,���򷵻س�ʱ

}

//����ʶ�����߳�
void ProcessResultThread(void *pParam)
{
	char filename[256]={0};
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("ProcessResultThread");

	char temp[256]={0};
	debug("ProcessResultThread ����  handle=0x%x", handleCanExit);

	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		if(dlg->LPRQueueResult.size()<1){
			Sleep(10);		
			continue;
		}
		TF_Result *result = dlg->LPRQueueResult.front();
		debug("ProcessResultThread Frame=%d  Plate=%s  (%d,%d)-(%d,%d)", nFrames, result->number, \
			result->PlateRect.iLeft, result->PlateRect.iTop,
			result->PlateRect.iRight, result->PlateRect.iBottom);

		dlg->m_list.SetItemState(0, 0, LVIS_SELECTED|LVIS_FOCUSED); //ȡ��ѡ��
		int column = 1;
		dlg->GetDlgItem(ID_LPR)->SetWindowText(result->number);
		int count = dlg->m_list.GetItemCount();
		int nRow = dlg->m_list.InsertItem(0, "");//����ͼƬ
		dlg->m_list.SetItemText(nRow, column++, result->number);//����
		sprintf(temp, "%g", result->fConfidence);
		dlg->m_list.SetItemText(nRow, column++, temp);//���Ŷ�
		sprintf(temp, "%3dms", result->takesTime);
		dlg->m_list.SetItemText(nRow, column++, temp);//��ʱ
		sprintf(temp, "%s", LPlateType[result->ePlateType]);
		dlg->m_list.SetItemText(nRow, column++, temp);//��������
		sprintf(temp, "%s", LVehicleColor[result->eVColor1]);
		dlg->m_list.SetItemText(nRow, column++, temp);//��������ɫ
		sprintf(temp, "%s", LVehicleColor[result->eVColor2]);
		dlg->m_list.SetItemText(nRow, column++, temp);//�������ɫ
		sprintf(temp, "%s", Directory[result->iMoveDir+1]);
		dlg->m_list.SetItemText(nRow, column++, temp);//�˶�����
		TF_Time *t = &result->sTime;
		sprintf(temp, "%d-%d-%d %d:%d:%d", t->iYear, t->iMonth, t->iDay, t->iHour, t->iMilliseconds, t->iSecond);
		dlg->m_list.SetItemText(nRow, column++, temp);//ʱ��

		dlg->m_list.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED); //ѡ��

		dlg->ShowPicture(result->pResultBits);

		//get Plate
		int w = result->PlateRect.iRight - result->PlateRect.iLeft;
		int h = result->PlateRect.iBottom - result->PlateRect.iTop;
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
					(pBit + result->PlateRect.iLeft*3 + (result->PlateRect.iTop+i) * WIDTHSTEP(dlg->imageWidth)),
					linestep );
			}
			dlg->ShowPlatePicture(plate, w, h );


			if(t->iYear>2010)
				sprintf(temp, "%d-%d-%d_%d.%d.%d", t->iYear, t->iMonth, t->iDay, t->iHour, t->iMilliseconds, t->iSecond);
			else{
				time_t timer;
				timer = time(NULL);
				struct tm *tblock;
				tblock = localtime(&timer);
				sprintf(temp,"%d-%d-%d_%d.%d.%d", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour, tblock->tm_min, tblock->tm_sec );
			}
			sprintf(filename, "%s/%s_%s_plate.bmp", dlg->m_imageDir, temp, result->number);
		debug(filename);
			VideoUtil::write24BitBmpFile(filename, w, h,(unsigned char*)plate, true, linestep);//����ͼƬ
			dlg->m_list.SetItemText(nRow, column++, filename);//����λ��  column = 9
			
			//	delete plate;

			sprintf(filename, "%s/%s_%s.bmp", dlg->m_imageDir, temp, result->number);
			VideoUtil::write24BitBmpFile(filename,dlg->imageWidth, dlg->imageHeight,(unsigned char*)pBit,  WIDTHSTEP(dlg->imageWidth));//ץ����дͼ
			dlg->m_list.SetItemText(nRow, column++, filename);//��дͼλ��  column = 10
	debug(filename);

			delete pBit;
		}
		debug("ProcessResultThread LPRQueueResult.front 0x%x", result);
		delete result;
		
	}
	debug("ProcessResultThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); //��1��SetEvent(hEvent)�����¼����źŲ�WaitForSingleObject��,��2������,Ҫ������SetEvnetһ��,���򷵻س�ʱ
	
}

//ʶ���߳�
void RecognitionThread(void *pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	HANDLE handleCanExit = dlg->ReginsterMyThread("RecognitionThread");
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("���ڷ���.....");

	TF_Rect recROI;
	debug("RecognitionThread ����  handle=0x%x", handleCanExit);
	nFrames = 0;
	int ret=0;
	int imageSize = 0;
	clock_t t1,t2;
	unsigned char *p  =0 , *p2=0;
	char temp[256]={0};
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
		memset(dlg->pTF_Result->number, 0, 20);//��գ���������Ƶ�����л�����������������
		dlg->pTF_Result->fConfidence = 0.0;////��գ���������Ƶ�����л�����������������

		if(dlg->imagesQueue.size()>0)
		{
			imageSize = dlg->imageWidth * dlg->imageHeight *3;
			debug("RecognitionThread wating  hAccessImage");\
		//	WaitForSingleObject(hAccessImage, INFINITE);//һֱ�ȴ����Է���
			debug("RecognitionThread getted  & imagesQueue.size=%d ", dlg->imagesQueue.size());
			if(dlg->imagesQueue.size()<1){
		//		SetEvent(hAccessImage);
				continue;
			}
			p = dlg->imagesQueue.front();
			dlg->imagesQueue.pop();

			debug("RecognitionThread imagesQueue.front ed 0x%x   queue size=%d ", p, dlg->imagesQueue.size());
			memcpy(dlg->imageDataForShow, p, imageSize);
			dlg->imagesQueuePlay.push(dlg->imageDataForShow);//For Play
			SetEvent(hShowVideoFrame);// start play
			memset(dlg->pTF_Result, 0, 20);
			dlg->pTF_Result->fConfidence = 0.0;
			t1 = clock();
			ret = 0;
			
			if(dlg->recognitionMode==PICTURE)
				ret = TFLPR_RecImage( p, dlg->imageWidth, dlg->imageHeight, dlg->pTF_Result, 0, dlg->pLPRInstance);//����ʶ��
			else
				ret = TFLPR_RecImage( p, dlg->imageWidth, dlg->imageHeight, dlg->pTF_Result, &recROI, dlg->pLPRInstance);//����ʶ��
			
			t2 = clock();
		//	SetEvent(hAccessImage);
			dlg->pTF_Result->takesTime = t2-t1;
			debug("RecognitionThread imagesQueue pre release 0x%x", p);

			dlg->timeNow = clock();
			sprintf(temp, "Rate:%d fps", nFrames*1000/(dlg->timeNow - dlg->timeStart));
			debug("RecognitionThread %s",temp );
			dlg->GetDlgItem(ID_STATUS)->SetWindowText(temp);
			
			//����ʶ����
			if(ret>0 && dlg->pTF_Result->fConfidence> 0){
				TF_Result *r = new (TF_Result);
				*r =  *dlg->pTF_Result;
				r->pResultBits = new unsigned char[imageSize];
				if(dlg->pTF_RecParma->iRecMode!=0) //������Ƶ
				{
					if(r->pResultBits!=NULL &&  dlg->pTF_Result->pResultBits!=NULL)
						memcpy(r->pResultBits , dlg->pTF_Result->pResultBits,imageSize );

				}else{ 
					if(dlg->recognitionMode==PICTURE) // ����ͼƬ
						memcpy(r->pResultBits , p, imageSize );
				}
				dlg->GetDlgItem(ID_LPR)->SetWindowText("");
				dlg->LPRQueueResult.push(r);
				debug("RecognitionThread LPRQueueResult.push  0x%x", r);

			}else{
				debug("-------------------------- δʶ�� ERROR[%d]: %s ------------------------------ " , ret, TF_ERROR[ret+3]);
				if(dlg->recognitionMode==PICTURE){
					dlg->GetDlgItem(ID_LPR)->SetWindowText("δʶ��");
					if(!dlg->fastProcess && dlg->imagesQueue.size()>0){//
						p2 = dlg->imagesQueue.front();
						dlg->imagesQueue.pop();
						delete p2;
					}
				}
			}
			delete p;
		}
	}
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("�������");
end:
	dlg->GetDlgItem(ID_STATUS)->SetWindowText("��������");
	debug("RecognitionThread �����˳�");
	SetEvent(handleCanExit);//���ÿ����˳���
	SetEvent(handleExit); 
	
}

//��Ƶ����ȡ�߳�
void VideoThread(void* pParam)
{
	CVLPRDemoDlg *dlg = (CVLPRDemoDlg*)pParam;
	
	HANDLE handleCanExit = dlg->ReginsterMyThread("VideoThread");
	debug("VideoThread ����  handle=0x%x", handleCanExit);
	long size = dlg->imageWidth * dlg->imageHeight * 3;
	unsigned char* pixBits  = 0;
	int ret=0;
	while(WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
		while(dlg->bPause && WaitForSingleObject(handleExit,0)!=WAIT_OBJECT_0){
			Sleep(100);
		}
		if(WaitForSingleObject(handleExit,0)==WAIT_OBJECT_0)
			break;
		if(WaitForSingleObject(handleVideoThread,0)==WAIT_OBJECT_0)
		{
			if(dlg->fastProcess) //��������ģʽ��ȱ��ᶪ֡
				ret = dlg->m_videoplay->getOneFrame();
			if(dlg->imagesQueue.size() < 10 )
			{
				if(!dlg->fastProcess)
					ret = dlg->m_videoplay->getOneFrame();
				if(ret==0)
				{
					dlg->imageWidth = dlg->m_videoplay->imageFrame->width;
					dlg->imageHeight = dlg->m_videoplay->imageFrame->height;
					size = dlg->imageWidth * dlg->imageHeight * 3;
					if(size<1)
						continue;
					dlg->recognitionMode = VIDEO;
					pixBits = new unsigned char[size];\
						memcpy(pixBits,  dlg->m_videoplay->imageFrame->imageData, size);
					dlg->imagesQueue.push(pixBits);\
						debug("VideoThread imagesQueue.push 0x%x   Qsize=%d  nFrame=%d ", pixBits, dlg->imagesQueue.size(), dlg->m_videoplay->iNowFrameNum);
					//	SetEvent(hAccessImage);
					if(dlg->bStop==false)
						SetEvent(handleVideoThread);
				}else{
					if(ret==-2){
						ResetEvent(handleVideoThread);
						debug("��Ƶ��ȡ���");
						MessageBox(0, "��Ƶ��ȡ���","", MB_OK);
					}
					else{
						debug("��Ƶ��ȡʧ��");
						SetEvent(handleVideoThread);
					}
				}
			}else{//if(dlg->imagesQueue.size() < 30 )
				if(dlg->bStop==false)
					SetEvent(handleVideoThread);
			}				
		}else{ // if(WaitForSingleObject(handleVideoThread,0)==WAIT_OBJECT_0)
			Sleep( dlg->imagesQueue.size()<<3 );
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


void CVLPRDemoDlg::OnBnClickedOpenVideo()
{
	//����Ƶ������
	CFileDialog fileOpenDlg(TRUE, _T("*.avi"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"avi files (*.avi;*.mp4) |*.avi;*.mp4;|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) 
		return ;

	int ret =0;
	recognitionMode = VIDEO;
	nFrames = 0;

	mVideoPath = fileOpenDlg.GetPathName();
	
	pTF_RecParma->iRecMode=2;//��Ƶʶ��ģʽ

	char cBuff[256]={0};

	ResetEvent(handleVideoThread);
	if(m_videoplay!=0)
		m_videoplay->release();
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
			if(imageDataForShow!=NULL)
				delete imageDataForShow;
			imageDataForShow = new unsigned char[size];

			VideoUtil::write24BitBmpFile("E:/temp/k1.bmp",imageWidth, imageHeight,(unsigned char*)m_videoplay->imageFrame->imageData);
			
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

	bPlay = true;
	bStop = false;
	bPause = false;

	GetDlgItem(BT_PAUSE)->EnableWindow(true);
	GetDlgItem(BT_STOP)->EnableWindow(true);
//	ReStartThread();
	timeStart = clock();
	SetEvent(handleVideoThread);
	SetEvent(hAccessImage);
	//_beginthread(VideoThread, 0 ,this);//��ȡ��Ƶ֡
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
void CVLPRDemoDlg::PlayVideo(unsigned char *pix){
	if(m_videoplay==NULL || m_videoplay->imageFrame==NULL)
		return;

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

	OnBnClickedStop();
	Sleep(1000);

	GetDlgItem(ID_STATUS)->SetWindowText("�����˳�......");
	if( CloseThread()>0)
		debug("�������˳� @ CDialog::OnClose()  �߳�δȫ���ر�");
	else
		debug("���������˳� @ CDialog::OnClose()");
	Sleep(1000);
	CDialog::OnClose();
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


void CVLPRDemoDlg::OnBnClickedOpenPicture()
{
	//��ͼƬ������
	//FileUtil::SelectFolder(this->m_hWnd, "ѡ��ͼƬ");
	CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), "",OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
		"image files (*.jpg;*.bmp;*.png) |*.jpg;*.bmp;*.png|All Files (*.*)|*.*||",NULL);
	if (fileOpenDlg.DoModal()!=IDOK) return ;

	mPicturePath = fileOpenDlg.GetPathName();
	char p[256] = {0};

	pTF_RecParma->iRecMode=0;//ͼƬʶ��ģʽ
	if(pLPRInstance)
		TFLPR_Uninit(pLPRInstance);

	pLPRInstance = TFLPR_Init(*pTF_RecParma);

	recognitionMode = PICTURE;

	listFiles(mPicturePath);


	Bitmap* image = KLoadBitmap(mPicturePath.GetBuffer(mPicturePath.GetLength()));
	DrawImg2Hdc(image, ID_VIDEO_WALL, this);
	DrawImg2Hdc(image, ID_PICTURE, this);
	LPRFromImage(image);
}


void CVLPRDemoDlg::LPRFromImage(Bitmap* image)
{
	long BufferLen;
	imageWidth = image->GetWidth();
	imageHeight = image->GetHeight();
	BufferLen= imageWidth * imageHeight * 3;

	if(imageDataForShow!=NULL)
		delete imageDataForShow;
	imageDataForShow = new unsigned char[BufferLen];

    unsigned char *buffer = new unsigned char[BufferLen];

	BitmapData bitmapData={0};
	bitmapData.Stride = WIDTHSTEP(imageWidth);
	bitmapData.Scan0 = new BYTE[bitmapData.Stride * imageHeight];
//	if(pImageBuffer!=0)\
		delete pImageBuffer;
	pImageBuffer=new unsigned char[BufferLen];
	Rect rect(0,0, imageWidth, imageHeight);
	image->LockBits(&rect, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &bitmapData); 
	
	memcpy(buffer, (BYTE*)bitmapData.Scan0, BufferLen); 
	
	delete bitmapData.Scan0;
	image->UnlockBits( &bitmapData);

	debug("LPRFromImage imagesQueue.size = %d", imagesQueue.size());
	imagesQueue.push(buffer);//������н��д���
	
}


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
	
	Bitmap* image = KLoadBitmap(filename);
	DrawImg2Hdc(image, ID_VIDEO_WALL, this);
//	DrawImg2Hdc(image, ID_PICTURE, this);
	LPRFromImage(image);

	if(filename!=0)
		delete filename;

	GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(false);
	Sleep(500);
	GetDlgItem(BT_NEXT_PICTURE)->EnableWindow(true);
}

void CVLPRDemoDlg::OnEnChangeImageDir()
{
	
}

void CVLPRDemoDlg::OnBnClickedCheck1()
{
	//��������ģʽ
	UpdateData(true);
	fastProcess = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
}

void CVLPRDemoDlg::OnBnClickedSetConfig()
{
	//

	if(ReadConfig())
	{
		configDlg.m_imageMaxWidth = plateMaxWidth;
		configDlg.m_imageMinWidth = plateMinWidth;
		configDlg.m_imageDir = m_imageDir;
	}
	if(configDlg.DoModal()==IDOK){
		plateMaxWidth = configDlg.m_imageMaxWidth;
		plateMinWidth = configDlg.m_imageMinWidth;
		m_imageDir = configDlg.m_imageDir;
		FileUtil::CreateFolders(m_imageDir.GetBuffer(m_imageDir.GetLength()));

	 	pTF_RecParma->iMaxPlateWidth = plateMaxWidth;
		pTF_RecParma->iMinPlateWidth = plateMinWidth;

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

		temp = m_list.GetItemText(nItem, 9 );
		Bitmap* imagePlate = KLoadBitmap(temp.GetBuffer(temp.GetLength()));
		DrawImg2Hdc(imagePlate, ID_LPR_PICTURE, this);

		temp = m_list.GetItemText(nItem, 10 );
		Bitmap* imageScreen = KLoadBitmap(temp.GetBuffer(temp.GetLength()));
		DrawImg2Hdc(imageScreen, ID_PICTURE, this);

	}
	*pResult = 0;
}
