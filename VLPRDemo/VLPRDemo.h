
// VLPRDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������

// CVLPRDemoApp:
// �йش����ʵ�֣������ VLPRDemo.cpp
//

class CVLPRDemoApp : public CWinAppEx
{
public:
	CVLPRDemoApp();
	static CString m_appPath;

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CVLPRDemoApp theApp;