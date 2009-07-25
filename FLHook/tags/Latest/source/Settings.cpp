#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

string			set_scCfgFile;

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
float			set_fRepChangePvP;

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
map<uint, uint> set_mapAutoMount;

//Items to not let exist in space
map<uint, uint> set_mapNoSpaceItems;

//Items to automatically mark when spawned
map<uint, uint> set_mapAutoMark;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	try {
	// init cfg filename
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);
		set_scCfgFile = string(szCurDir) + "\\FLHook.ini";
		char *szFLConfig = (char*)((char*)GetModuleHandle(0) + ADDR_FLCONFIG);

	// General
		set_iDebug = IniGetI(set_scCfgFile, "General", "Debug", 0);
		set_iAntiDockKill = IniGetI(set_scCfgFile, "General", "AntiDockKill", 0);
		set_bDieMsg = IniGetB(set_scCfgFile, "General", "EnableDieMsg", true);
		set_bDisableCharfileEncryption = IniGetB(set_scCfgFile, "General", "DisableCharfileEncryption", false);
		set_bChangeCruiseDisruptorBehaviour = IniGetB(set_scCfgFile, "General", "ChangeCruiseDisruptorBehaviour", false);
		set_iDisableNPCSpawns = IniGetI(set_scCfgFile, "General", "DisableNPCSpawns", 0);
		set_iAntiF1 = IniGetI(set_scCfgFile, "General", "AntiF1", 0);
		set_iDisconnectDelay = IniGetI(set_scCfgFile, "General", "DisconnectDelay", 0);
		set_iReservedSlots = IniGetI(set_scCfgFile, "General", "ReservedSlots", 0);
		set_fTorpMissileBaseDamageMultiplier = IniGetF(set_scCfgFile, "General", "TorpMissileBaseDamageMultiplier", 1.0f);
		set_iMaxGroupSize = IniGetI(set_scCfgFile, "General", "MaxGroupSize", 0);

		set_scHkRestartOpt = IniGetS(set_scCfgFile, "General", "RestartServerOpt", "");
		set_fBaseDockDmg = IniGetF(set_scCfgFile, "General", "BaseDockDamagePrevent", 0.0f);
		set_fSendCashTax = IniGetF(set_scCfgFile, "General", "SendcashTax", 5.0f);
		set_iSendCashTime = IniGetI(set_scCfgFile, "General", "SendcashTime", 0);
		set_iRenameCmdCharge = IniGetI(set_scCfgFile, "General", "RenameCmdCharge", 500000);
		set_fAutoMarkRadius = IniGetF(set_scCfgFile, "General", "AutoMarkRadius", 0.0f)*1000;
		set_bFlakVersion = IniGetB(set_scCfgFile, "General", "Is88Flak", false);
		set_bInviteOnCharChange = IniGetB(set_scCfgFile, "General", "AddOnCharChange", false);
		set_iShieldsUpTime = IniGetI(set_scCfgFile, "General", "ShieldsUpTime", 15);
		set_fSpinProtectMass = IniGetF(set_scCfgFile, "General", "SpinProtectionMass", -1.0f);
		set_fSpinImpulseMultiplier = IniGetF(set_scCfgFile, "General", "SpinProtectionMultiplier", -8.0f);
		set_iTransferCmdCharge = IniGetI(set_scCfgFile, "General", "TransferCmdCharge", 150000);
		set_fMaxRepValue = IniGetF(set_scCfgFile, "General", "MaxRepValue", 0.9f);
		set_fMinRepValue = IniGetF(set_scCfgFile, "General", "MinRepValue", -0.9f);
		set_iBeamCmd = IniGetI(set_scCfgFile, "General", "BeamCmdBehavior", 0);
		set_bSetAutoErrorMargin = IniGetB(set_scCfgFile, "General", "AutopilotErrorSet", false);
		set_fAutoErrorMargin = IniGetF(set_scCfgFile, "General", "AutopilotError", -1.0f);
		set_bDamageNPCsCollision = IniGetB(set_scCfgFile, "General", "CollisionDamageNPCs", false);
		set_bDropShieldsCloak = IniGetB(set_scCfgFile, "General", "DropShieldsCloak", false);

	// Kick
		set_iAntiBaseIdle = IniGetI(set_scCfgFile, "Kick", "AntiBaseIdle", 0);
		set_iAntiCharMenuIdle = IniGetI(set_scCfgFile, "Kick", "AntiCharMenuIdle", 0);
		set_iPingKickFrame = IniGetI(set_scCfgFile, "Kick", "PingKickFrame", 30);
		if(!set_iPingKickFrame) 
			set_iPingKickFrame = 60; // might lead to problems else
		set_iPingKick = IniGetI(set_scCfgFile, "Kick", "PingKick", 0);
		set_iLossKickFrame = IniGetI(set_scCfgFile, "Kick", "LossKickFrame", 30);
		if(!set_iLossKickFrame) 
			set_iLossKickFrame = 60; // might lead to problems else
		set_iLossKick = IniGetI(set_scCfgFile, "Kick", "LossKick", 0);

	// Style
		set_wscDeathMsgStyle = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgStyle", "0x19198C01"));
		set_wscDeathMsgStyleSys = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgStyleSys", "0x1919BD01"));
		set_wscKickMsg = stows(IniGetS(set_scCfgFile, "Style", "KickMsg", "<TRA data=\"0x0000FF10\" mask=\"-1\"/><TEXT>You will be kicked. Reason: %s</TEXT>"));
		set_iKickMsgPeriod = IniGetI(set_scCfgFile, "Style", "KickMsgPeriod", 5000);
		set_wscUserCmdStyle = stows(IniGetS(set_scCfgFile, "Style", "UserCmdStyle", "0x00FF0090"));
		set_wscAdminCmdStyle = stows(IniGetS(set_scCfgFile, "Style", "AdminCmdStyle", "0x00FF0090"));
		set_wscUniverseColor = stows(IniGetS(set_scCfgFile, "Style", "UniverseColor", "FFFFFF"));
		set_wscSystemColor = stows(IniGetS(set_scCfgFile, "Style", "SystemColor", "E6C684"));
		set_wscPMColor = stows(IniGetS(set_scCfgFile, "Style", "PMColor", "19BD3A"));
		set_wscGroupColor = stows(IniGetS(set_scCfgFile, "Style", "GroupColor", "FF7BFF"));
		set_wscSenderColor = stows(IniGetS(set_scCfgFile, "Style", "SenderColor", "FFFFFF"));

	// Socket
		set_bSocketActivated = IniGetB(set_scCfgFile, "Socket", "Activated", false);
		set_iPort = IniGetI(set_scCfgFile, "Socket", "Port", 0);
		set_iWPort = IniGetI(set_scCfgFile, "Socket", "WPort", 0);
		set_iEPort = IniGetI(set_scCfgFile, "Socket", "EPort", 0);
		set_iEWPort = IniGetI(set_scCfgFile, "Socket", "EWPort", 0);
		string scEncryptKey = IniGetS(set_scCfgFile, "Socket", "Key", "");
		if(scEncryptKey.length())
		{
			if(!set_BF_CTX)
				set_BF_CTX = (BLOWFISH_CTX*)malloc(sizeof(BLOWFISH_CTX));
			Blowfish_Init(set_BF_CTX, (unsigned char *)scEncryptKey.data(), scEncryptKey.length());
		}
		
	// UserCommands
		set_bUserCmdSetDieMsg = IniGetB(set_scCfgFile, "UserCommands", "SetDieMsg", false);
		set_bUserCmdSetDieMsgSize = IniGetB(set_scCfgFile, "UserCommands", "SetDieMsgSize", false);
		set_bUserCmdSetChatFont = IniGetB(set_scCfgFile, "UserCommands", "SetChatFont", false);
		set_bUserCmdIgnore = IniGetB(set_scCfgFile, "UserCommands", "Ignore", false);
		set_iUserCmdMaxIgnoreList = IniGetI(set_scCfgFile, "UserCommands", "MaxIgnoreListEntries", 30);
		set_bAutoBuy = IniGetB(set_scCfgFile, "UserCommands", "AutoBuy", false);

	// Death
		set_bChangeRepPvPDeath = IniGetB(set_scCfgFile, "Death", "ChangeRepPvPDeath", false);
		set_bCombineMissileTorpMsgs = IniGetB(set_scCfgFile, "Death", "CombineMissilesTorps", false);
		set_fDeathPenalty = IniGetF(set_scCfgFile, "Death", "DeathPenaltyFraction", 0.5f);
		set_iMaxDeathFactionCauses = IniGetI(set_scCfgFile, "Death", "NumDeathFactionReasons", 1);
		set_iMaxDeathEquipmentCauses = IniGetI(set_scCfgFile, "Death", "NumDeathEquipmentReasons", 1);
		set_fRepChangePvP = IniGetF(set_scCfgFile, "Death", "PvPRepChangeDeath", 0.0f);

	// NoPVP
		set_lstNoPVPSystems.clear();
		for(uint i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "System%u", i);
			string scSystem = IniGetS(set_scCfgFile, "NoPVP", szBuf, "");

			if(!scSystem.length())
				break;

			uint iSystemID;
			pub::GetSystemID(iSystemID, scSystem.c_str());
			set_lstNoPVPSystems.push_back(iSystemID);
		}

	// read chat suppress
		set_lstChatSuppress.clear();
		for(i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "Suppress%u", i);
			string scSuppress = IniGetS(set_scCfgFile, "Chat", szBuf, "");

			if(!scSuppress.length())
				break;

			set_lstChatSuppress.push_back(stows(scSuppress));
		}
	// Set system chat to go to universe?
		set_bSystemToUniverse = IniGetB(set_scCfgFile, "Chat", "SystemToUniverse", false);
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

	// MultiKillMessages
		set_MKM_bActivated = IniGetB(set_scCfgFile, "MultiKillMessages", "Activated", false);
		set_MKM_wscStyle = stows(IniGetS(set_scCfgFile, "MultiKillMessages", "Style", "0x1919BD01"));

		set_MKM_lstMessages.clear();
		list<INISECTIONVALUE> lstValues;
		IniGetSection(set_scCfgFile, "MultiKillMessages", lstValues);
		foreach(lstValues, INISECTIONVALUE, it)
		{
			if(!atoi(it->scKey.c_str()))
				continue;

			MULTIKILLMESSAGE mkm;
			mkm.iKillsInARow = atoi(it->scKey.c_str());
			mkm.wscMessage = stows(it->scValue);
			set_MKM_lstMessages.push_back(mkm);
		}

	// bans
		set_bBanAccountOnMatch = IniGetB(set_scCfgFile, "Bans", "BanAccountOnMatch", false);
		set_lstBans.clear();
		IniGetSection(set_scCfgFile, "Bans", lstValues);
		lstValues.pop_front();
		foreach(lstValues, INISECTIONVALUE, itisv)
			set_lstBans.push_back(stows(itisv->scKey));

	// cloak
		IniGetSection(set_scCfgFile, "CloakDevices", set_lstCloakDevices);
		set_iCloakExpireWarnTime = IniGetI(set_scCfgFile, "General", "CloakExpireWarnTime", 5000);

	//repair gun
		list<INISECTIONVALUE> lstRepairGunNames;
		IniGetSection(set_scCfgFile, "RepairGuns", lstRepairGunNames);
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

	//Alternate repair of Solars
		set_iRepairBaseTime = IniGetI(set_scCfgFile, "General", "AlternateRepairTime", 240);
		if(set_iRepairBaseTime % 2)//make sure it's an even number
			set_iRepairBaseTime++;
		IniGetSection(set_scCfgFile, "SolarRepair", lstValues);
		set_btSupressHealth->Clear();
		foreach(lstValues, INISECTIONVALUE, it2)
		{
			if(!ToFloat(stows(it2->scKey.c_str())))
				continue;
			FLOAT_WRAP *fSupress = new FLOAT_WRAP(ToFloat(stows(it2->scKey.c_str())));
			set_btSupressHealth->Add(fSupress);
		}
		set_fRepairBaseRatio = IniGetF(set_scCfgFile, "General", "AlternateRepairTimeRatio", 0.5f);
		set_fRepairBaseMaxHealth = IniGetF(set_scCfgFile, "General", "AlternateMaxRepair", 1.0f);
		
	//items that change player affil to that of base
		IniGetSection(set_scCfgFile, "BaseAffiliationChange", lstValues);
		set_vAffilItems.clear();
		foreach(lstValues, INISECTIONVALUE, it3)
		{
			uint iGoodID;
			pub::GetGoodID(iGoodID, it3->scKey.c_str());
			set_vAffilItems.push_back(iGoodID);
		}
		
	//Jump restriction
		IniGetSection(set_scCfgFile, "DockRestrictions", lstValues);
		set_btJRestrict->Clear();
		foreach(lstValues, INISECTIONVALUE, it4)
		{
			set_btJRestrict->Add(new DOCK_RESTRICTION(CreateID(it4->scKey.c_str()), CreateID((Trim(GetParam(it4->scValue, ',', 0))).c_str()), ToInt(Trim(GetParam(it4->scValue, ',', 1))), stows(Trim(GetParamToEnd(it4->scValue, ',', 2)))));
		}

	//Mount restriction
		IniGetSection(set_scCfgFile, "InternalMountRestrictions", lstValues);
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

	//No-PvP Tokens
		IniGetSection(set_scCfgFile, "NoPvpTokens", lstValues);
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

	//Mobile docking
		IniGetSection(set_scCfgFile, "MobileBases", lstValues);
		set_btMobDockShipArchIDs->Clear();
		foreach(lstValues, INISECTIONVALUE, it7)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, Trim(GetParam(it7->scKey, ',', 0)).c_str());
			MOBILE_SHIP *ms = new MOBILE_SHIP(iShipArchID, ToInt(Trim(GetParam(it7->scKey, ',', 1))));
			set_btMobDockShipArchIDs->Add(ms);
		}
		set_iMobileDockRadius = IniGetI(set_scCfgFile, "General", "MobileDockRadius", -1);
		set_iMobileDockOffset = IniGetI(set_scCfgFile, "General", "MobileDockOffset", -1);

	//Faction tag affiliation change init
		if(set_bFlakVersion)
			HkFindFactionMagnets(set_lstFactionTags);

	// load resource DLLs
		HkLoadDLLConf(szFLConfig);

	//Death Message singular factions
		IniGetSection(set_scCfgFile, "DeathMsgOneFaction", lstValues);
		set_btOneFactionDeathRep->Clear();
		foreach(lstValues, INISECTIONVALUE, it9)
		{
			uint iFactionID;
			pub::Reputation::GetReputationGroup(iFactionID, it9->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iFactionID);
			set_btOneFactionDeathRep->Add(uw);
		}

	//New character reputations
		HkLoadNewCharacter();

	//Items to take for death penalty
		IniGetSection(set_scCfgFile, "DeathPenaltyItems", lstValues);
		set_btDeathItems->Clear();
		foreach(lstValues, INISECTIONVALUE, it0)
		{
			DEATH_ITEM *di = new DEATH_ITEM(CreateID(it0->scKey.c_str()), ToFloat(it0->scValue));
			set_btDeathItems->Add(di);
		}

	//Ships to not apply the death penalty to
		IniGetSection(set_scCfgFile, "ExcludeDeathPenaltyShips", lstValues);
		set_btNoDeathPenalty->Clear();
		foreach(lstValues, INISECTIONVALUE, it10)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, it10->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iShipArchID);
			set_btNoDeathPenalty->Add(uw);
		}

	//Systems to not apply the death penalty in
		IniGetSection(set_scCfgFile, "ExcludeDeathPenaltySystems", lstValues);
		set_btNoDPSystems->Clear();
		foreach(lstValues, INISECTIONVALUE, it13)
		{
			uint iSystemID;
			pub::GetSystemID(iSystemID, it13->scKey.c_str());
			UINT_WRAP *uw = new UINT_WRAP(iSystemID);
			set_btNoDPSystems->Add(uw);
		}

	//RepChangeEffects
		HkLoadEmpathy(szFLConfig);

	//Items not allowed to trade
		IniGetSection(set_scCfgFile, "ExcludeTradeItems", lstValues);
		set_btNoTrade->Clear();
		foreach(lstValues, INISECTIONVALUE, it11)
		{
			UINT_WRAP *uw = new UINT_WRAP(CreateID(it11->scKey.c_str()));
			set_btNoTrade->Add(uw);
		}

	//Item restrictions
		IniGetSection(set_scCfgFile, "ItemRestrictions", lstValues);
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

	//Ship repair over time
		IniGetSection(set_scCfgFile, "ShipRepair", lstValues);
		set_mapShipRepair.clear();
		foreach(lstValues, INISECTIONVALUE, it13)
		{
			uint iShipArchID;
			pub::GetShipID(iShipArchID, Trim(it13->scKey).c_str());
			set_mapShipRepair[iShipArchID] = ToFloat(Trim(it13->scValue));
		}
		IniGetSection(set_scCfgFile, "ItemRepair", lstValues);
		set_mapItemRepair.clear();
		foreach(lstValues, INISECTIONVALUE, it14)
		{
			set_mapItemRepair[CreateID(Trim(it14->scKey).c_str())] = ToFloat(Trim(it14->scValue))/4;
		}

		IniGetSection(set_scCfgFile, "AutoMount", lstValues);
		set_mapAutoMount.clear();
		foreach(lstValues, INISECTIONVALUE, it15)
		{
			uint iGoodID = CreateID(Trim(it15->scKey).c_str());
			set_mapAutoMount[iGoodID] = iGoodID;
		}

		
		IniGetSection(set_scCfgFile, "NoSpaceItems", lstValues);
		set_mapNoSpaceItems.clear();
		foreach(lstValues, INISECTIONVALUE, it16)
		{
			uint iGoodID = CreateID(Trim(it16->scKey).c_str());
			set_mapNoSpaceItems[iGoodID] = iGoodID;
		}

		IniGetSection(set_scCfgFile, "AutoMarkItems", lstValues);
		set_mapAutoMark.clear();
		foreach(lstValues, INISECTIONVALUE, it17)
		{
			uint iGoodID = CreateID(Trim(it17->scKey).c_str());
			set_mapAutoMark[iGoodID] = iGoodID;
		}

	} catch(...) { ConPrint(L"Exception in %s, settings likely not loaded\n", stows(__FUNCTION__).c_str()); AddLog("Exception in %s", __FUNCTION__); }

}
