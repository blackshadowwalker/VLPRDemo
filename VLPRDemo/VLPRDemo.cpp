
// VLPRDemo.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "VLPRDemo.h"
#include "VLPRDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVLPRDemoApp

BEGIN_MESSAGE_MAP(CVLPRDemoApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CVLPRDemoApp ����

CVLPRDemoApp::CVLPRDemoApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CVLPRDemoApp ����

CVLPRDemoApp theApp;


// CVLPRDemoApp ��ʼ��

CString CVLPRDemoApp::m_appPath="";

BOOL CVLPRDemoApp::InitInstance()
{
//	skinppLoadSkin(_T("skin/AquaOS.ssk"));

	CString g_szOcxPath = this->m_pszHelpFilePath;
	CString g_szOcxName = this->m_pszExeName;

	g_szOcxName += ".HLP";
	int nTmp = g_szOcxName.GetLength();
	nTmp = g_szOcxPath.GetLength() - nTmp;
	g_szOcxPath = g_szOcxPath.Left(nTmp-1);

	m_appPath = g_szOcxPath;


	//��ʼ��GDI+.
	GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CVLPRDemoDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
