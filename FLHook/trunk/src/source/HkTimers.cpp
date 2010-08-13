#include <winsock2.h>
#include "wildcards.hh"
#include "hook.h"

/**************************************************************************************************************
Update average ping data
**************************************************************************************************************/

void HkTimerUpdatePingData()
{
	try {
		// for all players
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			if(ClientInfo[iClientID].tmF1TimeDisconnect)
				continue;

//			CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
//			u_long pPingPacket[2] = {3, 4, 5, 6, 7};
//			cdpClient->Send(&pPingPacket, sizeof(pPingPacket));
//			CDPServer::getServer()->SendTo(cdpClient, &pPingPacket, sizeof(pPingPacket));

			DPN_CONNECTION_INFO ci;
			if(HkGetConnectionStats(iClientID, ci) != HKE_OK)
				continue;

			///////////////////////////////////////////////////////////////
			// update ping data
			if(ClientInfo[iClientID].lstPing.size() >= set_iPingKickFrame)
			{
				// calculate average loss
				ClientInfo[iClientID].iAveragePing = 0;
				foreach(ClientInfo[iClientID].lstPing, uint, it)
					ClientInfo[iClientID].iAveragePing += (*it);

				ClientInfo[iClientID].iAveragePing /= (uint)ClientInfo[iClientID].lstPing.size();
			}

			// remove old pingdata
			while(ClientInfo[iClientID].lstPing.size() >= set_iPingKickFrame)
				ClientInfo[iClientID].lstPing.pop_back();

			ClientInfo[iClientID].lstPing.push_front(ci.dwRoundTripLatencyMS);
		}
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
Update average loss data
**************************************************************************************************************/

void HkTimerUpdateLossData()
{
	try {
		// for all players
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			if(ClientInfo[iClientID].tmF1TimeDisconnect)
				continue;

			DPN_CONNECTION_INFO ci;
			if(HkGetConnectionStats(iClientID, ci) != HKE_OK)
				continue;

			///////////////////////////////////////////////////////////////
			// update loss data
			if(ClientInfo[iClientID].lstLoss.size() >= (set_iLossKickFrame / (LOSS_INTERVALL / 1000)))
			{
				// calculate average loss
				ClientInfo[iClientID].iAverageLoss = 0;
				foreach(ClientInfo[iClientID].lstLoss, uint, it)
					ClientInfo[iClientID].iAverageLoss += (*it);

				ClientInfo[iClientID].iAverageLoss /= (uint)ClientInfo[iClientID].lstLoss.size();
			}

			// remove old lossdata
			while(ClientInfo[iClientID].lstLoss.size() >= (set_iLossKickFrame / (LOSS_INTERVALL / 1000)))
				ClientInfo[iClientID].lstLoss.pop_back();

//			CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
//			double d = cdpClient->GetLinkSaturation();		
//			ClientInfo[iClientID].lstLoss.push_front((uint)(cdpClient->GetLinkSaturation() * 100)); // loss per sec
			ClientInfo[iClientID].lstLoss.push_front(ci.dwBytesRetried - ClientInfo[iClientID].iLastLoss); // loss per sec
			ClientInfo[iClientID].iLastLoss = ci.dwBytesRetried;
		}
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
check if players should be kicked
**************************************************************************************************************/

void HkTimerCheckKick()
{
	try {
		// for all players
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if(ClientInfo[iClientID].tmKickTime)
			{	
				if(timeInMS() >= ClientInfo[iClientID].tmKickTime) 
					HkKick(ARG_CLIENTID(iClientID)); // kick time expired

				continue; // player will be kicked anyway
			}

			if(set_iAntiBaseIdle)
			{ // anti base-idle check
				uint iBaseID;
				pub::Player::GetBase(iClientID, iBaseID);
				if(iBaseID && ClientInfo[iClientID].iBaseEnterTime)
				{
					if((time(0) - ClientInfo[iClientID].iBaseEnterTime) >= set_iAntiBaseIdle)
					{
						HkAddKickLog(iClientID, L"Base idling");
						HkMsgAndKick(iClientID, L"Base idling", set_iKickMsgPeriod);
					}
				}
			}

			if(set_iAntiCharMenuIdle)
			{ // anti charmenu-idle check
				if(HkIsInCharSelectMenu(iClientID))	{
					if(!ClientInfo[iClientID].iCharMenuEnterTime)
						ClientInfo[iClientID].iCharMenuEnterTime = (uint)time(0);
					else if((time(0) - ClientInfo[iClientID].iCharMenuEnterTime) >= set_iAntiCharMenuIdle) {
						HkAddKickLog(iClientID, L"Charmenu idling");
						HkKick(ARG_CLIENTID(iClientID));
						continue;
					}
				} else
					ClientInfo[iClientID].iCharMenuEnterTime = 0;
			}

			if(set_iLossKick)
			{ // check if loss is too high
				if(ClientInfo[iClientID].iAverageLoss > set_iLossKick)
				{
					HkAddKickLog(iClientID, L"High loss");
					HkMsgAndKick(iClientID, L"High loss", set_iKickMsgPeriod);
				}
			}

			if(set_iPingKick)
			{ // check if ping is too high
				if(ClientInfo[iClientID].iAveragePing > set_iPingKick)
				{
					HkAddKickLog(iClientID, L"High ping");
					HkMsgAndKick(iClientID, L"High ping", set_iKickMsgPeriod);
				}
			}
		}
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
Check if NPC spawns should be disabled
**************************************************************************************************************/

void HkTimerNPCAndF1Check()
{
	try {
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if(ClientInfo[iClientID].tmF1Time && (timeInMS() >= ClientInfo[iClientID].tmF1Time)) { // f1
				Server.CharacterInfoReq(iClientID, false);
				ClientInfo[iClientID].tmF1Time = 0;
			} else if(ClientInfo[iClientID].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[iClientID].tmF1TimeDisconnect)) {
				ulong lArray[64] = {0};
				lArray[26] = iClientID;
				__asm
				{
					pushad
					lea ecx, lArray
					mov eax, [hModRemoteClient]
					add eax, ADDR_RC_DISCONNECT
					call eax ; disconncet
					popad
				}

				ClientInfo[iClientID].tmF1TimeDisconnect = 0;
				continue;
			}
		}

		// npc
		if(!g_bNPCForceDisabled)
		{
			if(set_iDisableNPCSpawns && (g_iServerLoad >= set_iDisableNPCSpawns))
				HkChangeNPCSpawn(true); // serverload too high, disable npcs
			else
				HkChangeNPCSpawn(false);
		}
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerSolarRepair()
{
	try {
		if(!btSolarList->Count())
		{
			return;
		}
		BinaryTreeIterator<SOLAR_REPAIR> *solarIter = new BinaryTreeIterator<SOLAR_REPAIR>(btSolarList);
		solarIter->First();
		for(long i=0; i<btSolarList->Count(); i++)
		{
			if(solarIter->Curr()->iTimeAfterDestroyed<0) //solar is alive
			{
				float fRelativeHealth;
				pub::SpaceObj::GetRelativeHealth(solarIter->Curr()->iObjectID, fRelativeHealth);
				if(fRelativeHealth<=set_fBaseDockDmg)
				{
					solarIter->Curr()->iTimeAfterDestroyed = 0;
				}
			}
			else //dead or being repaired
			{
				if(solarIter->Curr()->iTimeAfterDestroyed>((int)(set_iRepairBaseTime*set_fRepairBaseRatio)))
				{
					float fRelativeHealth;
					pub::SpaceObj::GetRelativeHealth(solarIter->Curr()->iObjectID, fRelativeHealth);
					if(fRelativeHealth<set_fRepairBaseMaxHealth)
					{
						pub::SpaceObj::SetRelativeHealth(solarIter->Curr()->iObjectID, fRelativeHealth + (1.0f/(set_iRepairBaseTime*set_fRepairBaseRatio))*set_fRepairBaseMaxHealth + 0.000001f);
					}
					else
						solarIter->Curr()->iTimeAfterDestroyed = -2;
				}
				else if(solarIter->Curr()->iTimeAfterDestroyed>set_iRepairBaseTime)
				{
					solarIter->Curr()->iTimeAfterDestroyed = -2;
				}
				solarIter->Curr()->iTimeAfterDestroyed++;
			}
			solarIter->Next();
		}
		delete solarIter;
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
**************************************************************************************************************/

CRITICAL_SECTION csIPResolve;
list<RESOLVE_IP> g_lstResolveIPs;
list<RESOLVE_IP> g_lstResolveIPsResult;
HANDLE hThreadResolver;

void HkThreadResolver()
{
	try {
		while(1)
		{
			EnterCriticalSection(&csIPResolve);
			list<RESOLVE_IP> lstMyResolveIPs = g_lstResolveIPs;
			g_lstResolveIPs.clear();
			LeaveCriticalSection(&csIPResolve);

			foreach(lstMyResolveIPs, RESOLVE_IP, it)
			{
				ulong addr = inet_addr(wstos(it->wscIP).c_str());
				hostent *host = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
				if(host)
					it->wscHostname = stows(host->h_name);
			}

			EnterCriticalSection(&csIPResolve);
			foreach(lstMyResolveIPs, RESOLVE_IP, it2)
			{
				if(it2->wscHostname.length())
					g_lstResolveIPsResult.push_back(*it2);
			}
			LeaveCriticalSection(&csIPResolve);

			Sleep(50);
		}
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerCheckResolveResults()
{
	try {
		EnterCriticalSection(&csIPResolve);
		foreach(g_lstResolveIPsResult, RESOLVE_IP, it)
		{
			if(it->iConnects != ClientInfo[it->iClientID].iConnects)
				continue; // outdated

			// check if banned
			foreach(set_lstBans, wstring, itb)
			{
				if(Wildcard::wildcardfit(wstos(*itb).c_str(), wstos(it->wscHostname).c_str()))
				{
					HkAddKickLog(it->iClientID, L"IP/Hostname ban(%s matches %s)", it->wscHostname.c_str(), (*itb).c_str());
					if(set_bBanAccountOnMatch)
						HkBan(ARG_CLIENTID(it->iClientID), true);
					HkKick(ARG_CLIENTID(it->iClientID));
				}
			}
			ClientInfo[it->iClientID].wscHostname = it->wscHostname;
		}

		g_lstResolveIPsResult.clear();
		LeaveCriticalSection(&csIPResolve);
	} catch(...) { LOG_EXCEPTION }
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerCloakHandler()
{
	try {
		// for all players
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			
			if (ClientInfo[iClientID].bCanCloak)
			{
				// send cloak state for uncloaked cloak-able players (only for them in space)
				// this is the code to fix the bug where players wouldnt always see uncloaked players

				uint iShip = 0;
				pub::Player::GetShip(iClientID, iShip);
				if (iShip)
				{

					if(!ClientInfo[iClientID].bCloaked && !ClientInfo[iClientID].bIsCloaking)
					{
						XActivateEquip ActivateEq;
						ActivateEq.bActivate = false;
						ActivateEq.l1 = iShip;
						ActivateEq.sID = ClientInfo[iClientID].iCloakSlot;
						Server.ActivateEquip(iClientID,ActivateEq);
					}
					
					// check cloak timings

					mstime tmTimeNow = timeInMS();

					if(ClientInfo[iClientID].bWantsCloak && (tmTimeNow - ClientInfo[iClientID].tmCloakTime) > ClientInfo[iClientID].iCloakWarmup)
						HkCloak(iClientID);

					if(ClientInfo[iClientID].bIsCloaking && (tmTimeNow - ClientInfo[iClientID].tmCloakTime) > ClientInfo[iClientID].iCloakingTime)
					{
						ClientInfo[iClientID].bIsCloaking = false;
						ClientInfo[iClientID].bCloaked = true;
						ClientInfo[iClientID].tmCloakTime = tmTimeNow;
					}

					mstime tmCloakRemaining = tmTimeNow - ClientInfo[iClientID].tmCloakTime;

					if(ClientInfo[iClientID].bCloaked && ClientInfo[iClientID].iCloakingTime && tmCloakRemaining > 0)
					{
						HkUnCloak(iClientID);
					}

					/*if(ClientInfo[iClientID].iCloakingTime)
					{
						PrintUserCmdText(iClientID, L"tmCloakRemaining: %I64d, %i, %i", tmCloakRemaining, (ClientInfo[iClientID].iCloakingTime - (set_iCloakExpireWarnTime - 500)), (ClientInfo[iClientID].iCloakingTime - set_iCloakExpireWarnTime));
					}*/

					//Does not work, dunno why
					/*if(ClientInfo[iClientID].bCloaked && ClientInfo[iClientID].iCloakingTime && tmCloakRemaining < (mstime)(ClientInfo[iClientID].iCloakingTime - (set_iCloakExpireWarnTime - 500)) && tmCloakRemaining >= (mstime)(ClientInfo[iClientID].iCloakingTime - set_iCloakExpireWarnTime))
					{
						PrintUserCmdText(iClientID, L"Warning: cloak will expire in %i seconds", set_iCloakExpireWarnTime/1000);
					}*/

					if(!ClientInfo[iClientID].bCloaked && tmCloakRemaining < (ClientInfo[iClientID].iCloakCooldown + 500) && tmCloakRemaining >= ClientInfo[iClientID].iCloakCooldown)
					{
						PrintUserCmdText(iClientID, L"Cloak has cooled down and is ready to be used!");
					}

				}

			}
		}

	} catch(...) { LOG_EXCEPTION }

}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerSpaceObjMark()
{
	try {
		if(set_fAutoMarkRadius<=0.0f) //automarking disabled
			return;

		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iShip, iClientID = HkGetClientIdFromPD(pPD);
			pub::Player::GetShip(iClientID, iShip);
			if(!iShip || ClientInfo[iClientID].fAutoMarkRadius<=0.0f) //docked or does not want any marking
				continue;
			uint iSystem;
			pub::Player::GetSystem(iClientID, iSystem);
			Vector VClientPos;
			Matrix MClientOri;
			pub::SpaceObj::GetLocation(iShip, VClientPos, MClientOri);

			for(uint i=0; i<ClientInfo[iClientID].vAutoMarkedObjs.size(); i++)
			{
				Vector VTargetPos;
				Matrix MTargetOri;
				pub::SpaceObj::GetLocation(ClientInfo[iClientID].vAutoMarkedObjs[i], VTargetPos, MTargetOri);	
				if(HkDistance3D(VTargetPos, VClientPos)>ClientInfo[iClientID].fAutoMarkRadius)
				{
					pub::Player::MarkObj(iClientID, ClientInfo[iClientID].vAutoMarkedObjs[i], 0);
					ClientInfo[iClientID].vDelayedAutoMarkedObjs.push_back(ClientInfo[iClientID].vAutoMarkedObjs[i]);
					if(i!=ClientInfo[iClientID].vAutoMarkedObjs.size()-1)
					{
						ClientInfo[iClientID].vAutoMarkedObjs[i] = ClientInfo[iClientID].vAutoMarkedObjs[ClientInfo[iClientID].vAutoMarkedObjs.size()-1];
						i--;
					}
					ClientInfo[iClientID].vAutoMarkedObjs.pop_back();
				}
			}

			for(uint i=0; i<ClientInfo[iClientID].vDelayedAutoMarkedObjs.size(); i++)
			{
				if(pub::SpaceObj::ExistsAndAlive(ClientInfo[iClientID].vDelayedAutoMarkedObjs[i]))
				{
					if(i!=ClientInfo[iClientID].vDelayedAutoMarkedObjs.size()-1)
					{
						ClientInfo[iClientID].vDelayedAutoMarkedObjs[i] = ClientInfo[iClientID].vDelayedAutoMarkedObjs[ClientInfo[iClientID].vDelayedAutoMarkedObjs.size()-1];
						i--;
					}
					ClientInfo[iClientID].vDelayedAutoMarkedObjs.pop_back();
					continue;
				}
				Vector VTargetPos;
				Matrix MTargetOri;
				pub::SpaceObj::GetLocation(ClientInfo[iClientID].vDelayedAutoMarkedObjs[i], VTargetPos, MTargetOri);	
				if(!(HkDistance3D(VTargetPos, VClientPos)>ClientInfo[iClientID].fAutoMarkRadius))
				{
					pub::Player::MarkObj(iClientID, ClientInfo[iClientID].vDelayedAutoMarkedObjs[i], 1);
					ClientInfo[iClientID].vAutoMarkedObjs.push_back(ClientInfo[iClientID].vDelayedAutoMarkedObjs[i]);
					if(i!=ClientInfo[iClientID].vDelayedAutoMarkedObjs.size()-1)
					{
						ClientInfo[iClientID].vDelayedAutoMarkedObjs[i] = ClientInfo[iClientID].vDelayedAutoMarkedObjs[ClientInfo[iClientID].vDelayedAutoMarkedObjs.size()-1];
						i--;
					}
					ClientInfo[iClientID].vDelayedAutoMarkedObjs.pop_back();
				}
			}

			for(uint i=0; i<vMarkSpaceObjProc.size(); i++)
			{
				uint iMarkSpaceObjProcShip = vMarkSpaceObjProc[i];
				if(set_bFlakVersion)
				{
					uint iType;
					pub::SpaceObj::GetType(iMarkSpaceObjProcShip, iType);
					if(iType!=OBJ_CAPITAL && ((ClientInfo[iClientID].bMarkEverything && iType==OBJ_FIGHTER)/* || iType==OBJ_FREIGHTER*/))
					{
						uint iSpaceObjSystem;
						pub::SpaceObj::GetSystem(iMarkSpaceObjProcShip, iSpaceObjSystem);
						Vector VTargetPos;
						Matrix MTargetOri;
						pub::SpaceObj::GetLocation(iMarkSpaceObjProcShip, VTargetPos, MTargetOri);	
						if(iSpaceObjSystem!=iSystem || HkDistance3D(VTargetPos, VClientPos)>ClientInfo[iClientID].fAutoMarkRadius)
						{
							ClientInfo[iClientID].vDelayedAutoMarkedObjs.push_back(iMarkSpaceObjProcShip);
						}
						else
						{
							pub::Player::MarkObj(iClientID, iMarkSpaceObjProcShip, 1);
							ClientInfo[iClientID].vAutoMarkedObjs.push_back(iMarkSpaceObjProcShip);
						}
					}
				}
				else //just mark everything
				{
					uint iSpaceObjSystem;
					pub::SpaceObj::GetSystem(iMarkSpaceObjProcShip, iSpaceObjSystem);
					Vector VTargetPos;
					Matrix MTargetOri;
					pub::SpaceObj::GetLocation(iMarkSpaceObjProcShip, VTargetPos, MTargetOri);	
					if(iSpaceObjSystem!=iSystem || HkDistance3D(VTargetPos, VClientPos)>ClientInfo[iClientID].fAutoMarkRadius)
					{
						ClientInfo[iClientID].vDelayedAutoMarkedObjs.push_back(iMarkSpaceObjProcShip);
					}
					else
					{
						pub::Player::MarkObj(iClientID, iMarkSpaceObjProcShip, 1);
						ClientInfo[iClientID].vAutoMarkedObjs.push_back(iMarkSpaceObjProcShip);
					}
				}
			}
			vMarkSpaceObjProc.clear();

		
		}
	} catch(...) { LOG_EXCEPTION }
}

list<DELAY_MARK> g_lstDelayedMarks;
void HkTimerMarkDelay()
{
	if(!g_lstDelayedMarks.size())
		return;

	mstime tmTimeNow = timeInMS();
	for(list<DELAY_MARK>::iterator mark = g_lstDelayedMarks.begin(); mark != g_lstDelayedMarks.end(); )
	{
		if(tmTimeNow-mark->time > 50)
		{
			Matrix mTemp;
			Vector vItem, vPlayer;
			pub::SpaceObj::GetLocation(mark->iObj, vItem, mTemp);
			uint iItemSystem;
			pub::SpaceObj::GetSystem(mark->iObj, iItemSystem);
			// for all players
			struct PlayerData *pPD = 0;
			while(pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				if(Players[iClientID].iSystemID == iItemSystem)
				{
					pub::SpaceObj::GetLocation(Players[iClientID].iSpaceObjID, vPlayer, mTemp);
					if(HkDistance3D(vPlayer, vItem) <= LOOT_UNSEEN_RADIUS)
					{
						HkMarkObject(iClientID, mark->iObj);
					}
				}
			}
			mark = g_lstDelayedMarks.erase(mark);
		}
		else
		{
			mark++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkTimerNPCDockHandler()
{
	try {
		for(uint i=0; i<vDelayedNPCDocks.size(); i++)
		{
			float fRelativeHealth;
			pub::SpaceObj::GetRelativeHealth(vDelayedNPCDocks[i].iDockTarget, fRelativeHealth);
			if(fRelativeHealth>=set_fBaseDockDmg)
			{
				pub::SpaceObj::Dock(vDelayedNPCDocks[i].iShip, vDelayedNPCDocks[i].iDockTarget, 0, vDelayedNPCDocks[i].dockResponse);
				if(i!=vDelayedNPCDocks.size()-1)
				{
					vDelayedNPCDocks[i] = vDelayedNPCDocks[vDelayedNPCDocks.size()-1];
					i--;
				}
				vDelayedNPCDocks.pop_back();
			}
		}
	} catch(...) { LOG_EXCEPTION }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

list<SHIP_REPAIR> g_lstRepairShips;
void HkTimerRepairShip()
{
	try {
		for(list<SHIP_REPAIR>::iterator ship = g_lstRepairShips.begin(); ship != g_lstRepairShips.end(); )
		{
			if(!pub::SpaceObj::ExistsAndAlive(ship->iObjID))
			{
				float fHealth, fMaxHealth;
				pub::SpaceObj::GetHealth(ship->iObjID, fHealth, fMaxHealth);
				if(fHealth && fHealth != fMaxHealth)
				{
					fHealth += ship->fIncreaseHealth;
					if(fHealth > fMaxHealth)
						pub::SpaceObj::SetRelativeHealth(ship->iObjID, 1.0f);
					else
						pub::SpaceObj::SetRelativeHealth(ship->iObjID, fHealth/fMaxHealth);
				}
				ship++;
			}
			else
			{
				ship = g_lstRepairShips.erase(ship);
			}
		}
	} catch(...) { LOG_EXCEPTION }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

list< pair<uint, DAMAGE_INFO> > lstSolarDestroyDelay;
void HkTimerSolarDestroyDelay()
{
	try {
		foreach(lstSolarDestroyDelay, pair<uint COMMA DAMAGE_INFO>, damage)
		{
			uint iSpaceObj = damage->first;
			float fHealth, fMaxHealth;
			pub::SpaceObj::GetHealth(iSpaceObj, fHealth, fMaxHealth);
			if(!fHealth)
			{
				pair< map<uint, list<DAMAGE_INFO> >::iterator, bool> findSpaceObj = mapSpaceObjDmgRec.insert(make_pair(iSpaceObj, list<DAMAGE_INFO>()));
				findSpaceObj.first->second.push_back(damage->second);
				SpaceObjDestroyed(iSpaceObj, true);
			}
		}
		lstSolarDestroyDelay.clear();
	} catch(...) { LOG_EXCEPTION }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

list<RESPAWN_DELAY> lstRespawnDelay;
void HkTimerRespawnDelay()
{
	try{
		mstime tmNow = timeInMS();
		for(list<RESPAWN_DELAY>::iterator respawn = lstRespawnDelay.begin(); respawn != lstRespawnDelay.end(); )
		{
			if(tmNow > respawn->tmCall)
			{
				HkIServerImpl::CharacterSelect(respawn->whatever, respawn->iClientID);
				respawn = lstRespawnDelay.erase(respawn);
			}
			else
				respawn++;
		}
	} catch(...) { LOG_EXCEPTION }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************************************************
Called when _SendMessage is sent
NOT USED ATM
**************************************************************************************************************/

struct MSGSTRUCT
{
	uint iShipID; // ? or charid? or archetype?
	uint iClientID;
};

void __stdcall HkCb_Message(uint iMsg, MSGSTRUCT *msg)
{
	try {
		uint i = msg->iClientID;

		FILE *f = fopen("msg.txt", "at");
		fprintf(f, "%.4d %.4d\n", iMsg, i);
		fclose(f);
	} catch(...) { LOG_EXCEPTION }
}

__declspec(naked) void _SendMessageHook()
{
	__asm 
	{
		push [esp+0Ch]
		push [esp+0Ch]
		call HkCb_Message
		ret
	}
}