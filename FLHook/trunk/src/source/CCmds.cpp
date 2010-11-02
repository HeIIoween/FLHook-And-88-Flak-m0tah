#include "global.h"
#include "server.h"
#include "CCmds.h"
#include "CInGame.h"

#define RIGHT_CHECK(a) if(!(this->rights & a)) { Print(L"ERR No permission\n"); return; }
#define RIGHT_CHECK_SUPERADMIN() if(!(this->rights == RIGHT_SUPERADMIN)) { Print(L"ERR No permission\n"); return; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCash(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;
	if(HKSUCCESS(HkGetCash(wscCharname, iCash)))
		Print(L"cash=%d\nOK\n", iCash);
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetCash(wstring wscCharname, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;
	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		HkAddCash(wscCharname, iAmount - iCash);
		CmdGetCash(wscCharname);
	} else
		PrintError();
}

void CCmds::CmdSetCashSec(wstring wscCharname, int iAmountCheck, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;

	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		if(iCash != iAmountCheck)
			Print(L"ERR Security check failed\n");
		else
			CmdSetCash(wscCharname, iAmount);
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCash(wstring wscCharname, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	if(HKSUCCESS(HkAddCash(wscCharname, iAmount)))
		CmdGetCash(wscCharname);
	else
		PrintError();
}

void CCmds::CmdAddCashSec(wstring wscCharname, int iAmountCheck, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;

	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		if(iCash != iAmountCheck)
			Print(L"ERR Security check failed\n");
		else
			CmdAddCash(wscCharname, iAmount);
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKick(wstring wscCharname, wstring wscReason)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkKickReason(wscCharname, wscReason)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBan(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkBan(wscCharname, true)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnban(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkBan(wscCharname, false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKickBan(wstring wscCharname, wstring wscReason)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(!HKSUCCESS(HkBan(wscCharname, true)))
	{
		PrintError();
		return;
	}

	if(!HKSUCCESS(HkKickReason(wscCharname, wscReason)))
	{
		PrintError();
		return;
	}

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetBaseStatus(wstring wscBasename)
{
	float fHealth;
	float fMaxHealth;

	if(HKSUCCESS(HkGetBaseStatus(wscBasename, fHealth, fMaxHealth)))
		Print(L"hitpts=%u hitptsmax=%u\nOK\n", (long)fHealth, (long)fMaxHealth);
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetClientId(wstring wscCharname)
{
	uint iClientID = HkGetClientIdFromCharname(wscCharname);
	if(iClientID == -1)
	{
		hkLastErr = HKE_PLAYER_NOT_LOGGED_IN;
		PrintError();
		return;
	}

	Print(L"clientid=%u\nOK\n", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBeam(wstring wscCharname, wstring wscBasename)
{
	RIGHT_CHECK(RIGHT_BEAM);

	try {
		if(HKSUCCESS(HkBeam(wscCharname, wscBasename)))
			Print(L"OK\n");
		else
			PrintError();
	} catch(...) { // exeption, kick player
		HkKick(wscCharname);
		Print(L"ERR exception occured, player kicked\n");
	}
}

void CCmds::CmdKBeam(wstring wscCharname, wstring wscBasename)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(HKSUCCESS(HkKillBeam(wscCharname, wscBasename)))
		Print(L"OK\n");
	else
		PrintError();
}

void CCmds::CmdInstantDock(wstring wscCharname)
{
	if(admin.iClientID) //dock to admin's target
	{
		uint iShip, iTarget;
		pub::Player::GetShip(admin.iClientID, iShip);
		if(!iShip)
		{
			Print(L"ERR You are docked.");
			return;
		}
		pub::SpaceObj::GetTarget(iShip, iTarget);
		if(!iTarget)
		{
			Print(L"ERR You do not have an object targeted.");
			return;
		}
		if(HKSUCCESS(HkInstantDock(wscCharname, iTarget)))
			Print(L"OK\n");
		else
			PrintError();
	}
	else
	{
		Print(L"ERR you are not a player");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKill(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(!wscCharname.length() && admin.iClientID) //Kill admin's target
	{
		uint iShip, iTarget;
		pub::Player::GetShip(admin.iClientID, iShip);
		if(!iShip)
		{
			Print(L"ERR You are docked.");
			return;
		}
		pub::SpaceObj::GetTarget(iShip, iTarget);
		if(!iTarget)
		{
			Print(L"ERR You do not have an object targeted.");
			return;
		}
		if(HkGetClientIDByShip(iTarget))
			wscAdminKiller = Players.GetActiveCharacterName(admin.iClientID);
		pub::SpaceObj::SetRelativeHealth(iTarget, 0.0f);
		Print(L"OK\n");
		return;
	}

	if(admin.iClientID)
		wscAdminKiller = Players.GetActiveCharacterName(admin.iClientID);
	else
		wscAdminKiller = L"Console";
	if(HKSUCCESS(HkKill(wscCharname)))
		Print(L"OK\n");
	else
	{
		wscAdminKiller = L"";
		PrintError();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdResetRep(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(HKSUCCESS(HkResetRep(wscCharname)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetRep(wstring wscCharname, wstring wscRepGroup, float fValue)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(HKSUCCESS(HkSetRep(wscCharname, wscRepGroup, fValue)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetRepRelative(wstring wscCharname, wstring wscRepGroup, float fValue)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(HKSUCCESS(HkSetRepRelative(wscCharname, wscRepGroup, fValue, false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetRep(wstring wscCharname, wstring wscRepGroup)
{
	RIGHT_CHECK(RIGHT_BEAM);

	float fValue;
	if(HKSUCCESS(HkGetRep(wscCharname, wscRepGroup, fValue)))
	{
		Print(L"rep=%f\nOK\n", fValue);
	}
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAffiliation(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_BEAM);

	wstring wscRepGroup;
	if(HKSUCCESS(HkGetAffiliation(wscCharname, wscRepGroup)))
	{
		Print(L"repgroup=" + wscRepGroup + L"\nOK\n");
	}
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAffiliation(wstring wscCharname, wstring wscRepGroup)
{
	RIGHT_CHECK(RIGHT_BEAM);

	if(HKSUCCESS(HkSetAffiliation(wscCharname, wscRepGroup)))
	{
		Print(L"OK\n");
	}
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsg(wstring wscCharname, wstring wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsg(wscCharname, wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgS(wstring wscSystemname, wstring wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsgS(wscSystemname, wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgU(wstring wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(!admin.wscAdminName.size() || !admin.iClientID)
	{
		CCmds::CmdMsgUc(wscText);
		return;
	}

	if(!HKSUCCESS(HkMsgU(admin.wscAdminName, wscText)))
		PrintError();
}

void CCmds::CmdMsgUc(wstring wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsgU(wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsg(wstring wscCharname, wstring wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsg(wscCharname, wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgS(wstring wscSystemname, wstring wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsgS(wscSystemname, wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgU(wstring wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsgU(wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdEnumCargo(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_CARGO);

	list<CARGO_INFO> lstCargo;
	int iRemainingHoldSize = 0;
	if(HKSUCCESS(HkEnumCargo(wscCharname, lstCargo, &iRemainingHoldSize))) {
		Print(L"remainingholdsize=%d\n", iRemainingHoldSize);
		foreach(lstCargo, CARGO_INFO, it)
		{
			Archetype::Equipment *eq = Archetype::GetEquipment(it->iArchID);
			//if(!(*it).bMounted)
			if(eq->iIDSName)
				Print(L"id=%u name=\"%s\" count=%d mission=%s\n", (*it).iID, HkGetWStringFromIDS(eq->iIDSName).c_str(), (*it).iCount, (*it).bMission ? L"yes" : L"no");
		}

		Print(L"OK\n");
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(wstring wscCharname, uint iID, uint iCount)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if(HKSUCCESS(HkRemoveCargo(wscCharname, iID, iCount)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCargo(wstring wscCharname, wstring wscGood, uint iCount, uint iMission)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if(HKSUCCESS(HkAddCargo(wscCharname, wscGood, iCount, iMission ? true : false)))
		Print(L"OK\n");
	else
		PrintError();
}

void CCmds::CmdAddCargo(wstring wscCharname, wstring wscGood, wstring wscHardpoint, uint iMission)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if(HKSUCCESS(HkAddCargo(wscCharname, wscGood, wscHardpoint, iMission ? true : false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRename(wstring wscCharname, wstring wscNewCharname)
{
	RIGHT_CHECK(RIGHT_CHARS);

	if(HKSUCCESS(HkRename(wscCharname, wscNewCharname, false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDeleteChar(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_CHARS);

	if(HKSUCCESS(HkRename(wscCharname, L"", true)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReadCharFile(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_CHARS);

	list<wstring> lstOut;
	if(HKSUCCESS(HkReadCharFile(wscCharname, lstOut))) {
		foreach(lstOut, wstring, it)
			Print(L"l %s\n", it->c_str());
		Print(L"OK\n");
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdWriteCharFile(wstring wscCharname, wstring wscData)
{
	RIGHT_CHECK(RIGHT_CHARS);

	if(HKSUCCESS(HkWriteCharFile(wscCharname, wscData)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintPlayerInfo(HKPLAYERINFO pi)
{
	Print(L"charname=%s clientid=%u ip=%s host=%s ping=%u loss=%u base=%s system=%s\n", pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS, ClientInfo[pi.iClientID].iAverageLoss, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdGetPlayerInfo(wstring wscCharname)
{
	HKPLAYERINFO pi;
	if(HKSUCCESS(HkGetPlayerInfo(wscCharname, pi, false)))
		PrintPlayerInfo(pi);
	else
		PrintError();

	Print(L"OK\n");
}

void CCmds::CmdGetPlayers()
{
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
		PrintPlayerInfo(*i);

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(HKPLAYERINFO pi)
{
	Print(L"Name: %s, ID: %u, IP: %s, Host: %s, Ping: %u, Loss: %u, Base: %s, System: %s\n", pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS, ClientInfo[pi.iClientID].iAverageLoss, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdXGetPlayerInfo(wstring wscCharname)
{
	HKPLAYERINFO pi;
	if(HKSUCCESS(HkGetPlayerInfo(wscCharname, pi, false)))
		XPrintPlayerInfo(pi);
	else
		PrintError();

	Print(L"OK\n");
}

void CCmds::CmdXGetPlayers()
{
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
		XPrintPlayerInfo(*i);

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIDs()
{
	wchar_t wszLine[128] = L"";
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
	{
		wchar_t wszBuf[1024];
		swprintf(wszBuf, L"%s = %u | ", (*i).wscCharname.c_str(), (*i).iClientID);
		if((wcslen(wszBuf) + wcslen(wszLine)) >= sizeof(wszLine)/2)	{
			Print(L"%s\n", wszLine);
			wcscpy(wszLine, wszBuf);
		} else
			wcscat(wszLine, wszBuf);
	}

	if(wcslen(wszLine))
		Print(L"%s\n", wszLine);
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAccountDirName(wstring wscCharname)
{
	wstring wscDir;
	if(HKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
		Print(L"dirname=%s\nOK\n", wscDir.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCharFileName(wstring wscCharname)
{
	wstring wscFilename;
	if(HKSUCCESS(HkGetCharFileName(wscCharname, wscFilename)))
		Print(L"charfilename=%s\nOK\n", wscFilename.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsOnServer(wstring wscCharname)
{
	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
	{
		hkLastErr = HKE_CHAR_DOES_NOT_EXIST;
		PrintError();
		return;
	}

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		Print(L"onserver=no\nOK\n");
	else
		Print(L"onserver=yes\nOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsLoggedIn(wstring wscCharname)
{
	if(HkGetClientIdFromCharname(wscCharname) != -1) {
		if(HkIsInCharSelectMenu(wscCharname))
			Print(L"loggedin=no\nOK\n");
		else
			Print(L"loggedin=yes\nOK\n");
	} else
		Print(L"loggedin=no\nOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMoneyFixList()
{
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);

		if(ClientInfo[iClientID].lstMoneyFix.size())
			Print(L"id=%u\n", iClientID);
	}

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdServerInfo()
{
	// calculate uptime
	FILETIME ftCreation;
	FILETIME ft;
	GetProcessTimes(GetCurrentProcess(), &ftCreation, &ft, &ft, &ft);
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ftNow;
	SystemTimeToFileTime(&st, &ftNow);
	__int64 iTimeCreation = (((__int64)ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
	__int64 iTimeNow = (((__int64) ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

	uint iUptime = (uint)((iTimeNow - iTimeCreation) / 10000000);
	uint iDays = (iUptime / (60*60*24));
	iUptime %= (60*60*24);
	uint iHours = (iUptime / (60*60));
	iUptime %= (60*60);
	uint iMinutes = (iUptime / 60);
	iUptime %= (60);
	uint iSeconds = iUptime;
	wchar_t wszUptime[16];
	swprintf(wszUptime, L"%.1u:%.2u:%.2u:%.2u", iDays, iHours, iMinutes, iSeconds);

	// print
	Print(L"serverload=%d npcspawn=%s uptime=%s\nOK\n", g_iServerLoad, g_bNPCDisabled ? L"disabled" : L"enabled", wszUptime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetGroupMembers(wstring wscCharname)
{
	list<GROUP_MEMBER> lstMembers;
	if(HKSUCCESS(HkGetGroupMembers(wscCharname, lstMembers))) {
		Print(L"groupsize=%d\n", lstMembers.size());
		foreach(lstMembers, GROUP_MEMBER, it)
			Print(L"id=%d charname=%s\n", it->iClientID, it->wscCharname.c_str());
		Print(L"OK\n");
	} else
		PrintError();
}

void CCmds::CmdGetGroup(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	uint groupID;
	if(HKSUCCESS(HkGetGroupID(wscCharname, groupID)))
		Print(L"groupid=%u\nOK\n", groupID);
	else
		PrintError();
}

void CCmds::CmdAddToGroup(wstring wscCharname, uint iGroupID)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkAddToGroup(wscCharname, iGroupID)))
		Print(L"OK\n");
	else
		PrintError();
}

void CCmds::CmdInviteToGroup(wstring wscCharname, wstring wscCharnameTo)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkInviteToGroup(wscCharname, wscCharnameTo)))
		Print(L"OK\n");
	else
		PrintError();
}

void CCmds::CmdRemoveFromGroup(wstring wscCharname, uint iGroupID)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkRemoveFromGroup(wscCharname, iGroupID)))
		Print(L"OK\n");
	else
		PrintError();
}

void CCmds::CmdSendGroupCash(uint iGroupID, int iAmount)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkSendGroupCash(iGroupID, iAmount)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSaveChar(wstring wscCharname)
{
	RIGHT_CHECK(RIGHT_SAVECHAR);

	if(HKSUCCESS(HkSaveChar(wscCharname)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetReservedSlot(wstring wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	bool bResult;
	if(HKSUCCESS(HkGetReservedSlot(wscCharname, bResult)))
		Print(L"reservedslot=%s\nOK\n", bResult ? L"yes" : L"no");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetReservedSlot(wstring wscCharname, int iReservedSlot)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkSetReservedSlot(wscCharname, iReservedSlot ? true : false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdmin(wstring wscCharname, wstring wscRights)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkSetAdmin(wscCharname, wscRights)))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdminRestriction(wstring wscCharname, wstring wscRestriction)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkSetAdminRestriction(wscCharname, wscRestriction)))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAdminRestriction(wstring wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	wstring wscRestriction;
	if(HKSUCCESS(HkGetAdminRestriction(wscCharname, wscRestriction)))
        Print(L"restriction=%s\nOK\n", wscRestriction.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAdmin(wstring wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	wstring wscRights;
	if(HKSUCCESS(HkGetAdmin(wscCharname, wscRights)))
        Print(L"rights=%s\nOK\n", wscRights.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDelAdmin(wstring wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkDelAdmin(wscCharname)))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddDPSystem(wstring wscSystem)
{
	RIGHT_CHECK(RIGHT_CASH);
	RIGHT_CHECK(RIGHT_CARGO);

	string scSystem = wstos(wscSystem);
	uint iSystemID = 0;
	pub::GetSystemID(iSystemID, scSystem.c_str());
	if(iSystemID)
	{
		UINT_WRAP *uw = new UINT_WRAP(iSystemID);
		set_btNoDPSystems->Add(uw);
		Print(L"OK\n");
	}
	else
		Print(L"ERR Invalid system\n");
}

void CCmds::CmdRemoveDPSystem(wstring wscSystem)
{
	RIGHT_CHECK(RIGHT_CASH);
	RIGHT_CHECK(RIGHT_CARGO);

	string scSystem = wstos(wscSystem);
	uint iSystemID = 0;
	pub::GetSystemID(iSystemID, scSystem.c_str());
	if(iSystemID)
	{
		UINT_WRAP uw = UINT_WRAP(iSystemID);
		if(set_btNoDPSystems->Delete(&uw))
			Print(L"OK\n");
		else
			Print(L"ERR Could not find system in exclusion list");
	}
	else
		Print(L"ERR Invalid system\n");
}

void CCmds::CmdSetDPFraction(float fFraction)
{
	RIGHT_CHECK(RIGHT_CASH);
	RIGHT_CHECK(RIGHT_CARGO);

	set_fDeathPenalty = fFraction;

	Print(L"OK\n");
}

void CCmds::CmdEnumDPSystems()
{
	if(set_btNoDPSystems->Count())
	{
		wstring wscSystems = L"";
		BinaryTreeIterator<UINT_WRAP> *DPIter = new BinaryTreeIterator<UINT_WRAP>(set_btNoDPSystems);
		DPIter->First();
		for(long i=0; i<set_btNoDPSystems->Count(); i++, DPIter->Next())
		{
			wscSystems += HkGetSystemNickByID(DPIter->Curr()->val) + L"|";
		}
		Print(wscSystems.substr(0, wscSystems.length()-1) + L"\n");
	}
	else
	{
		Print(L"No DP excluded systems\n");
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRehash()
{
	RIGHT_CHECK_SUPERADMIN();
	LoadSettings();
	ConPrint(L"Loaded settings...\n"); //Don't ask me why, but this prevents an exception from being thrown
	HookRehashed();
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnload(wstring wscParam)
{
	RIGHT_CHECK_SUPERADMIN();

	if(ToLower(wscParam).compare(L"nokick") != 0)
	{
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if(ClientInfo[iClientID].lstMoneyFix.size() || ClientInfo[iClientID].lstCargoFix.size())
				HkKick(ARG_CLIENTID(iClientID)); // money/cargo fix active, thus kick
		}
	}

	Print(L"OK\n");
	FLHookInitUnload();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRestartServer()
{
	RIGHT_CHECK_SUPERADMIN();
	if(HKSUCCESS(HkRestartServer()))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdShutdownServer()
{
	RIGHT_CHECK_SUPERADMIN();
	if(HKSUCCESS(HkShutdownServer()))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSpawns(wstring wscToggle)
{
	RIGHT_CHECK_SUPERADMIN();
	if(ToLower(wscToggle) == L"on")
	{
		HkChangeNPCSpawn(false);
	}
	else if(ToLower(wscToggle) == L"off")
	{
		HkChangeNPCSpawn(true);
	}
	else if(!wscToggle.length())
	{
		g_bNPCForceDisabled = false;
		Print(L"Control ceded to server load\n");
		return;
	}
	else
	{
		Print(L"ERR argument should be <on|off>\n");
		return;
	}
	g_bNPCForceDisabled = true;
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMission(wstring wscCharname)
{
    RIGHT_CHECK_SUPERADMIN();

    uint iClientID = HkGetClientIdFromCharname(wscCharname);

    if(iClientID != -1)
    {
        uint pAddress = (uint)hModContent + 0x114388;
        pub::Controller::CreateParms params = {pAddress, 1};
        ClientInfo[iClientID].iControllerID = ControllerCreate("Content.dll", "Mission_10", &params, (pub::Controller::PRIORITY)2);
        ControllerSend(ClientInfo[iClientID].iControllerID, 0x1000, 0);
        Print(L"OK\n");
    }
    else
        Print(L"InvalidID\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp(uint iPageNum)
{
	wstring wscHelpMsg[] =
	{
		L"version=" VERSION L"\n",
		L"[commands]\n",
		L"addcargo <charname> <good> <count> <mission>\n",
		L"addcargom <charname> <good> <hardpoint> <mission>\n",
		L"addcash <charname> <amount>\n",
		L"addcashsec <charname> <oldmoney> <amount>\n",
		L"adddpsystem <systemname>\n",
		L"addtogroup <charname> <group>\n",
		L"ban <charname>\n",
		L"beam <charname> <basename>\n",
		L"deladmin <charname>\n",
		L"deletechar <charname>\n",
		L"enumcargo <charname>\n",
		L"enumdpsystems\n",
		L"fmsg <charname> <xmltext>\n",
		L"fmsgs <systemname> <xmltext>\n",
		L"fmsgu <xmltext>\n",
		L"getaccountdirname <charname>\n",
		L"getadmin <charname>\n",
		L"getadminrestriction <charname>\n",
		L"getaffiliation <charname>\n",
		L"getbasestatus <basename>\n",
		L"getcash <charname>\n",
		L"getcharfilename <charname>\n",
		L"getclientid <charname>\n",
		L"getgroup <charname>\n",
		L"getgroupmembers <charname>\n",
		L"getplayerids\n",
		L"getplayerinfo <charname>\n",
		L"getplayers\n",
		L"getrep <charname> <repgroup>\n",
		L"instantdock <charname>\n",
		L"invitetogroup <charname> <fromcharname>\n",
		L"isloggedin <charname>\n",
		L"isonserver <charname>\n",
		L"kick <charname> <reason>\n",
		L"kickban <charname> <reason>\n",
		L"kill [charname]\n",
		L"moneyfixlist\n",
		L"msg <charname> <text>\n",
		L"msgs <systemname> <text>\n",
		L"msgu <text>\n",
		L"msguc <text>\n",
		L"mission <charname>\n",
		L"readcharfile <charname>\n",
		L"rehash\n",
		L"removecargo <charname> <id> <count>\n",
		L"removedpsystem <systemname>\n",
		L"removefromgroup <charname> <group>\n",
		L"rename <oldcharname> <newcharname>\n",
		L"resetrep <charname>\n",
		L"restartserver\n",
		L"savechar <charname>\n",
		L"sendgroupcash <group> <amount>\n",
		L"serverinfo\n",
		L"setadmin <charname> <rights>\n",
		L"setadminrestriction <charname> <restriction>\n",
		L"setaffiliation <charname> <repgroup>\n",
		L"setcash <charname> <amount>\n",
		L"setcashsec <charname> <oldmoney> <amount>\n",
		L"setdpfraction <decimal amount>\n",
		L"setrep <charname> <repgroup> <value>\n",
		L"setreprelative <charname> <repgroup> <value>\n",
		L"shutdownserver\n",
		L"spawns [on|off]\n",
		L"unban <charname>\n",
		L"unload\n",
		L"writecharfile <charname> <data>\n",
		L"xgetplayerinfo <charname>\n",
		L"xgetplayers\n",
		L"help <page number>\n",
	};

	if(admin.iClientID) //called in-game
	{
		wstring wscMsg = L"";
		uint iLimit = sizeof(wscHelpMsg)/sizeof(wstring);
		iLimit = iPageNum*30>iLimit ? iLimit : iPageNum*30;
		uint i=(iPageNum-1)*30;
		if(i >= iLimit)//no commands to be printed
		{
			Print(L"ERR Invalid page number\nUsage: help [page number]\n");
		}
		else
		{
			for(; i<iLimit; i++)
			{
				wscMsg += wscHelpMsg[i];
			}
			Print(wscMsg + L"OK\n");
		}
	}
	else
	{
		wstring wscMsg = L"";
		for(uint i=0; i<sizeof(wscHelpMsg)/sizeof(wstring); i++)
		{
			wscMsg += wscHelpMsg[i];
		}
		Print(wscMsg + L"OK\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdType(wstring wscCharname)
{
	HK_GET_CLIENTID_NORETURN(iClientID, wscCharname);
	if(iClientID == -1)
	{
		uint iType, iShip, iTarget;
		pub::Player::GetShip(admin.iClientID, iShip);
		pub::SpaceObj::GetTarget(iShip, iTarget);
		pub::SpaceObj::GetType(iTarget, iType);
		Print(L"type=%u\nOK\n", iType);
	}
	else
	{
		uint iType, iShip;
		pub::Player::GetShip(iClientID, iShip);
		pub::SpaceObj::GetType(iShip, iType);
		Print(L"type=%u\nOK\n", iType);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*void CCmds::CmdTest()
{
	Print(L"command=nothing\nOK");
}*/

void CCmds::CmdTest()
{
	uint iShip;
	pub::Player::GetShip(admin.iClientID, iShip);
	//pub::SpaceObj::LightFuse(iShip, wstos(ArgStr(1)).c_str(), 0);
	IObjRW *ship = GetIObjRW(iShip);
	uint iFuseID = CreateID(wstos(ArgStr(1)).c_str());
	if(ArgStr(2) == L"1")
		HkLightFuse(ship, iFuseID, 2.0f, 0.0f, -1.0f);
	else
		try{ HkUnLightFuse(ship, iFuseID, 0.0f); } catch(...) { LOG_EXCEPTION }
	ConPrint(L"OK\n");
}

/*void CCmds::CmdTest()
{
	wstring wscSystem = ArgStr(1);
	uint iSystem = Universe::get_system_id(wstos(wscSystem).c_str());
	ConPrint(L"system %u\n", iSystem);
	HkSwitchSystem(admin.iClientID, iSystem);
	uint iShip;
	pub::Player::GetShip(admin.iClientID, iShip);
	float fHealth;
	pub::SpaceObj::GetRelativeHealth(iShip, fHealth);
	Print(L"fHealth=%f", fHealth);
	uint iCurSys;
	pub::SpaceObj::GetSystem(iShip, iCurSys);
	pub::System::GetName(iCurSys, iCurSys);
	Print(HkGetWStringFromIDS(iCurSys));
	Print(L"ExistsAndAlive=%i", pub::SpaceObj::ExistsAndAlive(iShip));
	Print(L"OK\n");
}*/

/*void CCmds::CmdTest()
{
	uint iGoodID = 2851971011;
	pub::GetGoodID(iGoodID, wstos(ArgStr(1)).c_str());
	uint iTarget;
	pub::SpaceObj::GetTarget(Players[admin.iClientID].iSpaceObjID, iTarget);
	CEqObj *obj = HkGetEqObjFromObjRW(GetIObjRW(iTarget));
	//CEquipManager *equipment = (CEquipManager*)(((char*)obj) + 0xE4);
	//CEquipTraverser traverser;
	//CEquip *last = equipment->ReverseTraverse(traverser);
	EquipDesc ed = EquipDesc();
	//memset(&ed, 0, sizeof(ed));
	//ed.id = last->GetID() + 1;
	//ed.count = 1;
	//ed.archid = iGoodID;
	//ed.hp = 1.0f;
	//ed.set_hardpoint(EquipDesc::CARGO_BAY_HP_NAME);
	ed.set_count(ArgInt(2));
	ed.set_arch_id(iGoodID);
	ed.set_status(1.0f);
	obj->add_item(ed);
	//obj->update(1.0f, iTarget);
	Print(L"OK");
}*/

/*uint iSavedTarget = 0;
void CCmds::CmdTest(wstring wsc)
{
	uint iShip;
	pub::Player::GetShip(admin.iClientID, iShip);
	uint iTarget;
	pub::SpaceObj::GetTarget(iShip, iTarget);
	if(!iSavedTarget)
	{
		iSavedTarget = iTarget;
		Print(L"Saved Target");
	}
	else
	{
		pub::AI::submit_forced_target(iTarget, iSavedTarget);
		pub::AI::BaseOp *op = new pub::AI::BaseOp(pub::AI::M_TRAIL);
		//pub::AI::DirectiveDockOp *op = new pub::AI::DirectiveDockOp();
		pub::AI::SubmitState(iTarget, op);
		iSavedTarget = 0;
		Print(L"Sumitted State");
	}
}*/

//uint iOutputNum = 0;
/*void CCmds::CmdTest(wstring wsc)
{
	uint iLoadoutID;
	pub::GetLoadoutID(iLoadoutID, "n00b_package_loadout");
	EquipDescVector equipVect;
	pub::GetLoadout(equipVect, iLoadoutID);
	uint iShipID;
	pub::Player::GetShipID(admin.iClientID, iShipID);
	pub::Player::SetShipAndLoadout(admin.iClientID, iShipID, equipVect);
	/*uint iShip;
	pub::Player::GetShip(admin.iClientID, iShip);
	Vector v;
	Matrix m;
	pub::SpaceObj::GetLocation(iShip, v, m);
	Print(L"%f %f %f", m.data[0][0], m.data[1][0], m.data[2][0]);
	Print(L"%f %f %f", m.data[0][1], m.data[1][1], m.data[2][1]);
	Print(L"%f %f %f", m.data[0][2], m.data[1][2], m.data[2][2]);*/
	/*if(!ClientInfo[admin.iClientID].bMobileDocked)
	{
		uint iShip;
		pub::Player::GetShip(admin.iClientID, iShip);
		pub::SpaceObj::GetLocation(iShip, ClientInfo[admin.iClientID].Vlaunch, ClientInfo[admin.iClientID].Mlaunch);
		ClientInfo[admin.iClientID].bMobileDocked = true;
	}
	else
	{
		HkBeamInSys(ARG_CLIENTID(admin.iClientID), ClientInfo[admin.iClientID].Vlaunch, ClientInfo[admin.iClientID].Mlaunch);
		ClientInfo[admin.iClientID].bMobileDocked = false;
	}*/
	/*ulong *arr1 = Players[admin.iClientID].lArray1;
	ulong *arr2 = Players[admin.iClientID].lArray2;
	ulong *arr3 = Players[admin.iClientID].lArray3;
	ulong *arr4 = Players[admin.iClientID].lArray4;
	ulong *arr5 = Players[admin.iClientID].lArray5;
	FILE *f = fopen(("PlayerInfo" + itos(iOutputNum) + ".txt").c_str(), "w");

	for(uint i=0; i<153; i++)
	{
		fprintf(f, "%u\n", arr1[i] );
	}
	fprintf(f, "----------------------\n");
	for(uint i=0; i<58; i++)
	{
		fprintf(f, "%u\n", arr2[i] );
	}
	fprintf(f, "----------------------\n");
	for(uint i=0; i<33; i++)
	{
		fprintf(f, "%u\n", arr3[i] );
	}
	fprintf(f, "----------------------\n");
	for(uint i=0; i<2; i++)
	{
		fprintf(f, "%u\n", arr4[i] );
	}
	fprintf(f, "----------------------\n");
	for(uint i=0; i<9; i++)
	{
		fprintf(f, "%u\n", arr5[i] );
	}


	if(f)
	{
		fclose(f);
	};

	iOutputNum++;*/
/*	int iRep;
	pub::Player::GetRep(admin.iClientID, iRep);
	uint iRepGroupID;
	Reputation::Vibe::GetAffiliation(iRep, iRepGroupID, false);
	Print(L"%u\nOK\n", iRepGroupID);
*///}

/*void CCmds::CmdTest(wstring wscBasename)
{
	uint iShip;
	pub::Player::GetShip(admin.iClientID, iShip);

	uint iBaseID;
	string scBasename = wstos(wscBasename);
	if(pub::GetBaseID(iBaseID, scBasename.c_str()) == -4)
	{
		string scBaseShortcut = IniGetS(set_scCfgFile, "names", wstos(wscBasename), "");
		if(!scBaseShortcut.length())
		{
			Print(L"Error finding base\n");
			return;
		}

		if(pub::GetBaseID(iBaseID, scBaseShortcut.c_str()) == -4)
		{
			Print(L"Error finding base\n");
			return;
		}
	}

	Players[admin.iClientID].lBaseID = iBaseID;
	ConPrint(L"Change base to %u\n", iBaseID);
	uint iSpaceObjID = HkGetSpaceObjFromBaseID(iBaseID);
	if(iSpaceObjID)
	{
		uint iSystem;
		pub::SpaceObj::GetSystem(iSpaceObjID, iSystem);
		Players[admin.iClientID].lSystemID = iSystem;
		ConPrint(L"Change system to %u\n", iSystem);
	}

	Print(L"OK");
}*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgCharname(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	if(bID)
	{
		if(wscArg==L"$") //admin's client ID
			wscArg = stows(itos(admin.iClientID));
		return wscArg.replace((int) 0,(int) 0, L"id ");
	}
	else if(bShortCut)
		return wscArg.replace((int) 0,(int) 0, L"sc ");
	else
		return wscArg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCmds::ArgInt(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);
	return ToInt(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CCmds::ArgFloat(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);
	return ToFloat(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgStr(uint iArg)
{
	return GetParam(wscCurCmdString, ' ', iArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgStrToEnd(uint iArg)
{
	return GetParamToEnd(wscCurCmdString, ' ', iArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_CMD(a) !wscCmd.compare(L##a)

void CCmds::ExecuteCommandString(wstring wscCmdStr)
{
	try {
		if(set_iDebug >= 1)
			AddLog(wstos(GetAdminName() + L": " + wscCmdStr).c_str());

		bID = false;
		bShortCut = false;
		wscCurCmdString = wscCmdStr;

		wstring wscCmd = ToLower(GetParam(wscCmdStr, ' ', 0));
		if(wscCmd[wscCmd.length()-1] == '$') {
			bID = true;
			wscCmd.erase(wscCmd.length()-1, 1);
		} else if(wscCmd[wscCmd.length()-1] == '&') {
			bShortCut = true;
			wscCmd.erase(wscCmd.length()-1, 1);
		}

		if(IS_CMD("getcash")) {
			CmdGetCash(ArgCharname(1));
		} else if(IS_CMD("setcash")) {
			CmdSetCash(ArgCharname(1), ArgInt(2));
		} else if(IS_CMD("setcashsec")) {
			CmdSetCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
		} else if(IS_CMD("addcash")) {
			CmdAddCash(ArgCharname(1), ArgInt(2));
		} else if(IS_CMD("addcashsec")) {
			CmdAddCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
		} else if(IS_CMD("kick")) {
			CmdKick(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("ban")) {
			CmdBan(ArgCharname(1));
		} else if(IS_CMD("unban")) {
			CmdUnban(ArgCharname(1));
		} else if(IS_CMD("kickban")) {
			CmdKickBan(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("getbasestatus")) {
			CmdGetBaseStatus(ArgStr(1));
		} else if(IS_CMD("getclientid")) {
			CmdGetClientId(ArgCharname(1));
		} else if(IS_CMD("beam")) {
			CmdBeam(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("kbeam")) {
			CmdKBeam(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("instantdock")) {
			CmdInstantDock(ArgCharname(1));
		} else if(IS_CMD("kill")) {
			CmdKill(ArgCharname(1));
		} else if(IS_CMD("resetrep")) {
			CmdResetRep(ArgCharname(1));
		} else if(IS_CMD("setrep")) {
			CmdSetRep(ArgCharname(1), ArgStr(2), ArgFloat(3));
		} else if(IS_CMD("setreprelative")) {
			CmdSetRepRelative(ArgCharname(1), ArgStr(2), ArgFloat(3));
		} else if(IS_CMD("getrep")) {
			CmdGetRep(ArgCharname(1), ArgStr(2));
		} else if(IS_CMD("setaffiliation")) {
			CmdSetAffiliation(ArgCharname(1), ArgStr(2));
		} else if(IS_CMD("getaffiliation")) {
			CmdGetAffiliation(ArgCharname(1));
		} else if(IS_CMD("msg")) {
			CmdMsg(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("msgs")) {
			CmdMsgS(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("msgu") || IS_CMD("u")) {
			CmdMsgU(ArgStrToEnd(1));
		} else if(IS_CMD("msguc")) {
			CmdMsgUc(ArgStrToEnd(1));
		} else if(IS_CMD("fmsg")) {
			CmdFMsg(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("fmsgs")) {
			CmdFMsgS(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("fmsgu")) {
			CmdFMsgU(ArgStrToEnd(1));
		} else if(IS_CMD("enumcargo")) {
			CmdEnumCargo(ArgCharname(1));
		} else if(IS_CMD("removecargo")) {
			CmdRemoveCargo(ArgCharname(1), ArgInt(2), ArgInt(3));
		} else if(IS_CMD("addcargo")) {
			CmdAddCargo(ArgCharname(1), ArgStr(2), ArgInt(3), ArgInt(4));
		} else if(IS_CMD("addcargom")) {
			CmdAddCargo(ArgCharname(1), ArgStr(2), ArgStr(3), ArgInt(4));
		} else if(IS_CMD("rename")) {
			CmdRename(ArgCharname(1), ArgStr(2));
		} else if(IS_CMD("deletechar")) {
			CmdDeleteChar(ArgCharname(1));
		} else if(IS_CMD("readcharfile")) {
			CmdReadCharFile(ArgCharname(1));
		} else if(IS_CMD("writecharfile")) {
			CmdWriteCharFile(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("getplayerinfo")) {
			CmdGetPlayerInfo(ArgCharname(1));
		} else if(IS_CMD("getplayers")) {
			CmdGetPlayers();
		} else if(IS_CMD("xgetplayerinfo")) {
			CmdXGetPlayerInfo(ArgCharname(1));
		} else if(IS_CMD("xgetplayers")) {
			CmdXGetPlayers();
		} else if(IS_CMD("getplayerids")) {
			CmdGetPlayerIDs();
		} else if(IS_CMD("getaccountdirname")) {
			CmdGetAccountDirName(ArgCharname(1));
		} else if(IS_CMD("getcharfilename")) {
			CmdGetCharFileName(ArgCharname(1));
		} else if(IS_CMD("savechar")) {
			CmdSaveChar(ArgCharname(1));
		} else if(IS_CMD("isonserver")) {
			CmdIsOnServer(ArgCharname(1));
		} else if(IS_CMD("isloggedin")) {
			CmdIsLoggedIn(ArgCharname(1));
		} else if(IS_CMD("moneyfixlist")) {
			CmdMoneyFixList();
		} else if(IS_CMD("serverinfo")) {
			CmdServerInfo();
		} else if(IS_CMD("getgroupmembers")) {
			CmdGetGroupMembers(ArgCharname(1));
		} else if(IS_CMD("getgroup")) {
			CmdGetGroup(ArgCharname(1));
		} else if(IS_CMD("addtogroup")) {
			CmdAddToGroup(ArgCharname(1), ArgInt(2));
		} else if(IS_CMD("invitetogroup")) {
			CmdInviteToGroup(ArgCharname(1), ArgCharname(2));
		} else if(IS_CMD("removefromgroup")) {
			CmdRemoveFromGroup(ArgCharname(1), ArgInt(2));
		} else if(IS_CMD("sendgroupcash")) {
			CmdSendGroupCash(ArgInt(1), ArgInt(2));
		} else if(IS_CMD("getreservedslot")) {
			CmdGetReservedSlot(ArgCharname(1));
		} else if(IS_CMD("setreservedslot")) {
			CmdSetReservedSlot(ArgCharname(1), ArgInt(2));
		} else if(IS_CMD("setadmin")) {
			CmdSetAdmin(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("setadminrestriction")) {
			CmdSetAdminRestriction(ArgCharname(1), ArgStrToEnd(2));
		} else if(IS_CMD("getadminrestriction")) {
			CmdGetAdminRestriction(ArgCharname(1));
		} else if(IS_CMD("getadmin")) {
			CmdGetAdmin(ArgCharname(1));
		} else if(IS_CMD("deladmin")) {
			CmdDelAdmin(ArgCharname(1));
		} else if(IS_CMD("rehash")) {
			CmdRehash();
		} else if(IS_CMD("unload")) {
			CmdUnload(ArgStr(1));
		} else if(IS_CMD("restartserver")) {
			CmdRestartServer();
		} else if(IS_CMD("shutdownserver")) {
			CmdShutdownServer();
		} else if(IS_CMD("type")) {
			CmdType(ArgCharname(1));
		} else if(IS_CMD("adddpsystem")) {
			CmdAddDPSystem(ArgStr(1));
		} else if(IS_CMD("removedpsystem")) {
			CmdRemoveDPSystem(ArgStr(1));
		} else if(IS_CMD("setdpfraction")) {
			CmdSetDPFraction(ArgFloat(1));
		} else if(IS_CMD("enumdpsystems")) {
			CmdEnumDPSystems();
		} else if(IS_CMD("spawns")) {
			CmdSpawns(ArgStr(1));
		} else if(IS_CMD("mission")) {
            CmdMission(ArgCharname(1));
		} else if(IS_CMD("help")) {
			CmdHelp(ArgInt(1));
		} else if(IS_CMD("test")) {
			CmdTest();
		} else {
			Print(L"ERR unknown command\n");
		}

		admin.wscAdminName = L"";
		admin.iClientID = 0;

		if(set_iDebug >= 1)
			AddLog("finished");
	} catch(...) {
		if(set_iDebug >= 1)
			AddLog("exception");
		Print(L"ERR exception occured\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::SetRightsByString(string scRightStr)
{
	rights = RIGHT_NOTHING;
	scRightStr = ToLower(scRightStr);
	if(scRightStr.find("superadmin") != -1)
		rights |= RIGHT_SUPERADMIN;
	if(scRightStr.find("cash") != -1)
		rights |= RIGHT_CASH;
	if(scRightStr.find("kickban") != -1)
		rights |= RIGHT_KICKBAN;
	if(scRightStr.find("beam") != -1)
		rights |= RIGHT_BEAM;
	if(scRightStr.find("msg") != -1)
		rights |= RIGHT_MSG;
	if(scRightStr.find("savechar") != -1)
		rights |= RIGHT_SAVECHAR;
	if(scRightStr.find("cargo") != -1)
		rights |= RIGHT_CARGO;
	if(scRightStr.find("chars") != -1)
		rights |= RIGHT_CHARS;
	if(scRightStr.find("eventmode") != -1)
		rights |= RIGHT_EVENTMODE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintError()
{
	Print(L"ERR %s\n", HkErrGetText(this->hkLastErr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::Print(wstring wscText, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscText);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	DoPrint(wszBuf);
}
