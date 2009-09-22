#include "hook.h"

Archetype::AClassType TypeStrToEnum(string scType);
uint TypeStrToObjUint(string scType);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

// Config files
#define getCfgFile(setVar, key) \
		setVar = IniGetS(set_scCfgFile, "Data", key, ""); \
	if(!setVar.length()) \
		setVar = set_scCfgFile; \
	else \
		setVar = scDataPath + setVar; 
string			set_scCfgFile;
string			set_scCfgGeneralFile;
string			set_scCfgCloakFile;
string			set_scCfgCommandsFile;
string			set_scCfgDeathFile;
string			set_scCfgDockingFile;
string			set_scCfgItemsFile;
string			set_scCfgPVPFile;
string			set_scCfgRepairFile;
string			set_scCfgReputationFile;

// General
uint			set_iAntiDockKill;
int				set_iDebug = 0;
bool			set_bDieMsg;
bool			set_bDisableCharfileEncryption;
bool			set_bChangeCruiseDisruptorBehaviour;
uint			set_iAntiF1;
uint			set_iDisconnectDelay;
uint			set_iReservedSlots;
float			set_fTorpMissileBaseDamageMultiplier;
uint			set_iMaxGroupSize;

string			set_scHkRestartOpt;
float 			set_fBaseDockDmg;
float 			set_fSendCashTax;
int				set_iSendCashTime;
int 			set_iRenameCmdCharge;
float			set_fAutoMarkRadius;
bool			set_bFlakVersion;
bool			set_bInviteOnCharChange;
uint			set_iShieldsUpTime;
float			set_fSpinProtectMass;
float			set_fSpinImpulseMultiplier;
int				set_iTransferCmdCharge;
float			set_fMaxRepValue;
float			set_fMinRepValue;
int				set_iBeamCmd;
bool			set_bSetAutoErrorMargin;
float			set_fAutoErrorMargin;
bool			set_bDamageNPCsCollision;
bool			set_bDropShieldsCloak;

// Kick
uint			set_iAntiBaseIdle;
uint			set_iAntiCharMenuIdle;
uint			set_iPingKickFrame;
uint			set_iPingKick;
uint			set_iLossKickFrame;
uint			set_iLossKick;
uint			set_iDisableNPCSpawns;

// Style
wstring			set_wscDeathMsgStyle;
wstring			set_wscDeathMsgStyleSys;
uint			set_iKickMsgPeriod;
wstring			set_wscKickMsg;
wstring			set_wscUserCmdStyle;
wstring			set_wscAdminCmdStyle;
wstring			set_wscUniverseColor;
wstring			set_wscSystemColor;
wstring			set_wscPMColor;
wstring			set_wscGroupColor;
wstring			set_wscSenderColor;

// Socket
bool			set_bSocketActivated;
int				set_iPort;
int				set_iWPort;
int				set_iEPort;
int				set_iEWPort;
BLOWFISH_CTX	*set_BF_CTX = 0;

// UserCommands
bool			set_bUserCmdSetDieMsg;
bool			set_bUserCmdSetDieMsgSize;
bool			set_bUserCmdSetChatFont;
bool			set_bUserCmdIgnore;
uint			set_iUserCmdMaxIgnoreList;
bool			set_bAutoBuy;

// Death
bool			set_bChangeRepPvPDeath;
bool			set_bCombineMissileTorpMsgs;
float			set_fDeathPenalty;
uint			set_iMaxDeathFactionCauses;
uint			set_iMaxDeathEquipmentCauses;
float			set_fRepChangePvP;
float			set_fDeathPenaltyKiller;

// NoPVP
list<uint>		set_lstNoPVPSystems;

// Chat
bool			set_bSystemToUniverse;
list<wstring>	set_lstChatSuppress;

// MultiKillMessages
bool			set_MKM_bActivated;
wstring			set_MKM_wscStyle;
list<MULTIKILLMESSAGE> set_MKM_lstMessages;

// bans
bool			set_bBanAccountOnMatch;
list<wstring>	set_lstBans;

// cloak
list<INISECTIONVALUE> set_lstCloakDevices;
int				set_iCloakExpireWarnTime;

//repair gun
BinaryTree<REPAIR_GUN> *set_btRepairGun = new BinaryTree<REPAIR_GUN>();

//Alternate repair of STATION Solars
BinaryTree<FLOAT_WRAP> *set_btSupressHealth = new BinaryTree<FLOAT_WRAP>();
int				set_iRepairBaseTime;
float 			set_fRepairBaseRatio;
float			set_fRepairBaseMaxHealth;

//Items that change the affiliation of the player to that of the base when bought
vector<uint>	set_vAffilItems;

//JH/JG restriction by equipment
BinaryTree<DOCK_RESTRICTION> *set_btJRestrict = new BinaryTree<DOCK_RESTRICTION>();

//Internal Equipment mount restriction by shiparch
BinaryTree<MOUNT_RESTRICTION> *set_btMRestrict = new BinaryTree<MOUNT_RESTRICTION>();


//No-PvP Tokens
vector<uint>	set_vNoPvpGoodIDs;
vector<uint>	set_vNoPvpFactionIDs;

//Mobile docking
BinaryTree<MOBILE_SHIP> *set_btMobDockShipArchIDs = new BinaryTree<MOBILE_SHIP>();
int				set_iMobileDockRadius;
int				set_iMobileDockOffset;

list<FACTION_TAG> set_lstFactionTags;

//DLL Resources
vector<HINSTANCE> vDLLs;

//Death Message singular factions
BinaryTree<UINT_WRAP> *set_btOneFactionDeathRep = new BinaryTree<UINT_WRAP>();

//New character reputations
list<NEW_CHAR_REP> set_lstNewCharRep;

//Death penalty stuff
BinaryTree<DEATH_ITEM> *set_btDeathItems = new BinaryTree<DEATH_ITEM>(); //Items to take for death penalty
BinaryTree<UINT_WRAP> *set_btNoDeathPenalty = new BinaryTree<UINT_WRAP>(); //Ships to not apply the death penalty to
BinaryTree<UINT_WRAP> *set_btNoDPSystems = new BinaryTree<UINT_WRAP>(); //Systems to not apply the death penalty in

//RepChangeEffects
BinaryTree<REP_CHANGE_EFFECT> *set_btEmpathy = new BinaryTree<REP_CHANGE_EFFECT>();

//Items that cannot be traded
BinaryTree<UINT_WRAP> *set_btNoTrade = new BinaryTree<UINT_WRAP>();

//Items that cannot be bought when other item is in hold
BinaryTree<ITEM_RESTRICT> *set_btItemRestrictions = new BinaryTree<ITEM_RESTRICT>();

//Ship repair over time
map<uint, float> set_mapShipRepair;
map<uint, float> set_mapItemRepair;

//Items to automount upon ship purchase
set<uint> set_setAutoMount;

//Items to not let exist in space
set<uint> set_setNoSpaceItems;

//Items to automatically mark when spawned
set<uint> set_setAutoMark;

//Equipment to redirect damage to hull
set<uint> set_setEquipReDam;

//Systems to prevent dropping/trading items in
map<uint, map<Archetype::AClassType, char> > set_mapSysItemRestrictions;

//Solar death reputation adjustment
map<uint, float> set_mapNPCDeathRep;
uint set_iNPCDeathMessages;
uint set_iNPCDeathType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	try {
	// init cfg filename
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);
		set_scCfgFile = string(szCurDir) + "\\FLHook.ini";
		char *szFLConfig = (char*)((char*)GetModuleHandle(0) + ADDR_FLCONFIG);

	// Config file names
		string scDataPath = IniGetS(set_scCfgFile, "Data", "Path", "");
		if(scDataPath.length() && scDataPath[scDataPath.length()-1] != '\\')
			scDataPath += '\\';
		getCfgFile(set_scCfgGeneralFile, "General");
		getCfgFile(set_scCfgCloakFile, "Cloak");
		getCfgFile(set_scCfgRepairFile, "Repair");
		getCfgFile(set_scCfgDockingFile, "Docking");
		getCfgFile(set_scCfgDeathFile, "Death");
		getCfgFile(set_scCfgPVPFile, "PVP");
		getCfgFile(set_scCfgReputationFile, "Reputation");
		getCfgFile(set_scCfgItemsFile, "Items");
		getCfgFile(set_scCfgCommandsFile, "Commands");

		uint i;
		list<INISECTIONVALUE> lstValues;

	//General
		// General
		set_iDebug = IniGetI(set_scCfgGeneralFile, "General", "Debug", 0);
		set_bDieMsg = IniGetB(set_scCfgGeneralFile, "General", "EnableDieMsg", true);
		set_bDisableCharfileEncryption = IniGetB(set_scCfgGeneralFile, "General", "DisableCharfileEncryption", false);
		set_bChangeCruiseDisruptorBehaviour = IniGetB(set_scCfgGeneralFile, "General", "ChangeCruiseDisruptorBehaviour", false);
		set_iDisableNPCSpawns = IniGetI(set_scCfgGeneralFile, "General", "DisableNPCSpawns", 0);
		set_iAntiF1 = IniGetI(set_scCfgGeneralFile, "General", "AntiF1", 0);
		set_iDisconnectDelay = IniGetI(set_scCfgGeneralFile, "General", "DisconnectDelay", 0);
		set_iReservedSlots = IniGetI(set_scCfgGeneralFile, "General", "ReservedSlots", 0);
		set_fTorpMissileBaseDamageMultiplier = IniGetF(set_scCfgGeneralFile, "General", "TorpMissileBaseDamageMultiplier", 1.0f);
		set_iMaxGroupSize = IniGetI(set_scCfgGeneralFile, "General", "MaxGroupSize", 0);
		//New stuff in General
		set_bFlakVersion = IniGetB(set_scCfgGeneralFile, "General", "Is88Flak", false);
		set_bInviteOnCharChange = IniGetB(set_scCfgGeneralFile, "General", "AddOnCharChange", false);
		set_fSpinProtectMass = IniGetF(set_scCfgGeneralFile, "General", "SpinProtectionMass", -1.0f);
		set_fSpinImpulseMultiplier = IniGetF(set_scCfgGeneralFile, "General", "SpinProtectionMultiplier", -8.0f);
		set_bSetAutoErrorMargin = IniGetB(set_scCfgGeneralFile, "General", "AutopilotErrorSet", false);
		set_fAutoErrorMargin = IniGetF(set_scCfgGeneralFile, "General", "AutopilotError", -1.0f);
		set_bDamageNPCsCollision = IniGetB(set_scCfgGeneralFile, "General", "CollisionDamageNPCs", false);
		string scMarketData = IniGetS(set_scCfgGeneralFile, "General", "MarketData", "");
		if(scMarketData.length())
			BaseDataList_load_market_data(scMarketData.c_str());
		// Kick
		set_iAntiBaseIdle = IniGetI(set_scCfgGeneralFile, "Kick", "AntiBaseIdle", 0);
		set_iAntiCharMenuIdle = IniGetI(set_scCfgGeneralFile, "Kick", "AntiCharMenuIdle", 0);
		set_iPingKickFrame = IniGetI(set_scCfgGeneralFile, "Kick", "PingKickFrame", 30);
		if(!set_iPingKickFrame) 
			set_iPingKickFrame = 60; // might lead to problems else
		set_iPingKick = IniGetI(set_scCfgGeneralFile, "Kick", "PingKick", 0);
		set_iLossKickFrame = IniGetI(set_scCfgGeneralFile, "Kick", "LossKickFrame", 30);
		if(!set_iLossKickFrame) 
			set_iLossKickFrame = 60; // might lead to problems else
		set_iLossKick = IniGetI(set_scCfgGeneralFile, "Kick", "LossKick", 0);
		// Style
		set_wscKickMsg = stows(IniGetS(set_scCfgGeneralFile, "Style", "KickMsg", "<TRA data=\"0x0000FF10\" mask=\"-1\"/><TEXT>You will be kicked. Reason: %s</TEXT>"));
		set_iKickMsgPeriod = IniGetI(set_scCfgGeneralFile, "Style", "KickMsgPeriod", 5000);
		set_wscUserCmdStyle = stows(IniGetS(set_scCfgGeneralFile, "Style", "UserCmdStyle", "0x00FF0090"));
		set_wscAdminCmdStyle = stows(IniGetS(set_scCfgGeneralFile, "Style", "AdminCmdStyle", "0x00FF0090"));
		set_wscUniverseColor = stows(IniGetS(set_scCfgGeneralFile, "Style", "UniverseColor", "FFFFFF"));
		set_wscSystemColor = stows(IniGetS(set_scCfgGeneralFile, "Style", "SystemColor", "E6C684"));
		set_wscPMColor = stows(IniGetS(set_scCfgGeneralFile, "Style", "PMColor", "19BD3A"));
		set_wscGroupColor = stows(IniGetS(set_scCfgGeneralFile, "Style", "GroupColor", "FF7BFF"));
		set_wscSenderColor = stows(IniGetS(set_scCfgGeneralFile, "Style", "SenderColor", "FFFFFF"));
		// Socket
		set_bSocketActivated = IniGetB(set_scCfgGeneralFile, "Socket", "Activated", false);
		set_iPort = IniGetI(set_scCfgGeneralFile, "Socket", "Port", 0);
		set_iWPort = IniGetI(set_scCfgGeneralFile, "Socket", "WPort", 0);
		set_iEPort = IniGetI(set_scCfgGeneralFile, "Socket", "EPort", 0);
		set_iEWPort = IniGetI(set_scCfgGeneralFile, "Socket", "EWPort", 0);
		string scEncryptKey = IniGetS(set_scCfgGeneralFile, "Socket", "Key", "");
		if(scEncryptKey.length())
		{
			if(!set_BF_CTX)
				set_BF_CTX = (BLOWFISH_CTX*)malloc(sizeof(BLOWFISH_CTX));
			Blowfish_Init(set_BF_CTX, (unsigned char *)scEncryptKey.data(), scEncryptKey.length());
		}
		// read chat suppress
		set_lstChatSuppress.clear();
		for(i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "Suppress%u", i);
			string scSuppress = IniGetS(set_scCfgGeneralFile, "Chat", szBuf, "");

			if(!scSuppress.length())
				break;

			set_lstChatSuppress.push_back(stows(scSuppress));
		}
		// Set system chat to go to universe?
		set_bSystemToUniverse = IniGetB(set_scCfgGeneralFile, "Chat", "SystemToUniverse", false);
		void *chatCheckAddr = (void*)((char*)hModServer + ADDR_UNIVERSECHAT_CHECK);
		if(set_bSystemToUniverse)
		{ //Overwrite authorization check
			char szOverwrite = '\xEB';
			WriteProcMem(chatCheckAddr, (void*)&szOverwrite, 1);
		}
		else
		{
			char szOverwrite = '\x75';
			WriteProcMem(chatCheckAddr, (void*)&szOverwrite, 1);
		}
		// bans
		set_bBanAccountOnMatch = IniGetB(set_scCfgGeneralFile, "Bans", "BanAccountOnMatch", false);
		set_lstBans.clear();
		IniGetSection(set_scCfgFile, "Bans", lstValues);
		lstValues.pop_front();
		foreach(lstValues, INISECTIONVALUE, itisv)
			set_lstBans.push_back(stows(itisv->scKey));		
	
	//Misc settings
		//Faction tag affiliation change init
		if(set_bFlakVersion)
			HkFindFactionMagnets(set_lstFactionTags);
		// load resource DLLs
		HkLoadDLLConf(szFLConfig);
		//New character reputations
		HkLoadNewCharacter();
		//RepChangeEffects
		HkLoadEmpathy(szFLConfig);

	//Cloak
		//General
		set_bDropShieldsCloak = IniGetB(set_scCfgCloakFile, "General", "DropShieldsCloak", false);
		set_iCloakExpireWarnTime = IniGetI(set_scCfgCloakFile, "General", "CloakExpireWarnTime", 5000);
		//Devices
		IniGetSection(set_scCfgCloakFile, "CloakDevices", set_lstCloakDevices);

	//Commands
		//General
		set_fAutoMarkRadius = IniGetF(set_scCfgCommandsFile, "General", "AutoMarkRadius", 0.0f)*1000;
		set_iBeamCmd = IniGetI(set_scCfgCommandsFile, "General", "BeamCmdBehavior", 0);
		set_iRenameCmdCharge = IniGetI(set_scCfgCommandsFile, "General", "RenameCmdCharge", 500000);
		set_scHkRestartOpt = IniGetS(set_scCfgCommandsFile, "General", "RestartServerOpt", "");
		set_fSendCashTax = IniGetF(set_scCfgCommandsFile, "General", "SendcashTax", 5.0f);
		set_iSendCashTime = IniGetI(set_scCfgCommandsFile, "General", "SendcashTime", 0);
		set_iShieldsUpTime = IniGetI(set_scCfgCommandsFile, "General", "ShieldsUpTime", 15);
		set_iTransferCmdCharge = IniGetI(set_scCfgCommandsFile, "General", "TransferCmdCharge", 150000);
		// UserCommands
		set_bUserCmdSetDieMsg = IniGetB(set_scCfgCommandsFile, "UserCommands", "SetDieMsg", false);
		set_bUserCmdSetDieMsgSize = IniGetB(set_scCfgCommandsFile, "UserCommands", "SetDieMsgSize", false);
		set_bUserCmdSetChatFont = IniGetB(set_scCfgCommandsFile, "UserCommands", "SetChatFont", false);
		set_bUserCmdIgnore = IniGetB(set_scCfgCommandsFile, "UserCommands", "Ignore", false);
		set_iUserCmdMaxIgnoreList = IniGetI(set_scCfgCommandsFile, "UserCommands", "MaxIgnoreListEntries", 30);
		set_bAutoBuy = IniGetB(set_scCfgCommandsFile, "UserCommands", "AutoBuy", false);

	//Death
		//Style
		set_wscDeathMsgStyle = stows(IniGetS(set_scCfgDeathFile, "Style", "DeathMsgStyle", "0x19198C01"));
		set_wscDeathMsgStyleSys = stows(IniGetS(set_scCfgDeathFile, "Style", "DeathMsgStyleSys", "0x1919BD01"));
		//General
		set_bChangeRepPvPDeath = IniGetB(set_scCfgDeathFile, "General", "ChangeRepPvPDeath", false);
		set_bCombineMissileTorpMsgs = IniGetB(set_scCfgDeathFile, "General", "CombineMissilesTorps", false);
		set_fDeathPenalty = IniGetF(set_scCfgDeathFile, "General", "DeathPenaltyFraction", 0.0f);
		set_fDeathPenaltyKiller = IniGetF(set_scCfgDeathFile, "General", "DeathPenaltyKillerFraction", 0.0f);
		set_iNPCDeathMessages = 0;
		string scTypeList = IniGetS(set_scCfgDeathFile, "General", "NPCDeathMessages", "");
		if(scTypeList.length())
		{
			uint iParamIndex = 0;
			string scParam = Trim(GetParam(scTypeList, ',', iParamIndex));
			while(scParam.length())
			{
				set_iNPCDeathMessages |= TypeStrToObjUint(scParam);
				scParam = Trim(GetParam(scTypeList, ',', ++iParamIndex));
			}
		}
		set_iMaxDeathFactionCauses = IniGetI(set_scCfgDeathFile, "General", "NumDeathFactionReasons", 1);
		set_iMaxDeathEquipmentCauses = IniGetI(set_scCfgDeathFile, "General", "NumDeathEquipmentReasons", 1);
		set_fRepChangePvP = IniGetF(set_scCfgDeathFile, "General", "PvPRepChangeDeath", 0.0f);
		//Death Message singular factions
		IniGetSection(set_scCfgDeathFile, "DeathMsgOneFaction", lstValues);
		set_btOneFactionDeathRep->Clear();
		foreach(lstValues, INISECTIONVALUE, it9)
		{
			uint iFactionID;
			pub::Reputation::GetReputationGroup(iFactionID, it9->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iFactionID);
			set_btOneFactionDeathRep->Add(uw);
		}
		//Items to take for death penalty
		IniGetSection(set_scCfgDeathFile, "DeathPenaltyItems", lstValues);
		set_btDeathItems->Clear();
		foreach(lstValues, INISECTIONVALUE, it0)
		{
			DEATH_ITEM *di = new DEATH_ITEM(CreateID(it0->scKey.c_str()), ToFloat(it0->scValue));
			set_btDeathItems->Add(di);
		}
		//Ships to not apply the death penalty to
		IniGetSection(set_scCfgDeathFile, "ExcludeDeathPenaltyShips", lstValues);
		set_btNoDeathPenalty->Clear();
		foreach(lstValues, INISECTIONVALUE, it10)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, it10->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iShipArchID);
			set_btNoDeathPenalty->Add(uw);
		}
		//Systems to not apply the death penalty in
		IniGetSection(set_scCfgDeathFile, "ExcludeDeathPenaltySystems", lstValues);
		set_btNoDPSystems->Clear();
		foreach(lstValues, INISECTIONVALUE, it13)
		{
			uint iSystemID;
			pub::GetSystemID(iSystemID, it13->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iSystemID);
			set_btNoDPSystems->Add(uw);
		}
		//Solar death reputation adjustment
		IniGetSection(set_scCfgDeathFile, "NPCDeathRep", lstValues);
		set_mapNPCDeathRep.clear();
		set_iNPCDeathType = set_iNPCDeathMessages;
		foreach(lstValues, INISECTIONVALUE, it20)
		{
			uint iType = TypeStrToObjUint(Trim(it20->scKey));
			set_iNPCDeathType |= iType;
			set_mapNPCDeathRep[iType] = ToFloat(it20->scValue);
		}
		// MultiKillMessages
		set_MKM_bActivated = IniGetB(set_scCfgDeathFile, "MultiKillMessages", "Activated", false);
		set_MKM_wscStyle = stows(IniGetS(set_scCfgDeathFile, "MultiKillMessages", "Style", "0x1919BD01"));
		set_MKM_lstMessages.clear();
		IniGetSection(set_scCfgDeathFile, "MultiKillMessages", lstValues);
		foreach(lstValues, INISECTIONVALUE, it)
		{
			if(!atoi(it->scKey.c_str()))
				continue;
			MULTIKILLMESSAGE mkm;
			mkm.iKillsInARow = atoi(it->scKey.c_str());
			mkm.wscMessage = stows(it->scValue);
			set_MKM_lstMessages.push_back(mkm);
		}

	//Docking
		//General
		set_iAntiDockKill = IniGetI(set_scCfgDockingFile, "General", "AntiDockKill", 0);
		set_fBaseDockDmg = IniGetF(set_scCfgDockingFile, "General", "BaseDockDamagePrevent", 0.0f);
		set_iMobileDockRadius = IniGetI(set_scCfgDockingFile, "General", "MobileDockRadius", -1);
		set_iMobileDockOffset = IniGetI(set_scCfgDockingFile, "General", "MobileDockOffset", -1);
		//Mobile docking
		IniGetSection(set_scCfgDockingFile, "MobileBases", lstValues);
		set_btMobDockShipArchIDs->Clear();
		foreach(lstValues, INISECTIONVALUE, it7)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, Trim(GetParam(it7->scKey, ',', 0)).c_str());
			MOBILE_SHIP *ms = new MOBILE_SHIP(iShipArchID, ToInt(Trim(GetParam(it7->scKey, ',', 1))));
			set_btMobDockShipArchIDs->Add(ms);
		}
		//Dock restriction
		IniGetSection(set_scCfgDockingFile, "DockRestrictions", lstValues);
		set_btJRestrict->Clear();
		foreach(lstValues, INISECTIONVALUE, it4)
		{
			set_btJRestrict->Add(new DOCK_RESTRICTION(CreateID(it4->scKey.c_str()), CreateID((Trim(GetParam(it4->scValue, ',', 0))).c_str()), ToInt(Trim(GetParam(it4->scValue, ',', 1))), stows(Trim(GetParamToEnd(it4->scValue, ',', 2)))));
		}
		
	//Items
		//Mount restriction
		IniGetSection(set_scCfgItemsFile, "InternalMountRestrictions", lstValues);
		set_btMRestrict->Clear();
		foreach(lstValues, INISECTIONVALUE, it5)
		{
			BinaryTree<UINT_WRAP> *btShipArchIDs = new BinaryTree<UINT_WRAP>();
			uint iGoodID;
			pub::GetGoodID(iGoodID, it5->scKey.c_str());
			uint iParamIndex = 0;
			string scParam = Trim(GetParam(it5->scValue, ',', iParamIndex));
			while(scParam.length())
			{
				uint iShipArchID;
				pub::GetShipID(iShipArchID, scParam.c_str());
				btShipArchIDs->Add(new UINT_WRAP(iShipArchID));
				iParamIndex++;
				scParam = Trim(GetParam(it5->scValue, ',', iParamIndex));
			}
			MOUNT_RESTRICTION *mr = new MOUNT_RESTRICTION(iGoodID, btShipArchIDs);
			set_btMRestrict->Add(mr);
		}
		//Items not allowed to trade
		IniGetSection(set_scCfgItemsFile, "ExcludeTradeItems", lstValues);
		set_btNoTrade->Clear();
		foreach(lstValues, INISECTIONVALUE, it11)
		{
			UINT_WRAP *uw = new UINT_WRAP(CreateID(it11->scKey.c_str()));
			set_btNoTrade->Add(uw);
		}
		//Item restrictions
		IniGetSection(set_scCfgItemsFile, "ItemRestrictions", lstValues);
		set_btItemRestrictions->Clear();
		foreach(lstValues, INISECTIONVALUE, it12)
		{
			BinaryTree<UINT_WRAP> *btItems = new BinaryTree<UINT_WRAP>();
			uint iParamIndex = 0;
			string scParam = Trim(GetParam(it12->scValue, ',', iParamIndex));
			while(scParam.length())
			{
				btItems->Add(new UINT_WRAP(CreateID(scParam.c_str())));
				iParamIndex++;
				scParam = Trim(GetParam(it12->scValue, ',', iParamIndex));
			}
			ITEM_RESTRICT *ir = new ITEM_RESTRICT(CreateID(it12->scKey.c_str()), btItems);
			set_btItemRestrictions->Add(ir);
		}
		//Items to automount upon ship purchase
		IniGetSection(set_scCfgItemsFile, "AutoMount", lstValues);
		set_setAutoMount.clear();
		foreach(lstValues, INISECTIONVALUE, it15)
		{
			uint iGoodID = CreateID(Trim(it15->scKey).c_str());
			set_setAutoMount.insert(iGoodID);
		}
		//Items to not let exist in space
		IniGetSection(set_scCfgItemsFile, "NoSpaceItems", lstValues);
		set_setNoSpaceItems.clear();
		foreach(lstValues, INISECTIONVALUE, it16)
		{
			uint iGoodID = CreateID(Trim(it16->scKey).c_str());
			set_setNoSpaceItems.insert(iGoodID);
		}
		//Items to automatically mark when spawned
		IniGetSection(set_scCfgItemsFile, "AutoMarkItems", lstValues);
		set_setAutoMark.clear();
		foreach(lstValues, INISECTIONVALUE, it17)
		{
			uint iGoodID = CreateID(Trim(it17->scKey).c_str());
			set_setAutoMark.insert(iGoodID);
		}
		//Equipment to redirect damage to hull
		IniGetSection(set_scCfgItemsFile, "EquipmentRedirectDamage", lstValues);
		set_setEquipReDam.clear();
		foreach(lstValues, INISECTIONVALUE, it18)
		{
			uint iEquipID = CreateID(Trim(it18->scKey).c_str());
			set_setEquipReDam.insert(iEquipID);
		}
		//Systems to prevent dropping/trading items in
		IniGetSection(set_scCfgItemsFile, "SystemItemRestrictions", lstValues);
		set_mapSysItemRestrictions.clear();
		foreach(lstValues, INISECTIONVALUE, it19)
		{
			uint iSystemID;
			pub::GetSystemID(iSystemID, it19->scKey.c_str());
			map<Archetype::AClassType, char> mapItemTypes;
			uint iParamIndex = 0;
			string scParam = Trim(GetParam(it19->scValue, ',', iParamIndex));
			while(scParam.length())
			{
				mapItemTypes[TypeStrToEnum(scParam)] = 0;
				scParam = Trim(GetParam(it19->scValue, ',', ++iParamIndex));
			}
			set_mapSysItemRestrictions[iSystemID] = mapItemTypes;
		}		

	//PVP
		//No-PvP Tokens
		IniGetSection(set_scCfgPVPFile, "NoPvpTokens", lstValues);
		set_vNoPvpGoodIDs.clear();
		set_vNoPvpFactionIDs.clear();
		foreach(lstValues, INISECTIONVALUE, it6)
		{
			set_vNoPvpGoodIDs.push_back(CreateID(Trim(GetParam(it6->scKey, ',', 0)).c_str()));
			uint iFactionID;
			string scFaction = Trim(GetParam(it6->scKey, ',', 1));
			if(scFaction.length())
				pub::Reputation::GetReputationGroup(iFactionID, scFaction.c_str());
			else
				iFactionID = 0;
			set_vNoPvpFactionIDs.push_back(iFactionID);
		}
		// NoPVP
		set_lstNoPVPSystems.clear();
		for(i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "System%u", i);
			string scSystem = IniGetS(set_scCfgPVPFile, "NoPVP", szBuf, "");

			if(!scSystem.length())
				break;

			uint iSystemID;
			pub::GetSystemID(iSystemID, scSystem.c_str());
			set_lstNoPVPSystems.push_back(iSystemID);
		}
		
	//Repair
		//General
		set_iRepairBaseTime = IniGetI(set_scCfgRepairFile, "General", "AlternateRepairTime", 240);
		set_fRepairBaseRatio = IniGetF(set_scCfgRepairFile, "General", "AlternateRepairTimeRatio", 0.5f);
		set_fRepairBaseMaxHealth = IniGetF(set_scCfgRepairFile, "General", "AlternateMaxRepair", 1.0f);
		//Alternate repair of Solars
		if(set_iRepairBaseTime % 2)//make sure it's an even number
			set_iRepairBaseTime++;
		IniGetSection(set_scCfgRepairFile, "SolarRepair", lstValues);
		set_btSupressHealth->Clear();
		foreach(lstValues, INISECTIONVALUE, it2)
		{
			if(!ToFloat(stows(it2->scKey.c_str())))
				continue;
			FLOAT_WRAP *fSupress = new FLOAT_WRAP(ToFloat(stows(it2->scKey.c_str())));
			set_btSupressHealth->Add(fSupress);
		}
		//repair gun
		list<INISECTIONVALUE> lstRepairGunNames;
		IniGetSection(set_scCfgRepairFile, "RepairGuns", lstRepairGunNames);
		set_btRepairGun->Clear();
		foreach(lstRepairGunNames, INISECTIONVALUE, value)
		{
			Archetype::Equipment *gunEquip, *ammo;
			uint iGunID;
			iGunID = CreateID((*value).scKey.c_str());
			gunEquip = Archetype::GetEquipment(iGunID);
			ammo = Archetype::GetEquipment(gunEquip->iAmmoArchID);
			//ammo = (Archetype::Equipment*)(((char*)gunEquip) - 408);
			REPAIR_GUN *gun = new REPAIR_GUN(gunEquip->iAmmoArchID, ammo->fHullDamage);
			set_btRepairGun->Add(gun);
		}
		//Ship repair over time
		IniGetSection(set_scCfgRepairFile, "ShipRepair", lstValues);
		set_mapShipRepair.clear();
		foreach(lstValues, INISECTIONVALUE, it13)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, Trim(it13->scKey).c_str());
			set_mapShipRepair[iShipArchID] = ToFloat(Trim(it13->scValue));
		}
		IniGetSection(set_scCfgRepairFile, "ItemRepair", lstValues);
		set_mapItemRepair.clear();
		foreach(lstValues, INISECTIONVALUE, it14)
		{
			set_mapItemRepair[CreateID(Trim(it14->scKey).c_str())] = ToFloat(Trim(it14->scValue))/4;
		}
	
	//Reputation
		//General
		set_fMaxRepValue = IniGetF(set_scCfgReputationFile, "General", "MaxRepValue", 0.9f);
		set_fMinRepValue = IniGetF(set_scCfgReputationFile, "General", "MinRepValue", -0.9f);
		//items that change player affil to that of base
		IniGetSection(set_scCfgReputationFile, "BaseAffiliationChange", lstValues);
		set_vAffilItems.clear();
		foreach(lstValues, INISECTIONVALUE, it3)
		{
			uint iGoodID;
			pub::GetGoodID(iGoodID, it3->scKey.c_str());
			set_vAffilItems.push_back(iGoodID);
		}

	} catch(...) { ConPrint(L"Exception in %s, settings likely not loaded\n", stows(__FUNCTION__).c_str()); AddLog("Exception in %s", __FUNCTION__); }

}

Archetype::AClassType TypeStrToEnum(string scType)
{
	if(scType == "Commodity")
	  return Archetype::COMMODITY;
	if(scType == "Equipment")
	  return Archetype::EQUIPMENT;
	if(scType == "AttachedEquipment")
	  return Archetype::ATTACHED_EQUIPMENT;
	if(scType == "LootCrate")
	  return Archetype::LOOT_CRATE;
	if(scType == "CargoPod")
	  return Archetype::CARGO_POD;
	if(scType == "Power")
	  return Archetype::POWER;
	if(scType == "Engine")
	  return Archetype::ENGINE;
	if(scType == "Shield")
	  return Archetype::SHIELD;
	if(scType == "ShieldGenerator")
	  return Archetype::SHIELD_GENERATOR;
	if(scType == "Thruster")
	  return Archetype::THRUSTER;
	if(scType == "Launcher")
	  return Archetype::LAUNCHER;
	if(scType == "Gun")
	  return Archetype::GUN;
	if(scType == "MineDropper")
	  return Archetype::MINE_DROPPER;
	if(scType == "CounterMeasureDropper")
	  return Archetype::COUNTER_MEASURE_DROPPER;
	if(scType == "Scanner")
	  return Archetype::SCANNER;
	if(scType == "Light")
	  return Archetype::LIGHT;
	if(scType == "Tractor")
	  return Archetype::TRACTOR;
	if(scType == "RepairDroid")
	  return Archetype::REPAIR_DROID;
	if(scType == "RepairKit")
	  return Archetype::REPAIR_KIT;
	if(scType == "ShieldBattery")
	  return Archetype::SHIELD_BATTERY;
	if(scType == "CloakingDevice")
	  return Archetype::CLOAKING_DEVICE;
	if(scType == "TradeLaneEquip")
	  return Archetype::TRADE_LANE_EQUIP;
	if(scType == "Projectile")
	  return Archetype::PROJECTILE;
	if(scType == "Munition")
	  return Archetype::MUNITION;
	if(scType == "MineDropper")
	  return Archetype::MINE;
	if(scType == "CounterMeasure")
	  return Archetype::COUNTER_MEASURE;
	if(scType == "Armor")
	  return Archetype::ARMOR;
	return Archetype::ROOT;
}

uint TypeStrToObjUint(string scType)
{
	if(scType == "MOON")
	  return OBJ_MOON;
	if(scType == "PLANET")
	  return OBJ_PLANET;
	if(scType == "SUN")
	  return OBJ_SUN;
	if(scType == "BLACKHOLE")
	  return OBJ_BLACKHOLE;
	if(scType == "SATELLITE")
	  return OBJ_SATELLITE;
	if(scType == "DOCKING_RING")
	  return OBJ_DOCKING_RING;
	if(scType == "JUMP_GATE")
	  return OBJ_JUMP_GATE;
	if(scType == "TRADELANE_RING")
	  return OBJ_TRADELANE_RING;
	if(scType == "STATION")
	  return OBJ_STATION;
	if(scType == "WAYPOINT")
	  return OBJ_WAYPOINT;
	if(scType == "AIRLOCK_GATE")
	  return OBJ_AIRLOCK_GATE;
	if(scType == "JUMP_HOLE")
	  return OBJ_JUMP_HOLE;
	if(scType == "WEAPONS_PLATFORM")
	  return OBJ_WEAPONS_PLATFORM;
	if(scType == "DESTROYABLE_DEPOT")
	  return OBJ_DESTROYABLE_DEPOT;
	if(scType == "NON_TARGETABLE")
	  return OBJ_NON_TARGETABLE;
	if(scType == "MISSION_SATELLITE")
	  return OBJ_MISSION_SATELLITE;
	if(scType == "FIGHTER")
	  return OBJ_FIGHTER;
	if(scType == "FREIGHTER")
	  return OBJ_FREIGHTER;
	if(scType == "GUNBOAT")
	  return OBJ_GUNBOAT;
	if(scType == "CRUISER")
	  return OBJ_CRUISER;
	if(scType == "TRANSPORT")
	  return OBJ_TRANSPORT;
	if(scType == "CAPITAL")
	  return OBJ_CAPITAL;
	if(scType == "MINING")
	  return OBJ_MINING;
	if(scType == "GUIDED")
	  return OBJ_GUIDED;
	if(scType == "BULLET")
	  return OBJ_BULLET;
	if(scType == "MINE")
	  return OBJ_MINE;
	if(scType == "LOOT")
	  return OBJ_LOOT;
	if(scType == "ASTEROID")
	  return OBJ_ASTEROID;
    return OBJ_NONE;
}