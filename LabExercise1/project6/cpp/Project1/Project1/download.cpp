#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <urlmon.h>

#pragma comment(lib, "urlmon.lib")

using namespace std;

BOOL FileExistsStatus(const CHAR* path)
{
	DWORD dwAttribute = GetFileAttributes(path);
	if (dwAttribute == 0XFFFFFFFF) return false; //0XFFFFFFFF表示文件不存在
	else return true;
}

BOOL DownloadFiles(const CHAR* url, const CHAR* downloadPath)
{
	if (URLDownloadToFile(NULL, url, downloadPath, 0, 0) == S_OK && FileExistsStatus(downloadPath)) return true;
	else return false;
}

int main(int argc, char* argv[])
{
	if (DownloadFiles(argv[1], argv[2])) printf("OK!\n");
	else printf("Error!\n");
	getchar();
	return 0;
}
