#include <windows.h>
#include <string>
#include <map>
using namespace std;

extern "C" __declspec(dllexport) wstring* CreateWString(wchar_t *wszStr)
{
	wstring *wscStr = new wstring;
	*wscStr = wszStr;
	return wscStr;
}

extern "C" __declspec(dllexport) void FreeWString(wstring *wscStr)
{
	delete wscStr;
}

extern "C" __declspec(dllexport) string* CreateString(char *szStr)
{
	string *scStr = new string;
	*scStr = szStr;
	return scStr;
}

extern "C" __declspec(dllexport) void FreeString(string *scStr)
{
	delete scStr;
}
