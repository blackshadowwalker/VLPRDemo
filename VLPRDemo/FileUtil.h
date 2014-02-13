#pragma once

//#include <list>
//#include <string>
//using namespace std;

#include <list>
using namespace std;

#ifndef MAX_PATH
	#define MAX_PATH 256
#endif

class FileUtil
{
public:
	FileUtil(void);
	~FileUtil(void);

	static list<char*>* ListFiles(char *czPath, list<char*> &listResult, char *fileExt="*.*",bool fullPathName=false, bool bListDirectory=false, bool bListSub=false);
	static char* SelectFolder(HWND hwnd, char* title="Select Folder");
	static BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter);

	static bool CreateFolders(const char* folderPath);
	static bool RemoveDir(const char* szFileDir);
private:
	


};

char *GetDateTime(char *timeString=0);
void __cdecl debug(const char *format, ...);