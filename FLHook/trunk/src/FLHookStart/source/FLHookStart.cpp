#include <windows.h>
#include <stdio.h>
#include <string>
using namespace std;

class CAccount
{
	CAccount(CAccount &c);
	short s1;
	float f1;
	double d1;
};

typedef int (*_RockOn)(bool bStart, char *szCmdLine);

_RockOn RockOn;

void main(void)
{
	HMODULE hFLHook = LoadLibrary("FLHook.dll");

	wstring w;
	w = L"bla";

	RockOn = (_RockOn)GetProcAddress(hFLHook, "RockOn");
	if(strstr(GetCommandLine(), "execute")) {		
		RockOn(true, strstr(GetCommandLine(), "execute") + strlen("execute"));
	} else
		RockOn(false, "");

	FreeLibrary(hFLHook);
	return;
}