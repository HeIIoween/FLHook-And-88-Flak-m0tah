#include <math.h>
#include <float.h>
#include "hook.h"
#include "CInGame.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromAccount(CAccount *acc)
{
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		CAccount *cur;
		memcpy(&cur, (char*)pPD + 0x400, 4);
		if(cur == acc)
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			return iClientID;
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromPD(struct PlayerData *pPD)
{
	char *p1 = (char*)pPD;
	char *p2 = (char*)&Players;
	memcpy(&p2, p2, 4);
	return (uint)(((p1 - p2) / 0x418) + 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAccount* HkGetAccountByCharname(wstring wscCharname)
{
	flstr *str = CreateWString(wscCharname.c_str());
	CAccount *acc = Players.FindAccountFromCharacterName(*str);
	FreeWString(str);

	return acc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromCharname(wstring wscCharname)
{
	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
		return -1;

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		return -1;

	wchar_t *wszActiveCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
	if(!wszActiveCharname)
		return -1;
	
	wstring wscActiveCharname = wszActiveCharname;
	wscActiveCharname = ToLower(wscActiveCharname);
	if(wscActiveCharname.compare(ToLower(wscCharname)) != 0)
		return -1;

	return iClientID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetAccountID(CAccount *acc)
{
	wchar_t *wszID;
	memcpy(&wszID, (char*)acc+8, sizeof(wszID));

	return wszID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsEncoded(string scFilename)
{
	bool bRet = false;
	FILE *f = fopen(scFilename.c_str(), "r");
	if(!f)
		return false;

	char szMagic[] = "FLS1";
	char szFile[sizeof(szMagic)] = "";
	fread(szFile, 1, sizeof(szMagic), f);
	if(!strncmp(szMagic, szFile, sizeof(szMagic) - 1))
		bRet = true;
	fclose(f);

	return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(wstring wscCharname)
{
	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
		return false;

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		return false;

	uint iBase = 0;
	uint iSystem = 0;
	pub::Player::GetBase(iClientID, iBase);
	pub::Player::GetSystem(iClientID, iSystem);
	if(!iBase && !iSystem)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(uint iClientID)
{

	uint iBase = 0;
	uint iSystem = 0;
	pub::Player::GetBase(iClientID, iBase);
	pub::Player::GetSystem(iClientID, iSystem);
	if(!iBase && !iSystem)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsValidClientID(uint iClientID)
{

	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iID = HkGetClientIdFromPD(pPD);
		if(iID == iClientID)
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveId(wstring wscCharname, uint &iClientID)
{
	wscCharname = ToLower(wscCharname);
	if(wscCharname.find(L"id ") == 0)
	{
		uint iID = 0;
		swscanf(wscCharname.c_str(), L"id %u", &iID);
		if(!HkIsValidClientID(iID))
			return HKE_INVALID_CLIENT_ID;
		iClientID = iID;
		return HKE_OK;
	}

	return HKE_INVALID_ID_STRING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveShortCut(wstring wscShortcut, uint &_iClientID)
{
	wscShortcut = ToLower(wscShortcut);
	if(wscShortcut.find(L"sc ") != 0)
		return HKE_INVALID_SHORTCUT_STRING;

	wscShortcut = wscShortcut.substr(3);

	uint iClientIDFound = -1;
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);

		const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
		if(!wszCharname)
			continue;

		wstring wscCharname = wszCharname;
		if(ToLower(wscCharname).find(wscShortcut) != -1)
		{
			if(iClientIDFound == -1)
				iClientIDFound = iClientID;
			else
				return HKE_AMBIGUOUS_SHORTCUT;
		}
	}

	if(iClientIDFound == -1)
		return HKE_NO_MATCHING_PLAYER;

	_iClientID = iClientIDFound;
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIDByShip(uint iShip)
{
	for(uint i = 0; (i <= Players.GetMaxPlayerCount()); i++)
	{
		if(ClientInfo[i].iShip == iShip || ClientInfo[i].iShipOld == iShip)
			return i;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir)
{
	_GetFLName GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

	wstring wscID = HkGetAccountID(acc);
	char szDir[1024] = "";
	GetFLName(szDir, wscID.c_str());
	wscDir = stows(szDir);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(wstring wscCharname, wstring &wscDir)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	CAccount *acc;
	if(iClientID != -1)
		acc = Players.FindAccountFromClientID(iClientID);		
	else {
		if(!(acc = HkGetAccountByCharname(wscCharname)))
			return HKE_CHAR_DOES_NOT_EXIST;
	}

	return HkGetAccountDirName(acc, wscDir);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetCharFileName(wstring wscCharname, wstring &wscFilename)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	// getchafilename from clientid
	if(iClientID != -1)
		wscCharname = Players.GetActiveCharacterName(iClientID);

	_GetFLName GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

	char szBuf[1024] = "";
	GetFLName(szBuf, wscCharname.c_str());
	wscFilename = stows(szBuf);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetBaseNickByID(uint iBaseID)
{
	char szBasename[1024] = "";
	pub::GetBaseNickname(szBasename, sizeof(szBasename), iBaseID);
	return stows(szBasename);
}

string HkGetBaseNickSByID(uint iBaseID)
{
	char szBasename[1024] = "";
	pub::GetBaseNickname(szBasename, sizeof(szBasename), iBaseID);
	return szBasename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetSystemNickByID(uint iSystemID)
{
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return stows(szSystemname);
}

string HkGetSystemNickSByID(uint iSystemID)
{
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return szSystemname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetPlayerSystem(uint iClientID)
{
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return stows(szSystemname);
}

string HkGetPlayerSystemS(uint iClientID)
{
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return szSystemname;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkLockAccountAccess(CAccount *acc, bool bKick)
{
	char szJMP[] = { '\xEB' };
	char szJBE[] = { '\x76' };

	flstr *strAccID = CreateWString(HkGetAccountID(acc).c_str());
	if(!bKick)
		WriteProcMem((void*)0x06D52A6A, &szJMP, 1);

	Players.LockAccountAccess(*strAccID); // also kicks player on this account
	if(!bKick)
		WriteProcMem((void*)0x06D52A6A, &szJBE, 1);
	FreeWString(strAccID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkUnlockAccountAccess(CAccount *acc)
{
	flstr *strAccID = CreateWString(HkGetAccountID(acc).c_str());
	Players.UnlockAccountAccess(*strAccID);
	FreeWString(strAccID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkGetItemsForSale(uint iBaseID, list<uint> &lstItems)
{
	lstItems.clear();
	char szNOP[] = { '\x90', '\x90'};
	char szJNZ[] = { '\x75', '\x1D'};
	WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szNOP, 2); // patch, else we only get commodities
	uint iArray[1024];
	int iSize = sizeof(iArray)/sizeof(uint);
	pub::Market::GetCommoditiesForSale(iBaseID, (uint*const)&iArray, &iSize);
	WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szJNZ, 2);

	for(int i = 0; (i < iSize); i++)
		lstItems.push_back(iArray[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

IObjInspectImpl* HkGetInspect(uint iClientID)
{
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	uint iDunno;
	IObjInspectImpl *inspect;
	if(!GetShipInspect(iShip, inspect, iDunno))
	{
		return 0;
	}
	else
		return inspect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEquipManager* HkGetEquipMan(uint iShip)
{
	uint iDunno;
	IObjInspectImpl *inspect;
	if(!GetShipInspect(iShip, inspect, iDunno))
		return 0;
	char *szShip = (char*)inspect;
	szShip += 0x10;
	char *szEquip = *((char**)szShip);
	szEquip += 0xE4;
	return (CEquipManager*)szEquip;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ENGINE_STATE HkGetEngineState(uint iClientID)
{
	if(ClientInfo[iClientID].bTradelane)
		return ES_TRADELANE;
	else if(ClientInfo[iClientID].bCruiseActivated)
		return ES_CRUISE;
	else if(ClientInfo[iClientID].bThrusterActivated)
		return ES_THRUSTER;
	else if(!ClientInfo[iClientID].bEngineKilled)
		return ES_ENGINE;
	else
		return ES_KILLED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EQ_ITEM
{
	EQ_ITEM *next;
	uint i2;
	ushort s1;
	ushort sID;
	uint iGoodID;
	uint i3;
	bool bMounted;
	char sz[3];
	uint i6;
	uint iCount;
	bool bMission;
};

HK_ERROR HkGetGoodIDFromSID(uint iClientID, ushort sGoodID, uint &iGoodID)
{
	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);

	EQ_ITEM *eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM *eq;
	eq = eqLst->next;
	while(eq != eqLst)
	{
		if(eq->sID==sGoodID)
		{
			iGoodID = eq->iGoodID;
			return HKE_OK;
		}
		eq = eq->next;
	}
	
	return HKE_INVALID_GOOD;
}

HK_ERROR HkGetSIDFromGoodID(uint iClientID, ushort &sGoodID, uint iGoodID)
{
	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);

	EQ_ITEM *eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM *eq;
	eq = eqLst->next;
	while(eq != eqLst)
	{
		if(eq->iGoodID==iGoodID)
		{
			sGoodID = eq->sID;
			return HKE_OK;
		}
		eq = eq->next;
	}
	
	return HKE_INVALID_GOOD;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkRestartServer()
{
	bool bFoundTaskkill = false;
	char systemDir[MAX_PATH];
	GetSystemDirectory(systemDir, sizeof(systemDir));
	WIN32_FIND_DATA findData;
	string path = systemDir;
	path += "\\taskkill.exe";
	HANDLE hFile = FindFirstFile(path.c_str(), &findData);
	if(hFile!=INVALID_HANDLE_VALUE) 
		bFoundTaskkill = true;
	FindClose(hFile);
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	path = szCurDir;
	path += "\\taskkill.exe";
	hFile = FindFirstFile(path.c_str(), &findData);
	if(hFile!=INVALID_HANDLE_VALUE) 
		bFoundTaskkill = true;
	FindClose(hFile);
	if(!bFoundTaskkill)
		return HKE_NO_TASKKILL;

	string commands = "/C taskkill -im flserver.exe&&flserver.exe " + set_scHkRestartOpt;
	ShellExecute(NULL, "open", "cmd", commands.c_str(), szCurDir, SW_HIDE);
	FLHookInitUnload();
	return HKE_OK;
}

HK_ERROR HkShutdownServer()
{
	bool bFoundTaskkill = false;
	char systemDir[MAX_PATH];
	GetSystemDirectory(systemDir, sizeof(systemDir));
	WIN32_FIND_DATA findData;
	string path = systemDir;
	path += "\\taskkill.exe";
	HANDLE hFile = FindFirstFile(path.c_str(), &findData);
	if(hFile!=INVALID_HANDLE_VALUE) 
		bFoundTaskkill = true;
	FindClose(hFile);
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	path = szCurDir;
	path += "\\taskkill.exe";
	hFile = FindFirstFile(path.c_str(), &findData);
	if(hFile!=INVALID_HANDLE_VALUE) 
		bFoundTaskkill = true;
	FindClose(hFile);
	if(!bFoundTaskkill)
		return HKE_NO_TASKKILL;

	ShellExecute(NULL, "open", "taskkill", "-im flserver.exe", NULL, SW_HIDE);
	FLHookInitUnload();
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float HkDistance3D(Vector v1, Vector v2)
{
	float sq1 = v1.x-v2.x, sq2 = v1.y-v2.y, sq3 = v1.z-v2.z;
	return sqrt( sq1*sq1 + sq2*sq2 + sq3*sq3 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float HkGetAdjustedDistance(uint iShip1, uint iShip2)
{
	Vector v1, v2;
	Matrix m1, m2;
	pub::SpaceObj::GetLocation(iShip1, v1, m1);
	pub::SpaceObj::GetLocation(iShip2, v2, m2);
	uint iDunno;
	IObjInspectImpl *inspect1;
	GetShipInspect(iShip1, inspect1, iDunno);
	IObjInspectImpl *inspect2;
	GetShipInspect(iShip1, inspect2, iDunno);
	float fDistance = HkDistance3D(v1, v2) - inspect1->cobject()->hierarchy_radius() - inspect2->cobject()->hierarchy_radius();
	return fDistance>0 ? fDistance : 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Quaternion HkMatrixToQuaternion(Matrix m)
{
	Quaternion quaternion;
	quaternion.w = sqrt( max( 0, 1 + m.data[0][0] + m.data[1][1] + m.data[2][2] ) ) / 2; 
	quaternion.x = sqrt( max( 0, 1 + m.data[0][0] - m.data[1][1] - m.data[2][2] ) ) / 2; 
	quaternion.y = sqrt( max( 0, 1 - m.data[0][0] + m.data[1][1] - m.data[2][2] ) ) / 2; 
	quaternion.z = sqrt( max( 0, 1 - m.data[0][0] - m.data[1][1] + m.data[2][2] ) ) / 2; 
	quaternion.x = (float)_copysign( quaternion.x, m.data[2][1] - m.data[1][2] );
	quaternion.y = (float)_copysign( quaternion.y, m.data[0][2] - m.data[2][0] );
	quaternion.z = (float)_copysign( quaternion.z, m.data[1][0] - m.data[0][1] );

	return quaternion;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetSpaceObjFromBaseID(uint iBaseID)
{
	Universe::IBase *base = Universe::get_base(iBaseID);
	if(!base)
		return 0;
	return base->iSpaceObjID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string HkGetStringFromIDS(uint iIDS) //Only works for names
{
	if(!iIDS)
		return "";

	uint iDLL = iIDS / 0x10000;
	iIDS -= iDLL * 0x10000;

	char szBuf[512];
	if(LoadStringA(vDLLs[iDLL], iIDS, szBuf, 512))
		return szBuf;
	return "";
}

wstring HkGetWStringFromIDS(uint iIDS) //Only works for names
{
	if(!iIDS)
		return L"";

	uint iDLL = iIDS / 0x10000;
	iIDS -= iDLL * 0x10000;

	wchar_t wszBuf[512];
	if(LoadStringW(vDLLs[iDLL], iIDS, wszBuf, 512))
		return wszBuf;
	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

__declspec(naked) CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW)
{
	__asm
	{
		push ecx
		push edx
		mov ecx, [esp+12]
		mov edx, [ecx]
		call dword ptr[edx+0x150]
		pop edx
		pop ecx
		ret 4
	}
}