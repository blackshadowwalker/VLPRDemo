

#include <windows.h>  
#include <ShlObj.h> 
#include <stdio.h>
#include <time.h>   
#include <string>

#include "FileUtil.h"

using namespace std;

FileUtil::FileUtil(void)
{
}

FileUtil::~FileUtil(void)
{
}
//ɾ��Ŀ¼���ļ�
bool FileUtil::RemoveDir(const char* szFileDir)
{
	
	if(szFileDir==NULL  || !FindFirstFileExists(szFileDir, FILE_ATTRIBUTE_DIRECTORY) )
		return false;
	char *strDir =new char[256];
	char fileName[256] ={0}, temp[256]={0};
	memset(strDir,0, 256);
	memcpy(strDir, szFileDir, strlen(szFileDir));
	sprintf(temp, "%s\\*.*",strDir);
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile((temp),&wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		sprintf(fileName, "%s\\%s", strDir, wfd.cFileName);
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (stricmp(wfd.cFileName,".") != 0 &&
				stricmp(wfd.cFileName,"..") != 0)
				RemoveDir(fileName);
		}
		else
		{
			DeleteFile(fileName);
		}
	}
	while (FindNextFile(hFind,&wfd));
		FindClose(hFind);
	RemoveDirectory(szFileDir);
	delete strDir;
	
	return true;
	
} 

//�����༶Ŀ¼
bool FileUtil::CreateFolders(const char* folderPath)
{
	char  folder[MAX_PATH]={0};
	char  temp[MAX_PATH]={0};
	char *p=0;
	memcpy(folder, folderPath, strlen(folderPath));

	int len = strlen(folder);
	while( (p = strchr(folder, '\\'))!=NULL){
		*p='/';
	}
	if( folder[len-1]!='/')
		folder[len] = '/';
	len = strlen(folder);
	folder[len] = '\0';

	int index=2;
	BOOL isOk = true;
	
	p = folder;
	while((p = strchr(p, '/'))!=NULL)
	{
		memcpy(temp, folder, p-folder);
		p ++;
		isOk = (BOOL)CreateDirectory( temp, 0 );
		//if(!isOk�� 
		//	return false;
	}
	isOk = (BOOL)CreateDirectory( temp, 0 );
	
	return true;
}

/**
* �г�Ŀ¼�µ������ļ�
* @czPath : �ļ�Ŀ¼
* @listResult: ���ص��ļ��б�
* @fileExt: �ļ���չ���ƣ��� *.jpg
* @fullPathName: ���ص��ļ������Ƿ���дȫ·��������ֻ��д����
* @bListDirectory: �ļ��������Ƿ�����ļ��б�
* @bListSub: �Ƿ��г����ļ����е��ļ�
*
* @Return : &listResult
*/
list<char*>* FileUtil::ListFiles(char *czPath, list<char*> &listResult, char *fileExt, bool fullPathName, bool bListDirectory, bool bListSub)
{
	char file[MAX_PATH];
	lstrcpy(file,czPath);
	lstrcat(file,"\\");
	lstrcat(file,fileExt);

	WIN32_FIND_DATA wfd; 
	HANDLE Find = FindFirstFile(file,&wfd); 
	if (Find == INVALID_HANDLE_VALUE)  
		return NULL;

	
	while (FindNextFile(Find, &wfd))
	{
		if (wfd.cFileName[0] == '.') 
		{
			continue;
		} 
		char *szFindPath = new char[MAX_PATH];
		lstrcpy(szFindPath,czPath); 
		lstrcat(szFindPath,"\\"); 
		lstrcat(szFindPath,wfd.cFileName); 

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
		{
			if(bListSub)
				ListFiles(szFindPath, listResult,fileExt, bListDirectory, bListSub);  

			if(fullPathName != true)
				sprintf(szFindPath, wfd.cFileName, MAX_PATH);
			if(bListDirectory)
				listResult.push_back( szFindPath );
		}
		else{
			if(fullPathName != true)
				sprintf(szFindPath, wfd.cFileName, MAX_PATH);
			listResult.push_back( szFindPath );

		}
	}
	FindClose(Find);

	return &listResult;

}




//ѡ���ļ��� �Ի���
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x0040
#endif
char* FileUtil::SelectFolder(HWND hwnd, char* title)
{
	char *szFolder = new char[MAX_PATH]; //�õ��ļ�·��	
	memset(szFolder, 0, MAX_PATH);

	//HWND hwnd = hWnd->GetSafeHwnd();   //�õ����ھ��
#ifdef _SHGetMalloc_  
	LPMALLOC pMalloc;
	if (::SHGetMalloc(&pMalloc) == NOERROR)		//ȡ��IMalloc�������ӿ�
	{   
		BROWSEINFO		bi;
		TCHAR			pszBuffer[MAX_PATH];
		LPITEMIDLIST	pidl;   

		bi.hwndOwner		= hwnd;
		bi.pidlRoot			= NULL;
		bi.pszDisplayName   = pszBuffer;
		bi.lpszTitle		= _T(title); //ѡ��Ŀ¼�Ի�����ϲ��ֵı���
		//����½��ļ��а�ť BIF_NEWDIALOGSTYLE
		bi.ulFlags			=  BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS;
		bi.lpfn				= NULL;
		bi.lParam			= 0;
		bi.iImage			= 0;
		if ((pidl = ::SHBrowseForFolder(&bi)) != NULL)  //ȡ��IMalloc�������ӿ�
		{   
			if (::SHGetPathFromIDList(pidl, pszBuffer)) //���һ���ļ�ϵͳ·��
			{
				sprintf(szFolder, "%s",  pszBuffer);
			}
			else szFolder = '\0';
			pMalloc->Free(pidl);	//�ͷ��ڴ�

			//	MessageBox(m_strPath);
		}
		pMalloc->Release();			//�ͷŽӿ�
	}
#else  
	BROWSEINFO bi;  
	ZeroMemory(&bi,sizeof(BROWSEINFO));  
	LPITEMIDLIST pidl=(LPITEMIDLIST)CoTaskMemAlloc(sizeof(LPITEMIDLIST));  
	pidl = SHBrowseForFolder(&bi);  
	TCHAR * path = new TCHAR[MAX_PATH];  
	memset(path, 0, MAX_PATH);
	if(pidl != NULL)  
	{  
		if(SHGetPathFromIDList(pidl,path))
			sprintf(szFolder, "%s",  path);
		//MessageBox(NULL,path,TEXT(title),MB_OK);  
	}  
	else 
	{  
		delete szFolder;
		szFolder = 0;
	  //	MessageBox(NULL,TEXT("EMPTY"),TEXT("Choose"),MB_OK);  
	}  
	CoTaskMemFree(pidl);  
	delete path;   
#endif  

	return szFolder;
}


//�ж��ļ�/�ļ����Ƿ����
//�ж��ļ���	FindFirstFileExists(lpPath, FALSE);  
//�ж��ļ��У�	FindFirstFileExists(lpPath, FILE_ATTRIBUTE_DIRECTORY);  
BOOL FileUtil::FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter)  
{  
	WIN32_FIND_DATA fd;  
	HANDLE hFind = FindFirstFile(lpPath, &fd);  
	BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;  
	BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;  
	FindClose(hFind);  
	return RetValue;  
}  

char *GetDateTime(char *timeString)
{
	struct tm *tmt = 0;
	time_t t = time(0);
	tmt = localtime(&t);
	if(timeString==0)
		timeString = new char[32];
	memset(timeString, 0, 32);
	sprintf(timeString,"%4d-%02d-%02d  %02d:%02d:%02d", tmt->tm_year+1900, tmt->tm_mon+1, tmt->tm_mday, tmt->tm_hour, tmt->tm_min, tmt->tm_sec );
	return timeString;
}

void __cdecl debug(const char *format, ...)
{
	return ;

	char buf[4096]={0}, *p=buf;
	
	char *t = GetDateTime();
	sprintf(p,"%s ",t);
	p += strlen(p);

    va_list args;
    va_start(args, format);
    p += _vsnprintf(p, sizeof buf - 1, format, args);
    va_end(args);
  // while ( p > buf && isspace(p[-1]) )    *--p = '\0';
    *p++ = '\r';
    *p++ = '\n';
    *p = '\0';

    OutputDebugString(buf);
}