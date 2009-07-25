#include "wildcards.hh"
#include "hook.h"
#include "CInGame.h"

#define ISERVER_LOG() if(set_iDebug >= 2) AddLog(__FUNCSIG__);
#define ISERVER_LOGARG_WS(a) if(set_iDebug >= 2) AddLog("     " #a ": %s", wstos((const wchar_t*)a).c_str());
#define ISERVER_LOGARG_S(a) if(set_iDebug >= 2) AddLog("     " #a ": %s", (const char*)a);
#define ISERVER_LOGARG_UI(a) if(set_iDebug >= 2) AddLog("     " #a ": %u", (uint)a);
#define ISERVER_LOGARG_I(a) if(set_iDebug >= 2) AddLog("     " #a ": %d", (int)a);
#define ISERVER_LOGARG_F(a) if(set_iDebug >= 2) AddLog("     " #a ": %f", (float)a);
CInGame admin;
//Repair gun vars
float g_fRepairMaxHP;
float g_fRepairBeforeHP;
float g_fRepairDamage;
uint g_iRepairShip;
bool g_bRepairPendHit = false;
BinaryTree<SOLAR_REPAIR> *btSolarList = new BinaryTree<SOLAR_REPAIR>();
list<MOB_UNDOCKBASEKILL> lstUndockKill;

namespace HkIServerImpl
{

/**************************************************************************************************************
this is our "main" loop
**************************************************************************************************************/

// add timers here
typedef void (*_TimerFunc)();

struct TIMER
{
	_TimerFunc	proc;
	mstime		tmIntervallMS;
	mstime		tmLastCall;
};

TIMER Timers[] = 
{
	{ProcessPendingCommands,		50,					0},
	{HkTimerUpdatePingData,			1000,				0},
	{HkTimerUpdateLossData,			LOSS_INTERVALL,		0},
	{HkTimerCheckKick,				1000,				0},
	{HkTimerNPCAndF1Check,			50,					0},
	{HkTimerCheckIfBaseDestroyed,	1000,				0},
	{HkTimerSolarRepair,			1000,				0},
	{HkTimerCheckResolveResults,	0,					0},
	{HkTimerCloakHandler,			500,				0},
	{HkTimerSpaceObjMark,			2000,				0},
	{HkTimerNPCDockHandler,			2500,				0},
	{HkTimerRepairShip,				250,				0},
	{HkTimerMarkDelay,				150,				0},
};

int __stdcall Update(void)
{
	static bool bFirstTime = true;
	if(bFirstTime)
	{
		FLHookInit();
		bFirstTime = false;
	}

	// call timers
	for(uint i = 0; (i < sizeof(Timers)/sizeof(TIMER)); i++)
	{
		if((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS)
		{
			Timers[i].tmLastCall = timeInMS();
			Timers[i].proc();
		}
	}

	char *pData;
	memcpy(&pData, g_FLServerDataPtr + 0x40, 4);
	memcpy(&g_iServerLoad, pData + 0x204, 4);

	int iRet = Server.Update();
	return iRet;
}

/**************************************************************************************************************
Chat-Messages are hooked here
<Parameters>
cId:       Sender's ClientID
lP1:       size of rdlReader (used when extracting text from that buffer)
rdlReader: RenderDisplayList which contains the chat-text
cIdTo:     recipient's clientid(0x10000 = universe chat else when (cIdTo & 0x10000) = true -> system chat)
iP2:       ???
**************************************************************************************************************/

bool g_bInSubmitChat = false;
uint g_iTextLen = 0;
bool g_bChatAction = false;
void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1, void const *rdlReader, struct CHAT_ID cIdTo, int iP2)
{
	wchar_t wszBuf[1024] = L"";

	try {
		uint iClientID = cId.iID;

		// anti base-idle
		if(ClientInfo[iClientID].iBaseEnterTime)
		{
			ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);
		}

		if(cIdTo.iID == 0x10004)
		{
			g_bInSubmitChat = true;
			Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2);
			g_bInSubmitChat = false;
			return;
		}

		// extract text from rdlReader
		BinaryRDLReader rdl;
		uint iRet1;
		rdl.extract_text_from_buffer(wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader, lP1);
		wstring wscBuf = wszBuf;
		g_iTextLen = (uint)wscBuf.length();
		//Check for FLServer commands
		if(!wscBuf.find(L"/u ") || !wscBuf.find(L"/universe ") || !wscBuf.find(L"/s ") || !wscBuf.find(L"/system ") || !wscBuf.find(L"/g ") || !wscBuf.find(L"/group ") || !wscBuf.find(L"/l ") || !wscBuf.find(L"/local ") || !wscBuf.find(L"/leave") || !wscBuf.find(L"/lv") || !wscBuf.find(L"/join ") || !wscBuf.find(L"/j "))
			g_iTextLen -= 3;

		// check for user cmds
		if(UserCmd_Process(iClientID, wscBuf))
			return;

		if( wszBuf[0] == '.' && g_iTextLen>1 && wszBuf[1] != '.' )
		{ // flhook admin command
			CAccount *acc = Players.FindAccountFromClientID(iClientID);
			wstring wscAccDirname;

			HkGetAccountDirName(acc, wscAccDirname);
			string scAdminFile = scAcctPath + wstos(wscAccDirname) + "\\flhookadmin.ini";
			WIN32_FIND_DATA fd;
			HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
			if(hFind != INVALID_HANDLE_VALUE)
			{ // is admin
				FindClose(hFind);
				admin.ReadRights(scAdminFile);
				admin.iClientID = iClientID;
				admin.wscAdminName = Players.GetActiveCharacterName(iClientID);
				admin.ExecuteCommandString(wszBuf + 1);
				return;
			}
		}

		//System chat to universe chat setting
		if(set_bSystemToUniverse)
		{
			if(cIdTo.iID!=0x10003 && cIdTo.iID && cIdTo.iID & 0x00010000)
				cIdTo.iID = 0x00010000;
		}

		if(!wscBuf.find(L"/me "))
			g_bChatAction = true;
		else
			g_bChatAction = false;


		// process chat event
		if(set_bSocketActivated) //Only process if sockets enabled
		{
			wstring wscEvent;
			wscEvent.reserve(256);
			wscEvent = L"chat";
			wscEvent += L" from=";
			if(!cId.iID)
				wscEvent += L"console";
			else
				wscEvent += Players.GetActiveCharacterName(cId.iID);

			wscEvent += L" id=";
			wscEvent += stows(itos(cId.iID));

			wscEvent += L" type=";
			if(cIdTo.iID == 0x00010000)
				wscEvent += L"universe";
			else if(cIdTo.iID & 0x00010000)
				wscEvent += L"system";
			else {
				wscEvent += L"player";
				wscEvent += L" to=";

				if(!cIdTo.iID)
					wscEvent += L"console";
				else
					wscEvent += Players.GetActiveCharacterName(cIdTo.iID);

				wscEvent += L" idto=";
				wscEvent += stows(itos(cIdTo.iID));
			}

			wscEvent += L" text=";
			wscEvent += wscBuf;
			ProcessEvent(wscEvent);
		}

		// check if chat should be suppressed
		foreach(set_lstChatSuppress, wstring, i)
		{
			if((ToLower(wscBuf)).find(ToLower(*i)) == 0)
				return;
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	// send
	g_bInSubmitChat = true;
	try { 
		Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2);
	} catch(...) { AddLog("Exception in Server.SubmitChat"); }
	g_bInSubmitChat = false;
}

/**************************************************************************************************************
Called when player ship was created in space (after undock or login)
**************************************************************************************************************/

void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(iClientID);

	list <CARGO_INFO> lstCargo;
	try {
		ClientInfo[iClientID].iShip = iShip;
		ClientInfo[iClientID].iKillsInARow = 0;
		ClientInfo[iClientID].bCruiseActivated = false;
		ClientInfo[iClientID].bThrusterActivated = false;
		ClientInfo[iClientID].bEngineKilled = false;
		ClientInfo[iClientID].bTradelane = false;
		
		HkInitCloakSettings(iClientID);
		if(ClientInfo[iClientID].bCanCloak)
		{
			ClientInfo[iClientID].bMustSendUncloak = true;
		}

		// adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
		wstring wscCharname = ToLower(Players.GetActiveCharacterName(iClientID));
		foreach(ClientInfo[iClientID].lstMoneyFix, MONEY_FIX, i)
		{
			if(!(*i).wscCharname.compare(wscCharname))
			{
				HkAddCash(wscCharname, (*i).iAmount);
				ClientInfo[iClientID].lstMoneyFix.remove(*i);
				break;
			}
		}

		// adjust cargo, this is necessary when cargo was added while use was in charmenu/had other char selected
		for(list<CARGO_FIX>::iterator i2 = ClientInfo[iClientID].lstCargoFix.begin(); (i2 != ClientInfo[iClientID].lstCargoFix.end()); )
		{
			if(!(*i2).wscCharname.compare(wscCharname))
			{
				HkAddCargo(wscCharname, i2->iGoodID, i2->iCount, i2->bMission);
				i2 = ClientInfo[iClientID].lstCargoFix.erase(i2);
			}
			else
				i2++;
		}

		if(set_vNoPvpGoodIDs.size())
		{
			int iHold;
			HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
			ClientInfo[iClientID].bNoPvp = false;
			foreach(lstCargo, CARGO_INFO, cargo)
			{
				bool bBreak = false;
				for(uint i=0; i<set_vNoPvpGoodIDs.size(); i++)
				{
					if(cargo->bMounted && set_vNoPvpGoodIDs[i]==cargo->iArchID)
					{
						int iRep;
						pub::Player::GetRep(iClientID, iRep);
						uint iAffil;
						Reputation::Vibe::GetAffiliation(iRep, iAffil, false);
						if(set_vNoPvpFactionIDs[i] && iAffil!=set_vNoPvpFactionIDs[i])
						{
							pub::Reputation::SetReputation(iRep, set_vNoPvpFactionIDs[i], 0.9f);
							pub::Reputation::SetAffiliation(iRep, set_vNoPvpFactionIDs[i]);
						}
							
						ClientInfo[iClientID].bNoPvp = true;
						bBreak = true;
						break;
					}
				}
				if(bBreak)
					break;
			}
			if(!ClientInfo[iClientID].bNoPvp) //Might have previously had a no-pvp token, reset rep and/or affiliation with groups
			{
				int iRep;
				pub::Player::GetRep(iClientID, iRep);
				uint iAffil;
				Reputation::Vibe::GetAffiliation(iRep, iAffil, false);
				for(uint i=0; i<set_vNoPvpFactionIDs.size(); i++)
				{
					if(set_vNoPvpFactionIDs[i])
					{
						pub::Reputation::SetReputation(iRep, set_vNoPvpFactionIDs[i], 0.0f);
						if(iAffil==set_vNoPvpFactionIDs[i])
						{
							pub::Reputation::SetAffiliation(iRep, 0);
							break;
						}
					}
				}
			}
		}
				
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	g_iClientID = iClientID;
	//Launch hook, mobile docking stuffs
	if(ClientInfo[iClientID].bMobileDocked)
	{
		if(ClientInfo[iClientID].lstJumpPath.size()) //Not in same system, follow jump path
		{
			pub::SpaceObj::GetLocation(ClientInfo[iClientID].lstJumpPath.front(), g_Vlaunch, g_Mlaunch);
			g_Vlaunch.x -= g_Mlaunch.data[0][2]*750;
			g_Vlaunch.y -= g_Mlaunch.data[1][2]*750;
			g_Vlaunch.z -= g_Mlaunch.data[2][2]*750;
			g_Mlaunch.data[0][0] = -g_Mlaunch.data[0][0];
			g_Mlaunch.data[1][0] = -g_Mlaunch.data[1][0];
			g_Mlaunch.data[2][0] = -g_Mlaunch.data[2][0];
			g_Mlaunch.data[0][2] = -g_Mlaunch.data[0][2];
			g_Mlaunch.data[1][2] = -g_Mlaunch.data[1][2];
			g_Mlaunch.data[2][2] = -g_Mlaunch.data[2][2];
			g_bInPlayerLaunch = true;
			Server.PlayerLaunch(iShip, iClientID);
			g_bInPlayerLaunch = false;
			uint iShip;
			pub::Player::GetShip(iClientID, iShip);
			ClientInfo[iClientID].bPathJump = true;
			pub::SpaceObj::InstantDock(iShip, ClientInfo[iClientID].lstJumpPath.front(), 1);
			ClientInfo[iClientID].lstJumpPath.pop_front();
		}
		else if(ClientInfo[iClientID].iDockClientID) //same system and carrier still exists
		{
			uint iDockClientID = ClientInfo[iClientID].iDockClientID;
			uint iTargetBaseID;
			pub::Player::GetBase(iDockClientID, iTargetBaseID);
			if(iTargetBaseID) //carrier docked
			{
				uint iTargetShip = ClientInfo[iDockClientID].iLastSpaceObjDocked;
				if(iTargetShip) //got the spaceObj of the base alright
				{
					uint iType;
					pub::SpaceObj::GetType(iTargetShip, iType);
					pub::SpaceObj::GetLocation(iTargetShip, g_Vlaunch, g_Mlaunch);
					if(iType==32) //docking ring
					{
						g_Mlaunch.data[0][0] = -g_Mlaunch.data[0][0];
						g_Mlaunch.data[1][0] = -g_Mlaunch.data[1][0];
						g_Mlaunch.data[2][0] = -g_Mlaunch.data[2][0];
						g_Mlaunch.data[0][2] = -g_Mlaunch.data[0][2];
						g_Mlaunch.data[1][2] = -g_Mlaunch.data[1][2];
						g_Mlaunch.data[2][2] = -g_Mlaunch.data[2][2];
						g_Vlaunch.x += g_Mlaunch.data[0][0]*90;
						g_Vlaunch.y += g_Mlaunch.data[1][0]*90;
						g_Vlaunch.z += g_Mlaunch.data[2][0]*90;
					}
					else
					{
						g_Vlaunch.x += g_Mlaunch.data[0][1]*set_iMobileDockOffset;
						g_Vlaunch.y += g_Mlaunch.data[1][1]*set_iMobileDockOffset;
						g_Vlaunch.z += g_Mlaunch.data[2][1]*set_iMobileDockOffset;
					}
					g_bInPlayerLaunch = true;
					Server.PlayerLaunch(iShip, iClientID);
					g_bInPlayerLaunch = false;
					pub::Player::RevertCamera(iClientID);
				}
				else //backup: set player's base to that of target, hope they won't get kicked (this shouldn't happen)
				{
					Players[iClientID].iBaseID = iTargetBaseID;
					Server.PlayerLaunch(iShip, iClientID);
				}
			}
			else //carrier not docked
			{
				uint iTargetShip;
				pub::Player::GetShip(iDockClientID, iTargetShip);
				pub::SpaceObj::GetLocation(iTargetShip, g_Vlaunch, g_Mlaunch);
				g_Vlaunch.x += g_Mlaunch.data[0][1]*set_iMobileDockOffset;
				g_Vlaunch.y += g_Mlaunch.data[1][1]*set_iMobileDockOffset;
				g_Vlaunch.z += g_Mlaunch.data[2][1]*set_iMobileDockOffset;
				g_bInPlayerLaunch = true;
				Server.PlayerLaunch(iShip, iClientID);
				g_bInPlayerLaunch = false;
				pub::Player::RevertCamera(iClientID);
			}

		}
		else //same system, carrier does not exist
		{
			g_Vlaunch = ClientInfo[iClientID].Vlaunch;
			g_Mlaunch = ClientInfo[iClientID].Mlaunch;
			g_bInPlayerLaunch = true;
			Server.PlayerLaunch(iShip, iClientID);
			g_bInPlayerLaunch = false;
			pub::Player::RevertCamera(iClientID);
		}
	}
	else //regular launch
	{
		Server.PlayerLaunch(iShip, iClientID);
	}

	try {
		//Mobile docking
		uint iShipArchID;
		pub::Player::GetShipID(iClientID, iShipArchID);
		MOBILE_SHIP uwFind = MOBILE_SHIP(iShipArchID);
		MOBILE_SHIP *uwFound = set_btMobDockShipArchIDs->Find(&uwFind);
		if(uwFound) //ship is mobile base
		{
			ClientInfo[iClientID].bMobileBase = true;
			ClientInfo[iClientID].iMaxPlayersDocked = uwFound->iMaxNumOccupants;
		}
		else
		{
			ClientInfo[iClientID].bMobileBase = false;
		}

		//Print Death Penalty notice
		if(set_fDeathPenalty)
		{
			UINT_WRAP uwShip = UINT_WRAP(Players[iClientID].iShipArchID);
			UINT_WRAP uwSystem = UINT_WRAP(Players[iClientID].iSystemID);
			if(!set_btNoDeathPenalty->Find(&uwShip) && !set_btNoDPSystems->Find(&uwSystem))
			{
				int iCash;
				HkGetCash(ARG_CLIENTID(iClientID), iCash);
				float fValue;
				pub::Player::GetAssetValue(iClientID, fValue);
				ClientInfo[iClientID].iDeathPenaltyCredits = (int)(fValue * set_fDeathPenalty);
				if(ClientInfo[iClientID].bDisplayDPOnLaunch)
					PrintUserCmdText(iClientID, L"Notice: the death penalty for your ship will be %i credits.  Type /dp for more information.", ClientInfo[iClientID].iDeathPenaltyCredits);
			}
			else
				ClientInfo[iClientID].iDeathPenaltyCredits = 0;
		}

		//Add ship to list of ships to repair
		if(set_mapItemRepair.size())
		{
			if(!lstCargo.size())
			{
				int iHold;
				HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
			}
			foreach(lstCargo, CARGO_INFO, cargo)
			{
				if(!cargo->bMounted)
					continue;
				map<uint,float>::iterator repair = set_mapItemRepair.find(cargo->iArchID);
				if(repair != set_mapItemRepair.end())
				{
					SHIP_REPAIR sr;
					sr.iObjID = iShip;
					sr.fIncreaseHealth = repair->second;
					g_lstRepairShips.push_back(sr);
					break;
				}
			}
		}
		map<uint,float>::iterator repair = set_mapShipRepair.find(iShipArchID);
		if(repair != set_mapShipRepair.end())
		{
			SHIP_REPAIR sr;
			sr.iObjID = iShip;
			sr.fIncreaseHealth = repair->second;
			g_lstRepairShips.push_back(sr);
		}

		if(!ClientInfo[iClientID].iLastExitedBaseID)
		{
			ClientInfo[iClientID].iLastExitedBaseID = 1;

			// event
			ProcessEvent(L"spawn char=%s id=%d system=%s", 
					Players.GetActiveCharacterName(iClientID), 
					iClientID,
					HkGetPlayerSystem(iClientID).c_str());
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player fires a weapon
**************************************************************************************************************/

void __stdcall FireWeapon(unsigned int iClientID, struct XFireWeaponInfo const &wpn)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.FireWeapon(iClientID, wpn);
}

/**************************************************************************************************************
Called when one player hits a target with a gun
<Parameters>
ci:  only figured out where dwTargetShip is ...
**************************************************************************************************************/

void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const & ci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	uint iClientIDTarget;

	try {
		iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
		if(iClientIDTarget && !AllowPlayerDamage(iClientID, iClientIDTarget))
			return;

		//Check if weapon is a repair gun
		REPAIR_GUN FindGun = REPAIR_GUN(ci.iProjectileArchID, 0.0f);
		REPAIR_GUN *RepGun = set_btRepairGun->Find(&FindGun);
		if(RepGun)
		{
			float maxHealth, curHealth;
			pub::SpaceObj::GetHealth(ci.dwTargetShip, curHealth, maxHealth);
			uint iType;
			pub::SpaceObj::GetType(ci.dwTargetShip, iType);
			if(iType!=2) //object is a planet (not damagable)
			{
				if(!curHealth)
				{
					if((uint)(ci.dwTargetShip/1000000000))//Object is dead and a solar; boost health here
					{
						float fSetHealth = RepGun->damage/maxHealth;
						if(fSetHealth>1.0f)
							fSetHealth = 1.0f;
						pub::SpaceObj::SetRelativeHealth(ci.dwTargetShip, fSetHealth);
					}
					return;
				}
				g_fRepairMaxHP = maxHealth;
				g_fRepairBeforeHP = curHealth;
				g_iRepairShip = ci.dwTargetShip;
				g_fRepairDamage = RepGun->damage;
				g_bRepairPendHit = true;
			}
		}
		else
		{
			g_bRepairPendHit = false;

			//ClientInfo[iClientID].tmLastWeaponHit = timeInMS();
		}

		/*uint iType;
		pub::SpaceObj::GetType(ci.dwTargetShip, iType);
		PrintUniverseText(L"type=%u", iType);*/

	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }	

	iDmgTo = iClientIDTarget;
	Server.SPMunitionCollision(ci, iClientID);
}

/**************************************************************************************************************
Called when player moves his ship
**************************************************************************************************************/

void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		if(ClientInfo[iClientID].bCanCloak && ClientInfo[iClientID].bMustSendUncloak && !ClientInfo[iClientID].bIsCloaking) {
			HkUnCloak(iClientID);
			ClientInfo[iClientID].bMustSendUncloak = false;
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.SPObjUpdate(ui, iClientID);
}
/**************************************************************************************************************
Called when one player collides with a space object and is damaged
**************************************************************************************************************/

void __stdcall SPObjCollision(struct SSPObjCollisionInfo const &ci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	bool bShieldsUp = false;
	float fHealthBefore = 0.0f;
	uint iShip;
	try {
		uint iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
		if(iClientIDTarget && !AllowPlayerDamage(iClientID, iClientIDTarget))
			return;
		
		pub::Player::GetShip(iClientID, iShip);
		uint iType;
		pub::SpaceObj::GetType(ci.dwTargetShip, iType);
		if(iType == 65536) //Is FIGHTER
		{
			float fMass;
			pub::SpaceObj::GetMass(iShip, fMass);
			//PrintUniverseText(L"mass=%f, type=%u", inspect->get_mass(), iType);
			if(set_fSpinProtectMass!=-1.0f && !iClientIDTarget && fMass>=set_fSpinProtectMass)
			{
				Vector V1, V2;
				pub::SpaceObj::GetMotion(ci.dwTargetShip, V1, V2);
				pub::SpaceObj::GetMass(ci.dwTargetShip, fMass);
				V1.x *= set_fSpinImpulseMultiplier * fMass;
				V1.y *= set_fSpinImpulseMultiplier * fMass;
				V1.z *= set_fSpinImpulseMultiplier * fMass;
				V2.x *= set_fSpinImpulseMultiplier * fMass;
				V2.y *= set_fSpinImpulseMultiplier * fMass;
				V2.z *= set_fSpinImpulseMultiplier * fMass;
				//PrintUserCmdText(iClientID, L"Boink! %f %f %f", V1.x, V1.y, V1.z);
				pub::SpaceObj::AddImpulse(ci.dwTargetShip, V1, V2);
			}
		}

		if(set_bDamageNPCsCollision && !iClientIDTarget)
		{
			float fMaxHealth;
			pub::SpaceObj::GetShieldHealth(iShip, fHealthBefore, fMaxHealth, bShieldsUp);
			if(!bShieldsUp)
			{
				pub::SpaceObj::GetHealth(iShip, fHealthBefore, fMaxHealth);
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.SPObjCollision(ci, iClientID);

	try {
		float fDamage;
		if(fHealthBefore && !pub::SpaceObj::ExistsAndAlive(ci.dwTargetShip) && !pub::SpaceObj::ExistsAndAlive(iShip))
		{
			float fHealth, fHealthTarget, fMaxHealth;
			if(bShieldsUp)
			{
				pub::SpaceObj::GetShieldHealth(iShip, fHealth, fMaxHealth, bShieldsUp);
			}
			else
			{
				pub::SpaceObj::GetHealth(iShip, fHealth, fMaxHealth);
			}
			fDamage = fHealthBefore - fHealth;
			pub::SpaceObj::GetShieldHealth(ci.dwTargetShip, fHealthTarget, fMaxHealth, bShieldsUp);
			if(bShieldsUp)
			{
				if(fHealthTarget)
				{
					fHealth = fHealthTarget - fDamage;
					HkSetShieldHealth(ci.dwTargetShip, iClientID, iShip, DC_GUN, fHealth);
				}
			}
			else
			{
				pub::SpaceObj::GetHealth(ci.dwTargetShip, fHealthTarget, fMaxHealth);
				if(fHealthTarget)
				{
					fHealth = fHealthTarget - fDamage;
					HkSetHullHealth(ci.dwTargetShip, iClientID, iShip, DC_GUN, fHealth);
				}
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player has undocked and is now ready to fly
**************************************************************************************************************/

void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iShip);

	uint iClientID;
	try {
		iClientID = HkGetClientIDByShip(iShip);
		if(iClientID)
			ClientInfo[iClientID].tmSpawnTime = timeInMS(); // save for anti-dockkill
		else
			return;
		
		// event
		ProcessEvent(L"launch char=%s id=%d base=%s system=%s", 
				Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(ClientInfo[iClientID].iLastExitedBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.LaunchComplete(iBaseID, iShip);

	try {
		//For those whose carrier has died, kill and set last base
		list<MOB_UNDOCKBASEKILL>::iterator killClient = lstUndockKill.begin();
		for(killClient = lstUndockKill.begin(); killClient!=lstUndockKill.end(); killClient++)
		{
			if(iClientID==killClient->iClientID)
			{
				Players[iClientID].iPrevBaseID = killClient->iBaseID;
				if(killClient->bKill)
					HkKill(ARG_CLIENTID(iClientID));
				lstUndockKill.erase(killClient);
				return;
			}
		}
		//marking, marks are reset when docking, so re-mark them on launch
		for(uint i=0; i<ClientInfo[iClientID].vMarkedObjs.size(); i++)
		{
			if(pub::SpaceObj::ExistsAndAlive(ClientInfo[iClientID].vMarkedObjs[i]))
			{
				if(i!=ClientInfo[iClientID].vMarkedObjs.size()-1)
				{
					ClientInfo[iClientID].vMarkedObjs[i] = ClientInfo[iClientID].vMarkedObjs[ClientInfo[iClientID].vMarkedObjs.size()-1];
					i--;
				}
				ClientInfo[iClientID].vMarkedObjs.pop_back();
				continue;
			}
			pub::Player::MarkObj(iClientID, ClientInfo[iClientID].vMarkedObjs[i], 1);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player selects a character
**************************************************************************************************************/

void __stdcall CharacterSelect(struct CHARACTER_ID const & cId, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_S(&cId);
	ISERVER_LOGARG_UI(iClientID);

	uint iOldGroupID;
	try {
		//group re-add on char change
		if(set_bInviteOnCharChange)
		{
			HkGetGroupID(ARG_CLIENTID(iClientID), iOldGroupID);
		}

		//Ship bought at dealer check
		uint iShipID;
		pub::Player::GetShipID(iClientID, iShipID);
		if(ClientInfo[iClientID].iShipID && ClientInfo[iClientID].iShipID!=iShipID)
		{
			HkNewShipBought(iClientID);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	
	wstring wscCharBefore;
	try {
		const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
		wscCharBefore = wszCharname ? Players.GetActiveCharacterName(iClientID) : L"";
		ClientInfo[iClientID].iLastExitedBaseID = 0;
		Server.CharacterSelect(cId, iClientID);
	} catch(...) {
		HkAddKickLog(iClientID, L"Corrupt charfile?");
		HkKick(ARG_CLIENTID(iClientID));
		return;
	}

	try {
		wstring wscCharname = Players.GetActiveCharacterName(iClientID);

		if(wscCharBefore.compare(wscCharname) != 0)
		{
			//Remove kbeam cargo restore
			for(uint i=0; i<vRestoreKBeamClientIDs.size(); i++)
			{
				if(iClientID==vRestoreKBeamClientIDs[i])
				{
					if(i!=vRestoreKBeamClientIDs.size()-1)
					{
						vRestoreKBeamClientIDs[i] = vRestoreKBeamClientIDs[vRestoreKBeamClientIDs.size()-1];
						vRestoreKBeamCargo[i] = vRestoreKBeamCargo[vRestoreKBeamCargo.size()-1];
					}
					vRestoreKBeamClientIDs.pop_back();
					vRestoreKBeamCargo.pop_back();
					break;
				}
			}

			LoadUserCharSettings(iClientID);

			if(set_bInviteOnCharChange)
			{
				HkAddToGroup(ARG_CLIENTID(iClientID), iOldGroupID);
			}

			ClientInfo[iClientID].iShipID = 0;

			ClientInfo[iClientID].bCharInfoReqAfterDeath = false;

		}

		// anti-cheat check
		list <CARGO_INFO> lstCargo;
		int iHold;
		HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
		foreach(lstCargo, CARGO_INFO, it)
		{
			if((*it).iCount < 0)
			{
				HkAddCheaterLog(wscCharname, L"Negative good-count, likely to have cheated in the past");

				wchar_t wszBuf[256];
				swprintf(wszBuf, L"Possible cheating detected (%s)", wscCharname.c_str());
				HkMsgU(wszBuf);
				HkBan(ARG_CLIENTID(iClientID), true);
				HkKick(ARG_CLIENTID(iClientID));
			}
		}

		// event
		CAccount *acc = Players.FindAccountFromClientID(iClientID);
		wstring wscDir;
		HkGetAccountDirName(acc, wscDir);
		HKPLAYERINFO pi;
		HkGetPlayerInfo(ARG_CLIENTID(iClientID), pi, false);
		ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s", 
				wscCharname.c_str(),
				wscDir.c_str(),
				iClientID,
				pi.wscIP.c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player enters base
**************************************************************************************************************/

bool bIgnoreCancelMobDock = false; //ignore removal from carrier docked-list
void __stdcall BaseEnter(unsigned int iBaseID, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iClientID);
	
	Server.BaseEnter(iBaseID, iClientID);	

	try {
		//mobile docking - remove from target's docked list
		ClientInfo[iClientID].iLastEnteredBaseID = iBaseID;
		if(!bIgnoreCancelMobDock)
		{
			if(ClientInfo[iClientID].bMobileDocked)
			{
				wstring wscBase = ToLower(HkGetBaseNickByID(iBaseID));
				if(wscBase.find(L"mobile_proxy_base")==-1)
				{
					if(ClientInfo[iClientID].iDockClientID)
					{
						ClientInfo[ClientInfo[iClientID].iDockClientID].lstPlayersDocked.remove(iClientID);
						ClientInfo[iClientID].iDockClientID = 0;
					}
					ClientInfo[iClientID].bMobileDocked = false;
				}
			}
		}
		else
			bIgnoreCancelMobDock = false;

		//Automarking
		ClientInfo[iClientID].vAutoMarkedObjs.clear();
		ClientInfo[iClientID].vDelayedAutoMarkedObjs.clear();

		// adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
		wstring wscCharname = ToLower(Players.GetActiveCharacterName(iClientID));
		foreach(ClientInfo[iClientID].lstMoneyFix, MONEY_FIX, i)
		{
			if(!(*i).wscCharname.compare(wscCharname))
			{
				HkAddCash(wscCharname, (*i).iAmount);
				ClientInfo[iClientID].lstMoneyFix.remove(*i);
				break;
			}
		}

		// adjust cargo, this is necessary when cash was added while use was in charmenu/had other char selected
		for(list<CARGO_FIX>::iterator i2 = ClientInfo[iClientID].lstCargoFix.begin(); (i2 != ClientInfo[iClientID].lstCargoFix.end()); )
		{
			if(!(*i2).wscCharname.compare(wscCharname))
			{
				HkAddCargo(wscCharname, i2->iGoodID, i2->iCount, i2->bMission);
				i2 = ClientInfo[iClientID].lstCargoFix.erase(i2);
			}
			else
				i2++;
		}

		//add items if KillBeam was used
		if(vRestoreKBeamClientIDs.size())
		{
			int iRemaining;
			list<CARGO_INFO> lstAfterCargo;
			HkEnumCargo(ARG_CLIENTID(iClientID), lstAfterCargo, iRemaining);
			for(uint i=0; i<vRestoreKBeamClientIDs.size(); i++)
			{
				if(iClientID==vRestoreKBeamClientIDs[i])
				{
					foreach(vRestoreKBeamCargo[i], CARGO_INFO, bcargo)
					{
						if(!bcargo->bMounted) //add to cargo
						{
							bool bContinue = false;
							foreach(lstAfterCargo, CARGO_INFO, acargo) //check to see if dropped
							{
								if(bcargo->iArchID==acargo->iArchID)
								{
									bContinue = true;
									break;
								}
							}
							if(bContinue)
								continue;
							HkAddCargo(ARG_CLIENTID(iClientID), bcargo->iArchID, bcargo->iCount, bcargo->bMission);
						}
					}
					if(i!=vRestoreKBeamClientIDs.size()-1)
					{
						vRestoreKBeamClientIDs[i] = vRestoreKBeamClientIDs[vRestoreKBeamClientIDs.size()-1];
						vRestoreKBeamCargo[i] = vRestoreKBeamCargo[vRestoreKBeamCargo.size()-1];
					}
					vRestoreKBeamClientIDs.pop_back();
					vRestoreKBeamCargo.pop_back();
					break;
				}
			}
		}

		// autobuy
		// moved below
		//if(set_bAutoBuy)
		//	HkPlayerAutoBuy(iClientID, iBaseID);

		// clear damage tracking
		ClientInfo[iClientID].lstDmgRec.clear();

		//Death penalty
		HkPenalizeDeath(ARG_CLIENTID(iClientID), true);

		// event
		ProcessEvent(L"baseenter char=%s id=%d base=%s system=%s", 
				Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(iBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player exits base
**************************************************************************************************************/

void __stdcall BaseExit(unsigned int iBaseID, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iClientID);

	try {
		// autobuy
		if(set_bAutoBuy)
			HkPlayerAutoBuy(iClientID, iBaseID);

		ClientInfo[iClientID].iBaseEnterTime = 0;
		ClientInfo[iClientID].iLastExitedBaseID = iBaseID;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.BaseExit(iBaseID, iClientID);

	try {
		const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);

		// event
		ProcessEvent(L"baseexit char=%s id=%d base=%s system=%s", 
				Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(iBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}
/**************************************************************************************************************
Called when player connects
**************************************************************************************************************/

void __stdcall OnConnect(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		if(ClientInfo[iClientID].tmF1TimeDisconnect > timeInMS())
			return;

		ClientInfo[iClientID].iConnects++;
		ClearClientInfo(iClientID);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.OnConnect(iClientID);

	try {
		// event
		wstring wscIP;
		HkGetPlayerIP(iClientID, wscIP);
		ProcessEvent(L"connect id=%d ip=%s", 
				iClientID,
				wscIP.c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player disconnects
**************************************************************************************************************/
//Note that the base will ALWAYS be 0 inside of this method

void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	Vector VCharFilePos;
	wstring wscPlayerName = L"", wscSystem = L"";
	const wchar_t *wszCharname;
	bool bDeathPenaltyOnEnter;
	try {
		ClientInfo[iClientID].lstMoneyFix.clear();
		ClientInfo[iClientID].lstCargoFix.clear();

		if (ClientInfo[iClientID].bInWrapGate)   
		{
			// OMG JH disconnection HAXER, KILL KILL KILL NOW
			uint iShip;
			pub::Player::GetShip(iClientID, iShip);
			pub::SpaceObj::SetInvincible(iShip, false, false, 0);
			pub::SpaceObj::SetRelativeHealth(iShip, 0.0f); // kill the player 
		}

		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		//mobile docking
		if(ClientInfo[iClientID].bMobileDocked)
		{
			if(!iShip) //Docked at carrier
			{
				if(ClientInfo[iClientID].iDockClientID) //carrier still exists
				{
					Players[iClientID].iPrevBaseID = Players[ClientInfo[iClientID].iDockClientID].iPrevBaseID;
					uint iTargetShip;
					pub::Player::GetShip(ClientInfo[iClientID].iDockClientID, iTargetShip);
					if(iTargetShip) //carrier in space
					{
						Matrix m;
						pub::SpaceObj::GetLocation(iTargetShip, VCharFilePos, m);
						wscPlayerName = Players.GetActiveCharacterName(iClientID);
					}
					else //carrier docked
					{
						Players[iClientID].iBaseID = Players[ClientInfo[iClientID].iDockClientID].iBaseID;
					}
					wscSystem = HkGetPlayerSystem(iClientID);
					//remove from docked client list
					ClientInfo[ClientInfo[iClientID].iDockClientID].lstPlayersDocked.remove(iClientID);
				}
				else //carrier does not exist, use stored location info
				{
					if(ClientInfo[iClientID].lstJumpPath.size())
					{
						uint iSystemID;
						pub::GetSystemGateConnection(ClientInfo[iClientID].lstJumpPath.back(), iSystemID);
						wscSystem = HkGetSystemNickByID(iSystemID);
					}
					list<MOB_UNDOCKBASEKILL>::iterator killClient = lstUndockKill.begin(); //Find the last_base
					for(killClient = lstUndockKill.begin(); killClient!=lstUndockKill.end(); killClient++)
					{
						if(iClientID==killClient->iClientID)
						{
							Players[iClientID].iBaseID = killClient->iBaseID;
							Players[iClientID].iPrevBaseID = killClient->iBaseID;
							lstUndockKill.erase(killClient);
							break;
						}
					}
					VCharFilePos = ClientInfo[iClientID].Vlaunch;
					wscPlayerName = Players.GetActiveCharacterName(iClientID);
				}
			}
			else //in space, update last base only
			{
				if(ClientInfo[iClientID].iDockClientID) //carrier still exists
				{
					Players[iClientID].iPrevBaseID = Players[ClientInfo[iClientID].iDockClientID].iPrevBaseID;
				}
			}
		}
		if(ClientInfo[iClientID].lstPlayersDocked.size())
		{
			Matrix m;
			Vector v;
			uint iShip;
			pub::Player::GetShip(iClientID, iShip);
			if(iShip)
				pub::SpaceObj::GetLocation(iShip, v, m);
			foreach(ClientInfo[iClientID].lstPlayersDocked, uint, dockedClientID) //go through all of the docked players and deal with them
			{
				uint iDockedShip;
				pub::Player::GetShip(*dockedClientID, iDockedShip);
				if(iDockedShip) //player is in space
				{
					Players[*dockedClientID].iPrevBaseID = iShip ? Players[iClientID].iPrevBaseID : Players[iClientID].iBaseID;
					if(!ClientInfo[*dockedClientID].lstJumpPath.size())
					{
						ClientInfo[*dockedClientID].bMobileDocked = false;
					}
				}
				else //player is docked
				{
					if(iShip) //carrier is in space
					{
						ClientInfo[*dockedClientID].Vlaunch = v;
						ClientInfo[*dockedClientID].Mlaunch = m;
						MOB_UNDOCKBASEKILL dKill;
						dKill.iClientID = *dockedClientID;
						dKill.iBaseID = Players[iClientID].iPrevBaseID;
						dKill.bKill = false;
						lstUndockKill.push_back(dKill);
					}
					else //carrier is docked
					{
						uint iTargetShip = ClientInfo[iClientID].iLastSpaceObjDocked;
						MOB_UNDOCKBASEKILL dKill;
						dKill.iClientID = *dockedClientID;
						dKill.iBaseID = ClientInfo[iClientID].iLastEnteredBaseID;
						dKill.bKill = false;
						lstUndockKill.push_back(dKill);
						if(iTargetShip) //got the spaceObj of the base alright
						{
							uint iType;
							pub::SpaceObj::GetType(iTargetShip, iType);
							pub::SpaceObj::GetLocation(iTargetShip, ClientInfo[*dockedClientID].Vlaunch, ClientInfo[*dockedClientID].Mlaunch);
							if(iType==32)
							{
								ClientInfo[*dockedClientID].Mlaunch.data[0][0] = -ClientInfo[*dockedClientID].Mlaunch.data[0][0];
								ClientInfo[*dockedClientID].Mlaunch.data[1][0] = -ClientInfo[*dockedClientID].Mlaunch.data[1][0];
								ClientInfo[*dockedClientID].Mlaunch.data[2][0] = -ClientInfo[*dockedClientID].Mlaunch.data[2][0];
								ClientInfo[*dockedClientID].Mlaunch.data[0][2] = -ClientInfo[*dockedClientID].Mlaunch.data[0][2];
								ClientInfo[*dockedClientID].Mlaunch.data[1][2] = -ClientInfo[*dockedClientID].Mlaunch.data[1][2];
								ClientInfo[*dockedClientID].Mlaunch.data[2][2] = -ClientInfo[*dockedClientID].Mlaunch.data[2][2];
								ClientInfo[*dockedClientID].Vlaunch.x += ClientInfo[*dockedClientID].Mlaunch.data[0][0]*90;
								ClientInfo[*dockedClientID].Vlaunch.y += ClientInfo[*dockedClientID].Mlaunch.data[1][0]*90;
								ClientInfo[*dockedClientID].Vlaunch.z += ClientInfo[*dockedClientID].Mlaunch.data[2][0]*90;
							}
							else
							{
								ClientInfo[*dockedClientID].Vlaunch.x += ClientInfo[*dockedClientID].Mlaunch.data[0][1]*set_iMobileDockOffset;
								ClientInfo[*dockedClientID].Vlaunch.y += ClientInfo[*dockedClientID].Mlaunch.data[1][1]*set_iMobileDockOffset;
								ClientInfo[*dockedClientID].Vlaunch.z += ClientInfo[*dockedClientID].Mlaunch.data[2][1]*set_iMobileDockOffset;
							}
						}
						else //backup: set player's base to that of target, hope they won't get kicked (this shouldn't happen)
						{
							Players[*dockedClientID].iBaseID = Players[iClientID].iBaseID;
						}
					}
				}
				ClientInfo[*dockedClientID].iDockClientID = 0;
			}
			ClientInfo[iClientID].lstPlayersDocked.clear();
		}

		//Remove kbeam cargo restore
		for(uint i=0; i<vRestoreKBeamClientIDs.size(); i++)
		{
			if(iClientID==vRestoreKBeamClientIDs[i])
			{
				if(i!=vRestoreKBeamClientIDs.size()-1)
				{
					vRestoreKBeamClientIDs[i] = vRestoreKBeamClientIDs[vRestoreKBeamClientIDs.size()-1];
					vRestoreKBeamCargo[i] = vRestoreKBeamCargo[vRestoreKBeamCargo.size()-1];
				}
				vRestoreKBeamClientIDs.pop_back();
				vRestoreKBeamCargo.pop_back();
				break;
			}
		}

		//Check the undock kill list, just in case
		foreach(lstUndockKill, MOB_UNDOCKBASEKILL, bkill)
		{
			if(bkill->iClientID == iClientID)
			{
				lstUndockKill.erase(bkill);
				break;
			}
		}

		if(!iShip)
		{
			uint iShipID;
			pub::Player::GetShipID(iClientID, iShipID);
			if(ClientInfo[iClientID].iShipID && ClientInfo[iClientID].iShipID!=iShipID)
			{
				HkNewShipBought(iClientID);
			}
		}

		bDeathPenaltyOnEnter = ClientInfo[iClientID].bDeathPenaltyOnEnter;

		if(!ClientInfo[iClientID].bDisconnected)
		{
			ClientInfo[iClientID].bDisconnected = true;

			// event
			wszCharname = Players.GetActiveCharacterName(iClientID);
			ProcessEvent(L"disconnect char=%s id=%d", 
					(wszCharname ? wszCharname : L""), 
					iClientID);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.DisConnect(iClientID, p2);

	try{
		//mobile docking - add position, system to char file
		if(wscPlayerName.length())
		{
			list<wstring> lstCharFile;
			if(HKHKSUCCESS(HkReadCharFile(wscPlayerName, lstCharFile)))
			{
				wchar_t wszPos[32];
				swprintf(wszPos, L"pos = %f, %f, %f", VCharFilePos.x, VCharFilePos.y, VCharFilePos.z);
				list<wstring>::iterator line = lstCharFile.begin();
				wstring wscCharFile = L"";
				bool bReplacedBase = false, bFoundPos = false, bFoundSystem = wscSystem.length() ? false : true;
				for(line = lstCharFile.begin(); line!=lstCharFile.end(); line++)
				{
					wstring wscNewLine = *line;
					if(!bReplacedBase && line->find(L"base")==0)
					{
						wscNewLine = L"last_" + *line;
						bReplacedBase = true;
						continue; //for now
					}
					if(!bFoundPos && line->find(L"system")==0)
					{
						if(!bFoundSystem)
						{
							wscNewLine = L"system = " + wscSystem;
							bFoundSystem = true;
						}
						wscNewLine += L"\n";
						wscNewLine += wszPos;
						bFoundPos = true;
					}
					wscCharFile += wscNewLine + L"\n";
				}
				wscCharFile.substr(0, wscCharFile.length()-1);
				HkWriteCharFile(wscPlayerName, wscCharFile);
			}
		}

		if(bDeathPenaltyOnEnter)
		{
			HkPenalizeDeath(wszCharname, true);
		}

	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when trade is being terminated
**************************************************************************************************************/

void __stdcall TerminateTrade(unsigned int iClientID, int iAccepted)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_I(iAccepted);

	Server.TerminateTrade(iClientID, iAccepted);

	try {
		if(iAccepted)
		{ // save both chars to prevent cheating in case of server crash
			HkSaveChar(ARG_CLIENTID(iClientID));
			if(ClientInfo[iClientID].iTradePartner)
				HkSaveChar(ARG_CLIENTID(ClientInfo[iClientID].iTradePartner));
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when new trade request
**************************************************************************************************************/

void __stdcall InitiateTrade(unsigned int iClientID1, unsigned int iClientID2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID1);
	ISERVER_LOGARG_UI(iClientID2);

	try {
		// save traders client-ids
		ClientInfo[iClientID1].iTradePartner = iClientID2;
		ClientInfo[iClientID2].iTradePartner = iClientID1;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.InitiateTrade(iClientID1, iClientID2);
}

/**************************************************************************************************************
Called when equipment is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateEquip(unsigned int iClientID, struct XActivateEquip const &aq)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		if((aq.sID == 0x23) && !aq.bActivate)
			ClientInfo[iClientID].bCruiseActivated = false; // enginekill enabled

		ClientInfo[iClientID].bEngineKilled = !aq.bActivate;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.ActivateEquip(iClientID, aq);
}

/**************************************************************************************************************
Called when cruise engine is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateCruise(unsigned int iClientID, struct XActivateCruise const &ac)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bCruiseActivated = ac.bActivate;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.ActivateCruise(iClientID, ac);
}

/**************************************************************************************************************
Called when thruster is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateThrusters(unsigned int iClientID, struct XActivateThrusters const &at)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bThrusterActivated = at.bActivate;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	Server.ActivateThrusters(iClientID, at);
}

/**************************************************************************************************************
Called when player sells good on a base
**************************************************************************************************************/

void __stdcall GFGoodSell(struct SGFGoodSellInfo const &gsi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		// anti base-idle
		ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);

		// anti-cheat check
		list <CARGO_INFO> lstCargo;
		int iHold;
		HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
		foreach(lstCargo, CARGO_INFO, it)
		{
			if(((*it).iArchID == gsi.iArchID) && (abs(gsi.iCount) > (*it).iCount))
			{
				const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
				HkAddCheaterLog(wszCharname, L"Sold more good than possible");

				wchar_t wszBuf[256];
				swprintf(wszBuf, L"Possible cheating detected (%s)", wszCharname);
				HkMsgU(wszBuf);
				HkBan(ARG_CLIENTID(iClientID), true);
				HkKick(ARG_CLIENTID(iClientID));
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.GFGoodSell(gsi, iClientID);
}

/**************************************************************************************************************
Called when player connects or pushes f1 or respawns after dying
**************************************************************************************************************/

void __stdcall CharacterInfoReq(unsigned int iClientID, bool p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	Vector VCharFilePos;
	wstring wscPlayerName = L"", wscSystem = L"";
	try {
		if (ClientInfo[iClientID].bInWrapGate)   
		{
			// OMG JH F1 HAXER, KILL KILL KILL NOW
			uint iShip;
			pub::Player::GetShip(iClientID, iShip);
			//if(iShip)
			pub::SpaceObj::SetInvincible(iShip, false, false, 0);
			pub::SpaceObj::SetRelativeHealth(iShip, 0.0f); // kills the player
		}

		if(!ClientInfo[iClientID].bCharSelected)
			ClientInfo[iClientID].bCharSelected = true;
		else { // pushed f1
			uint iShip = 0;
			pub::Player::GetShip(iClientID, iShip);

			//mobile docking
			if(!ClientInfo[iClientID].bCharInfoReqAfterDeath)
			{
				if(ClientInfo[iClientID].bMobileDocked)
				{
					if(!iShip) //Docked at carrier
					{
						if(ClientInfo[iClientID].iDockClientID) //carrier still exists
						{
							Players[iClientID].iPrevBaseID = Players[ClientInfo[iClientID].iDockClientID].iPrevBaseID;
							uint iTargetShip;
							pub::Player::GetShip(ClientInfo[iClientID].iDockClientID, iTargetShip);
							if(iTargetShip) //carrier in space
							{
								Matrix m;
								pub::SpaceObj::GetLocation(iTargetShip, VCharFilePos, m);
								wscPlayerName = Players.GetActiveCharacterName(iClientID);
							}
							else //carrier docked
							{
								Players[iClientID].iBaseID = Players[ClientInfo[iClientID].iDockClientID].iBaseID;
							}
							wscSystem = HkGetPlayerSystem(iClientID);
							//remove from docked client list
							ClientInfo[ClientInfo[iClientID].iDockClientID].lstPlayersDocked.remove(iClientID);
						}
						else //carrier does not exist, use stored location info
						{
							if(ClientInfo[iClientID].lstJumpPath.size())
							{
								uint iSystemID;
								pub::GetSystemGateConnection(ClientInfo[iClientID].lstJumpPath.back(), iSystemID);
								wscSystem = HkGetSystemNickByID(iSystemID);
							}
							list<MOB_UNDOCKBASEKILL>::iterator killClient = lstUndockKill.begin(); //Find the last_base
							for(killClient = lstUndockKill.begin(); killClient!=lstUndockKill.end(); killClient++)
							{
								if(iClientID==killClient->iClientID)
								{
									Players[iClientID].iBaseID = killClient->iBaseID;
									Players[iClientID].iPrevBaseID = killClient->iBaseID;
									lstUndockKill.erase(killClient);
									break;
								}
							}
							VCharFilePos = ClientInfo[iClientID].Vlaunch;
							wscPlayerName = Players.GetActiveCharacterName(iClientID);
						}
					}
					else //in space, update last base only
					{
						if(ClientInfo[iClientID].iDockClientID) //carrier still exists
						{
							Players[iClientID].iPrevBaseID = Players[ClientInfo[iClientID].iDockClientID].iPrevBaseID;
						}
					}
				}
				if(ClientInfo[iClientID].lstPlayersDocked.size())
				{
					Matrix m;
					Vector v;
					uint iShip;
					pub::Player::GetShip(iClientID, iShip);
					if(iShip)
						pub::SpaceObj::GetLocation(iShip, v, m);
					foreach(ClientInfo[iClientID].lstPlayersDocked, uint, dockedClientID) //go through all of the docked players and deal with them
					{
						uint iDockedShip;
						pub::Player::GetShip(*dockedClientID, iDockedShip);
						if(iDockedShip) //player is in space
						{
							Players[*dockedClientID].iPrevBaseID = iShip ? Players[iClientID].iPrevBaseID : Players[iClientID].iBaseID;
							if(!ClientInfo[*dockedClientID].lstJumpPath.size())
							{
								ClientInfo[*dockedClientID].bMobileDocked = false;
							}
						}
						else //player is docked
						{
							if(iShip) //carrier is in space
							{
								ClientInfo[*dockedClientID].Vlaunch = v;
								ClientInfo[*dockedClientID].Mlaunch = m;
								MOB_UNDOCKBASEKILL dKill;
								dKill.iClientID = *dockedClientID;
								dKill.iBaseID = Players[iClientID].iPrevBaseID;
								dKill.bKill = false;
								lstUndockKill.push_back(dKill);
							}
							else //carrier is docked
							{
								uint iTargetShip = ClientInfo[iClientID].iLastSpaceObjDocked;
								MOB_UNDOCKBASEKILL dKill;
								dKill.iClientID = *dockedClientID;
								dKill.iBaseID = ClientInfo[iClientID].iLastEnteredBaseID;
								dKill.bKill = false;
								lstUndockKill.push_back(dKill);
								if(iTargetShip) //got the spaceObj of the base alright
								{
									uint iType;
									pub::SpaceObj::GetType(iTargetShip, iType);
									pub::SpaceObj::GetLocation(iTargetShip, ClientInfo[*dockedClientID].Vlaunch, ClientInfo[*dockedClientID].Mlaunch);
									if(iType==32)
									{
										ClientInfo[*dockedClientID].Mlaunch.data[0][0] = -ClientInfo[*dockedClientID].Mlaunch.data[0][0];
										ClientInfo[*dockedClientID].Mlaunch.data[1][0] = -ClientInfo[*dockedClientID].Mlaunch.data[1][0];
										ClientInfo[*dockedClientID].Mlaunch.data[2][0] = -ClientInfo[*dockedClientID].Mlaunch.data[2][0];
										ClientInfo[*dockedClientID].Mlaunch.data[0][2] = -ClientInfo[*dockedClientID].Mlaunch.data[0][2];
										ClientInfo[*dockedClientID].Mlaunch.data[1][2] = -ClientInfo[*dockedClientID].Mlaunch.data[1][2];
										ClientInfo[*dockedClientID].Mlaunch.data[2][2] = -ClientInfo[*dockedClientID].Mlaunch.data[2][2];
										ClientInfo[*dockedClientID].Vlaunch.x += ClientInfo[*dockedClientID].Mlaunch.data[0][0]*90;
										ClientInfo[*dockedClientID].Vlaunch.y += ClientInfo[*dockedClientID].Mlaunch.data[1][0]*90;
										ClientInfo[*dockedClientID].Vlaunch.z += ClientInfo[*dockedClientID].Mlaunch.data[2][0]*90;
									}
									else
									{
										ClientInfo[*dockedClientID].Vlaunch.x += ClientInfo[*dockedClientID].Mlaunch.data[0][1]*set_iMobileDockOffset;
										ClientInfo[*dockedClientID].Vlaunch.y += ClientInfo[*dockedClientID].Mlaunch.data[1][1]*set_iMobileDockOffset;
										ClientInfo[*dockedClientID].Vlaunch.z += ClientInfo[*dockedClientID].Mlaunch.data[2][1]*set_iMobileDockOffset;
									}
								}
								else //backup: set player's base to that of target, hope they won't get kicked (this shouldn't happen)
								{
									Players[*dockedClientID].iBaseID = Players[iClientID].iBaseID;
								}
							}
						}
						ClientInfo[*dockedClientID].iDockClientID = 0;
					}
					ClientInfo[iClientID].lstPlayersDocked.clear();
				}
			}
			else //Player respawning
			{
				ClientInfo[iClientID].bCharInfoReqAfterDeath = false;
			}

			if(iShip)
			{ // in space
				ClientInfo[iClientID].tmF1Time = timeInMS() + set_iAntiF1;
				return;
			}

		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	
	try {
//		HkAddConnectLog(iClientID);
		Server.CharacterInfoReq(iClientID, p2);
	} catch(...) { // something is wrong with charfile
		HkAddKickLog(iClientID, L"Corrupt charfile?");
		HkKick(ARG_CLIENTID(iClientID));
		return;
	}

	try{
		//mobile docking - add position to char file
		if(wscPlayerName.length())
		{
			list<wstring> lstCharFile;
			if(HKHKSUCCESS(HkReadCharFile(wscPlayerName, lstCharFile)))
			{
				wchar_t wszPos[32];
				swprintf(wszPos, L"pos = %f, %f, %f", VCharFilePos.x, VCharFilePos.y, VCharFilePos.z);
				list<wstring>::iterator line = lstCharFile.begin();
				wstring wscCharFile = L"";
				bool bReplacedBase = false, bFoundPos = false, bFoundSystem = wscSystem.length() ? false : true;
				for(line = lstCharFile.begin(); line!=lstCharFile.end(); line++)
				{
					wstring wscNewLine = *line;
					if(!bReplacedBase && line->find(L"base")==0)
					{
						wscNewLine = L"last_" + *line;
						bReplacedBase = true;
						continue; //for now
					}
					if(!bFoundPos && line->find(L"system")==0)
					{
						if(!bFoundSystem)
						{
							wscNewLine = L"system = " + wscSystem;
							bFoundSystem = true;
						}
						wscNewLine += L"\n";
						wscNewLine += wszPos;
						bFoundPos = true;
					}
					wscCharFile += wscNewLine + L"\n";
				}
				wscCharFile.substr(0, wscCharFile.length()-1);
				HkWriteCharFile(wscPlayerName, wscCharFile);
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player jumps in system
**************************************************************************************************************/

void __stdcall JumpInComplete(unsigned int iSystemID, unsigned int iShip)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iSystemID);
	ISERVER_LOGARG_UI(iShip);

	Server.JumpInComplete(iSystemID, iShip);

	try {
		uint iClientID = HkGetClientIDByShip(iShip);
		if(!iClientID)
			return;

		//mobile docking - update last base to that of the proxy base in new system for those docked but in space, also fix up jump path
		if(ClientInfo[iClientID].lstPlayersDocked.size())
		{
			string scBase = HkGetPlayerSystemS(iClientID) + "_Mobile_Proxy_Base";
			uint iBaseID = 0;
			pub::GetBaseID(iBaseID, (scBase).c_str());
			if(iBaseID)
			{
				foreach(ClientInfo[iClientID].lstPlayersDocked, uint, dockedClientID)
				{
					Players[*dockedClientID].iPrevBaseID = iBaseID;
					for(list<uint>::iterator jumpObj = ClientInfo[*dockedClientID].lstJumpPath.begin(); jumpObj!=ClientInfo[*dockedClientID].lstJumpPath.end(); jumpObj++)
					{
						uint iJumpSystemID;
						pub::SpaceObj::GetSystem(*jumpObj, iJumpSystemID);
						if(iJumpSystemID==iSystemID)
						{
							ClientInfo[*dockedClientID].lstJumpPath.erase(jumpObj, ClientInfo[*dockedClientID].lstJumpPath.end());
							break;
						}
					}
				}
			}
		}
		// mobile docking switching systems
		if(ClientInfo[iClientID].lstJumpPath.size()) //player is traveling to carrier
		{
			ClientInfo[iClientID].bPathJump = true;
			pub::SpaceObj::InstantDock(iShip, ClientInfo[iClientID].lstJumpPath.front(), 1);
			ClientInfo[iClientID].lstJumpPath.pop_front();
			return;
		}
		else
		{
			if(ClientInfo[iClientID].bPathJump)
			{
				if(ClientInfo[iClientID].iDockClientID) //carrier still exists
				{
					uint iTargetShip;
					pub::Player::GetShip(ClientInfo[iClientID].iDockClientID, iTargetShip);
					if(iTargetShip) //carrier in space
					{
						Vector vBeam;
						Matrix mBeam;
						pub::SpaceObj::GetLocation(iTargetShip, vBeam, mBeam);
						vBeam.x += mBeam.data[0][1]*set_iMobileDockOffset;
						vBeam.y += mBeam.data[1][1]*set_iMobileDockOffset;
						vBeam.z += mBeam.data[2][1]*set_iMobileDockOffset;
						HkBeamInSys(ARG_CLIENTID(iClientID), vBeam, mBeam);
					}
					else //carrier docked
					{
						uint iTargetShip = ClientInfo[ClientInfo[iClientID].iDockClientID].iLastSpaceObjDocked;
						if(iTargetShip) //got the spaceObj of the base alright
						{
							uint iType;
							pub::SpaceObj::GetType(iTargetShip, iType);
							Vector vBeam;
							Matrix mBeam;
							pub::SpaceObj::GetLocation(iTargetShip, vBeam, mBeam);
							if(iType==32)
							{
								mBeam.data[0][0] = -mBeam.data[0][0];
								mBeam.data[1][0] = -mBeam.data[1][0];
								mBeam.data[2][0] = -mBeam.data[2][0];
								mBeam.data[0][2] = -mBeam.data[0][2];
								mBeam.data[1][2] = -mBeam.data[1][2];
								mBeam.data[2][2] = -mBeam.data[2][2];
								vBeam.x += mBeam.data[0][0]*90;
								vBeam.y += mBeam.data[1][0]*90;
								vBeam.z += mBeam.data[2][0]*90;
							}
							else
							{
								vBeam.x += mBeam.data[0][1]*set_iMobileDockOffset;
								vBeam.y += mBeam.data[1][1]*set_iMobileDockOffset;
								vBeam.z += mBeam.data[2][1]*set_iMobileDockOffset;
							}
							HkBeamInSys(ARG_CLIENTID(iClientID), vBeam, mBeam);
						}
					}
				}
				else //carrier does not exist, use stored info
				{
					HkBeamInSys(ARG_CLIENTID(iClientID), ClientInfo[iClientID].Vlaunch, ClientInfo[iClientID].Mlaunch);
					ClientInfo[iClientID].bMobileDocked = false;
				}
				ClientInfo[iClientID].bPathJump = false;
			}
		}

		ClientInfo[iClientID].bInWrapGate = false;

		if(ClientInfo[iClientID].bCanCloak && (ClientInfo[iClientID].bCloaked || ClientInfo[iClientID].bIsCloaking)) 
			ClientInfo[iClientID].bMustSendUncloak = true;

		//Make player damageable
		pub::SpaceObj::SetInvincible(iShip, false, false, 0);

		//marking - check to see if any of the delayed marks correspond to new system
		vector<uint> vTempMark;
		for(uint i=0; i<ClientInfo[iClientID].vDelayedSystemMarkedObjs.size(); i++)
		{
			if(pub::SpaceObj::ExistsAndAlive(ClientInfo[iClientID].vDelayedSystemMarkedObjs[i]))
			{
				if(i!=ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1)
				{
					ClientInfo[iClientID].vDelayedSystemMarkedObjs[i] = ClientInfo[iClientID].vDelayedSystemMarkedObjs[ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1];
					i--;
				}
				ClientInfo[iClientID].vDelayedSystemMarkedObjs.pop_back();
				continue;
			}

			uint iTargetSystem;
			pub::SpaceObj::GetSystem(ClientInfo[iClientID].vDelayedSystemMarkedObjs[i], iTargetSystem);
			if(iTargetSystem==iSystemID)
			{
				pub::Player::MarkObj(iClientID, ClientInfo[iClientID].vDelayedSystemMarkedObjs[i], 1);
				vTempMark.push_back(ClientInfo[iClientID].vDelayedSystemMarkedObjs[i]);
				if(i!=ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1)
				{
					ClientInfo[iClientID].vDelayedSystemMarkedObjs[i] = ClientInfo[iClientID].vDelayedSystemMarkedObjs[ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1];
					i--;
				}
				ClientInfo[iClientID].vDelayedSystemMarkedObjs.pop_back();
			}
		}
		for(i=0; i<ClientInfo[iClientID].vMarkedObjs.size(); i++)
		{
			if(!pub::SpaceObj::ExistsAndAlive(ClientInfo[iClientID].vMarkedObjs[i]))
				ClientInfo[iClientID].vDelayedSystemMarkedObjs.push_back(ClientInfo[iClientID].vMarkedObjs[i]);
		}
		ClientInfo[iClientID].vMarkedObjs = vTempMark;

		//Death Penalty - calculate DP and display message to character if switching from a system on the exclude list
		if(set_fDeathPenalty && !ClientInfo[iClientID].iDeathPenaltyCredits)
		{
			UINT_WRAP uwShip = UINT_WRAP(Players[iClientID].iShipArchID);
			UINT_WRAP uwSystem = UINT_WRAP(Players[iClientID].iSystemID);
			if(!set_btNoDeathPenalty->Find(&uwShip) && !set_btNoDPSystems->Find(&uwSystem))
			{
				int iCash;
				HkGetCash(ARG_CLIENTID(iClientID), iCash);
				float fValue;
				pub::Player::GetAssetValue(iClientID, fValue);
				ClientInfo[iClientID].iDeathPenaltyCredits = (int)(fValue * set_fDeathPenalty);
				if(ClientInfo[iClientID].bDisplayDPOnLaunch)
					PrintUserCmdText(iClientID, L"Notice: the death penalty for your ship will be %i credits.  Type /dp for more information.", ClientInfo[iClientID].iDeathPenaltyCredits);
			}
		}

		// event
		ProcessEvent(L"jumpin char=%s id=%d system=%s", 
				Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetSystemNickByID(iSystemID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player jumps out of system
**************************************************************************************************************/

void __stdcall SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(iClientID);

	try {
		//mobile docking
		if(ClientInfo[iClientID].lstPlayersDocked.size())
		{
			uint iTarget;
			pub::SpaceObj::GetTarget(iShip, iTarget);
			foreach(ClientInfo[iClientID].lstPlayersDocked, uint, dockedClientID)
			{
				uint iDockedShip;
				pub::Player::GetShip(*dockedClientID, iDockedShip);
				if(!iDockedShip)
					ClientInfo[*dockedClientID].lstJumpPath.push_back(iTarget);
			}
		}

		// event
		ProcessEvent(L"switchout char=%s id=%d system=%s", 
				Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	//Make player invincible to fix JHs/JGs near mine fields sometimes exploding player while jumping (in jump tunnel)
	pub::SpaceObj::SetInvincible(iShip, true, true, 0);

    ClientInfo[iClientID].bInWrapGate = true;

	Server.SystemSwitchOutComplete(iShip, iClientID);
}

/**************************************************************************************************************
Called when player logs in
**************************************************************************************************************/

void __stdcall Login(struct SLoginInfo const &li, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_WS(&li);
	ISERVER_LOGARG_UI(iClientID);

	Server.Login(li, iClientID);

	try {
		if(iClientID > Players.GetMaxPlayerCount())
			return; // lalala DisconnectDelay bug

		if(!HkIsValidClientID(iClientID))
			return; // player was kicked

		// check for ip ban
		wstring wscIP;
		HkGetPlayerIP(iClientID, wscIP);

		foreach(set_lstBans, wstring, itb)
		{
			if(Wildcard::wildcardfit(wstos(*itb).c_str(), wstos(wscIP).c_str()))
			{
				HkAddKickLog(iClientID, L"IP/Hostname ban(%s matches %s)", wscIP.c_str(), (*itb).c_str());
				if(set_bBanAccountOnMatch)
					HkBan(ARG_CLIENTID(iClientID), true);
				HkKick(ARG_CLIENTID(iClientID));
			}
		}

		// resolve
		RESOLVE_IP rip;
		rip.wscIP = wscIP;
		rip.wscHostname = L"";
		rip.iConnects = ClientInfo[iClientID].iConnects; // security check so that wrong person doesnt get banned
		rip.iClientID = iClientID;
		EnterCriticalSection(&csIPResolve);
		g_lstResolveIPs.push_back(rip);
		LeaveCriticalSection(&csIPResolve);

		// count players
		struct PlayerData *pPD = 0;
		uint iPlayers = 0;
		while(pPD = Players.traverse_active(pPD))
			iPlayers++;

		if(iPlayers > (Players.GetMaxPlayerCount() -  set_iReservedSlots))
		{ // check if player has a reserved slot
			CAccount *acc = Players.FindAccountFromClientID(iClientID);
			wstring wscDir; 
			HkGetAccountDirName(acc, wscDir); 
			string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

			bool bReserved = IniGetB(scUserFile, "Settings", "ReservedSlot", false);
			if(!bReserved)
			{
				HkKick(ARG_CLIENTID(iClientID));
				return;
			}
		}

		LoadUserSettings(iClientID);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called on item spawn
**************************************************************************************************************/

void __stdcall MineAsteroid(unsigned int p1, class Vector const &vPos, unsigned int iLookID, unsigned int iGoodID, unsigned int iCount, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_UI(vPos);
	ISERVER_LOGARG_UI(iLookID);
	ISERVER_LOGARG_UI(iGoodID);
	ISERVER_LOGARG_UI(iCount);
	ISERVER_LOGARG_UI(iClientID);

	/*try {
		PrintUniverseText(L"MineAsteroid %u %u %u %u %u", p1, iLookID, iGoodID, iCount, iClientID);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }*/

	Server.MineAsteroid(p1, vPos, iLookID, iGoodID, iCount, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GoTradelane(unsigned int iClientID, struct XGoTradelane const &gtl)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bTradelane = true;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	Server.GoTradelane(iClientID, gtl);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall StopTradelane(unsigned int iClientID, unsigned int p2, unsigned int p3, unsigned int p4)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);

	try {
		ClientInfo[iClientID].bTradelane = false;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.StopTradelane(iClientID, p2, p3, p4);
}

////////////////////////////////////////////////////////

void __stdcall AbortMission(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.AbortMission(p1, p2);
}

void __stdcall AcceptTrade(unsigned int iClientID, bool p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	Server.AcceptTrade(iClientID, p2);
}

/**************************************************************************************************************
Called when player adds an item to a trade request
**************************************************************************************************************/
void __stdcall AddTradeEquip(unsigned int iClientID, struct EquipDesc const &ed)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.AddTradeEquip(iClientID, ed);

	try {
		UINT_WRAP uw = UINT_WRAP(ed.archid);
		if(set_btNoTrade->Find(&uw))
		{
			pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("objective_failed"));
			pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("none_available"));
			Server.TerminateTrade(iClientID, 0); //cancel trade
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

}

void __stdcall BaseInfoRequest(unsigned int p1, unsigned int p2, bool p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	Server.BaseInfoRequest(p1, p2, p3);
}

void __stdcall CharacterSkipAutosave(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.CharacterSkipAutosave(iClientID);
}

void __stdcall CommComplete(unsigned int p1, unsigned int p2, unsigned int p3,enum CommResult cr)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(cr);

	Server.CommComplete(p1, p2, p3, cr);
}

void __stdcall Connect(char const *p1, unsigned short *p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_WS(p2);

	Server.Connect(p1, p2); // doesn't do anything
}

void __stdcall CreateNewCharacter(struct SCreateCharacterInfo const & scci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.CreateNewCharacter(scci, iClientID);
}

void __stdcall DelTradeEquip(unsigned int iClientID, struct EquipDesc const &ed)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.DelTradeEquip(iClientID, ed);
}

void __stdcall DestroyCharacter(struct CHARACTER_ID const &cId, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_S(&cId);

	Server.DestroyCharacter(cId, iClientID);
}

void __stdcall Dock(unsigned int const &p1, unsigned int const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.Dock(p1, p2);
}

void __stdcall DumpPacketStats(char const *p1)
{
	ISERVER_LOG();

	Server.DumpPacketStats(p1);
}

void __stdcall ElapseTime(float p1)
{
	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);

	Server.ElapseTime(p1);
}

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.GFGoodBuy(gbi, iClientID);
}

void __stdcall GFGoodVaporized(struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.GFGoodVaporized(gvi, iClientID);
}

void __stdcall GFObjSelect(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.GFObjSelect(p1, p2);
}

unsigned int __stdcall GetServerID(void)
{
	ISERVER_LOG();

	return Server.GetServerID();
}

char const * __stdcall GetServerSig(void)
{
	ISERVER_LOG();

	return Server.GetServerSig();
}

void __stdcall GetServerStats(struct ServerStats &ss)
{
	ISERVER_LOG();

	Server.GetServerStats(ss);
}

void __stdcall Hail(unsigned int p1, unsigned int p2, unsigned int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	Server.Hail(p1, p2, p3);
}

void __stdcall InterfaceItemUsed(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.InterfaceItemUsed(p1, p2);
}

void __stdcall JettisonCargo(unsigned int iClientID, struct XJettisonCargo const &jc)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.JettisonCargo(iClientID, jc);
}

/**************************************************************************************************************
Called when player enters a room on a base
**************************************************************************************************************/
void __stdcall LocationEnter(unsigned int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.LocationEnter(p1, iClientID);

	try {
		// anti base-idle
		ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Called when player exits a room on a base
**************************************************************************************************************/
void __stdcall LocationExit(unsigned int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.LocationExit(p1, iClientID);

	try {
		uint iShipID;
		pub::Player::GetShipID(iClientID, iShipID);
		if(ClientInfo[iClientID].iShipID && ClientInfo[iClientID].iShipID!=iShipID)
		{
			HkNewShipBought(iClientID);
		}
		ClientInfo[iClientID].iShipID = iShipID;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

void __stdcall LocationInfoRequest(unsigned int p1,unsigned int p2, bool p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	Server.LocationInfoRequest(p1, p2, p3);
}

void __stdcall MissionResponse(unsigned int p1, unsigned long p2, bool p3, unsigned int p4)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);

	Server.MissionResponse(p1, p2, p3, p4);
}

void __stdcall MissionSaveB(unsigned int iClientID, unsigned long p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	Server.MissionSaveB(iClientID, p2);
}

void __stdcall NewCharacterInfoReq(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.NewCharacterInfoReq(iClientID); // doesn't do anything
}

void __stdcall PopUpDialog(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.PopUpDialog(p1, p2);
}

void __stdcall PushToServer(class CDAPacket *packet)
{
	ISERVER_LOG();

	Server.PushToServer(packet); // doesn't do anything
}

void __stdcall RTCDone(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.RTCDone(p1, p2);
}

/**************************************************************************************************************
Called when an item (Commodity, equipment) is bought
**************************************************************************************************************/
/*p1 = GoodID
p2 = ?, seems to have something to do with the hardpoint
p3 = number of items
p4 = health
p5 = mounted
p6 = client-id*/
void __stdcall ReqAddItem(unsigned int p1, char const *p2, int p3, float p4, bool p5, unsigned int p6)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_F(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(p6);

	bool bFoundItemRestriction = false;
	try {
		if(p5)
		{
			MOUNT_RESTRICTION mrFind = MOUNT_RESTRICTION(p1);
			MOUNT_RESTRICTION *mrFound = set_btMRestrict->Find(&mrFind);
			/*Check to make sure ship can mount good*/
			if(mrFound)
			{
				uint iShipArchID;
				pub::Player::GetShipID(p6, iShipArchID);
				UINT_WRAP uw = UINT_WRAP(iShipArchID);
				if(!mrFound->btShipArchIDs->Find(&uw))
				{
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("cancelled"));
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("loaded_into_cargo_hold"));
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("no_place_to_mount"));
					p5 = false; //block mount
				}
			}
		}

		//Item restrictions
		ITEM_RESTRICT irFind = ITEM_RESTRICT(p1);
		ITEM_RESTRICT *irFound = set_btItemRestrictions->Find(&irFind);
		if(irFound)
		{
			int iRemaining;
			list<CARGO_INFO> lstCargo;
			HkEnumCargo(ARG_CLIENTID(p6), lstCargo, iRemaining);
			foreach(lstCargo, CARGO_INFO, cargo)
			{
				UINT_WRAP uw = UINT_WRAP(cargo->iArchID);
				if(irFound->btItems->Find(&uw))
				{
					bFoundItemRestriction = true;
					break;
				}
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	
	Server.ReqAddItem(p1, p2, p3, p4, p5, p6);

	try {
		if(bFoundItemRestriction)
		{
			ushort iID;
			HkGetSIDFromGoodID(p6, iID, p1);
			pub::Player::RemoveCargo(p6, iID, p3);
			uint iBaseID;
			pub::Player::GetBase(p6, iBaseID);
			float fPrice = 0;
			pub::Market::GetPrice(iBaseID, p1, fPrice);
			if(fPrice)
				HkAddCash(ARG_CLIENTID(p6), (int)fPrice);
			pub::Player::SendNNMessage(p6, pub::GetNicknameId("cancelled"));
			pub::Player::SendNNMessage(p6, pub::GetNicknameId("not_interested"));
		}

		/*Code for rep change when items are bought*/
		if(p5)//Item is being added mounted
		{
			for(uint i = 0 ; (i < set_vAffilItems.size()); i++) //Items set in INI
			{
				if(set_vAffilItems[i]==p1)
				{
					uint iBaseID;
					pub::Player::GetBase(p6, iBaseID);
					iBaseID = HkGetSpaceObjFromBaseID(iBaseID);
					int iBaseRep, iPlayerRep;
					pub::SpaceObj::GetSolarRep(iBaseID, iBaseRep);
					pub::Player::GetRep(p6, iPlayerRep);
					uint iRepGroupID, iOldRepGroupID;
					Reputation::Vibe::GetAffiliation(iBaseRep, iRepGroupID, false);
					Reputation::Vibe::GetAffiliation(iPlayerRep, iOldRepGroupID, false);
					if(iOldRepGroupID!=iRepGroupID)
						Reputation::Vibe::SetAffiliation(iPlayerRep, iRepGroupID, false);
					return;
				}
			}
			foreach(set_lstFactionTags, FACTION_TAG, tag) //Auto-detected items in misc_equip.ini
			{
				if((*tag).iArchID==p1)
				{
					uint iRepGroupID, iOldRepGroupID;
					pub::Reputation::GetReputationGroup(iRepGroupID, (*tag).scFaction.c_str()); 
					int iPlayerRep;
					pub::Player::GetRep(p6, iPlayerRep);
					Reputation::Vibe::GetAffiliation(iPlayerRep, iOldRepGroupID, false);
					if(iOldRepGroupID!=iRepGroupID)
						Reputation::Vibe::SetAffiliation(iPlayerRep, iRepGroupID, false);
					break;
				}
			}
		}

		// anti base-idle
		ClientInfo[p6].iBaseEnterTime = (uint)time(0);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

}

/**************************************************************************************************************
Called when equipping internal equipment
**************************************************************************************************************/
void __stdcall ReqModifyItem(unsigned short p1, char const *p2, int p3, float p4, bool p5, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);
	ISERVER_LOGARG_F(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(iClientID);

	try {
		if(p5)
		{
			uint iGoodID;
			if(HKHKSUCCESS(HkGetGoodIDFromSID(iClientID, p1, iGoodID)))
			{
				MOUNT_RESTRICTION mrFind = MOUNT_RESTRICTION(iGoodID);
				MOUNT_RESTRICTION *mrFound = set_btMRestrict->Find(&mrFind);
				/*Check to make sure ship can mount good*/
				if(mrFound)
				{
					uint iShipArchID;
					pub::Player::GetShipID(iClientID, iShipArchID);
					UINT_WRAP uw = UINT_WRAP(iShipArchID);
					if(!mrFound->btShipArchIDs->Find(&uw))
					{
						//remove and then add cargo to fix client mounted disagreement
						pub::Player::RemoveCargo(iClientID, p1, p3);
						pub::Player::AddCargo(iClientID, iGoodID, p3, 1, false);
						pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("no_place_to_mount"));
						return;
					}
				}
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.ReqModifyItem(p1, p2, p3, p4, p5, iClientID);
	
	try {
		// anti base-idle
		ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

void __stdcall ReqCargo(class EquipDescList const &edl, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqCargo(edl, iClientID); // doesn't do anything
}

void __stdcall ReqChangeCash(int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqChangeCash(p1, iClientID);
}

void __stdcall ReqCollisionGroups(class std::list<struct CollisionGroupDesc,class std::allocator<struct CollisionGroupDesc> > const &p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqCollisionGroups(p1, iClientID);
}

void __stdcall ReqDifficultyScale(float p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqDifficultyScale(p1, iClientID);
}

/**************************************************************************************************************
Called when equipping all equipment other than internal equipment
**************************************************************************************************************/
void __stdcall ReqEquipment(class EquipDescList const &edl, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqEquipment(edl, iClientID);

	try {
		// anti base-idle
		ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

void __stdcall ReqHullStatus(float p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqHullStatus(p1, iClientID);
}

void __stdcall ReqRemoveItem(unsigned short p1, int p2, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_I(p2);
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqRemoveItem(p1, p2, iClientID);
}

void __stdcall ReqSetCash(int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(p1);
	ISERVER_LOGARG_UI(iClientID);

	Server.ReqSetCash(p1, iClientID);
}

void __stdcall ReqShipArch(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.ReqShipArch(p1, p2);
}

void __stdcall RequestBestPath(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.RequestBestPath(p1, p2, p3);
}

void __stdcall RequestCancel(int p1, unsigned int p2, unsigned int p3, unsigned long p4, unsigned int p5)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);
	ISERVER_LOGARG_UI(p5);

	try{
		if(!p1) //docking
		{
			foreach(ClientInfo[p5].lstRemCargo, CARGO_REMOVE, cargo)
			{
				HkAddCargo(ARG_CLIENTID(p5), cargo->iGoodID, cargo->iCount, false);
			}
			ClientInfo[p5].lstRemCargo.clear();
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.RequestCancel(p1, p2, p3, p4, p5);
}

void __stdcall RequestCreateShip(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.RequestCreateShip(iClientID);
}

/**************************************************************************************************************
Called upon flight change (goto, dock, formation)
**************************************************************************************************************/
/*p1 = iType? ==0 if docking, ==1 if formation
p2 = iShip of person docking
p3 = iShip of dock/formation target
p4 seems to be 0 all the time
p5 seems to be 0 all the time
p6 = iClientID*/
void __stdcall RequestEvent(int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned long p5, unsigned int p6)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(p6);
	
	try {
		//Moved to SpaceObjDock, kept here to avoid silly docking responses from bases
		if(!p1)//docking
		{
			float fRelativeHealth;
			pub::SpaceObj::GetRelativeHealth(p3, fRelativeHealth);
			if(fRelativeHealth<set_fBaseDockDmg)
			{
				pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
				PrintUserCmdText(p6, L"The docking ports are damaged");
				return;
			}

			//mobile docking
			if(!((uint)(p3/1000000000))) //Docking at non solar
			{
				if(ClientInfo[p6].bMobileBase) //prevent mobile bases from docking with each other
				{
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
					return;
				}
				uint iTargetClientID = HkGetClientIDByShip(p3);
				if(iTargetClientID)
				{
					if(!ClientInfo[iTargetClientID].bMobileBase)
					{
						pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
						return;
					}
					//Make sure players are in same group
					uint iGroupID = Players.GetGroupID(p6), iTargetGroupID = Players.GetGroupID(iTargetClientID);
					if(!iGroupID || !iTargetGroupID || iGroupID!=iTargetGroupID)
					{
						pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
						return;
					}
					//Dock player at mobile dock base in system
					Matrix mClient, mTarget;
					Vector vClient, vTarget;
					pub::SpaceObj::GetLocation(p2, vClient, mClient);
					pub::SpaceObj::GetLocation(p3, vTarget, mTarget);
					uint iDunno;
					IObjInspectImpl *inspect;
					GetShipInspect(p3, inspect, iDunno);
					if(HkDistance3D(vClient, vTarget) > set_iMobileDockRadius+inspect->cobject()->hierarchy_radius()) //outside of docking radius
					{
						pub::Player::SendNNMessage(p6, pub::GetNicknameId("nnv_dock_too_far"));
						return;
					}
					if(!HkIsOkayToDock(p6, iTargetClientID))
					{
						pub::Player::SendNNMessage(p6, pub::GetNicknameId("insufficient_cargo_space"));
						return;
					}
					string scSystem = HkGetPlayerSystemS(p6);
					string scBase = scSystem + "_Mobile_Proxy_Base";
					uint iBaseID;
					if(pub::GetBaseID(iBaseID, (scBase).c_str()) == -4) //base does not exist
					{
						ConPrint(L"WARNING: Mobile docking proxy base does not exist in system " + stows(scSystem) + L"\n");
						pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
						return;
					}
					uint iTargetProxyObj = CreateID(scBase.c_str());
					bIgnoreCancelMobDock = true;
					pub::SpaceObj::InstantDock(p2, iTargetProxyObj, 1);
					ClientInfo[iTargetClientID].lstPlayersDocked.remove(p6);
					ClientInfo[iTargetClientID].lstPlayersDocked.push_back(p6);
					ClientInfo[p6].bMobileDocked = true;
					ClientInfo[p6].iDockClientID = iTargetClientID;
					return;
				}
				else
				{
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("dock_disallowed"));
					return;
				}
			} 
			if(ClientInfo[p6].bMobileBase)
			{
				ClientInfo[p6].iLastSpaceObjDocked = p3;
			}

			ClientInfo[p6].lstRemCargo.clear();
			DOCK_RESTRICTION jrFind = DOCK_RESTRICTION(p3);
			DOCK_RESTRICTION *jrFound = set_btJRestrict->Find(&jrFind);
			if(jrFound)
			{
				list<CARGO_INFO> lstCargo;
				int iSpaceRemaining;
				bool bPresent = false;
				HkEnumCargo(ARG_CLIENTID(p6), lstCargo, iSpaceRemaining);
				foreach(lstCargo, CARGO_INFO, cargo)
				{
					if(cargo->iArchID == jrFound->iArchID) //Item is present
					{
						if(jrFound->iCount > 0)
						{
							if(cargo->iCount >= jrFound->iCount)
								bPresent = true;
						}
						else if(jrFound->iCount < 0)
						{
							if(cargo->iCount >= -jrFound->iCount)
							{
								bPresent = true;
								CARGO_REMOVE cm;
								cm.iGoodID = cargo->iArchID;
								cm.iCount = -jrFound->iCount;
								ClientInfo[p6].lstRemCargo.push_back(cm);
								pub::Player::RemoveCargo(p6, cargo->iID, -jrFound->iCount);
							}
						}
						else
						{
							if(cargo->bMounted)
								bPresent = true;
						}
						break;
					}
				}
				if(!bPresent)
				{
					pub::Player::SendNNMessage(p6, pub::GetNicknameId("info_access_denied"));
					PrintUserCmdText(p6, jrFound->wscDeniedMsg);
					return; //block dock
				}
			}
			ClientInfo[p6].bCheckedDock = true;
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	Server.RequestEvent(p1, p2, p3, p4, p5, p6);
}

void __stdcall RequestGroupPositions(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.RequestGroupPositions(p1, p2, p3);
}

void __stdcall RequestPlayerStats(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.RequestPlayerStats(p1, p2, p3);
}

void __stdcall RequestRankLevel(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.RequestRankLevel(p1, p2, p3);
}

void __stdcall RequestTrade(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	Server.RequestTrade(p1, p2);
}

void __stdcall SPBadLandsObjCollision(struct SSPBadLandsObjCollisionInfo const &p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.SPBadLandsObjCollision(p1, iClientID);
}

void __stdcall SPRequestInvincibility(unsigned int p1, bool p2, enum InvincibilityReason p3, unsigned int p4)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);

	Server.SPRequestInvincibility(p1, p2, p3, p4);
}

void __stdcall SPRequestUseItem(struct SSPUseItem const &p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.SPRequestUseItem(p1, iClientID);
}

void __stdcall SPScanCargo(unsigned int const &p1, unsigned int const &p2, unsigned int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	Server.SPScanCargo(p1, p2, p3);
}

void __stdcall SaveGame(struct CHARACTER_ID const &cId, unsigned short const *p2, unsigned int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_S(&cId);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_UI(p3);

	Server.SaveGame(cId, p2, p3);
}

void __stdcall SetActiveConnection(enum EFLConnection p1)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);

	Server.SetActiveConnection(p1); // doesn't do anything
}

void __stdcall SetInterfaceState(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.SetInterfaceState(p1, p2, p3);
}

void __stdcall SetManeuver(unsigned int iClientID, struct XSetManeuver const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.SetManeuver(iClientID, p2);
}

void __stdcall SetMissionLog(unsigned int iClientID, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.SetMissionLog(iClientID, p2, p3);
}

uint iLastSetTarget = 0;
/**************************************************************************************************************
Called when player selects a new target
**************************************************************************************************************/
void __stdcall SetTarget(unsigned int iClientID, struct XSetTarget const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.SetTarget(iClientID, p2);
	
	try {
		//Capture solar objects for alternate solar objects repair
		if(p2.iTargetSpaceID!=iLastSetTarget && (uint)(p2.iTargetSpaceID/1000000000)) //target is a solar
		{
			/*uint iType;
			pub::SpaceObj::GetType(p2.iTargetSpaceID, iType);
			PrintUserCmdText(iClientID, L"type=%u", iType);*/
			float fHealth, fMaxHealth;
			pub::SpaceObj::GetHealth(p2.iTargetSpaceID, fHealth, fMaxHealth);
			FLOAT_WRAP fw = FLOAT_WRAP(fMaxHealth);
			if(set_btSupressHealth->Find(&fw))
			{
				SOLAR_REPAIR *sr = new SOLAR_REPAIR(p2.iTargetSpaceID, -1);
				sr->iObjectID = p2.iTargetSpaceID;
				if(!fHealth)
				{
					sr->iTimeAfterDestroyed = 0;
				}
				btSolarList->Add(sr);
			}
		}
		iLastSetTarget = p2.iTargetSpaceID;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

void __stdcall SetTradeMoney(unsigned int iClientID, unsigned long p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	Server.SetTradeMoney(iClientID, p2);
}

void __stdcall SetVisitedState(unsigned int iClientID, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.SetVisitedState(iClientID, p2, p3);
}

void __stdcall SetWeaponGroup(unsigned int iClientID, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.SetWeaponGroup(iClientID, p2, p3);
}

void __stdcall Shutdown(void)
{
	ISERVER_LOG();

	Server.Shutdown();
}

bool __stdcall Startup(struct SStartupInfo const &p1)
{
	ISERVER_LOG();

	return Server.Startup(p1);
}

void __stdcall StopTradeRequest(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	Server.StopTradeRequest(iClientID);
}

/**************************************************************************************************************
Called when player starts to tractor an object
**************************************************************************************************************/
void __stdcall TractorObjects(unsigned int iClientID, struct XTractorObjects const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	//Deny tractor if dead
	float fHealth;
	pub::Player::GetRelativeHealth(iClientID, fHealth);
	if(!fHealth)
	{
		pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("nnv_tractor_failure"));
		pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_ship_destroyed"));
		return;
	}

	Server.TractorObjects(iClientID, p2);
}

void __stdcall TradeResponse(unsigned char const *p1, int p2, unsigned int iClientID)
{
	ISERVER_LOG();
///	ISERVER_LOGARG_S(p1);
	ISERVER_LOGARG_I(p2);
	ISERVER_LOGARG_UI(iClientID);

	Server.TradeResponse(p1, p2, iClientID);
}

/**************************************************************************************************************
IServImpl hook entries
**************************************************************************************************************/

HOOKENTRY hookEntries[] =
{
	{(FARPROC)SubmitChat,				-0x08, 0},
	{(FARPROC)FireWeapon,				0x000, 0},
	{(FARPROC)ActivateEquip,			0x004, 0},
	{(FARPROC)ActivateCruise,			0x008, 0},
	{(FARPROC)ActivateThrusters,		0x00C, 0},
	{(FARPROC)SetTarget,				0x010, 0},
	{(FARPROC)TractorObjects,			0x014, 0},
	{(FARPROC)GoTradelane,				0x018, 0},
	{(FARPROC)StopTradelane,			0x01C, 0},
	{(FARPROC)JettisonCargo,			0x020, 0},
	{(FARPROC)Startup,					0x024, 0},
	{(FARPROC)Shutdown,					0x028, 0},
//	{(FARPROC)Update,					0x02C, 0}, // no need to add here, already hooked in DllMain
	{(FARPROC)ElapseTime,				0x030, 0},
	{(FARPROC)PushToServer,				0x034, 0}, // or SetActiveConnection?!
//	{(FARPROC)SwapConnections,			0x038, 0}, // ???
	{(FARPROC)Connect,					0x03C, 0},
	{(FARPROC)DisConnect,				0x040, 0},
	{(FARPROC)OnConnect,				0x044, 0},
	{(FARPROC)Login,					0x048, 0},
	{(FARPROC)CharacterInfoReq,			0x04C, 0},
	{(FARPROC)CharacterSelect,			0x050, 0},
	{(FARPROC)SetActiveConnection,		0x054, 0}, // or PushToServer?
	{(FARPROC)CreateNewCharacter,		0x058, 0},
	{(FARPROC)DestroyCharacter,			0x05C, 0},
	{(FARPROC)CharacterSkipAutosave,	0x060, 0},
	{(FARPROC)ReqShipArch,				0x064, 0},
	{(FARPROC)ReqHullStatus,			0x068, 0},
	{(FARPROC)ReqCollisionGroups,		0x06C, 0},
	{(FARPROC)ReqEquipment,				0x070, 0},
	{(FARPROC)ReqCargo,					0x074, 0},
	{(FARPROC)ReqAddItem,				0x078, 0},
	{(FARPROC)ReqRemoveItem,			0x07C, 0},
	{(FARPROC)ReqModifyItem,			0x080, 0},
	{(FARPROC)ReqSetCash,				0x084, 0},
	{(FARPROC)ReqChangeCash,			0x088, 0},
	{(FARPROC)BaseEnter,				0x08C, 0},
	{(FARPROC)BaseExit,					0x090, 0},
	{(FARPROC)LocationEnter,			0x094, 0},
	{(FARPROC)LocationExit,				0x098, 0},
	{(FARPROC)BaseInfoRequest,			0x09C, 0},
	{(FARPROC)LocationInfoRequest,		0x0A0, 0},
	{(FARPROC)GFObjSelect,				0x0A4, 0},
	{(FARPROC)GFGoodVaporized,			0x0A8, 0},
	{(FARPROC)MissionResponse,			0x0AC, 0},
	{(FARPROC)TradeResponse,			0x0B0, 0},
	{(FARPROC)GFGoodBuy,				0x0B4, 0},
	{(FARPROC)GFGoodSell,				0x0B8, 0},
	{(FARPROC)SystemSwitchOutComplete,	0x0BC, 0},
	{(FARPROC)PlayerLaunch,				0x0C0, 0},
	{(FARPROC)LaunchComplete,			0x0C4, 0},
	{(FARPROC)JumpInComplete,			0x0C8, 0},
	{(FARPROC)Hail,						0x0CC, 0},
	{(FARPROC)SPObjUpdate,				0x0D0, 0},
	{(FARPROC)SPMunitionCollision,		0x0D4, 0},
	{(FARPROC)SPBadLandsObjCollision,	0x0D8, 0},
	{(FARPROC)SPObjCollision,			0x0DC, 0},
	{(FARPROC)SPRequestUseItem,			0x0E0, 0},
	{(FARPROC)SPRequestInvincibility,	0x0E4, 0},
	{(FARPROC)SaveGame,					0x0E8, 0},
	{(FARPROC)MissionSaveB,				0x0EC, 0},
	{(FARPROC)RequestEvent,				0x0F0, 0},
	{(FARPROC)RequestCancel,			0x0F4, 0},
	{(FARPROC)MineAsteroid,				0x0F8, 0},
	{(FARPROC)CommComplete,				0x0FC, 0},
	{(FARPROC)RequestCreateShip,		0x100, 0},
	{(FARPROC)SPScanCargo,				0x104, 0},
	{(FARPROC)SetManeuver,				0x108, 0},
	{(FARPROC)InterfaceItemUsed,		0x10C, 0},
	{(FARPROC)AbortMission,				0x110, 0},
	{(FARPROC)RTCDone,					0x114, 0},
	{(FARPROC)SetWeaponGroup,			0x118, 0},
	{(FARPROC)SetVisitedState,			0x11C, 0},
	{(FARPROC)RequestBestPath,			0x120, 0},
	{(FARPROC)RequestPlayerStats,		0x124, 0},
	{(FARPROC)PopUpDialog,				0x128, 0},
	{(FARPROC)RequestGroupPositions,	0x12C, 0},
	{(FARPROC)SetMissionLog,			0x130, 0},
	{(FARPROC)SetInterfaceState,		0x134, 0},
	{(FARPROC)RequestRankLevel,			0x138, 0},
	{(FARPROC)InitiateTrade,			0x13C, 0},
	{(FARPROC)TerminateTrade,			0x140, 0},
	{(FARPROC)AcceptTrade,				0x144, 0},
	{(FARPROC)SetTradeMoney,			0x148, 0},
	{(FARPROC)AddTradeEquip,			0x14C, 0},
	{(FARPROC)DelTradeEquip,			0x150, 0},
	{(FARPROC)RequestTrade,				0x154, 0},
	{(FARPROC)StopTradeRequest,			0x158, 0},
	{(FARPROC)ReqDifficultyScale,		0x15C, 0},
	{(FARPROC)GetServerID,				0x160, 0},
	{(FARPROC)GetServerSig,				0x164, 0},
	{(FARPROC)DumpPacketStats,			0x168, 0},
	{(FARPROC)Dock,						0x16C, 0},
};

}
