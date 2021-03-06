#include "hook.h"

wstring wscAdminKiller = L"";

/**************************************************************************************************************
**************************************************************************************************************/

wstring SetSizeToSmall(wstring wscDataFormat)
{
	uint iFormat = wcstoul(wscDataFormat.c_str() + 2, 0, 16);
	wchar_t wszStyleSmall[32];
	wcscpy(wszStyleSmall, wscDataFormat.c_str());
	swprintf(wszStyleSmall + wcslen(wszStyleSmall) - 2, L"%02X", 0x90 | (iFormat & 7));
	return wszStyleSmall;
}

/**************************************************************************************************************
Send "Death: ..." chat-message
**************************************************************************************************************/

void SendDeathMsg(wstring wscMsg, uint iSystemID, uint iClientIDVictim, uint iClientIDKiller)
{
	// encode xml string(default and small)
	// non-sys
	wstring wscXMLMsg = L"<TRA data=\"" + set_wscDeathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsg += XMLText(wscMsg);
	wscXMLMsg += L"</TEXT>";

	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
		return;

	wstring wscStyleSmall = SetSizeToSmall(set_wscDeathMsgStyle);
	wstring wscXMLMsgSmall = wstring(L"<TRA data=\"") + wscStyleSmall + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSmall += XMLText(wscMsg);
	wscXMLMsgSmall += L"</TEXT>";
	char szBufSmall[0xFFFF];
	uint iRetSmall;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmall, szBufSmall, sizeof(szBufSmall), iRetSmall)))
		return;

	// sys
	wstring wscXMLMsgSys = L"<TRA data=\"" + set_wscDeathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSys += XMLText(wscMsg);
	wscXMLMsgSys += L"</TEXT>";
	char szBufSys[0xFFFF];
	uint iRetSys;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSys, szBufSys, sizeof(szBufSys), iRetSys)))
		return;

	wstring wscStyleSmallSys = SetSizeToSmall(set_wscDeathMsgStyleSys);
	wstring wscXMLMsgSmallSys = L"<TRA data=\"" + wscStyleSmallSys + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSmallSys += XMLText(wscMsg);
	wscXMLMsgSmallSys += L"</TEXT>";
	char szBufSmallSys[0xFFFF];
	uint iRetSmallSys;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmallSys, szBufSmallSys, sizeof(szBufSmallSys), iRetSmallSys)))
		return;

	// send
	// for all players
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);

		char *szXMLBuf;
		int iXMLBufRet;
		char *szXMLBufSys;
		int iXMLBufRetSys;
		if(set_bUserCmdSetDieMsgSize && (ClientInfo[iClientID].dieMsgSize == CS_SMALL)) {
			szXMLBuf = szBufSmall;
			iXMLBufRet = iRetSmall;
			szXMLBufSys = szBufSmallSys;
			iXMLBufRetSys = iRetSmallSys;
		} else {
			szXMLBuf = szBuf;
			iXMLBufRet = iRet;
			szXMLBufSys = szBufSys;
			iXMLBufRetSys = iRetSys;
		}

		if(!set_bUserCmdSetDieMsg)
		{ // /set diemsg disabled, thus send to all
			if(iSystemID == iClientSystemID)
				HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
			else
				HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
			continue;
		}

		if(ClientInfo[iClientID].dieMsg == DIEMSG_NONE)
			continue;
		else if((ClientInfo[iClientID].dieMsg == DIEMSG_SYSTEM) && (iSystemID == iClientSystemID))  
			HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
		else if((ClientInfo[iClientID].dieMsg == DIEMSG_SELF) && ((iClientID == iClientIDVictim) || (iClientID == iClientIDKiller)))
			HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
		else if(ClientInfo[iClientID].dieMsg == DIEMSG_ALL) {
			if(iSystemID == iClientSystemID)
				HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
			else
				HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
		}
	}
}

/**************************************************************************************************************
Called when ship was destroyed
**************************************************************************************************************/

struct SHIP_INFLICTOR
{
	SHIP_INFLICTOR(uint i) {iInflictor = i; fTotalDamage = 0.0f; fCollisionDamage = 0.0f; fGunDamage = 0.0f; fTorpedoDamage = 0.0f; fMissileDamage = 0.0f; fMineDamage = 0.0f; fOtherDamage = 0.0f;}
	bool operator==(SHIP_INFLICTOR si) { return si.iInflictor==iInflictor; }
	bool operator>=(SHIP_INFLICTOR si) { return si.iInflictor>=iInflictor; }
	bool operator<=(SHIP_INFLICTOR si) { return si.iInflictor<=iInflictor; }
	bool operator>(SHIP_INFLICTOR si) { return si.iInflictor>iInflictor; }
	bool operator<(SHIP_INFLICTOR si) { return si.iInflictor<iInflictor; }

	uint iInflictor;

	float fTotalDamage; //excluding OtherDamage
	float fCollisionDamage;
	float fGunDamage;
	float fTorpedoDamage;
	float fMissileDamage;
	float fMineDamage;
	float fOtherDamage;
};
struct FACTION_INFLICTOR
{
	FACTION_INFLICTOR(long i) {iInflictor = i; iNumShips = 0; fTotalDamage = 0.0f; fCollisionDamage = 0.0f; fGunDamage = 0.0f; fTorpedoDamage = 0.0f; fMissileDamage = 0.0f; fMineDamage = 0.0f; fOtherDamage = 0.0f;}
	bool operator==(FACTION_INFLICTOR fi) { return fi.iInflictor==iInflictor; }
	bool operator>=(FACTION_INFLICTOR fi) { return fi.iInflictor>=iInflictor; }
	bool operator<=(FACTION_INFLICTOR fi) { return fi.iInflictor<=iInflictor; }
	bool operator>(FACTION_INFLICTOR fi) { return fi.iInflictor>iInflictor; }
	bool operator<(FACTION_INFLICTOR fi) { return fi.iInflictor<iInflictor; }

	long iInflictor;
	uint iNumShips; //if 0 iInflictor is the (-)client-id, else it is affiliation

	float fTotalDamage; //excluding OtherDamage
	float fCollisionDamage;
	float fGunDamage;
	float fTorpedoDamage;
	float fMissileDamage;
	float fMineDamage;
	float fOtherDamage;
};
struct fFACTION_INFLICTOR
{
	fFACTION_INFLICTOR(FACTION_INFLICTOR fi) {iInflictor = fi.iInflictor; iNumShips = fi.iNumShips; fTotalDamage = fi.fTotalDamage; fCollisionDamage = fi.fCollisionDamage; fGunDamage = fi.fGunDamage; fTorpedoDamage = fi.fTorpedoDamage; fMissileDamage = fi.fMissileDamage; fMineDamage = fi.fMineDamage; fOtherDamage = fi.fOtherDamage;}
	bool operator==(fFACTION_INFLICTOR ffi) { return ffi.fTotalDamage==fTotalDamage; }
	bool operator>=(fFACTION_INFLICTOR ffi) { return ffi.fTotalDamage<=fTotalDamage; }
	bool operator<=(fFACTION_INFLICTOR ffi) { return ffi.fTotalDamage>=fTotalDamage; }
	bool operator>(fFACTION_INFLICTOR ffi) { return ffi.fTotalDamage<fTotalDamage; }
	bool operator<(fFACTION_INFLICTOR ffi) { return ffi.fTotalDamage>fTotalDamage; }

	long iInflictor;
	uint iNumShips; //if 0 iInflictor is the (-)client-id, else it is affiliation

	float fTotalDamage; //excluding OtherDamage
	float fCollisionDamage;
	float fGunDamage;
	float fTorpedoDamage;
	float fMissileDamage;
	float fMineDamage;
	float fOtherDamage;
};
struct DAMAGE_ENTRY
{
	DAMAGE_ENTRY(float d, uint c) { fDamage = d; iCause = c;}
	bool operator==(DAMAGE_ENTRY de) { return de.fDamage==fDamage; }
	bool operator>=(DAMAGE_ENTRY de) { return de.fDamage<=fDamage; }
	bool operator<=(DAMAGE_ENTRY de) { return de.fDamage>=fDamage; }
	bool operator>(DAMAGE_ENTRY de) { return de.fDamage<fDamage; }
	bool operator<(DAMAGE_ENTRY de) { return de.fDamage>fDamage; }

	float fDamage;
	uint iCause;
};

void __stdcall ShipDestroyed(DamageList *_dmg, char *szECX, uint iKill)
{
	try {
		DamageList dmg;
		try { dmg = *_dmg; } catch(...) { return; }
		// get client id
		char *szP;
		memcpy(&szP, szECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);

		if(iClientID && iKill) { // a player was killed
			//mobile docking - clear list of docked players, set them to die
			if(ClientInfo[iClientID].lstPlayersDocked.size())
			{
				uint iBaseID = Players[iClientID].iPrevBaseID;
				foreach(ClientInfo[iClientID].lstPlayersDocked, uint, dockedClientID)
				{
					ClientInfo[*dockedClientID].bMobileDocked = false;
					ClientInfo[*dockedClientID].iDockClientID = 0;
					ClientInfo[*dockedClientID].lstJumpPath.clear();
					uint iDockedShip;
					pub::Player::GetShip(*dockedClientID, iDockedShip);
					if(iDockedShip) //docked player is in space
					{
						Players[*dockedClientID].iPrevBaseID = iBaseID;
					}
					else
					{
						MOB_UNDOCKBASEKILL dKill;
						dKill.iClientID = *dockedClientID;
						dKill.iBaseID = iBaseID;
						dKill.bKill = true;
						lstUndockKill.push_back(dKill);
					}
				}
				ClientInfo[iClientID].lstPlayersDocked.clear();
			}
			//mobile docking - update docked base to one in current system
			if(ClientInfo[iClientID].bMobileDocked)
			{
				if(ClientInfo[iClientID].iDockClientID)
				{
					uint iBaseID;
					pub::GetBaseID(iBaseID, (HkGetPlayerSystemS(ClientInfo[iClientID].iDockClientID) + "_Mobile_Proxy_Base").c_str());
					Players[iClientID].iPrevBaseID = iBaseID;
				}
			}
			ClientInfo[iClientID].bCharInfoReqAfterDeath = true;

			//Generate death message
			try{
				if(!bSupressDeathMsg)
				{
					BinaryTree<SHIP_INFLICTOR> *btShipsInflict = new BinaryTree<SHIP_INFLICTOR>();
					while(ClientInfo[iClientID].lstDmgRec.size())
					{
						SHIP_INFLICTOR siFind = SHIP_INFLICTOR(ClientInfo[iClientID].lstDmgRec.front().iInflictor);
						SHIP_INFLICTOR *siFound = btShipsInflict->Find(&siFind);
						if(siFound)
						{
							if(set_bCombineMissileTorpMsgs)
							{
								switch(ClientInfo[iClientID].lstDmgRec.front().iCause)
								{
								case DC_GUN:
									siFound->fGunDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MISSILE:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MINE:
									siFound->fMineDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_COLLISION:
									siFound->fCollisionDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_TORPEDO:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								default:
									siFound->fOtherDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								}
							}
							else
							{
								switch(ClientInfo[iClientID].lstDmgRec.front().iCause)
								{
								case DC_GUN:
									siFound->fGunDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MISSILE:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MINE:
									siFound->fMineDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_COLLISION:
									siFound->fCollisionDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_TORPEDO:
									siFound->fTorpedoDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								default:
									siFound->fOtherDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								}
							}
						}
						else
						{
							SHIP_INFLICTOR *siFound = new SHIP_INFLICTOR(ClientInfo[iClientID].lstDmgRec.front().iInflictor);
							if(set_bCombineMissileTorpMsgs)
							{
								switch(ClientInfo[iClientID].lstDmgRec.front().iCause)
								{
								case DC_GUN:
									siFound->fGunDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MISSILE:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MINE:
									siFound->fMineDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_COLLISION:
									siFound->fCollisionDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_TORPEDO:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								default:
									siFound->fOtherDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								}
							}
							else
							{
								switch(ClientInfo[iClientID].lstDmgRec.front().iCause)
								{
								case DC_GUN:
									siFound->fGunDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MISSILE:
									siFound->fMissileDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_MINE:
									siFound->fMineDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_COLLISION:
									siFound->fCollisionDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								case DC_TORPEDO:
									siFound->fTorpedoDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									siFound->fTotalDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								default:
									siFound->fOtherDamage += ClientInfo[iClientID].lstDmgRec.front().fDamage;
									break;
								}
							}
							btShipsInflict->Add(siFound);
						}
						ClientInfo[iClientID].lstDmgRec.pop_front();
					}

					BinaryTree<FACTION_INFLICTOR> *btFactionsInflict = new BinaryTree<FACTION_INFLICTOR>();
					BinaryTreeIterator<SHIP_INFLICTOR> *shipsIter = new BinaryTreeIterator<SHIP_INFLICTOR>(btShipsInflict);
					shipsIter->First();
					for(long i=0; i<btShipsInflict->Count(); i++)
					{
						uint iKillerIDTest = HkGetClientIDByShip(shipsIter->Curr()->iInflictor);
						if(iKillerIDTest)
						{
							FACTION_INFLICTOR *fiPlayer = new FACTION_INFLICTOR(-(long)iKillerIDTest);
							fiPlayer->fTotalDamage = shipsIter->Curr()->fTotalDamage;
							fiPlayer->fGunDamage = shipsIter->Curr()->fGunDamage;
							fiPlayer->fMissileDamage = shipsIter->Curr()->fMissileDamage;
							fiPlayer->fMineDamage = shipsIter->Curr()->fMineDamage;
							fiPlayer->fCollisionDamage = shipsIter->Curr()->fCollisionDamage;
							fiPlayer->fTorpedoDamage = shipsIter->Curr()->fTorpedoDamage;
							fiPlayer->fOtherDamage = shipsIter->Curr()->fOtherDamage;
							btFactionsInflict->Add(fiPlayer);
						}
						else
						{
							int iRep;
							pub::SpaceObj::GetRep(shipsIter->Curr()->iInflictor, iRep);
							if(!iRep)
							{
								shipsIter->Next();
								continue;
							}
							uint iAffil;
							Reputation::Vibe::GetAffiliation(iRep, iAffil, false);
							if(!iAffil)
							{
								shipsIter->Next();
								continue;
							}
							FACTION_INFLICTOR fiFind = FACTION_INFLICTOR(iAffil);
							FACTION_INFLICTOR *fiFound = btFactionsInflict->Find(&fiFind);
							if(fiFound)
							{
								fiFound->iNumShips++;
								fiFound->fTotalDamage += shipsIter->Curr()->fTotalDamage;
								fiFound->fGunDamage += shipsIter->Curr()->fGunDamage;
								fiFound->fMissileDamage += shipsIter->Curr()->fMissileDamage;
								fiFound->fMineDamage += shipsIter->Curr()->fMineDamage;
								fiFound->fCollisionDamage += shipsIter->Curr()->fCollisionDamage;
								fiFound->fTorpedoDamage += shipsIter->Curr()->fTorpedoDamage;
								fiFound->fOtherDamage += shipsIter->Curr()->fOtherDamage;
							}
							else
							{
								fiFound = new FACTION_INFLICTOR(iAffil);
								fiFound->iNumShips++;
								fiFound->fTotalDamage = shipsIter->Curr()->fTotalDamage;
								fiFound->fGunDamage = shipsIter->Curr()->fGunDamage;
								fiFound->fMissileDamage = shipsIter->Curr()->fMissileDamage;
								fiFound->fMineDamage = shipsIter->Curr()->fMineDamage;
								fiFound->fCollisionDamage = shipsIter->Curr()->fCollisionDamage;
								fiFound->fTorpedoDamage = shipsIter->Curr()->fTorpedoDamage;
								fiFound->fOtherDamage = shipsIter->Curr()->fOtherDamage;
								btFactionsInflict->Add(fiFound);
							}
						}
						shipsIter->Next();
					}
					delete shipsIter;
					delete btShipsInflict;
					list<fFACTION_INFLICTOR> lstFactionsInflict;
					BinaryTreeIterator<FACTION_INFLICTOR> *factionsIter = new BinaryTreeIterator<FACTION_INFLICTOR>(btFactionsInflict);
					factionsIter->First();
					for(long i2=0; i2<btFactionsInflict->Count(); i2++)
					{
						lstFactionsInflict.push_back(*(factionsIter->Curr()));
						factionsIter->Next();
					}
					delete factionsIter;
					delete btFactionsInflict;

					wstring wscDeathMsg = L"Death: " + wstring(Players.GetActiveCharacterName(iClientID)) + L" was killed";
					if(!lstFactionsInflict.size())
					{
						uint iSystemID;
						pub::Player::GetSystem(iClientID, iSystemID);
						SendDeathMsg(wscDeathMsg + L".", iSystemID, iClientID, 0);
					}
					else
					{
						lstFactionsInflict.sort();
						uint iKillerID = 0;
						list<wstring> lstCauses;
						uint iNumCauses = set_iMaxDeathFactionCauses ? (set_iMaxDeathFactionCauses>lstFactionsInflict.size() ? lstFactionsInflict.size() : set_iMaxDeathFactionCauses) : lstFactionsInflict.size();
						uint j = 0;
						while(j < iNumCauses)
						{
							list<DAMAGE_ENTRY> lstDamages;
							if(lstFactionsInflict.back().fCollisionDamage)
								lstDamages.push_back(DAMAGE_ENTRY(lstFactionsInflict.back().fCollisionDamage, DC_COLLISION));
							if(lstFactionsInflict.back().fGunDamage)
								lstDamages.push_back(DAMAGE_ENTRY(lstFactionsInflict.back().fGunDamage, DC_GUN));
							if(lstFactionsInflict.back().fTorpedoDamage)
								lstDamages.push_back(DAMAGE_ENTRY(lstFactionsInflict.back().fTorpedoDamage, DC_TORPEDO));
							if(lstFactionsInflict.back().fMissileDamage)
								lstDamages.push_back(DAMAGE_ENTRY(lstFactionsInflict.back().fMissileDamage, DC_MISSILE));
							if(lstFactionsInflict.back().fMineDamage)
								lstDamages.push_back(DAMAGE_ENTRY(lstFactionsInflict.back().fMineDamage, DC_MINE));
							lstDamages.sort();
							list<wstring> lstTypes;
							uint iNumTypes = set_iMaxDeathEquipmentCauses ? (set_iMaxDeathEquipmentCauses>lstDamages.size() ? lstDamages.size() : set_iMaxDeathEquipmentCauses) : lstDamages.size();
							uint k = 0;
							while(k < iNumTypes)
							{
								switch(lstDamages.back().iCause)
								{
								case DC_GUN:
									lstTypes.push_back(L"guns");
									break;
								case DC_MISSILE:
									if(set_bCombineMissileTorpMsgs)
										lstTypes.push_back(L"missiles/torpedoes");
									else
										lstTypes.push_back(L"missiles");
									break;
								case DC_MINE:
									lstTypes.push_back(L"mines");
									break;
								case DC_COLLISION:
									lstTypes.push_back(L"collisions");
									break;
								case DC_TORPEDO:
									lstTypes.push_back(L"torpedoes");
									break;
								}
								lstDamages.pop_back();
								k++;
							}
							wstring wscDamages = L"";
							if(lstTypes.size())
							{
								if(lstTypes.size() == 1)
								{
									wscDamages = lstTypes.back();
								}
								else if(lstTypes.size() == 2)
								{
									wscDamages = lstTypes.front() + L" and " + lstTypes.back();
								}
								else
								{
									k = 0;
									foreach(lstTypes, wstring, wscType)
									{
										if(k == lstTypes.size()-1)
											wscDamages += L", and " + *wscType;
										else if(k == 0)
											wscDamages += *wscType;
										else
											wscDamages += L", " + *wscType;
										k++;
									}
								}
							}

							if(!lstFactionsInflict.back().iNumShips) //is player
							{
								if(j == 0)
								{
									iKillerID = -lstFactionsInflict.back().iInflictor;
									lstCauses.push_back(L"by " + wstring(Players.GetActiveCharacterName(iKillerID)) + (wscDamages.length() ? (L" with " + wscDamages) : L""));
								}
								else
								{
									lstCauses.push_back(L"by " + wstring(Players.GetActiveCharacterName(-lstFactionsInflict.back().iInflictor)) + (wscDamages.length() ? (L" with " + wscDamages) : L""));
								}
							}
							else //NPC
							{
								uint iNameID;
								pub::Reputation::GetShortGroupName(lstFactionsInflict.back().iInflictor, iNameID);
								if(!iNameID)
								{
									if(wscDamages == L"mines")
									{
										// Mines don't have affiliations
										lstCauses.push_back(L"by mines");
									}
									else
									{
										// seems to be erroneous, drop from list
										if(iNumCauses != lstFactionsInflict.size() + lstCauses.size())
											iNumCauses++;
										lstFactionsInflict.pop_back();
										j++;
										continue;
										/*if(lstFactionsInflict.back().iNumShips == 1)
										{
											lstCauses.push_back(L"by a ship" + (wscDamages.length() ? (L" with " + wscDamages) : L""));
										}
										else
											lstCauses.push_back(L"by " + stows(itos(lstFactionsInflict.back().iNumShips)) + L" ships" + (wscDamages.length() ? (L" with " + wscDamages) : L""));*/
									}
								}
								else
								{
									UINT_WRAP uw = UINT_WRAP(lstFactionsInflict.back().iInflictor);
									if(set_btOneFactionDeathRep->Find(&uw))
									{
										pub::Reputation::GetGroupName(lstFactionsInflict.back().iInflictor, iNameID);
										wstring wscGroupName = HkGetWStringFromIDS(iNameID);
										lstCauses.push_back(L"by " + wscGroupName + (wscDamages.length() ? (L" with " + wscDamages) : L""));
									}
									else
									{
										wstring wscGroupName = HkGetWStringFromIDS(iNameID);
										if(lstFactionsInflict.back().iNumShips == 1)
										{
											if(wscGroupName[wscGroupName.length()-1] == L's')
												wscGroupName = wscGroupName.substr(0, wscGroupName.length()-1);
											if(wscGroupName[0]==L'A' || wscGroupName[0]==L'E' || wscGroupName[0]==L'I' || wscGroupName[0]==L'O' || wscGroupName[0]==L'U')
												lstCauses.push_back(L"by an " + wscGroupName + L" ship" + (wscDamages.length() ? (L" with " + wscDamages) : L""));
											else
												lstCauses.push_back(L"by a " + wscGroupName + L" ship" + (wscDamages.length() ? (L" with " + wscDamages) : L""));
										}
										else
											lstCauses.push_back(L"by " + stows(itos(lstFactionsInflict.back().iNumShips)) + L" " + wscGroupName + (wscDamages.length() ? (L" with " + wscDamages) : L""));
									}
								}
							}
							j++;
							lstFactionsInflict.pop_back();
						}

						if(lstCauses.size() == 1)
						{
							wscDeathMsg += L" " + lstCauses.back() + L".";
						}
						else if(lstCauses.size() == 2)
						{
							wscDeathMsg += L" " + lstCauses.front() + L" and " + lstCauses.back() + L".";
						}
						else
						{
							uint k = 0;
							foreach(lstCauses, wstring, wscCause)
							{
								if(k == lstCauses.size()-1)
									wscDeathMsg += L"; and " + *wscCause + L".";
								else if(k == 0)
									wscDeathMsg += L" " + *wscCause;
								else
									wscDeathMsg += L"; " + *wscCause;
								k++;
							}
						}

						uint iSystemID;
						pub::Player::GetSystem(iClientID, iSystemID);
						SendDeathMsg(wscDeathMsg, iSystemID, iClientID, iKillerID);

						// Death penalty
						ClientInfo[iClientID].bDeathPenaltyOnEnter = true;
						HkPenalizeDeath(ARG_CLIENTID(iClientID), false);

						// MultiKillMessages
						if((set_MKM_bActivated) && iKillerID && (iClientID != iKillerID))
						{
							wstring wscKiller = Players.GetActiveCharacterName(iKillerID);

							ClientInfo[iKillerID].iKillsInARow++;
							foreach(set_MKM_lstMessages, MULTIKILLMESSAGE, it)
							{
								if(it->iKillsInARow == ClientInfo[iKillerID].iKillsInARow)
								{
									wstring wscXMLMsg = L"<TRA data=\"" + set_MKM_wscStyle + L"\" mask=\"-1\"/> <TEXT>";
									wscXMLMsg += XMLText(ReplaceStr(it->wscMessage, L"%player", wscKiller));
									wscXMLMsg += L"</TEXT>";
									
									char szBuf[0xFFFF];
									uint iRet;
									if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
										break;

									// for all players in system...
									struct PlayerData *pPD = 0;
									while(pPD = Players.traverse_active(pPD))
									{
										uint iClientID = HkGetClientIdFromPD(pPD);
										uint iClientSystemID = 0;
										pub::Player::GetSystem(iClientID, iClientSystemID);
										if((iClientID == iKillerID) || ((iSystemID == iClientSystemID) && (((ClientInfo[iClientID].dieMsg == DIEMSG_ALL) || (ClientInfo[iClientID].dieMsg == DIEMSG_SYSTEM)) || !set_bUserCmdSetDieMsg)))
											HkFMsgSendChat(iClientID, szBuf, iRet);
									}
								}
							}
						}

						// Adjust rep from kill
						if(iKillerID && (iClientID != iKillerID) && set_bChangeRepPvPDeath)
						{
							int iDeadRep;
							pub::Player::GetRep(iClientID, iDeadRep);
							uint iDeadRepGroupID;
							Reputation::Vibe::GetAffiliation(iDeadRep, iDeadRepGroupID, false);
							HkSetRepRelative(ARG_CLIENTID(iKillerID), iDeadRepGroupID, set_fRepChangePvP, set_fRepChangePvP ? true : false);
						}

						// Increment kill count
						if(iKillerID && (iClientID != iKillerID))
						{
							int iNumKills;
							pub::Player::GetNumKills(iKillerID, iNumKills);
							iNumKills++;
							pub::Player::SetNumKills(iKillerID, iNumKills);
						}
					}
				}
				else //Admin killed player
				{
					if(wscAdminKiller.length())
					{
						uint iSystemID;
						pub::Player::GetSystem(iClientID, iSystemID);
						SendDeathMsg(L"Death: " + wstring(Players.GetActiveCharacterName(iClientID)) + L" was vaporized by " + wscAdminKiller + L"'s .kill fury", iSystemID, iClientID, 0);
						wscAdminKiller = L"";
					}
				}
			}
			catch(...)
			{
				ClientInfo[iClientID].lstDmgRec.clear();

				uint iSystemID;
				pub::Player::GetSystem(iClientID, iSystemID);
				SendDeathMsg(L"Death: " + wstring(Players.GetActiveCharacterName(iClientID)) + L" was killed", iSystemID, iClientID, 0);
				AddLog("Exception while formulating death message");
			}

		}

		if(iClientID)
		{
			ClientInfo[iClientID].iShipOld = ClientInfo[iClientID].iShip;
			ClientInfo[iClientID].iShip = 0;
		}

		bSupressDeathMsg = false;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

FARPROC fpOldShipDestroyed;

__declspec(naked) void ShipDestroyedHook()
{
	__asm 
	{
		mov eax, [esp+0Ch] ; +4
		mov edx, [esp+4]
		push ecx
		push edx
		push ecx
		push eax
		call ShipDestroyed
		pop ecx
		mov eax, [fpOldShipDestroyed]
		jmp eax
	}
}
/**************************************************************************************************************
Called when base was destroyed
**************************************************************************************************************/

void BaseDestroyed(uint iObject, uint iClientIDBy)
{
	uint iID;
	pub::SpaceObj::GetSolarArchetypeID(iObject, iID);

	Universe::IBase *base = Universe::GetFirstBase();
	while(base)
	{
		if(base->iSpaceObjID == iObject)
			break;
		base = Universe::GetNextBase();
	}

	char *szBaseName = "";
	if(base)
	{
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
	}
	
	ProcessEvent(L"basedestroy basename=%s objecthash=%u solarhash=%u by=%s",
					stows(szBaseName).c_str(),
					iObject,
					iID,
					Players.GetActiveCharacterName(iClientIDBy));

}
