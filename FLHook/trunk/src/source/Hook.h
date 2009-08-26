#ifndef _HOOK_
#define _HOOK_

#include <time.h>
#if _MSC_VER == 1200
#include "xtrace.h" // __FUNCTION__ macro for vc6
#endif
#include "global.h"
#include "flcodec.h"
#include "server.h"
#include "remoteclient.h"
#include "dalib.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

#define HKHKSUCCESS(a) ((a) == HKE_OK)
#define HKSUCCESS(a) ((hkLastErr = (a)) == HKE_OK)
#define HKFAILED(a) !HKSUCCESS(a)

#define SRV_ADDR(a) ((char*)hModServer + a)
#define DALIB_ADDR(a) ((char*)hModDaLib + a)
#define FLSERVER_ADDR(a) ((char*)hProcFL + a)
#define CONTENT_ADDR(a) ((char*)hModContent + a)
#define COMMON_ADDR(a) ((char*)hModCommon + a)
#define ARG_CLIENTID(a) (wstring(L"id ") + stows(itos(a)))


#define ADDR_UPDATE 0x1BAB4
#define ADDR_ANTIDIEMSG 0x39124
#define ADDR_DISCFENCR 0x6E10D
#define ADDR_DISCFENCR2 0x6BFA6
#define ADDR_CRCANTICHEAT 0x6FAF0
#define ADDR_RCSENDCHAT 0x7F30
#define ADDR_CPLIST 0x43D74
#define ADDR_CDPSERVER 0xA284 // 065CA284
#define ADDR_CREATECHAR 0x6B790 // 06D4B790
#define ADDR_FLNEW 0x80012 // 06D60012
#define ADDR_SERVERFLSERVER 0x1BC90 // 0041BC90
#define ADDR_DISABLENPCSPAWNS1 0x5987B // 06EF987B
#define ADDR_DISABLENPCSPAWNS2 0x59CD3 // 06EF9CD3
#define ADDR_DATAPTR 0x277EC // 004277EC
#define ADDR_RC_DISCONNECT 0x93E0 // 06B393E0
#define ADDR_DALIB_DISC_SUPPRESS 0x49C6 // 065C49C6
#define ADDR_SRV_GETCOMMODITIES 0x32EC2 // 06D12EC2
#define ADDR_SRV_MAXGROUPSIZE 0x3A068 // 06D1A068
#define ADDR_SRV_MAXGROUPSIZE2 0x3A46E // 06D1A46E
#define ADDR_SRV_GETIOBJRW 0x20670 // 06D00670
#define ADDR_SRV_GETINSPECT 0x206C0 // 06D006C0
#define ADDR_SRV_ADDCARGODOCKED 0x6EFC0 // 06D4EFC0
#define ADDR_SRV_SPAWN_ITEM 0x28A40
#define ADDR_COMMON_VFTABLE_MINE 0x139C64
#define ADDR_COMMON_VFTABLE_CM 0x139C90
#define ADDR_COMMON_VFTABLE_GUN 0x139C38
#define ADDR_RMCLIENT_LAUNCH 0x5B40
#define ADDR_RMCLIENT_CLIENT 0x43D74
#define ADDR_FLCONFIG 0x25410
#define ADDR_AUTOPILOT_ERROR 0x13E6D0
#define ADDR_UNIVERSECHAT_CHECK 0x399AB

#define LOSS_INTERVALL 4000

#define HK_GET_CLIENTID(a, b) \
	bool bIdString = false; \
	if(b.find(L"id ") == 0) bIdString = true; \
	uint a; \
	if(admin.iClientID && admin.wscCharRestrict.length()) \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					return hkErr; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
				{ \
					if(ToLower(b).find(admin.wscCharRestrict) == -1) \
						return HKE_NAME_NOT_MATCH_RESTRICTION; \
					a = HkGetClientIdFromCharname(b); \
				} \
			} else \
				return hkErr; \
		} \
		else \
		{ \
			if(ToLower(Players.GetActiveCharacterName(a)).find(admin.wscCharRestrict) == -1) \
				return HKE_NAME_NOT_MATCH_RESTRICTION; \
		} \
	} \
	else \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					return hkErr; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
					a = HkGetClientIdFromCharname(b); \
			} else \
				return hkErr; \
		} \
	} \

#define HK_GET_CLIENTID2(a, b) \
	bIdString = false; \
	if(b.find(L"id ") == 0) bIdString = true; \
	uint a; \
	if(admin.iClientID && admin.wscCharRestrict.length()) \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					return hkErr; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
				{ \
					if(ToLower(b).find(admin.wscCharRestrict) == -1) \
						return HKE_NAME_NOT_MATCH_RESTRICTION; \
					a = HkGetClientIdFromCharname(b); \
				} \
			} else \
				return hkErr; \
		} \
		else \
		{ \
			if(ToLower(Players.GetActiveCharacterName(a)).find(admin.wscCharRestrict) == -1) \
				return HKE_NAME_NOT_MATCH_RESTRICTION; \
		} \
	} \
	else \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					return hkErr; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
					a = HkGetClientIdFromCharname(b); \
			} else \
				return hkErr; \
		} \
	} \

#define HK_GET_CLIENTID_NORETURN(a, b) \
	bool bIdString = false; \
	if(b.find(L"id ") == 0) bIdString = true; \
	uint a; \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					a = -1; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
					a = HkGetClientIdFromCharname(b); \
			} else \
				a = -1; \
		} \
	} \

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// typedefs
typedef void (__stdcall *_RCSendChatMsg)(uint iId, uint iTo, uint iSize, void *pRDL);
typedef void (__stdcall *_CRCAntiCheat)();
typedef void (__stdcall *_CreateChar)(const wchar_t *wszName);
typedef void (__cdecl *_GetFLName)(char *szBuf, const wchar_t *wszStr);
typedef bool (__cdecl *_GetShipInspect)(uint &iShip, IObjInspectImpl* &inspect, uint &iDunno);
typedef IObjRW * (__cdecl *_GetIObjRW)(uint iShip);
typedef bool (__stdcall *_AddCargoDocked)(uint iGoodID, CacheString *&hardpoint, int iNumItems, float fHealth, int bMounted, int bMission, uint iOne);

extern _GetShipInspect GetShipInspect;
extern _GetIObjRW GetIObjRW;
extern _AddCargoDocked AddCargoDocked;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enums

enum HK_ERROR
{
	HKE_OK,						
	HKE_PLAYER_NOT_LOGGED_IN,		
	HKE_CHAR_DOES_NOT_EXIST,		
	HKE_COULD_NOT_DECODE_CHARFILE,
	HKE_COULD_NOT_ENCODE_CHARFILE,
	HKE_INVALID_BASENAME,			
	HKE_UNKNOWN_ERROR,			
	HKE_INVALID_CLIENT_ID,		
	HKE_INVALID_GROUP_ID,			
	HKE_INVALID_ID_STRING,		
	HKE_INVALID_SYSTEM,			
	HKE_PLAYER_NOT_IN_SPACE,
	HKE_PLAYER_NOT_DOCKED,
	HKE_PLAYER_NO_ADMIN,			
	HKE_WRONG_XML_SYNTAX,			
	HKE_INVALID_GOOD,				
	HKE_NO_CHAR_SELECTED,			
	HKE_CHARNAME_ALREADY_EXISTS,	
	HKE_CHARNAME_TOO_LONG,		
	HKE_CHARNAME_TOO_SHORT,		
	HKE_AMBIGUOUS_SHORTCUT,		
	HKE_NO_MATCHING_PLAYER,		
	HKE_INVALID_SHORTCUT_STRING,	
	HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID,
	HKE_INVALID_REP_GROUP,
	HKE_COULD_NOT_GET_PATH,
	HKE_NO_TASKKILL,
	HKE_SYSTEM_NO_MATCH,
	HKE_OBJECT_NO_DOCK,
	HKE_CARGO_WONT_FIT,
	HKE_NAME_NOT_MATCH_RESTRICTION,
};

enum DIEMSGTYPE
{
	DIEMSG_ALL = 0,
	DIEMSG_SYSTEM = 1,
	DIEMSG_NONE = 2,
	DIEMSG_SELF = 3,
};

enum CHATSIZE
{
	CS_DEFAULT = 0,
	CS_SMALL = 1,
	CS_BIG = 2,
};

enum CHATSTYLE
{
	CST_DEFAULT = 0,
	CST_BOLD = 1,
	CST_ITALIC = 2,
	CST_UNDERLINE = 3,
};

enum ENGINE_STATE
{
	ES_CRUISE,
	ES_THRUSTER,
	ES_ENGINE,
	ES_KILLED,
	ES_TRADELANE
};

enum EQ_TYPE
{
	ET_GUN,
	ET_TORPEDO,
	ET_CD,
	ET_MISSILE,
	ET_MINE,
	ET_CM,
	ET_OTHER
};

enum DamageCause
{
	DC_NOTHING = 0,
	DC_COLLISION = 1,
	DC_GUN = 2,
	DC_TORPEDO = 5,
	DC_MISSILE = 6,
	DC_MINE = 7,
	DC_UNKNOWN = 19,
	DC_HOOK = 24
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// structs

struct HOOKENTRY
{
	FARPROC fpProc;
	long	dwRemoteAddress;
	FARPROC fpOldProc;
};

struct CARGO_INFO
{
	uint	iID;
	int		iCount;
	uint	iArchID;
	bool	bMission;
	bool	bMounted;
};

// money stuff
struct MONEY_FIX
{
	wstring		wscCharname;
	int			iAmount;

	bool operator==(MONEY_FIX mf1)
	{
		if(!wscCharname.compare(mf1.wscCharname))
			return true;

		return false;
	};
};

// ignore
struct IGNORE_INFO
{
	wstring wscCharname;
	wstring wscFlags;
};

// resolver
struct RESOLVE_IP
{
	uint iClientID;
	uint iConnects;
	wstring wscIP;
	wstring wscHostname;
};

//damage info
struct DAMAGE_INFO
{
	uint iInflictor;
	uint iCause;
	float fDamage;
};

//autobuy
struct AUTOBUY_CARTITEM
{
	uint iArchID;
	uint iCount;
};

//cargo stuff
struct CARGO_FIX
{
	wstring wscCharname;
	uint iGoodID;
	uint iCount;
	bool bMission;
};

//docking restrictions cargo remove
struct CARGO_REMOVE
{
	uint iGoodID;
	int iCount;
};

struct CLIENT_INFO
{
// kill msgs
	uint		iShip;
	uint		iShipOld;
	mstime		tmSpawnTime;

	DamageList	dmgLast;

// money cmd
	list<MONEY_FIX> lstMoneyFix;

// anticheat
	uint		iTradePartner;

// change cruise disruptor behaviour
	bool		bCruiseActivated;
	bool		bThrusterActivated;
	bool		bEngineKilled;
	bool		bTradelane;

// idle kicks
	uint		iBaseEnterTime;
	uint		iCharMenuEnterTime;

// connection data	
	list<uint>	lstLoss;
	uint		iLastLoss;
	uint		iAverageLoss;
	list<uint>	lstPing;
	uint		iAveragePing;

// msg, wait and kick
	mstime		tmKickTime;

// eventmode
	uint		iLastExitedBaseID;
	bool		bDisconnected;

// f1 laming
	bool		bCharSelected;
	mstime		tmF1Time;
	mstime		tmF1TimeDisconnect;
	mstime		tmLastWeaponHit;

// ignore usercommand
	list<IGNORE_INFO> lstIgnore;
	bool bIgnoreUniverse;

// user settings
	DIEMSGTYPE	dieMsg;
	CHATSIZE	dieMsgSize;
	CHATSTYLE	dieMsgStyle;
	CHATSIZE	chatSize;
	CHATSTYLE	chatStyle;

// autobuy
	list<AUTOBUY_CARTITEM> lstAutoBuyItems;

// MultiKillMessages
	uint		iKillsInARow;

// bans
	uint		iConnects; // incremented when player connects

// cloak
	uint		iCloakingTime;
	uint		iCloakWarmup;
	uint		iCloakCooldown;
	uint		iCloakSlot;
	bool		bCanCloak;
	bool		bCloaked;
	bool		bWantsCloak;
	bool		bIsCloaking;
	mstime		tmCloakTime;
	bool		bMustSendUncloak;

//AFK Messages
	wstring		wscAfkMsg;
	list<uint>	iSentIds;

//Marking
	bool		bMarkEverything;
	bool		bIgnoreGroupMark;
	float		fAutoMarkRadius;
	vector<uint> vMarkedObjs; 
	vector<uint> vDelayedSystemMarkedObjs;
	vector<uint> vAutoMarkedObjs; 
	vector<uint> vDelayedAutoMarkedObjs;

// F1 when going through JH/JG thingy
	bool		bInWrapGate;

//No-PvP token
	bool		bNoPvp;

//Shield user commands
	float		fShieldHealth;
   
//Mobile docking
	bool		bMobileDocked;
	uint		iDockClientID;
	list<uint>	lstJumpPath;
	bool		bPathJump;
	bool		bCharInfoReqAfterDeath;
	Vector		Vlaunch;
	Matrix		Mlaunch;
	bool		bMobileBase;
	int			iMaxPlayersDocked;
	list<uint>	lstPlayersDocked;
	uint		iLastSpaceObjDocked;
	uint		iLastEnteredBaseID;

//Death message information
	list<DAMAGE_INFO> lstDmgRec;

//shieldsdown time
	mstime		tmShieldsDownCmd;

//cargo cmd
	list<CARGO_FIX> lstCargoFix;

//Death penalty
	bool bDeathPenaltyOnEnter;
	bool bDisplayDPOnLaunch;
	int iDeathPenaltyCredits;

//Docking restrictions
	bool bCheckedDock;
	list<CARGO_REMOVE> lstRemCargo;

//Mission scripting fix
	uint iControllerID;

// other
	wstring		wscHostname;
	uint		iShipID;
};

// taken from directplay
typedef struct _DPN_CONNECTION_INFO{
    DWORD   dwSize;
    DWORD   dwRoundTripLatencyMS;
    DWORD   dwThroughputBPS;
    DWORD   dwPeakThroughputBPS;
    DWORD   dwBytesSentGuaranteed;
    DWORD   dwPacketsSentGuaranteed;
    DWORD   dwBytesSentNonGuaranteed;
    DWORD   dwPacketsSentNonGuaranteed;
    DWORD   dwBytesRetried;
    DWORD   dwPacketsRetried;
    DWORD   dwBytesDropped;   
    DWORD   dwPacketsDropped; 
    DWORD   dwMessagesTransmittedHighPriority;
    DWORD   dwMessagesTimedOutHighPriority;
    DWORD   dwMessagesTransmittedNormalPriority;
    DWORD   dwMessagesTimedOutNormalPriority;
    DWORD   dwMessagesTransmittedLowPriority;
    DWORD   dwMessagesTimedOutLowPriority;
    DWORD   dwBytesReceivedGuaranteed;
    DWORD   dwPacketsReceivedGuaranteed;
    DWORD   dwBytesReceivedNonGuaranteed;
    DWORD   dwPacketsReceivedNonGuaranteed;
    DWORD   dwMessagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct HKPLAYERINFO
{
	uint iClientID;
	wstring wscCharname;
	wstring wscBase;
	wstring wscSystem;
	uint iSystem;
	uint iShip;
	DPN_CONNECTION_INFO ci;
	wstring wscIP;
	wstring wscHostname;
};

// patch stuff
struct PATCH_INFO_ENTRY
{
	ulong pAddress;
	void *pNewValue;
	uint iSize;
	void *pOldValue;
	bool bAlloced;
};

struct PATCH_INFO
{
	char	*szBinName;
	ulong	pBaseAddress;

	PATCH_INFO_ENTRY piEntries[128];
};

struct DATA_MARKETITEM
{
	uint iArchID;
	float fRep;
};

struct BASE_INFO
{
	uint	iBaseID;
	string	scBasename;
	uint	iObjectID;
	bool	bDestroyed;
	list<DATA_MARKETITEM> lstMarketMisc;
	list<DATA_MARKETITEM> lstMarketCommodities;
};

struct GROUP_MEMBER
{
	uint iClientID;
	wstring wscCharname;
};

//Player in-system beaming
struct LAUNCH_PACKET
{
	uint iShip;
	uint iDunno[2];
	float fRotate[4];
	float fPos[3];
};
class Quaternion
{
public:
	float w,x,y,z;
};

// cID
struct CHARACTER_ID
{
	char charfilename[15];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prototypes
void PrintUniverseText(wstring wscText, ...);

// HkInit
bool InitHookExports();
void UnloadHookExports();
void HookRehashed();
void LoadUserCharSettings(uint iClientID);

// HkFuncTools
uint HkGetClientIdFromAccount(CAccount *acc);
uint HkGetClientIdFromPD(struct PlayerData *pPD);
CAccount* HkGetAccountByCharname(wstring wscCharname);
uint HkGetClientIdFromCharname(wstring wscCharname);
wstring HkGetAccountID(CAccount *acc);
bool HkIsEncoded(string scFilename);
bool HkIsInCharSelectMenu(wstring wscCharname);
bool HkIsInCharSelectMenu(uint iClientID);
bool HkIsValidClientID(uint iClientID);
HK_ERROR HkResolveId(wstring wscCharname, uint &iClientID);
HK_ERROR HkResolveShortCut(wstring wscShortcut, uint &iClientID);
uint HkGetClientIDByShip(uint iShip);
HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir);
HK_ERROR HkGetAccountDirName(wstring wscCharname, wstring &wscDir);
HK_ERROR HkGetCharFileName(wstring wscCharname, wstring &wscFilename);
wstring HkGetBaseNickByID(uint iBaseID);
string HkGetBaseNickSByID(uint iBaseID);
wstring HkGetPlayerSystem(uint iClientID);
string HkGetSystemNickSByID(uint iSystemID);
wstring HkGetSystemNickByID(uint iSystemID);
string HkGetPlayerSystemS(uint iClientID);
void HkLockAccountAccess(CAccount *acc, bool bKick);
void HkUnlockAccountAccess(CAccount *acc);
void HkGetItemsForSale(uint iBaseID, list<uint> &lstItems);
IObjInspectImpl* HkGetInspect(uint iClientID);
CEquipManager* HkGetEquipMan(uint iShip);
ENGINE_STATE HkGetEngineState(uint iClientID);
HK_ERROR HkGetGoodIDFromSID(uint iClientID, ushort sGoodID, uint &iGoodID);
HK_ERROR HkGetSIDFromGoodID(uint iClientID, ushort &sGoodID, uint iGoodID);
HK_ERROR HkRestartServer();
HK_ERROR HkShutdownServer();
float HkDistance3D(Vector v1, Vector v2);
float HkGetAdjustedDistance(uint iShip1, uint iShip2);
Quaternion HkMatrixToQuaternion(Matrix m);
uint HkGetSpaceObjFromBaseID(uint iBaseID);
string HkGetStringFromIDS(uint iIDS);
wstring HkGetWStringFromIDS(uint iIDS);
CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW);

// HkFuncMsg
HK_ERROR HkMsg(int iClientID, wstring wscMessage);
HK_ERROR HkMsg(wstring wscCharname, wstring wscMessage);
HK_ERROR HkMsgS(wstring wscSystemname, wstring wscMessage);
HK_ERROR HkMsgU(wstring wscMessage);
HK_ERROR HkMsgU(wstring wscSender, wstring wscMessage);
HK_ERROR HkMsgU(wstring wscSender, wstring wscMessage, wstring wscFont);
HK_ERROR HkFMsgEncodeXML(wstring wscXML, char *szBuf, uint iSize, uint &iRet);
HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize);
HK_ERROR HkFMsg(uint iClientID, wstring wscXML);
HK_ERROR HkFMsg(wstring wscCharname, wstring wscXML);
HK_ERROR HkFMsgS(wstring wscSystemname, wstring wscXML);
HK_ERROR HkFMsgU(wstring wscXML);

// HkFuncPlayers
HK_ERROR HkGetCash(wstring wscCharname, int &iCash);
HK_ERROR HkAddCash(wstring wscCharname, int iAmount);
HK_ERROR HkKick(CAccount *acc);
HK_ERROR HkKick(wstring wscCharname);
HK_ERROR HkKickReason(wstring wscCharname, wstring wscReason);
HK_ERROR HkBan(wstring wscCharname, bool bBan);
HK_ERROR HkBeam(wstring wscCharname, wstring wscBasename);
HK_ERROR HkBeamInSys(wstring wscCharname, Vector vOffsetVector, Matrix mOrientation);
HK_ERROR HkInstantDock(wstring wscCharname, uint iDockObj);
extern vector< list<CARGO_INFO> > vRestoreKBeamCargo;
extern vector<uint> vRestoreKBeamClientIDs;
extern bool bSupressDeathMsg;
HK_ERROR HkKillBeam(wstring wscCharname, wstring wscBasename);
HK_ERROR HkSaveChar(wstring wscCharname);
HK_ERROR HkEnumCargo(wstring wscCharname, list<CARGO_INFO> &lstCargo, int *iRemainingHoldSize);
HK_ERROR HkRemoveCargo(wstring wscCharname, uint iID, int iCount);
HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, int iCount, bool bMission);
HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, CacheString *hardpoint, bool bMission);
HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, int iCount, bool bMission);
HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, wstring wscHardpoint, bool bMission);
HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, string scHardpoint, bool bMission);
HK_ERROR HkRename(wstring wscCharname, wstring wscNewCharname, bool bOnlyDelete);
HK_ERROR HkMsgAndKick(uint iClientID, wstring wscReason, uint iIntervall);
HK_ERROR HkKill(wstring wscCharname);
void HkPenalizeDeath(wstring wscCharname, uint iKillerID, bool bSecondTime);
list<wstring> HkGetDeathPenaltyItems(uint iClientID);
HK_ERROR HkGetReservedSlot(wstring wscCharname, bool &bResult);
HK_ERROR HkSetReservedSlot(wstring wscCharname, bool bReservedSlot);
void HkPlayerAutoBuy(uint iClientID, uint iBaseID);
HK_ERROR HkResetRep(wstring wscCharname);
HK_ERROR HkGetGroupMembers(wstring wscCharname, list<GROUP_MEMBER> &lstMembers);
HK_ERROR HkGetGroupID(wstring wscCharname, uint &iGroupID);
HK_ERROR HkAddToGroup(wstring wscCharname, uint iGroupID);
HK_ERROR HkInviteToGroup(wstring wscCharname, wstring wscCharFrom);
HK_ERROR HkRemoveFromGroup(wstring wscCharname, uint iGroupID);
HK_ERROR HkSendGroupCash(uint iGroupID, int iAmount);
HK_ERROR HkSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
HK_ERROR HkSetRepRelative(wstring wscCharname, wstring wscRepGroup, float fValue, bool bUseDestruction);
HK_ERROR HkSetRepRelative(wstring wscCharname, uint iRepGroupID, float fValue, bool bUseDestruction);
HK_ERROR HkGetRep(wstring wscCharname, wstring wscRepGroup, float &fValue);
HK_ERROR HkSetAffiliation(wstring wscCharname, wstring wscRepGroup);
HK_ERROR HkGetAffiliation(wstring wscCharname, wstring &wscRepGroup);
HK_ERROR HkReadCharFile(wstring wscCharname, list<wstring> &lstOutput);
HK_ERROR HkWriteCharFile(wstring wscCharname, wstring wscData);
HK_ERROR HkGetTimePlayed(wstring wscCharname, uint &iSeconds);
HK_ERROR HkCloak(uint iClientID);
HK_ERROR HkUnCloak(uint iClientID);
HK_ERROR HkInitCloakSettings(uint iClientID);
char HkMarkObject(uint iClientID, uint iObject);
void HkUnMarkObject(uint iClientID, uint iObject);
void HkUnMarkAllObjects(uint iClientID);
bool HkIsOkayToDock(uint iClientID, uint iTargetClientID);
void HkNewShipBought(uint iClientID);

// HkFuncLog
void HkHandleCheater(uint iClientID, bool bBan, wstring wscReason, ...);
bool HkAddCheaterLog(wstring wscCharname, wstring wscReason);
bool HkAddKickLog(uint iClientID, wstring wscReason, ...);
bool HkAddConnectLog(uint iClientID);

// HkFuncOther
void HkGetPlayerIP(uint iClientID, wstring &wscIP);
HK_ERROR HkGetPlayerInfo(wstring wscCharname, HKPLAYERINFO &pi, bool bAlsoCharmenu);
list<HKPLAYERINFO> HkGetPlayers();
HK_ERROR HkGetConnectionStats(uint iClientID, DPN_CONNECTION_INFO &ci);
HK_ERROR HkSetAdmin(wstring wscCharname, wstring wscRights);
HK_ERROR HkSetAdminRestriction(wstring wscCharname, wstring wscRestriction);
HK_ERROR HkGetAdminRestriction(wstring wscCharname, wstring &wscRestriction);
HK_ERROR HkGetAdmin(wstring wscCharname, wstring &wscRights);
HK_ERROR HkDelAdmin(wstring wscCharname);
HK_ERROR HkChangeNPCSpawn(bool bDisable);
HK_ERROR HkGetBaseStatus(wstring wscBasename, float &fHealth, float &fMaxHealth);
HK_ERROR HkFindFactionMagnets(list<FACTION_TAG> &tags);
void HkSetShieldHealth(uint iSpaceObj, DamageCause cause, float fHealth);
void HkSetShieldHealth(uint iSpaceObj, uint iSpaceObjGiver, DamageCause cause, float fHealth);
void HkSetShieldHealth(uint iSpaceObj, uint iClientIDGiver, uint iSpaceObjGiver, DamageCause cause, float fHealth);
void HkSetHullHealth(uint iSpaceObj, DamageCause cause, float fHealth);
void HkSetHullHealth(uint iSpaceObj, uint iSpaceObjGiver, DamageCause cause, float fHealth);
void HkSetHullHealth(uint iSpaceObj, uint iClientIDGiver, uint iSpaceObjGiver, DamageCause cause, float fHealth);
void HkSetXHealth(uint iSpaceObj, ushort iSubObjID, DamageCause cause, float fHealth);
void HkSetXHealth(uint iSpaceObj, uint iSpaceObjGiver, ushort iSubObjID, DamageCause cause, float fHealth);
void HkSetXHealth(uint iSpaceObj, uint iClientIDGiver, uint iSpaceObjGiver, ushort iSubObjID, DamageCause cause, float fHealth);
void HkTest(int iArg, int iArg2, int iArg3);

// HkFLIni
HK_ERROR HkFLIniGet(wstring wscCharname, wstring wscKey, wstring &wscRet);
HK_ERROR HkFLIniWrite(wstring wscCharname, wstring wscKey, wstring wscValue);

wstring HkErrGetText(HK_ERROR hkErr);
void ClearClientInfo(uint iClientID);
void LoadUserSettings(uint iClientID);

// HkCbUserCmd
bool UserCmd_Process(uint iClientID, wstring wscCmd);
void UserCmd_SetDieMsg(uint iClientID, wstring wscParam);
void UserCmd_SetChatFont(uint iClientID, wstring wscParam);
void PrintUserCmdText(uint iClientID, wstring wscText, ...);

//HkOther
extern bool g_bInPlayerLaunch;
extern uint g_iClientID;
extern Vector g_Vlaunch;
extern Matrix g_Mlaunch;
void _HkCb_LaunchPos();
int __cdecl SpaceObjCreate(unsigned int & iShip, struct pub::SpaceObj::ShipInfo const & shipinfo);
extern vector<uint> vMarkSpaceObjProc;
int __cdecl SpaceObjDock(unsigned int const &iShip, unsigned int const & iDockTarget, int iDunno, enum DOCK_HOST_RESPONSE response);
void _HkCb_SpawnItem();
void _SpawnItemOrig();
//int __cdecl SolarSpaceObjCreate(unsigned int & iShip, struct pub::SpaceObj::SolarInfo const & solarinfo);
struct DOCK_INFO
{
	uint iShip;
	uint iDockTarget;
	DOCK_HOST_RESPONSE dockResponse;
};
extern vector<DOCK_INFO> vDelayedNPCDocks;
//int __cdecl SpaceObjLightFuse(unsigned int const &iShip ,char const *sz ,float fDunno);
//int __cdecl SpaceObjDestroy(unsigned int iShip,enum DestroyType dType);
/*struct NPC_JUMP
{
	uint iShip;
	uint iJumpGate;
	uint iSystemID;
	uint iNumTimes;
};*/
unsigned int __cdecl ControllerCreate(char const *szDunno1, char const *szDunno2, struct pub::Controller::CreateParms const *dunno, enum pub::Controller::PRIORITY iDunno);
int __cdecl ControllerSend(unsigned int const &iControllerID, int iDunno, void const *ReallyDunno);
void __cdecl ControllerDestroy(unsigned int iControllerID);

// HkDeath
void ShipDestroyedHook();
void BaseDestroyed(uint iObject, uint iClientIDBy);
extern wstring wscAdminKiller;

// HkDamage
void _HookMissileTorpHit();
void _HkCb_AddDmgEntry();
void _HkCb_GeneralDmg();
bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget);
void _HkCb_NonGunWeaponHitsBase();
extern FARPROC fpOldNonGunWeaponHitsBase;
extern bool g_gNonGunHitsBase;
//for modifying health //The alignment was not planned, but happened nonetheless
extern bool bTakeOverDmgEntry;
extern float fTakeOverHealth;
extern ushort iTakeOverSubObj;
extern uint iTakeOverClientID;
extern uint iTakeOverSpaceObj;
extern DamageCause dcTakeOver;

//HkCbIServerImpl
extern float g_fRepairMaxHP;
extern float g_fRepairBeforeHP;
extern float g_fRepairDamage;
extern uint g_iRepairShip;
extern bool g_bRepairPendHit;
struct SOLAR_REPAIR
{
	SOLAR_REPAIR(uint oID, int time) {iObjectID = oID; iTimeAfterDestroyed = time; }
	bool operator==(SOLAR_REPAIR sr) { return sr.iObjectID==iObjectID; }
	bool operator>=(SOLAR_REPAIR sr) { return sr.iObjectID>=iObjectID; }
	bool operator<=(SOLAR_REPAIR sr) { return sr.iObjectID<=iObjectID; }
	bool operator>(SOLAR_REPAIR sr) { return sr.iObjectID>iObjectID; }
	bool operator<(SOLAR_REPAIR sr) { return sr.iObjectID<iObjectID; }
	uint	iObjectID;
	int		iTimeAfterDestroyed;  //in seconds
};
extern BinaryTree<SOLAR_REPAIR> *btSolarList;
struct MOB_UNDOCKBASEKILL
{
	uint iClientID;
	uint iBaseID;
	bool bKill;
};
extern list<MOB_UNDOCKBASEKILL> lstUndockKill;

// HkCbCallbacks
void _SendMessageHook();
void __stdcall HkCb_SendChat(uint iId, uint iTo, uint iSize, void *pRDL);

// HkCbDisconnect
void _DisconnectPacketSent();
extern FARPROC fpOldDiscPacketSent;

// HkTimers
void HkTimerUpdatePingData();
void HkTimerUpdateLossData();
void HkTimerCheckKick();
void HkTimerNPCAndF1Check();
void HkTimerCheckIfBaseDestroyed();
void HkTimerSolarRepair();
void HkThreadResolver();
void HkTimerCheckResolveResults();
void HkTimerCloakHandler();
void HkTimerSpaceObjMark();
struct DELAY_MARK { uint iObj; mstime time; };
extern list<DELAY_MARK> g_lstDelayedMarks;
void HkTimerMarkDelay();
void HkTimerNPCDockHandler();
void HkTimerBeamDelayHandler();
void HkTimerRepairShip();

extern list<BASE_INFO> lstBases;
extern CRITICAL_SECTION csIPResolve;
extern list<RESOLVE_IP> g_lstResolveIPs;
extern list<RESOLVE_IP> g_lstResolveIPsResult;
extern HANDLE hThreadResolver;
struct SHIP_REPAIR { uint iObjID; float fIncreaseHealth; };
extern list<SHIP_REPAIR> g_lstRepairShips;

// namespaces
namespace HkIServerImpl 
{
void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1, void const *rdlReader, struct CHAT_ID cIdTo, int iP2);
int __stdcall Update(void);
void __stdcall RequestEvent(int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned long p5, unsigned int p6);
void __stdcall BaseExit(unsigned int iBaseID, unsigned int iClientID);
extern bool g_bInSubmitChat;
extern uint g_iTextLen;
extern bool g_bChatAction;
extern HOOKENTRY hookEntries[91];
}

// HkDataBaseMarket
bool HkLoadMiscBaseMarket();
bool HkLoadCommoditiesBaseMarket();

//HkDataBaseOther
void HkLoadNewCharacter();
void HkLoadEmpathy(const char *szFLConfigFile);
void HkLoadDLLConf(const char *szFLConfigFile);

// imported base pointers
IMPORT PlayerDB Players;
extern "C"  IMPORT IServerImpl Server;
// variables
extern uint	iDmgTo;

extern bool g_bMsg;
extern bool g_bMsgS;
extern bool g_bMsgU;

extern FARPROC fpOldShipDestroyed;
extern FARPROC fpOldMissileTorpHit;
extern FARPROC fpOldGeneralDmg;
extern FARPROC fpOldLaunchPos;

extern CDPClientProxy **g_cClientProxyArray;
extern void *pClient;

extern _RCSendChatMsg RCSendChatMsg;
extern _CRCAntiCheat CRCAntiCheat;
extern _CreateChar CreateChar;

extern string scAcctPath;

extern CLIENT_INFO ClientInfo[201];
extern CDPServer *cdpSrv;
extern uint g_iServerLoad;
extern bool g_bNPCDisabled;
extern bool g_bNPCForceDisabled;
extern char *g_FLServerDataPtr;

extern map<uint, map<Archetype::AClassType, char> > set_mapSysItemRestrictions;

#endif