#ifndef _GLOBAL_
#define _GLOBAL_
#pragma warning(disable: 4311 4786)

// includes
#include "binarytree.h"
#include "blowfish.h"
#include <windows.h>
#include <stdio.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
using namespace std;

// defines
#define VERSION L"'88 Flak v1.6.7 dev build 23"

#define TIME_UPDATE 50
#define IMPORT __declspec(dllimport)
#define COMMA , //So you can pass templated types in type
#define foreach(lst, type, var) for(list< type >::iterator var = lst.begin(); (var != lst.end()); var++)
#define foreachreverse(lst, type, var) for(list< type >::reverse_iterator var = lst.rbegin(); (var != lst.rend()); var++)
#define LOG_ENTER() { if(set_iDebug >= 3) AddLog(__FUNCTION__ " stepin"); }
#define LOG_RETURN(a) { if(set_iDebug >= 3) AddLog(__FUNCTION__ " return: " #a); return a; }
#define LOG_RETURN_S(a) { \
	if(set_iDebug >= 3) {\
		char szBuf[1024]; \
		sprintf(szBuf, __FUNCTION__ " return: %s", a); \
		AddLog(szBuf); \
	} \
	return a;  \
	}
#define LOG_RETURN_WS(a) { \
	if(set_iDebug >= 3) {\
		string scStr = wstos(a); \
		char szBuf[1024]; \
		sprintf(szBuf, __FUNCTION__ " return: %s", scStr.c_str()); \
		AddLog(szBuf); \
	} \
	return a;  \
	}
#define LOG_RETURN_I(a) { \
	if(set_iDebug >= 3) {\
		char szBuf[1024]; \
		sprintf(szBuf, __FUNCTION__ " return: %d", (int)a); \
		AddLog(szBuf); \
	} \
	return a;  \
	}

#define LOG_LEAVE() if(set_iDebug >= 3) AddLog(__FUNCTION__ " stepout");
#define LOG_EXCEPTION AddLog("Exception in %s", __FUNCTION__); AddExceptionInfoLog();

// typedefs
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned __int64 mstime;

// custom fl wstring
#if _MSC_VER == 1200
// vc6
	#define __FUNCSIG__ __FUNCTION__ // not supported
	typedef class std::basic_string<unsigned short,struct ci_wchar_traits> flstr;
	typedef class std::basic_string<char,struct ci_char_traits> flstrs;
#else
// vc7
	typedef class std::basic_string<unsigned short,struct ci_wchar_traits> flstr;
	typedef class std::basic_string<char,struct ci_char_traits> flstrs;
#endif

typedef flstr* (*_CreateWString)(const wchar_t *wszStr);
typedef void (*_FreeWString)(flstr *wscStr);
typedef flstrs* (*_CreateString)(const char *szStr);
typedef void (*_FreeString)(flstrs *scStr);

// structures
struct INISECTIONVALUE
{
	string scKey;
	string scValue;
};

struct MULTIKILLMESSAGE
{
	uint iKillsInARow;
	wstring wscMessage;
};

//wrappers
struct UINT_WRAP
{
	UINT_WRAP(uint u) {val = u;}
	bool operator==(UINT_WRAP uw) { return uw.val==val; }
	bool operator>=(UINT_WRAP uw) { return uw.val>=val; }
	bool operator<=(UINT_WRAP uw) { return uw.val<=val; }
	bool operator>(UINT_WRAP uw) { return uw.val>val; }
	bool operator<(UINT_WRAP uw) { return uw.val<val; }
	uint val;
};
struct INT_WRAP
{
	INT_WRAP(int u) {val = u;}
	bool operator==(INT_WRAP iw) { return iw.val==val; }
	bool operator>=(INT_WRAP iw) { return iw.val>=val; }
	bool operator<=(INT_WRAP iw) { return iw.val<=val; }
	bool operator>(INT_WRAP iw) { return iw.val>val; }
	bool operator<(INT_WRAP iw) { return iw.val<val; }
	int val;
};
struct FLOAT_WRAP
{
	FLOAT_WRAP(float f) {val = f;}
	bool operator==(FLOAT_WRAP fw) { return fw.val==val; }
	bool operator>=(FLOAT_WRAP fw) { return fw.val>=val; }
	bool operator<=(FLOAT_WRAP fw) { return fw.val<=val; }
	bool operator>(FLOAT_WRAP fw) { return fw.val>val; }
	bool operator<(FLOAT_WRAP fw) { return fw.val<val; }
	float val;
};

// functions
bool FLHookInit();
void FLHookInitUnload();
void FLHookUnload();
void ConPrint(const wstring wscText, ...);
void ProcessEvent(const wstring wscText, ...);
void LoadSettings();
void ProcessPendingCommands();

// tools
wstring stows(string scText);
string wstos(wstring wscText);
string itos(int i);
string utos(uint i);
string ftos(float f);
wstring ftows(float f);
string IniGetS(string scFile, string scApp, string scKey, string scDefault);
int IniGetI(string scFile, string scApp, string scKey, int iDefault);
bool IniGetB(string scFile, string scApp, string scKey, bool bDefault);
void IniWrite(string scFile, string scApp, string scKey, string scValue);
void WriteProcMem(void *pAddress, void *pMem, int iSize);
void ReadProcMem(void *pAddress, void *pMem, int iSize);
wstring ToLower(wstring wscStr);
string ToLower(string scStr);
int ToInt(wstring wscStr);
int ToInt(string scStr);
uint ToUint(wstring wscStr);
uint ToUint(string scStr);
void AddLog(const char *szString, ...);
wstring XMLText(wstring wscText);
wstring GetParam(wstring wscLine, wchar_t wcSplitChar, uint iPos);
string GetParam(string scLine, char cSplitChar, uint iPos);
wstring GetParamToEnd(wstring wscLine, wchar_t wcSplitChar, uint iPos);
string GetParamToEnd(string scLine, char cSplitChar, uint iPos);
wstring Trim(wstring wscIn);
string Trim(string scIn);
wstring ReplaceStr(wstring wscSource, wstring wscSearchFor, wstring wscReplaceWith);
void IniDelSection(string scFile, string scApp);
void IniWriteW(string scFile, string scApp, string scKey, wstring wscValue);
wstring IniGetWS(string scFile, string scApp, string scKey, wstring wscDefault);
wstring ToMoneyStr(int iCash);
float IniGetF(string scFile, string scApp, string scKey, float fDefault);
void IniGetSection(string scFile, string scApp, list<INISECTIONVALUE> &lstValues);
float ToFloat(wstring wscStr);
float ToFloat(string scStr);
mstime timeInMS();
void dumpHex(string scFile, void *ptr, uint iNumBytes);
void SwapBytes(void *ptr, uint iLen);
bool CmpStrStart(const string &scSource, const char *scFind);
bool CmpStrStart(const wstring &wscSource, const wchar_t *wscFind);
void AddExceptionInfoLog();

// variables
extern HANDLE hProcFL;
extern HMODULE hModServer;
extern HMODULE hModCommon;
extern HMODULE hModRemoteClient;
extern HMODULE hMe;
extern HMODULE hModDPNet;
extern HMODULE hModDaLib;
extern HMODULE hModContent;
extern _CreateWString CreateWString;
extern _FreeWString FreeWString;
extern _CreateString CreateString;
extern _FreeString FreeString;
extern FILE *fLog;
extern FARPROC fpOldUpdate;

// setting variables
//TODO: clean this section up
extern string set_scCfgFile;
extern string set_scCfgGeneralFile;
extern string set_scCfgCloakFile;
extern string set_scCfgRepairFile;
extern string set_scCfgDockingFile;
extern string set_scCfgDeathFile;
extern string set_scCfgPVPFile;
extern string set_scCfgReputationFile;
extern string set_scCfgItemsFile;
extern string set_scCfgCommandsFile;

extern uint set_iAntiDockKill;
extern list<uint> set_lstNoPVPSystems;
extern list<wstring> set_lstChatSuppress;
extern bool set_bSystemToUniverse;
extern bool set_bSocketActivated;
extern int set_iDebug;
extern wstring set_wscDeathMsgStyle;
extern wstring set_wscDeathMsgStyleSys;
extern bool	set_bDieMsg;
extern bool	set_bDisableCharfileEncryption;
extern uint	set_iAntiBaseIdle;
extern uint	set_iAntiCharMenuIdle;
extern bool set_bChangeCruiseDisruptorBehaviour;
extern bool	set_bUserCmdSetDieMsg;
extern bool	set_bUserCmdSetChatFont;
extern uint	set_iPingKickFrame;
extern uint	set_iPingKick;
extern uint	set_iLossKickFrame;
extern uint	set_iLossKick;
extern bool	set_bUserCmdIgnore;
extern uint	set_iDisableNPCSpawns;
extern int	set_iPort;
extern int	set_iWPort;
extern int	set_iEPort;
extern int	set_iEWPort;
extern BLOWFISH_CTX	*set_BF_CTX;
extern wstring set_wscKickMsg;
extern wstring set_wscUserCmdStyle;
extern wstring set_wscAdminCmdStyle;
extern wstring set_wscUniverseColor;
extern wstring set_wscSystemColor;
extern wstring set_wscPMColor;
extern wstring set_wscGroupColor;
extern wstring set_wscSenderColor;
extern uint	set_iKickMsgPeriod;
extern string set_scHkRestartOpt;
extern uint	set_iAntiF1;
extern uint	set_iUserCmdMaxIgnoreList;
extern uint	set_iReservedSlots;
extern uint	set_iDisconnectDelay;
extern bool	set_bUserCmdAutoBuy;
extern float set_fTorpMissileBaseDamageMultiplier;
extern bool set_MKM_bActivated;
extern wstring set_MKM_wscStyle;
extern list<MULTIKILLMESSAGE> set_MKM_lstMessages;
extern bool	set_bUserCmdSetDieMsgSize;
extern uint	set_iMaxGroupSize;
extern list<wstring> set_lstBans;
extern bool	set_bBanAccountOnMatch;
extern bool set_bSetAutoErrorMargin;
extern float set_fAutoErrorMargin;
extern bool set_bDamageNPCsCollision;
extern bool set_bDropShieldsCloak;
extern float set_fDeathPenaltyKiller;

extern list<INISECTIONVALUE> set_lstCloakDevices;
extern int set_iCloakExpireWarnTime;

struct REPAIR_GUN
{
	REPAIR_GUN(uint p, float d) { damage = d; projectileID = p; }
	float	damage;
	uint	projectileID;
	bool operator==(REPAIR_GUN rg) { return rg.projectileID==projectileID; }
	bool operator>=(REPAIR_GUN rg) { return rg.projectileID>=projectileID; }
	bool operator<=(REPAIR_GUN rg) { return rg.projectileID<=projectileID; }
	bool operator>(REPAIR_GUN rg) { return rg.projectileID>projectileID; }
	bool operator<(REPAIR_GUN rg) { return rg.projectileID<projectileID; }
};
extern BinaryTree<REPAIR_GUN> *set_btRepairGun;

extern BinaryTree<FLOAT_WRAP> *set_btSupressHealth;
extern int set_iRepairBaseTime;
extern float set_fRepairBaseRatio;
extern float set_fRepairBaseMaxHealth;

extern float set_fBaseDockDmg;
extern float set_fSendCashTax;
extern int set_iSendCashTime;
extern int set_iRenameCmdCharge;

extern vector<uint> set_vAffilItems;

struct DOCK_RESTRICTION
{
	DOCK_RESTRICTION(uint obj, uint arch, uint count, wstring denied) { iJumpObjID = obj; iArchID = arch; iCount = count; wscDeniedMsg = denied; }
	DOCK_RESTRICTION(uint obj) { iJumpObjID = obj; }
	uint iJumpObjID;
	uint iArchID;
	int iCount;
	wstring wscDeniedMsg;
	bool operator==(DOCK_RESTRICTION dr) { return dr.iJumpObjID==iJumpObjID; }
	bool operator>=(DOCK_RESTRICTION dr) { return dr.iJumpObjID>=iJumpObjID; }
	bool operator<=(DOCK_RESTRICTION dr) { return dr.iJumpObjID<=iJumpObjID; }
	bool operator>(DOCK_RESTRICTION dr) { return dr.iJumpObjID>iJumpObjID; }
	bool operator<(DOCK_RESTRICTION dr) { return dr.iJumpObjID<iJumpObjID; }
};
extern BinaryTree<DOCK_RESTRICTION> *set_btJRestrict;

struct MOUNT_RESTRICTION
{
	MOUNT_RESTRICTION(uint good, BinaryTree<UINT_WRAP> *shiparchs) { iGoodID = good; btShipArchIDs = shiparchs; }
	MOUNT_RESTRICTION(uint good) { iGoodID = good; }
	uint iGoodID;
	BinaryTree<UINT_WRAP> *btShipArchIDs;
	bool operator==(MOUNT_RESTRICTION mr) { return mr.iGoodID==iGoodID; }
	bool operator>=(MOUNT_RESTRICTION mr) { return mr.iGoodID>=iGoodID; }
	bool operator<=(MOUNT_RESTRICTION mr) { return mr.iGoodID<=iGoodID; }
	bool operator>(MOUNT_RESTRICTION mr) { return mr.iGoodID>iGoodID; }
	bool operator<(MOUNT_RESTRICTION mr) { return mr.iGoodID<iGoodID; }
};
extern BinaryTree<MOUNT_RESTRICTION> *set_btMRestrict;

extern float set_fAutoMarkRadius;
extern vector<uint> set_vNoPvpGoodIDs;
extern vector<uint> set_vNoPvpFactionIDs;

extern bool set_bFlakVersion;

struct MOBILE_SHIP
{
	MOBILE_SHIP(uint shipid, int num_occupants) { iShipID = shipid; iMaxNumOccupants = num_occupants; }
	MOBILE_SHIP(uint shipid) { iShipID = shipid; iMaxNumOccupants = 0; }
	uint iShipID;
	int iMaxNumOccupants;
	bool operator==(MOBILE_SHIP ms) { return ms.iShipID==iShipID; }
	bool operator>=(MOBILE_SHIP ms) { return ms.iShipID>=iShipID; }
	bool operator<=(MOBILE_SHIP ms) { return ms.iShipID<=iShipID; }
	bool operator>(MOBILE_SHIP ms) { return ms.iShipID>iShipID; }
	bool operator<(MOBILE_SHIP ms) { return ms.iShipID<iShipID; }
};
extern BinaryTree<MOBILE_SHIP> *set_btMobDockShipArchIDs;
extern int set_iMobileDockRadius;
extern int set_iMobileDockOffset;

extern bool set_bInviteOnCharChange;

struct FACTION_TAG
{
	uint	iArchID;
	string	scFaction;
	bool	bBaseFaction;
};
extern list<FACTION_TAG> set_lstFactionTags;

extern bool set_bCombineMissileTorpMsgs;
extern uint set_iMaxDeathFactionCauses;
extern uint set_iMaxDeathEquipmentCauses;

extern vector<HINSTANCE> vDLLs;

extern uint	set_iShieldsUpTime;

extern float set_fSpinProtectMass;
extern float set_fSpinImpulseMultiplier;

extern BinaryTree<UINT_WRAP> *set_btOneFactionDeathRep;

extern int set_iTransferCmdCharge;

struct NEW_CHAR_REP
{
	uint iRepGroupID;
	float fRep;
};
extern list<NEW_CHAR_REP> set_lstNewCharRep;

extern float set_fDeathPenalty;
struct DEATH_ITEM
{
	DEATH_ITEM(uint good, float chance) {iGoodID = good; fChance = chance; }
	DEATH_ITEM(uint good) {iGoodID = good; fChance = 0.0f; }
	uint iGoodID;
	float fChance;
	bool operator==(DEATH_ITEM di) { return di.iGoodID==iGoodID; }
	bool operator>=(DEATH_ITEM di) { return di.iGoodID>=iGoodID; }
	bool operator<=(DEATH_ITEM di) { return di.iGoodID<=iGoodID; }
	bool operator>(DEATH_ITEM di) { return di.iGoodID>iGoodID; }
	bool operator<(DEATH_ITEM di) { return di.iGoodID<iGoodID; }
};
extern BinaryTree<DEATH_ITEM> *set_btDeathItems;
extern BinaryTree<UINT_WRAP> *set_btNoDeathPenalty;
extern BinaryTree<UINT_WRAP> *set_btNoDPSystems;

struct EMPATHY_RATE
{
	EMPATHY_RATE(uint group, float rate) { iGroupID = group; fRate = rate; }
	EMPATHY_RATE(uint group) { iGroupID = group; }
	uint iGroupID;
	float fRate;
	bool operator==(EMPATHY_RATE er) { return er.iGroupID==iGroupID; }
	bool operator>=(EMPATHY_RATE er) { return er.iGroupID>=iGroupID; }
	bool operator<=(EMPATHY_RATE er) { return er.iGroupID<=iGroupID; }
	bool operator>(EMPATHY_RATE er) { return er.iGroupID>iGroupID; }
	bool operator<(EMPATHY_RATE er) { return er.iGroupID<iGroupID; }
};
struct REP_CHANGE_EFFECT
{
	REP_CHANGE_EFFECT(uint group, float destruction_event, BinaryTree<EMPATHY_RATE> *empathy_rates) { iGroupID = group; fObjectDestructionEvent = destruction_event; btEmpathyRates = empathy_rates; }
	REP_CHANGE_EFFECT(uint group) { iGroupID = group; fObjectDestructionEvent = 0.0f; btEmpathyRates = new BinaryTree<EMPATHY_RATE>(); }
	//~REP_CHANGE_EFFECT() { delete btEmpathyRates; }
	BinaryTree<EMPATHY_RATE> *btEmpathyRates;
	uint iGroupID;
	float fObjectDestructionEvent;
	bool operator==(REP_CHANGE_EFFECT rce) { return rce.iGroupID==iGroupID; }
	bool operator>=(REP_CHANGE_EFFECT rce) { return rce.iGroupID>=iGroupID; }
	bool operator<=(REP_CHANGE_EFFECT rce) { return rce.iGroupID<=iGroupID; }
	bool operator>(REP_CHANGE_EFFECT rce) { return rce.iGroupID>iGroupID; }
	bool operator<(REP_CHANGE_EFFECT rce) { return rce.iGroupID<iGroupID; }
};
extern BinaryTree<REP_CHANGE_EFFECT> *set_btEmpathy;
extern float set_fMaxRepValue;
extern float set_fMinRepValue;
extern float set_fRepChangePvP;
extern bool set_bChangeRepPvPDeath;

extern int set_iBeamCmd;
extern BinaryTree<UINT_WRAP> *set_btNoTrade;

struct ITEM_RESTRICT
{
	ITEM_RESTRICT(uint good, BinaryTree<UINT_WRAP> *items) { iGoodID = good; btItems = items; }
	ITEM_RESTRICT(uint good) { iGoodID = good; }
	BinaryTree<UINT_WRAP> *btItems;
	uint iGoodID;
	bool operator==(ITEM_RESTRICT ir) { return ir.iGoodID==iGoodID; }
	bool operator>=(ITEM_RESTRICT ir) { return ir.iGoodID>=iGoodID; }
	bool operator<=(ITEM_RESTRICT ir) { return ir.iGoodID<=iGoodID; }
	bool operator>(ITEM_RESTRICT ir) { return ir.iGoodID>iGoodID; }
	bool operator<(ITEM_RESTRICT ir) { return ir.iGoodID<iGoodID; }
};
extern BinaryTree<ITEM_RESTRICT> *set_btItemRestrictions;

extern map<uint, float> set_mapShipRepair;
extern map<uint, float> set_mapItemRepair;

extern set<uint> set_setAutoMount;

extern set<uint> set_setNoSpaceItems;

extern set<uint> set_setAutoMark;

extern set<uint> set_setEquipReDam;

extern map<uint, float> set_mapNPCDeathRep;
extern uint set_iNPCDeathMessages;
extern uint set_iNPCDeathType;

extern float set_fMinTagRep;
extern bool set_bUserCmdTag;

extern uint set_iRespawnDelay;

extern bool set_bMissileDmgToMine;
extern bool set_bTorpDmgToMine;

extern bool ControllerActive;

#endif
