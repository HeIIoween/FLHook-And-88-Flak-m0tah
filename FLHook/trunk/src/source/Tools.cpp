#include <time.h>
#include "global.h"
#include <Psapi.h>
#include "exceptioninfo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring stows(string scText)
{
	int iSize = MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, 0, 0);
	wchar_t *wszText = new wchar_t[iSize];
	MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, wszText, iSize);
	wstring wscRet = wszText;
	delete[] wszText;
	return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string wstos(wstring wscText)
{
	uint iLen = (uint)wscText.length() + 1;
	char *szBuf = new char[iLen];
	WideCharToMultiByte(CP_ACP, 0, wscText.c_str(), -1, szBuf, iLen, 0, 0);
	string scRet = szBuf;
	delete[] szBuf;
	return scRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string itos(int i)
{
	char szBuf[16];
	sprintf(szBuf, "%d", i);
	return szBuf;
}

string utos(uint i)
{
	char szBuf[16];
	sprintf(szBuf, "%u", i);
	return szBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string ftos(float f)
{
	char szBuf[16];
	sprintf(szBuf, "%f", f);
	return szBuf;
}

wstring ftows(float f)
{
	wchar_t wszBuf[16];
	swprintf(wszBuf, L"%g", f);
	return wszBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring ToLower(wstring wscStr)
{
	for(uint i = 0; (i < wscStr.length()); i++)
		wscStr[i] = towlower(wscStr[i]);

	return wscStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string ToLower(string scStr)
{
	for(uint i = 0; (i < scStr.length()); i++)
		scStr[i] = tolower(scStr[i]);

	return scStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ToInt(wstring wscStr)
{
	return _wtoi(wscStr.c_str());
}

int ToInt(string scStr)
{
	return atoi(scStr.c_str());
}

uint ToUint(wstring wscStr)
{
	return _wtol(wscStr.c_str());
}

uint ToUint(string scStr)
{
	return atol(scStr.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ToFloat(wstring wscStr)
{
	return (float)_wtof(wscStr.c_str());
}

float ToFloat(string scStr)
{
	return (float)atof(scStr.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 999.999.999

wstring ToMoneyStr(int iCash)
{
	int iMillions = iCash / 1000000;
	int iThousands = (iCash % 1000000) / 1000;
	int iRest = (iCash % 1000);
	wchar_t wszBuf[32];
	
	if(iMillions)
		swprintf(wszBuf, L"%d,%.3d,%.3d", iMillions, iThousands, iRest);
	else if(iThousands)
		swprintf(wszBuf, L"%d,%.3d", iThousands, iRest);
	else
		swprintf(wszBuf, L"%d", iRest);

	return wszBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string IniGetS(string scFile, string scApp, string scKey, string scDefault)
{
	char szRet[2048];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(), szRet, sizeof(szRet), scFile.c_str());
	return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(string scFile, string scApp, string scKey, int iDefault)
{
	return GetPrivateProfileInt(scApp.c_str(), scKey.c_str(), iDefault, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(string scFile, string scApp, string scKey, float fDefault)
{
	char szRet[2048];
	char szDefault[16];
	sprintf(szDefault, "%f", fDefault);
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), szDefault, szRet, sizeof(szRet), scFile.c_str());
	return (float)atof(szRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(string scFile, string scApp, string scKey, bool bDefault)
{
	return ToLower(IniGetS(scFile, scApp, scKey, bDefault ? "yes" : "no")).compare("yes") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(string scFile, string scApp, string scKey, string scValue)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(string scFile, string scApp, string scKey, wstring wscValue)
{
	string scValue = "";
	for(uint i = 0; (i < wscValue.length()); i++)
	{
		char cHiByte = wscValue[i] >> 8;
		char cLoByte = wscValue[i] & 0xFF;
		char szBuf[8];
		sprintf(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		scValue += szBuf;
	}
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring IniGetWS(string scFile, string scApp, string scKey, wstring wscDefault)
{
	char szRet[2048];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet, sizeof(szRet), scFile.c_str());
	string scValue = szRet;
	if(!scValue.length())
		return wscDefault;

	wstring wscValue = L"";
	long lHiByte;
	long lLoByte;
	while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}

	return wscValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(string scFile, string scApp)
{
	WritePrivateProfileSection(scApp.c_str(), "", scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniGetSection(string scFile, string scApp, list<INISECTIONVALUE> &lstValues)
{
	lstValues.clear();
	char szBuf[0xFFFF];
	GetPrivateProfileSection(scApp.c_str(), szBuf, sizeof(szBuf), scFile.c_str());
	char *szNext = szBuf;
	while(strlen(szNext) > 0)
	{
		INISECTIONVALUE isv;
		char szKey[0xFFFF] = "";
		char szValue[0xFFFF] = "";
		sscanf(szNext, "%[^=]=%[^\n]", szKey, szValue);
		isv.scKey = szKey;
		isv.scValue = szValue;
		lstValues.push_back(isv);

		szNext += strlen(szNext) + 1;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring XMLText(wstring wscText)
{
	wstring wscRet;
	for(uint i = 0; (i < wscText.length()); i++)
	{
		if(wscText[i] == '<')
			wscRet.append(L"&#60;");
		else if(wscText[i] == '>')
			wscRet.append(L"&#62;");
		else if(wscText[i] == '&')
			wscRet.append(L"&#38;");
		else
			wscRet.append(1, wscText[i]);
	}

	return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteProcMem(void *pAddress, void *pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadProcMem(void *pAddress, void *pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	ReadProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddLog(const char *szString, ...)
{
	char szBufString[1024];
	va_list marker;
	va_start(marker, szString);
	_vsnprintf(szBufString, sizeof(szBufString)-1, szString, marker);

	char szBuf[64];
	time_t tNow = time(0);
	struct tm *t = localtime(&tNow);
	strftime(szBuf, sizeof(szBuf), "%d.%m.%Y %H:%M:%S", t);
	fprintf(fLog, "[%s] %s\n", szBuf, szBufString);
	fflush(fLog);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring GetParam(wstring wscLine, wchar_t wcSplitChar, uint iPos)
{
	uint i = 0, j = 0;
 
	wstring wscResult = L"";
	for(i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++)
	{
		if(wscLine[j] == wcSplitChar)
		{
			while(((j + 1) < wscLine.length()) && (wscLine[j+1] == wcSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}
 
		if(i == iPos)
			wscResult += wscLine[j];
	}
 
	return wscResult;
}

string GetParam(string scLine, char cSplitChar, uint iPos)
{
	uint i = 0, j = 0;
 
	string scResult = "";
	for(i = 0, j = 0; (i <= iPos) && (j < scLine.length()); j++)
	{
		if(scLine[j] == cSplitChar)
		{
			while(((j + 1) < scLine.length()) && (scLine[j+1] == cSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}
 
		if(i == iPos)
			scResult += scLine[j];
	}
 
	return scResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring GetParamToEnd(wstring wscLine, wchar_t wcSplitChar, uint iPos)
{
	for(uint i = 0, iCurArg = 0; (i < wscLine.length()); i++)
	{
		if(wscLine[i] == wcSplitChar)
		{
			iCurArg++;

			if(iCurArg == iPos)
				return wscLine.substr(i + 1);

			while(((i + 1) < wscLine.length()) && (wscLine[i+1] == wcSplitChar))
				i++; // skip "whitechar"
		}
	}

	return L"";
}

string GetParamToEnd(string scLine, char cSplitChar, uint iPos)
{
	for(uint i = 0, iCurArg = 0; (i < scLine.length()); i++)
	{
		if(scLine[i] == cSplitChar)
		{
			iCurArg++;

			if(iCurArg == iPos)
				return scLine.substr(i + 1);

			while(((i + 1) < scLine.length()) && (scLine[i+1] == cSplitChar))
				i++; // skip "whitechar"
		}
	}

	return "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring Trim(wstring wscIn)
{
	while(wscIn.length() && (wscIn[0]==L' ' || wscIn[0]==L'	' || wscIn[0]==L'\n' || wscIn[0]==L'\r') )
	{
		wscIn = wscIn.substr(1);
	}
	while(wscIn.length() && (wscIn[wscIn.length()-1]==L' ' || wscIn[wscIn.length()-1]==L'	' || wscIn[wscIn.length()-1]==L'\n' || wscIn[wscIn.length()-1]==L'\r') )
	{
		wscIn = wscIn.substr(0, wscIn.length()-1);
	}
	return wscIn;
}

string Trim(string scIn)
{
	while(scIn.length() && (scIn[0]==' ' || scIn[0]=='	' || scIn[0]=='\n' || scIn[0]=='\r') )
	{
		scIn = scIn.substr(1);
	}
	while(scIn.length() && (scIn[scIn.length()-1]==L' ' || scIn[scIn.length()-1]=='	' || scIn[scIn.length()-1]=='\n' || scIn[scIn.length()-1]=='\r') )
	{
		scIn = scIn.substr(0, scIn.length()-1);
	}
	return scIn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring ReplaceStr(wstring wscSource, wstring wscSearchFor, wstring wscReplaceWith)
{
	uint lPos, sPos = 0;

	while((lPos = (uint)wscSource.find(wscSearchFor, sPos)) != -1)
	{
		wscSource.replace(lPos, wscSearchFor.length(), wscReplaceWith);
		sPos = lPos + wscReplaceWith.length();
	}

	return wscSource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

mstime timeInMS()
{
	mstime iCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
	mstime iFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
	return iCount / (iFreq / 1000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void dumpHex(string scFile, void *ptr, uint iNumBytes)
{
	FILE *f = fopen(scFile.c_str(), "wb");
	if(!f)
		return;

	char *cPtr = (char*)ptr;
	for(uint i=0; i<iNumBytes; i++)
	{
		fprintf(f, "%c", *cPtr);
		cPtr++;
	}

	fclose(f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SwapBytes(void *ptr, uint iLen)
{
	if(iLen % 4)
		return;

	for(uint i=0; i<iLen; i+=4)
	{
		char *ptr1 = (char*)ptr + i;
		unsigned long temp;
		memcpy(&temp, ptr1, 4);
		char *ptr2 = (char*)&temp;
		memcpy(ptr1, ptr2+3, 1);
		memcpy(ptr1+1, ptr2+2, 1);
		memcpy(ptr1+2, ptr2+1, 1);
		memcpy(ptr1+3, ptr2, 1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CmpStrStart(const string &scSource, const char *scFind)
{
	for(uint i = 0; i < scSource.length() && scFind[i]; i++)
	{
		if(scSource[i] != scFind[i])
			return false;
	}
	return true;
}

bool CmpStrStart(const wstring &wscSource, const wchar_t *wscFind)
{
	for(uint i = 0; i < wscSource.length() && wscFind[i]; i++)
	{
		if(wscSource[i] != wscFind[i])
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HMODULE GetModuleAddr(uint iAddr)
{
	HMODULE hModArr[1024];
	DWORD iArrSizeNeeded;
	HANDLE hProcess = GetCurrentProcess();
	if(EnumProcessModules(hProcess, hModArr, sizeof(hModArr), &iArrSizeNeeded))
	{
		if(iArrSizeNeeded > sizeof(hModArr))
			iArrSizeNeeded = sizeof(hModArr);
		iArrSizeNeeded /= sizeof(HMODULE);
		for(uint i = 0; i < iArrSizeNeeded; i++)
		{
			MODULEINFO mi;
			if(GetModuleInformation(hProcess, hModArr[i], &mi, sizeof(mi)))
			{
				if(((uint)mi.lpBaseOfDll) < iAddr && (uint)mi.lpBaseOfDll + (uint)mi.SizeOfImage > iAddr)
				{
					return hModArr[i];
				}
			}
		}
	}
	return 0;
}

void AddExceptionInfoLog()
{
	try{
		EXCEPTION_RECORD const *exception = GetCurrentExceptionRecord();
		_CONTEXT const *reg = GetCurrentExceptionContext();

		if(exception)
		{
			DWORD iCode = exception->ExceptionCode;
			uint iAddr = (uint)exception->ExceptionAddress;
			uint iOffset = 0;
			HMODULE hModExc = GetModuleAddr(iAddr);
			char szModName[MAX_PATH] = "";
			if(hModExc)
			{
				iOffset = iAddr - (uint)hModExc;
				GetModuleFileName(hModExc, szModName, sizeof(szModName));
			}

			AddLog("Code=%x Offset=%x Module=\"%s\"", iCode, iOffset, strrchr(szModName, '\\')+1);
			if(iCode == 0xE06D7363 && exception->NumberParameters == 3) //C++ exception
			{
				_s__ThrowInfo *info = (_s__ThrowInfo*)exception->ExceptionInformation[2];
				const _s__CatchableType *const (*typeArr)[] = &info->pCatchableTypeArray->arrayOfCatchableTypes;
				std::exception *obj = (std::exception*)exception->ExceptionInformation[1];
				const char *szName = info && info->pCatchableTypeArray && info->pCatchableTypeArray->nCatchableTypes ? (*typeArr)[0]->pType->name : "";
				int i = 0;
				for(; i < info->pCatchableTypeArray->nCatchableTypes; i++)
				{
					if(!strcmp(".?AVexception@@", (*typeArr)[i]->pType->name))
						break;
				}
				const char *szMessage = i != info->pCatchableTypeArray->nCatchableTypes ? obj->what() : "";
				//C++ exceptions are triggered by RaiseException in kernel32, so use ebp to get the return address
				iOffset = 0;
				szModName[0] = 0;
				if(reg)
				{
					iAddr = *((*((uint**)reg->Ebp)) + 1);
					hModExc = GetModuleAddr(iAddr);
					if(hModExc)
					{
						iOffset = iAddr - (uint)hModExc;
						GetModuleFileName(hModExc, szModName, sizeof(szModName));
					}
				}
				AddLog("Name=\"%s\" Message=\"%s\" Offset=%x Module=\"%s\"", szName, szMessage, iOffset, strrchr(szModName, '\\')+1);
			}	
		}
		else
			AddLog("No exception information available");
		if(reg)
			AddLog("eax=%x ebx=%x ecx=%x edx=%x edi=%x esi=%x ebp=%x eip=%x esp=%x",
				reg->Eax, reg->Ebx, reg->Ecx, reg->Edx, reg->Edi, reg->Esi, reg->Ebp, reg->Eip, reg->Esp);
		else
			AddLog("No register information available");
	} catch(...) { AddLog("Exception in AddExceptionInfoLog!"); }
}