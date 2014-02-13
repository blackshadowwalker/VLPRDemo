#pragma once

//#include <list>
//#include <string>
//using namespace std;

#ifndef MAX_PATH
	#define MAX_PATH 256
#endif

class FileUtil
{
public:
	FileUtil(void);
	~FileUtil(void);

	//static list<string>	ListFiles(const char *czPath, list<string> listResult, bool bListDirectory, bool bListSub);
	static char* SelectFolder(HWND hwnd, char* title="Select Folder");
	static BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter);

	bool CreateFolders(const char* folderPath);
	bool RemoveDir(const char* szFileDir);
private:
	


};

char *GetDateTime(char *timeString=0);
void __cdecl debug(const char *format, ...);