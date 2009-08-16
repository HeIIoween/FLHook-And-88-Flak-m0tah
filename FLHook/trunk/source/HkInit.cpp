#include <process.h>
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

PATCH_INFO piServerDLL = 
{
	"server", 0x6CE0000,
	{
		{0x6D67274,		&ShipDestroyedHook,							4, &fpOldShipDestroyed,			false},
		{0x6D641EC,		&_HkCb_AddDmgEntry,							4, 0,							false},
		{0x6D67320,		&_HookMissileTorpHit,						4, &fpOldMissileTorpHit,		false},
		{0x6D65448,		&_HookMissileTorpHit,						4, 0,							false},		
		{0x6D67670,		&_HookMissileTorpHit,						4, 0,							false},		
		{0x6D653F4,		&_HkCb_GeneralDmg,							4, &fpOldGeneralDmg,			false},
		{0x6D672CC,		&_HkCb_GeneralDmg,							4, 0,							false},
		{0x6D6761C,		&_HkCb_GeneralDmg,							4, 0,							false},
		{0x6D67668,		&_HkCb_NonGunWeaponHitsBase,				4, &fpOldNonGunWeaponHitsBase,	false},
		{0x6D6420C,     &_HkCb_LaunchPos,							4, &fpOldLaunchPos,				false},
		
		{0,0,0,0} // terminate
	}
};

PATCH_INFO piRemoteClientDLL = 
{
	"remoteclient", 0x6B30000,
	{
		{0x6B6BB80,		&HkCb_SendChat,								4, &RCSendChatMsg,			false},

		{0,0,0,0} // terminate
	}
};

PATCH_INFO piDaLibDLL = 
{
	"dalib", 0x65C0000,
	{
		{0x65C4BEC,		&_DisconnectPacketSent,						4, &fpOldDiscPacketSent,	false},

		{0,0,0,0} // terminate
	}
};

PATCH_INFO piContentDLL = 
{
	"content", 0x6EA0000,
	{
		{0x6FB350C,		&SpaceObjCreate,							4, 0,						false},
		{0x6FB358C,		&SpaceObjDock,								4, 0,						false},
		//{0x6FB3614,		&ControllerCreate,							4, 0,						false},
		//{0x6FB360C,		&ControllerSend,							4, 0,						false},
		//{0x6FB3610,		&ControllerDestroy,							4, 0,						false},
		//{0x6FB33C4,		&SpaceObjLightFuse,							4, 0,						false},
		//{0x6FB33C8,		&SpaceObjDestroy,							4, 0,						false},
		//{0x6FB34D4,		&SolarSpaceObjCreate,						4, 0,						false},
		//{0x6FB3460,			&SystemScanObjects,						4, 0,						false},

		{0,0,0,0} // terminate
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Patch(PATCH_INFO &pi)
{
	HMODULE hMod = GetModuleHandle(pi.szBinName);
	if(!hMod)
	{
		ConPrint(L"An error occured hooking " + stows(pi.szBinName) + L"\n");
		return false;
	}

	for(uint i = 0; (i < sizeof(pi.piEntries)/sizeof(PATCH_INFO_ENTRY)); i++)
	{
		if(!pi.piEntries[i].pAddress)
			break;

		char *pAddress = (char*)hMod + (pi.piEntries[i].pAddress - pi.pBaseAddress);
		if(!pi.piEntries[i].pOldValue) {
			pi.piEntries[i].pOldValue = new char[pi.piEntries[i].iSize];
			pi.piEntries[i].bAlloced = true;
		} else
			pi.piEntries[i].bAlloced = false;

		ReadProcMem(pAddress, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		WriteProcMem(pAddress, &pi.piEntries[i].pNewValue, pi.piEntries[i].iSize);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestorePatch(PATCH_INFO &pi)
{
	HMODULE hMod = GetModuleHandle(pi.szBinName);
	if(!hMod)
	{
		ConPrint(L"An error occured unhooking " + stows(pi.szBinName) + L"\n");
		return false;
	}

	for(uint i = 0; (i < sizeof(pi.piEntries)/sizeof(PATCH_INFO_ENTRY)); i++)
	{
		if(!pi.piEntries[i].pAddress)
			break;

		char *pAddress = (char*)hMod + (pi.piEntries[i].pAddress - pi.pBaseAddress);
		WriteProcMem(pAddress, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		if(pi.piEntries[i].bAlloced)
			delete[] pi.piEntries[i].pOldValue;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FARPROC fpOldDmgList;
FARPROC fpOldWSASendTo;
char *szRCSendChatHook;
CDPClientProxy **g_cClientProxyArray;

CDPServer *cdpSrv;

_CRCAntiCheat CRCAntiCheat;
_CreateChar CreateChar;

string scAcctPath;

void *pClient;

CLIENT_INFO ClientInfo[201];

uint g_iServerLoad = 0;

char *g_FLServerDataPtr;

_GetShipInspect GetShipInspect;
_AddCargoDocked AddCargoDocked;


/**************************************************************************************************************
clear the clientinfo
**************************************************************************************************************/

void ClearClientInfo(uint iClientID)
{
	ClientInfo[iClientID].dieMsg = DIEMSG_ALL;
	ClientInfo[iClientID].iShip = 0;
	ClientInfo[iClientID].iShipOld = 0;
	ClientInfo[iClientID].tmSpawnTime = 0;
	ClientInfo[iClientID].lstMoneyFix.clear();
	ClientInfo[iClientID].iTradePartner = 0;
	ClientInfo[iClientID].iBaseEnterTime = 0;
	ClientInfo[iClientID].iCharMenuEnterTime = 0;
	ClientInfo[iClientID].bCruiseActivated = false;
	ClientInfo[iClientID].lstLoss.clear();
	ClientInfo[iClientID].iLastLoss = 0;
	ClientInfo[iClientID].iAverageLoss = 0;
	ClientInfo[iClientID].lstPing.clear();
	ClientInfo[iClientID].iAveragePing = 0;
	ClientInfo[iClientID].tmKickTime = 0;
	ClientInfo[iClientID].iLastExitedBaseID = 0;
	ClientInfo[iClientID].bDisconnected = false;
	ClientInfo[iClientID].bCharSelected = false;
	ClientInfo[iClientID].tmF1Time = 0;
	ClientInfo[iClientID].tmF1TimeDisconnect = 0;

	DamageList dmg;
	ClientInfo[iClientID].dmgLast = dmg;
	ClientInfo[iClientID].dieMsgSize = CS_DEFAULT;
	ClientInfo[iClientID].chatSize = CS_DEFAULT;
	ClientInfo[iClientID].chatStyle = CST_DEFAULT;

	ClientInfo[iClientID].lstAutoBuyItems.clear();

	ClientInfo[iClientID].lstIgnore.clear();
	ClientInfo[iClientID].bIgnoreUniverse = false;
	ClientInfo[iClientID].iKillsInARow = 0;
	ClientInfo[iClientID].wscHostname = L"";
	ClientInfo[iClientID].bEngineKilled = false;
	ClientInfo[iClientID].bThrusterActivated = false;
	ClientInfo[iClientID].bTradelane = false;

	ClientInfo[iClientID].bMustSendUncloak = false;
	ClientInfo[iClientID].iCloakingTime = 0;
	ClientInfo[iClientID].iCloakWarmup = 0;
	ClientInfo[iClientID].iCloakCooldown = 0;
	ClientInfo[iClientID].iCloakSlot = 0;
	ClientInfo[iClientID].bCanCloak = false;
	ClientInfo[iClientID].bCloaked = false;
	ClientInfo[iClientID].bIsCloaking = false;
	ClientInfo[iClientID].bWantsCloak = false;
	ClientInfo[iClientID].tmCloakTime = 0;

	ClientInfo[iClientID].wscAfkMsg = L"";
	ClientInfo[iClientID].iSentIds.clear();

	ClientInfo[iClientID].bMarkEverything = false;
	ClientInfo[iClientID].vMarkedObjs.clear();
	ClientInfo[iClientID].vDelayedSystemMarkedObjs.clear();
	ClientInfo[iClientID].vAutoMarkedObjs.clear();
	ClientInfo[iClientID].vDelayedAutoMarkedObjs.clear();

	ClientInfo[iClientID].bNoPvp = false;

	ClientInfo[iClientID].bInWrapGate = false;

	ClientInfo[iClientID].fShieldHealth = 0.0f;
	
	ClientInfo[iClientID].bMobileDocked = false;
	ClientInfo[iClientID].iDockClientID = 0;
	ClientInfo[iClientID].lstJumpPath.clear();
	ClientInfo[iClientID].bPathJump = false;
	ClientInfo[iClientID].bCharInfoReqAfterDeath = false;
	ClientInfo[iClientID].Vlaunch.x = 0;
	ClientInfo[iClientID].Vlaunch.y = 0;
	ClientInfo[iClientID].Vlaunch.z = 0;
	for(uint i=0; i<3; i++)
	{
		for(uint j=0; j<3; j++)
		{
			ClientInfo[iClientID].Mlaunch.data[i][j] = 0.0f;
		}
	}
	ClientInfo[iClientID].bMobileBase = false;
	ClientInfo[iClientID].lstPlayersDocked.clear();
	ClientInfo[iClientID].iLastSpaceObjDocked = 0;
	ClientInfo[iClientID].iLastEnteredBaseID = 0;
	ClientInfo[iClientID].lstDmgRec.clear();
	ClientInfo[iClientID].tmShieldsDownCmd = 0;
	ClientInfo[iClientID].iShipID = 0;
	ClientInfo[iClientID].lstCargoFix.clear();
	ClientInfo[iClientID].bDeathPenaltyOnEnter = false;
	ClientInfo[iClientID].bDisplayDPOnLaunch = false;
	ClientInfo[iClientID].iDeathPenaltyCredits = 0;
	ClientInfo[iClientID].bCheckedDock = false;
	ClientInfo[iClientID].lstRemCargo.clear();
}

/**************************************************************************************************************
load settings from flhookhuser.ini
**************************************************************************************************************/

void LoadUserSettings(uint iClientID)
{
	CAccount *acc = Players.FindAccountFromClientID(iClientID);
	wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	// read diemsg settings
	ClientInfo[iClientID].dieMsg = (DIEMSGTYPE)IniGetI(scUserFile, "settings", "DieMsg", DIEMSG_ALL);
	ClientInfo[iClientID].dieMsgSize = (CHATSIZE)IniGetI(scUserFile, "settings", "DieMsgSize", CS_DEFAULT);

	// read chatstyle settings
	ClientInfo[iClientID].chatSize = (CHATSIZE)IniGetI(scUserFile, "settings", "ChatSize", CS_DEFAULT);
	ClientInfo[iClientID].chatStyle = (CHATSTYLE)IniGetI(scUserFile, "settings", "ChatStyle", CST_DEFAULT);

	// read ignorelist
	ClientInfo[iClientID].lstIgnore.clear();
	for(int i = 1; ; i++)
	{
		wstring wscIgnore = IniGetWS(scUserFile, "IgnoreList", itos(i), L"");
		if(!wscIgnore.length())
			break;

		IGNORE_INFO ii;
		ii.wscCharname = GetParam(wscIgnore, ' ', 0);
		ii.wscFlags = GetParam(wscIgnore, ' ', 1);
		ClientInfo[iClientID].lstIgnore.push_back(ii);
	}
	ClientInfo[iClientID].bIgnoreUniverse = IniGetB(scUserFile, "settings", "IgnoreUniverse", false);

}

/**************************************************************************************************************
load settings from flhookhuser.ini (specific to character)
**************************************************************************************************************/

void LoadUserCharSettings(uint iClientID)
{
	CAccount *acc = Players.FindAccountFromClientID(iClientID);
	wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	wstring wscFilename;
	HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
	string scFilename = wstos(wscFilename);

	// read autobuy
	string scSection = "autobuy_" + scFilename;
	list<INISECTIONVALUE> lstValues;
	IniGetSection(scUserFile, scSection, lstValues);
	ClientInfo[iClientID].lstAutoBuyItems.clear();
	foreach(lstValues, INISECTIONVALUE, it)
	{
		AUTOBUY_CARTITEM item;
		item.iArchID = ToUint(it->scKey);
		if(!item.iArchID)
			continue;
		item.iCount = ToUint(it->scValue);
		ClientInfo[iClientID].lstAutoBuyItems.push_back(item);
	}

	scSection = "general_" + scFilename;

	// read automarking settings
	ClientInfo[iClientID].bMarkEverything = IniGetB(scUserFile, scSection, "automarkenabled", false);
	ClientInfo[iClientID].bIgnoreGroupMark = IniGetB(scUserFile, scSection, "ignoregroupmarkenabled", false);
	ClientInfo[iClientID].fAutoMarkRadius = IniGetF(scUserFile, scSection, "automarkradius", set_fAutoMarkRadius);

	// read death penalty settings
	ClientInfo[iClientID].bDeathPenaltyOnEnter = IniGetB(scUserFile, scSection, "deathpenalty", false);
	if(ClientInfo[iClientID].bDeathPenaltyOnEnter)
		IniWrite(scUserFile, scSection, "deathpenalty", "no");
	ClientInfo[iClientID].bDisplayDPOnLaunch = IniGetB(scUserFile, scSection, "DPnotice", true);
}

/**************************************************************************************************************
install the callback hooks
**************************************************************************************************************/

bool InitHookExports()
{
	char	*pAddress;

	// init critial sections
	InitializeCriticalSection(&csIPResolve);
	DWORD dwID;
	DWORD dwParam[34]; // else release version crashes, dont ask me why...
	hThreadResolver = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HkThreadResolver, &dwParam, 0, &dwID);
 
	GetShipInspect = (_GetShipInspect)SRV_ADDR(ADDR_SRV_GETINSPECT);
	AddCargoDocked = (_AddCargoDocked)SRV_ADDR(ADDR_SRV_ADDCARGODOCKED);

	// get export that failed with link-lib
	pClient = GetProcAddress(hModRemoteClient, "Client");

	// install IServerImpl callbacks in remoteclient.dll
	char *pServer = (char*)&Server;
	memcpy(&pServer, pServer, 4);
	for(uint i = 0; (i < sizeof(HkIServerImpl::hookEntries)/sizeof(HOOKENTRY)); i++)
	{
		char *pAddress = pServer + HkIServerImpl::hookEntries[i].dwRemoteAddress;
		ReadProcMem(pAddress, &HkIServerImpl::hookEntries[i].fpOldProc, 4);
		WriteProcMem(pAddress, &HkIServerImpl::hookEntries[i].fpProc, 4);
	}

	//content.dll
	Patch(piContentDLL);

	// server.dll
	Patch(piServerDLL);
	Patch(piRemoteClientDLL);
	Patch(piDaLibDLL);

	// Spawn item Hook
	pAddress = SRV_ADDR(ADDR_SRV_SPAWN_ITEM);
	char szOver[] = { '\xB8', 0, 0, 0, 0, '\xFF', '\xD0' };
	uint *iAddr = (uint*)((char*)&szOver + 1);
	*iAddr = reinterpret_cast<uint>((void*)_HkCb_SpawnItem);
	char szBuf[7];
	ReadProcMem(pAddress, szBuf, 7);
	WriteProcMem((void*)_SpawnItemOrig, szBuf, 7);
	WriteProcMem(pAddress, szOver, 7);

	// crc anti-cheat
	CRCAntiCheat = (_CRCAntiCheat) ((char*)hModServer + ADDR_CRCANTICHEAT);

	// get CDPServer
	pAddress = DALIB_ADDR(ADDR_CDPSERVER);
	ReadProcMem(pAddress, &cdpSrv, 4);
	 
	// read g_FLServerDataPtr(used for serverload calc)
	pAddress = FLSERVER_ADDR(ADDR_DATAPTR);
	ReadProcMem(pAddress , &g_FLServerDataPtr, 4);

	// some setting relate hooks
	HookRehashed();

	// get client proxy array, used to retrieve player pings/ips
	pAddress = (char*)hModRemoteClient + ADDR_CPLIST;
	char *szTemp;
	ReadProcMem(pAddress, &szTemp, 4);
	szTemp += 0x10;
	memcpy(&g_cClientProxyArray, &szTemp, 4);

	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	scAcctPath = string(szDataPath) + "\\Accts\\MultiPlayer\\";

	// clear ClientInfo
	for(i = 0; (i < sizeof(ClientInfo)/sizeof(CLIENT_INFO)); i++)
	{
		ClientInfo[i].iConnects = 0; // only set to 0 on start
		ClearClientInfo(i);
	}

	// fill client info struct for players that are already on the server(when FLHookStart was used)
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, it)
	{
		ClientInfo[it->iClientID].iShip = it->iShip;
		LoadUserSettings(it->iClientID);
	}

	// read bases
	lstBases.clear();
	Universe::IBase *base = Universe::GetFirstBase();
	while(base)
	{
		BASE_INFO bi;
		bi.bDestroyed = false;
		bi.iObjectID = base->iSpaceObjID;
		char *szBaseName = "";
		__asm
		{
			pushad
			mov ecx, [base]
			mov eax, [base]
			mov eax, [eax]
			call [eax+4]
			mov [szBaseName], eax
			popad
		}

		bi.scBasename = szBaseName;
		bi.iBaseID = CreateID(szBaseName);
		lstBases.push_back(bi);
		base = Universe::GetNextBase();
	}

	// read base market data from ini
	HkLoadMiscBaseMarket();
	HkLoadCommoditiesBaseMarket();

	return true;
}

/**************************************************************************************************************
uninstall the callback hooks
**************************************************************************************************************/

void UnloadHookExports()
{
	char *pAddress;

	// uninstall IServerImpl callbacks in remoteclient.dll
	void *pServer = GetProcAddress(hModServer, "Server");
	memcpy(&pServer, pServer, 4);
	for(uint i = 0; (i < sizeof(HkIServerImpl::hookEntries)/sizeof(HOOKENTRY)); i++)
	{
		void *pAddress = (void*)((char*)pServer + HkIServerImpl::hookEntries[i].dwRemoteAddress);
		WriteProcMem(pAddress, &HkIServerImpl::hookEntries[i].fpOldProc, 4);
	}

	// reset npc spawn setting
	HkChangeNPCSpawn(false);


	// restore other hooks
	RestorePatch(piServerDLL);
	RestorePatch(piRemoteClientDLL);
	RestorePatch(piDaLibDLL);
	RestorePatch(piContentDLL);

	// restore spawn item Hook
	pAddress = SRV_ADDR(ADDR_SRV_SPAWN_ITEM);
	char szBuf[7];
	ReadProcMem((void*)_SpawnItemOrig, szBuf, 7);
	WriteProcMem(pAddress, szBuf, 7);

	// anti-death-msg
	char szOld[] = { '\x74' };
	pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
	WriteProcMem(pAddress, szOld, 1);
}

/**************************************************************************************************************
settings were rehashed
sometimes adjustments need to be made after a rehash
**************************************************************************************************************/

void HookRehashed()
{
	char *pAddress;

	// anti-deathmsg
	if(set_bDieMsg)	{ // disables the "old" "A Player has died: ..." messages
		char szJMP[] = { '\xEB' };
		pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
		WriteProcMem(pAddress, szJMP, 1);
	} else {
		char szOld[] = { '\x74' };
		pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
		WriteProcMem(pAddress, szOld, 1);
	}

	// charfile encyption(doesn't get disabled when unloading FLHook)
	if(set_bDisableCharfileEncryption) {
		char szBuf[] = { '\x14', '\xB3' };
		pAddress = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(pAddress, szBuf, 2);
		pAddress = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(pAddress, szBuf, 2);
	} else {
		char szBuf[] = { '\xE4', '\xB4' };
		pAddress = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(pAddress, szBuf, 2);
		pAddress = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(pAddress, szBuf, 2);
	}

	// maximum group size
	if(set_iMaxGroupSize > 0) {
		char cNewGroupSize = set_iMaxGroupSize & 0xFF;
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
	} else { // default
		char cNewGroupSize = 8;
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
	}

	// autopilot error margin
	if(set_bSetAutoErrorMargin)
	{
		pAddress = COMMON_ADDR(ADDR_AUTOPILOT_ERROR);
		WriteProcMem(pAddress, &set_fAutoErrorMargin, 4);
	}
}

