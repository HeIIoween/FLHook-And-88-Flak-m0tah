#include "global.h"
#include <Psapi.h>
#include "hook.h"
#include "CConsole.h"
#include "CSocket.h"

// structs
struct SOCKET_CONNECTION
{
	wstring wscPending;
	CSocket	csock;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE hProcFL = 0;
HMODULE hModServer = 0;
HMODULE hModCommon = 0;
HMODULE hModRemoteClient = 0;
HMODULE hMe = 0;
HMODULE hModDPNet = 0;
HMODULE hModDaLib = 0;
HMODULE hModContent = 0;
HANDLE hConsoleThread;
HMODULE hWString;

HANDLE hConsoleIn;
HANDLE hConsoleOut;
HANDLE hConsoleErr;

SOCKET sListen = INVALID_SOCKET;
SOCKET sWListen = INVALID_SOCKET;
SOCKET sEListen = INVALID_SOCKET;
SOCKET sEWListen = INVALID_SOCKET;

list<wstring*> lstConsoleCmds;
list<SOCKET_CONNECTION*> lstSockets;
list<SOCKET_CONNECTION*> lstDelete;

CRITICAL_SECTION cs;

FILE *fLog;

bool bExecuted = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

_CreateWString CreateWString;
_FreeWString FreeWString;
_CreateString CreateString;
_FreeString FreeString;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Init();

/**************************************************************************************************************
DllMain
**************************************************************************************************************/

FARPROC fpOldUpdate;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(bExecuted)
		return TRUE;

	char szFile[MAX_PATH];
	GetModuleFileName(0, szFile, sizeof(szFile));
	wstring wscFileName = ToLower(stows(szFile));

	if(wscFileName.find(L"flserver.exe") != -1)
	{ // start FLHook
		bExecuted = true;

		// redirect IServerImpl::Update
		FARPROC fpLoop = (FARPROC)HkIServerImpl::Update;
		void *pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_UPDATE);
		ReadProcMem(pAddress, &fpOldUpdate, 4);
		WriteProcMem(pAddress, &fpLoop, 4);
	}
	return TRUE;
}

/**************************************************************************************************************
inject FLHook.dll into running flserver.exe or start flserver.exe and then inject it
**************************************************************************************************************/

extern "C" int __declspec(dllexport) RockOn(bool bStart, char *szCmdLine)
{
	DWORD dwNeeded;

	if(bStart) {
		STARTUPINFO si;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		si.wShowWindow = SW_SHOWNORMAL;
		PROCESS_INFORMATION pi;
		if(!CreateProcess("flserver.exe", szCmdLine, 0, 0, FALSE, /*CREATE_SUSPENDED*/ 0, 0, 0, &si, &pi))
		{
			printf("ERROR: flserver.exe could not be executed\n");
			system("pause");
			return 0;
		}

		printf("flserver.exe executed...\n");
		Sleep(1000); // bad but working 8[
		hProcFL = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
	} else {
		DWORD dwProcesses[10000];
		EnumProcesses((DWORD*)&dwProcesses, sizeof(dwProcesses), &dwNeeded);

		for(uint i = 0; (i < dwNeeded/sizeof(DWORD)); i++)
		{
			char szFile[MAX_PATH];

			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcesses[i]);

/*			if(!GetProcessImageFileName(hProcess, szFile, sizeof(szFile)))
				continue;

			strlwr(szFile);
			if(strstr(szFile, "flserver.exe"))
			{
				hProcFL = hProcess;
				break;
			} */

			if(hProcess)
			{
				HMODULE hMod;
				DWORD cbNeeded;
				DWORD dwNameLen = 0;
				if(EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
					dwNameLen = GetModuleBaseName(hProcess, hMod, szFile, sizeof(szFile));

				_strlwr(szFile);
				if(strstr(szFile, "flserver.exe"))
				{
					hProcFL = hProcess;
					break;
				}

				CloseHandle(hProcess); // wenn nicht flserver , dann handle zumachen
			}
		} 

		if(!hProcFL)
		{
			printf("ERROR: flserver.exe is not executed\n");
			system("pause");
			return 0;
		} 

		printf("flserver.exe process found...\n");
	}

	HMODULE hModules[1024];
	EnumProcessModules(hProcFL, hModules, sizeof(hModules), &dwNeeded);
	for(uint i = 0; (i < (dwNeeded / sizeof(HMODULE))); i++)
	{
		char szModName[MAX_PATH];

		if(GetModuleFileNameEx(hProcFL, hModules[i], szModName, sizeof(szModName)))
		{
			string scModName = ToLower(szModName);
			if(scModName.find("flhook.dll") != -1)
			{
				printf("ERROR: flhook.dll already loaded\n");
				return 0;
			}
		}
	}

	// start injection...
	// get path to me (dll)
	char szMe[MAX_PATH];
	GetModuleFileName(GetModuleHandle("FLHook"), szMe, sizeof(szMe));

	// load library
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA");

	void *pMem = VirtualAllocEx(hProcFL, 0, strlen(szMe) + 1, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcFL, pMem, szMe, strlen(szMe) + 1, 0);
	DWORD id;
	HANDLE hThread = CreateRemoteThread(hProcFL, 0, 0, pfnThreadRtn, pMem, 0, &id);
	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcFL, pMem, 0, MEM_RELEASE);
	return 1;
}

/**************************************************************************************************************
thread that reads console input
**************************************************************************************************************/

void ReadConsoleEvents()
{
	while(1)
	{
		DWORD dwBytesRead;
		char szCmd[1024];
		memset(szCmd, 0, sizeof(szCmd));
		if(ReadConsole(hConsoleIn, szCmd, sizeof(szCmd), &dwBytesRead, 0))
		{
			string scCmd = szCmd;
			if(scCmd[scCmd.length()-1] == '\n')
				scCmd = scCmd.substr(0, scCmd.length()-1);
			if(scCmd[scCmd.length()-1] == '\r')
				scCmd = scCmd.substr(0, scCmd.length()-1);

			wstring wscCmd = stows(scCmd);
			EnterCriticalSection(&cs);
			wstring *pwscCmd = new wstring;
			*pwscCmd = wscCmd;
			lstConsoleCmds.push_back(pwscCmd);
			LeaveCriticalSection(&cs);
		}
	} 
}

/**************************************************************************************************************
handles console events
**************************************************************************************************************/

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
	switch(dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		{
			FLHookInitUnload();
			return TRUE;
		} break;
	}

	return FALSE;
}

/**************************************************************************************************************
init
**************************************************************************************************************/

bool FLHookInit()
{
	bool bInitHookExports = false;
	InitializeCriticalSection(&cs);
	hProcFL = GetModuleHandle(0);

	try {
		// start console
		AllocConsole();
		SetConsoleTitle("FLHook");
		SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);

		ConPrint(L"Welcome to FLHook Console (" VERSION L")\n");

		DWORD id;
		DWORD dwParam;
		hConsoleThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReadConsoleEvents, &dwParam, 0, &id);

		// get module handles
		fLog = fopen("FLHook.log", "at");
		if(!(hModServer = GetModuleHandle("server")))
			throw "server.dll not loaded";
		if(!(hModRemoteClient = GetModuleHandle("remoteclient")))
			throw "remoteclient.dll not loaded";
		if(!(hModCommon = GetModuleHandle("common")))
			throw "common.dll not loaded";
		if(!(hModDPNet = GetModuleHandle("dpnet")))
			throw "dpnet.dll not loaded";
		if(!(hModDaLib = GetModuleHandle("dalib")))
			throw "dalib.dll not loaded";
		if(!(hModContent = GetModuleHandle("content")))
			throw "content.dll not loaded";
		if(!(hMe = GetModuleHandle("FLHook")))
			throw "FLHook.dll not loaded";
		if(!(hWString = LoadLibrary("FLHookWString.dll")))
			throw "FLHookWString.dll not found";
		if(!(CreateWString = (_CreateWString)GetProcAddress(hWString, "CreateWString")))
			throw "FLHookWString.dll: CreateWString not found";
		if(!(FreeWString = (_FreeWString)GetProcAddress(hWString, "FreeWString")))
			throw "FLHookWString.dll: FreeWString not found";
		if(!(CreateString = (_CreateString)GetProcAddress(hWString, "CreateString")))
			throw "FLHookWString.dll: CreateString not found";
		if(!(FreeString = (_FreeString)GetProcAddress(hWString, "FreeString")))
			throw "FLHookWString.dll: FreeString not found";

		// load settings
		LoadSettings();

		// init hooks
		if(!InitHookExports())
			throw "InitHookExports failed";

		bInitHookExports = true;

		if(set_bSocketActivated)
		{ // listen to socket
			WSADATA wsa;
			WSAStartup(MAKEWORD(2, 0), &wsa);
			if(set_bSocketActivated)
			{

				if(set_iPort)
				{
					sListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iPort);
					if(bind(sListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "ascii: socket-bind failed, port already in use?";

					if(listen(sListen, SOMAXCONN) != 0)
						throw "ascii: socket-listen failed";

					ConPrint(L"socket(ascii): socket connection listening\n");
				}

				if(set_iWPort)
				{
					sWListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iWPort);
					if(bind(sWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "unicode: socket-bind failed, port already in use?";

					if(listen(sWListen, SOMAXCONN) != 0)
						throw "unicode: socket-listen failed";

					ConPrint(L"socket(unicode): socket connection listening\n");
				}

				if(set_iEPort)
				{
					sEListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iEPort);
					if(bind(sEListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "encrypted: socket-bind failed, port already in use?";

					if(listen(sEListen, SOMAXCONN) != 0)
						throw "encrypted: socket-listen failed";

					ConPrint(L"socket(encrypted-ascii): socket connection listening\n");
				}

				if(set_iEWPort)
				{
					sEWListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iEWPort);
					if(bind(sEWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "encrypted-unicode: socket-bind failed, port already in use?";

					if(listen(sEWListen, SOMAXCONN) != 0)
						throw "encrypted-unicode: socket-listen failed";

					ConPrint(L"socket(encrypted-unicode): socket connection listening\n");
				}

			}
		}
	} catch(char *szError) {
		if(bInitHookExports)
			UnloadHookExports();

		if(sListen != INVALID_SOCKET)
		{
			closesocket(sListen);
			sListen = INVALID_SOCKET;
		}

		if(sWListen != INVALID_SOCKET)
		{
			closesocket(sWListen);
			sWListen = INVALID_SOCKET;
		}

		if(sEListen != INVALID_SOCKET)
		{
			closesocket(sEListen);
			sEListen = INVALID_SOCKET;
		}

		if(sEWListen != INVALID_SOCKET)
		{
			closesocket(sEWListen);
			sEWListen = INVALID_SOCKET;
		}

		ConPrint(L"ERROR: " + stows(szError));
		return false;
	}
	return true; 
}

/**************************************************************************************************************
unload FLHook
**************************************************************************************************************/

void FLHookInitUnload()
{
	//For BountyHunt
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	
	TerminateThread(hThreadResolver, 0);

	// unload update hook
	void *pAddress = (void*)((char*)hProcFL + ADDR_UPDATE);
	WriteProcMem(pAddress, &fpOldUpdate, 4);

	// unload hooks
	UnloadHookExports();

	// unload rest
	DWORD id;
	DWORD dwParam;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FLHookUnload, &dwParam, 0, &id);
}

void FLHookUnload()
{
	// close log
	fclose(fLog);

	// unload console
	TerminateThread(hConsoleThread, 0);
	SetConsoleCtrlHandler(ConsoleHandler, FALSE);
	FreeConsole();

	// quit network sockets
	if(sListen != INVALID_SOCKET)
		closesocket(sListen);
	if(sWListen != INVALID_SOCKET)
		closesocket(sWListen);
	if(sEListen != INVALID_SOCKET)
		closesocket(sEListen);
	if(sEWListen != INVALID_SOCKET)
		closesocket(sEWListen);

	for(list<SOCKET_CONNECTION*>::iterator i = lstSockets.begin(); (i != lstSockets.end()); i++)
	{
		closesocket((*i)->csock.s);
		delete *i;
	}
	lstSockets.clear();

	// delete binary trees
	delete set_btRepairGun;
	delete btSolarList;
	delete set_btSupressHealth;
	delete set_btMobDockShipArchIDs;
	delete set_btOneFactionDeathRep;
	delete set_btEmpathy;
	delete set_btDeathItems;
	delete set_btNoDeathPenalty;
	delete set_btNoDPSystems;
	delete set_btMRestrict;
	delete set_btJRestrict;
	// free libraries
	for(uint i=0; i<vDLLs.size(); i++)
	{
		FreeLibrary(vDLLs[i]);
	}
	// free blowfish encryption data
	if(set_BF_CTX)
	{
		ZeroMemory(set_BF_CTX, sizeof(set_BF_CTX));
		free(set_BF_CTX);
	}

	// misc
	DeleteCriticalSection(&cs);

	// finish
	FreeLibrary(hWString);
	FreeLibraryAndExitThread(hMe, 0); 
}

/**************************************************************************************************************
process a socket command
return true -> close socket connection
**************************************************************************************************************/

bool ProcessSocketCmd(SOCKET_CONNECTION *sc, wstring wscCmd)
{
	if(!ToLower(wscCmd).find(L"quit")) { // quit connection
		sc->csock.DoPrint(L"Goodbye.\r\n");
		ConPrint(L"socket: connection closed\n");
		return true;
	} else if(!(sc->csock.bAuthed)) { // not authenticated yet
		wstring wscLwr = ToLower(wscCmd);
		if(wscLwr.find(L"pass") != 0)
		{
			sc->csock.Print(L"ERR Please authenticate first\n");
			return false;
		}

		wchar_t wszPass[256];
		if(wscCmd.length() >= sizeof(wszPass))
		{
			sc->csock.Print(L"ERR Wrong password\n");
			return false;
		}
		swscanf(wscCmd.c_str(), L"pass %s", wszPass);

		// read passes from ini
		for(uint i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "pass%u", i);
			string scPass = IniGetS(set_scCfgFile, "Socket", szBuf, "");
			sprintf(szBuf, "rights%u", i);
			string scRights = IniGetS(set_scCfgFile, "Socket", szBuf, "");

			if(!scPass.length()) {
				sc->csock.Print(L"ERR Wrong password\n");
				ConPrint(L"socket: socket authentication failed(invalid pass)\n");
				return false;
			} else if(!scPass.compare(wstos(wszPass))) {
				sc->csock.bAuthed = true;
				sc->csock.SetRightsByString(scRights);
				sc->csock.Print(L"OK\n");
				ConPrint(L"socket: socket authentication successful\n");
				return false;
			}
		}
	} else { // execute admin command
		if(wscCmd[wscCmd.length()-1] == '\n')
			wscCmd = wscCmd.substr(0, wscCmd.length()-1);
		if(wscCmd[wscCmd.length()-1] == '\r')
			wscCmd = wscCmd.substr(0, wscCmd.length()-1);

		if(!wscCmd.compare(L"eventmode")) {
			if(sc->csock.rights & RIGHT_EVENTMODE) {
				sc->csock.Print(L"OK\n");
				sc->csock.bEventMode = true;
			} else {
				sc->csock.Print(L"ERR No permission\n");
			}		
		} else
			sc->csock.ExecuteCommandString(wscCmd);

		return false;
	}
}

/**************************************************************************************************************
write text to console
**************************************************************************************************************/

void ConPrint(wstring wscText, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscText);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	string scText = wstos(wszBuf);
	WriteConsole(hConsoleOut, scText.c_str(), (DWORD)scText.length(), 0, 0);
}

/**************************************************************************************************************
send event to all sockets which are in eventmode
**************************************************************************************************************/

void ProcessEvent(wstring wscText, ...)
{
	wchar_t wszBuf[1024] = L"";
	va_list marker;
	va_start(marker, wscText);
	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	foreach(lstSockets, SOCKET_CONNECTION*, i)
	{
		if((*i)->csock.bEventMode)
			(*i)->csock.Print(L"%s\n", wszBuf);
	}
}

/**************************************************************************************************************
check for pending admin commands in console or socket and execute them
**************************************************************************************************************/

struct timeval tv = {0, 0};
CConsole AdminConsole;

void ProcessPendingCommands()
{
	try {
		// check for new console commands
		EnterCriticalSection(&cs);
		while(lstConsoleCmds.size())
		{
			wstring *pwscCmd = lstConsoleCmds.front();
			lstConsoleCmds.pop_front();
			AdminConsole.ExecuteCommandString(*pwscCmd);
			delete pwscCmd;
		}
		LeaveCriticalSection(&cs);

		if(sListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.bUnicode = false;
				sc->wscPending = L"";
				lstSockets.push_back(sc);
				ConPrint(L"socket(ascii): new socket connection\n");
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sWListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				lstSockets.push_back(sc);
				ConPrint(L"socket(unicode): new socket connection\n");
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sEListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.bUnicode = false;
				sc->wscPending = L"";
				// encryption
				sc->csock.bEncrypted = true;
				sc->csock.bfc = set_BF_CTX;
				lstSockets.push_back(sc);
				ConPrint(L"socket(encrypted-ascii): new socket connection\n");
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sEWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEWListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				// encryption
				sc->csock.bEncrypted = true;
				sc->csock.bfc = set_BF_CTX;
				lstSockets.push_back(sc);
				ConPrint(L"socket(encrypted-unicode): new socket connection\n");
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}
		
		// check for pending socket-commands
		foreach(lstSockets, SOCKET_CONNECTION*, i)
		{
			SOCKET_CONNECTION *sc = *i;

			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sc->csock.s, &fds);
			struct timeval tv = {0, 0};
			if(select(0, &fds, 0, 0, &tv))
			{ // data to be read
				ulong lSize;
				ioctlsocket(sc->csock.s, FIONREAD, &lSize);
				char *szData = new char[lSize + 1];
				memset(szData, 0, lSize + 1);
				if(recv(sc->csock.s, szData, lSize, 0) <= 0)
				{
					ConPrint(L"socket: socket connection closed\n");
					delete[] szData;
					lstDelete.push_back(sc);
					continue;
				}

				// enqueue commands (terminated by \n)
				wstring wscData;
				if(sc->csock.bEncrypted)
				{
					SwapBytes(szData, lSize);
					Blowfish_Decrypt(sc->csock.bfc, szData, lSize); 
					SwapBytes(szData, lSize);
				}
				if(sc->csock.bUnicode)
					wscData = (wchar_t*)szData;
				else
					wscData = stows(szData);

				wstring wscTmp = sc->wscPending + wscData;
				wscData = wscTmp;

				list<wstring> lstCmds;
				wstring wscCmd;
				for(uint i = 0; (i < wscData.length()); i++)
				{
					if(!wscData.substr(i, 2).compare(L"\r\n")) {
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
						i++;
					} else if(wscData[i] == '\n') {
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
					} else 
						wscCmd.append(1, wscData[i]);
				}

				sc->wscPending = wscCmd;

				// process cmds
				foreach(lstCmds, wstring, it)
				{
					if(ProcessSocketCmd(sc, (*it)))
					{
						lstDelete.push_back(sc);
						break;
					}
				}

				delete[] szData;
			}
		}

		// delete closed connections
		foreach(lstDelete, SOCKET_CONNECTION*, it)
		{
			closesocket((*it)->csock.s);
			lstSockets.remove(*it);
			delete (*it);
		}

		lstDelete.clear();
	} catch(...) { 
		AddLog("Exception in %s", __FUNCTION__); 
		throw "exception"; 
	}
}
