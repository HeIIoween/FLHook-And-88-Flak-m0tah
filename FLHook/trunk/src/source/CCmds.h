#ifndef _CCMDS_
#define _CCMDS_

#include "hook.h"

// enums
enum CCMDS_RIGHTS
{
	RIGHT_NOTHING		= 0,
	RIGHT_SUPERADMIN	= 0xFFFFFFFF,
	RIGHT_CASH			= (1 << 0),
	RIGHT_KICKBAN		= (1 << 1),
	RIGHT_BEAM			= (1 << 2),
	RIGHT_MSG			= (1 << 3),
	RIGHT_SAVECHAR		= (1 << 4),
	RIGHT_EVENTMODE		= (1 << 5),
	RIGHT_CARGO			= (1 << 6),
	RIGHT_CHARS			= (1 << 7),
};

// class
class CCmds
{
	bool bID;
	bool bShortCut;

public:
	DWORD rights;
	wstring wscCharRestrict;
	HK_ERROR hkLastErr;

	void PrintError();

// commands
	void CmdGetCash(wstring wscCharname);
	void CmdSetCash(wstring wscCharname, int iAmount);
	void CmdSetCashSec(wstring wscCharname, int iAmountCheck, int iAmount);
	void CmdAddCash(wstring wscCharname, int iAmount);
	void CmdAddCashSec(wstring wscCharname, int iAmountCheck, int iAmount);

	void CmdKick(wstring wscCharname, wstring wscReason);
	void CmdBan(wstring wscCharname);
	void CmdUnban(wstring wscCharname);
	void CmdKickBan(wstring wscCharname, wstring wscReason);

	void CmdBeam(wstring wscCharname, wstring wscBasename);
	void CmdKBeam(wstring wscCharname, wstring wscBasename);
	void CmdKill(wstring wscCharname);
	void CmdInstantDock(wstring wscCharname);

	void CmdResetRep(wstring wscCharname);
	void CmdSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
	void CmdSetRepRelative(wstring wscCharname, wstring wscRepGroup, float fValue);
	void CmdGetRep(wstring wscCharname, wstring wscRepGroup);
	void CmdGetAffiliation(wstring wscCharname);
	void CmdSetAffiliation(wstring wscCharname, wstring wscRepGroup);

	void CmdMsg(wstring wscCharname, wstring wscText);
	void CmdMsgS(wstring wscSystemname, wstring wscText);
	void CmdMsgU(wstring wscText);
	void CmdMsgUc(wstring wscText);
	void CmdFMsg(wstring wscCharname, wstring wscXML);
	void CmdFMsgS(wstring wscSystemname, wstring wscXML);
	void CmdFMsgU(wstring wscXML);

	void CmdEnumCargo(wstring wscCharname);
	void CmdRemoveCargo(wstring wscCharname, uint iID, uint iCount);
	void CmdAddCargo(wstring wscCharname, wstring wscGood, uint iCount, uint iMission);
	void CmdAddCargo(wstring wscCharname, wstring wscGood, wstring wscHardpoint, uint iMission);

	void CmdRename(wstring wscCharname, wstring wscNewCharname);
	void CmdDeleteChar(wstring wscCharname);

	void CmdReadCharFile(wstring wscCharname);
	void CmdWriteCharFile(wstring wscCharname, wstring wscData);

	void CmdGetBaseStatus(wstring wscBasename);
	void CmdGetClientId(wstring wscCharname);
	void PrintPlayerInfo(HKPLAYERINFO pi);
	void CmdGetPlayerInfo(wstring wscCharname);
	void CmdGetPlayers();
	void XPrintPlayerInfo(HKPLAYERINFO pi);
	void CmdXGetPlayerInfo(wstring wscCharname);
	void CmdXGetPlayers();
	void CmdGetPlayerIDs();
	void CmdHelp(uint iPageNum);
	void CmdGetAccountDirName(wstring wscCharname);
	void CmdGetCharFileName(wstring wscCharname);
	void CmdIsOnServer(wstring wscCharname);
	void CmdIsLoggedIn(wstring wscCharname);
	void CmdMoneyFixList();
	void CmdServerInfo();

	void CmdGetGroupMembers(wstring wscCharname);
	void CmdGetGroup(wstring wscCharname);
	void CmdAddToGroup(wstring wscCharname, uint iGroupID);
	void CmdInviteToGroup(wstring wscCharname, wstring wscCharnameTo);
	void CmdRemoveFromGroup(wstring wscCharname, uint iGroupID);
	void CmdSendGroupCash(uint iGroupID, int iAmount);

	void CmdSaveChar(wstring wscCharname);

	void CmdGetReservedSlot(wstring wscCharname);
	void CmdSetReservedSlot(wstring wscCharname, int iReservedSlot);
	void CmdSetAdmin(wstring wscCharname, wstring wscRights);
	void CmdSetAdminRestriction(wstring wscCharname, wstring wscRestriction);
	void CmdGetAdminRestriction(wstring wscCharname);
	void CmdGetAdmin(wstring wscCharname);
	void CmdDelAdmin(wstring wscCharname);
	void CmdRehash();
	void CmdUnload(wstring wscParam);
	void CmdRestartServer();
	void CmdShutdownServer();
	void CmdAddDPSystem(wstring wscSystem);
	void CmdRemoveDPSystem(wstring wscSystem);
	void CmdSetDPFraction(float fFraction);
	void CmdEnumDPSystems();

	void CmdSpawns(wstring wscToggle);

	void CmdType(wstring wscCharname);
	void CmdTest(wstring wsc);
//
	wstring ArgCharname(uint iArg);
	int ArgInt(uint iArg);
	float ArgFloat(uint iArg);
	wstring ArgStr(uint iArg);
	wstring ArgStrToEnd(uint iArg);
	void ExecuteCommandString(wstring wscCmd);

	void SetRightsByString(string scRightStr);
	void Print(wstring wscText, ...);
	virtual void DoPrint(wstring wscText) {};
	virtual wstring GetAdminName() { return L""; };

	wstring wscCurCmdString;
};

#endif