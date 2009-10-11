#include <fstream>
#include "hook.h"
#include "CInGame.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetCash(wstring wscCharname, int &iCash)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
		return HKE_NO_CHAR_SELECTED;
	else if((iClientID != -1) && !HkIsInCharSelectMenu(iClientID)) { // player logged in
		pub::Player::InspectCash(iClientID, iCash);
		return HKE_OK;
	} else { // player not logged in
		wstring wscDir;
		if(!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
		wstring wscFile;
		if(!HKHKSUCCESS(HkGetCharFileName(wscCharname, wscFile)) || !wscDir.length() || !wscFile.length())
			return HKE_COULD_NOT_GET_PATH;

		string scCharFile  = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
		if(HkIsEncoded(scCharFile)) {
			string scCharFileNew = scCharFile + ".ini";
			if(!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
				return HKE_COULD_NOT_DECODE_CHARFILE;

			iCash = IniGetI(scCharFileNew, "Player", "money", -1);
			DeleteFile(scCharFileNew.c_str());
		} else {
			iCash = IniGetI(scCharFile, "Player", "money", -1);
		}

		return HKE_OK;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkAddCash(wstring wscCharname, int iAmount)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	uint iClientIDAcc = 0;
	if(iClientID == -1) {
		acc = HkGetAccountByCharname(wscCharname);
		if(!acc)
			return HKE_CHAR_DOES_NOT_EXIST;
		iClientIDAcc = HkGetClientIdFromAccount(acc);
	} else
		iClientIDAcc = iClientID;

	if((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
		return HKE_NO_CHAR_SELECTED;
	else if((iClientID != -1) && !HkIsInCharSelectMenu(iClientID)) { // player logged in
		pub::Player::AdjustCash(iClientID, iAmount);
		return HKE_OK;
	} else { // player not logged in
		wstring wscDir;
		if(!HKHKSUCCESS(HkGetAccountDirName(acc, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
		
		wstring wscFile;
		if(!HKHKSUCCESS(HkGetCharFileName(wscCharname, wscFile)) || !wscDir.length() || !wscFile.length())
			return HKE_COULD_NOT_GET_PATH;

		string scCharFile  = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
		int iRet;
		if(HkIsEncoded(scCharFile)) {
			string scCharFileNew = scCharFile + ".ini";

			if(!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
				return HKE_COULD_NOT_DECODE_CHARFILE;

			iRet = IniGetI(scCharFileNew, "Player", "money", -1);
			IniWrite(scCharFileNew, "Player", "money", itos(iRet + iAmount));

			if(!flc_encode(scCharFileNew.c_str(), scCharFile.c_str()))
				return HKE_COULD_NOT_ENCODE_CHARFILE;

			DeleteFile(scCharFileNew.c_str());
		} else {
			iRet = IniGetI(scCharFile, "Player", "money", -1);
			IniWrite(scCharFile, "Player", "money", itos(iRet + iAmount));
		}

		if(HkIsInCharSelectMenu(wscCharname) || (iClientIDAcc != -1))
		{ // money fix in case player logs in with this account
			bool bFound = false;
			wscCharname = ToLower(wscCharname);
			foreach(ClientInfo[iClientIDAcc].lstMoneyFix, MONEY_FIX, i)
			{
				if((*i).wscCharname == wscCharname)
				{
					(*i).iAmount += iAmount;
					bFound = true;
				}
			}

			if(!bFound)
			{
				MONEY_FIX mf;
				mf.wscCharname = wscCharname;
				mf.iAmount = iAmount;
				ClientInfo[iClientIDAcc].lstMoneyFix.push_back(mf);
			}
		}

		return HKE_OK;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkKick(CAccount *acc)
{
	acc->ForceLogout();
	return HKE_OK;
}

HK_ERROR HkKick(wstring wscCharname)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	CAccount *acc = Players.FindAccountFromClientID(iClientID);
	acc->ForceLogout();
	return HKE_OK;
}

HK_ERROR HkKickReason(wstring wscCharname, wstring wscReason)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	if(wscReason.length())
		HkMsgAndKick(iClientID, wscReason, set_iKickMsgPeriod);
	else
		HkKick(Players.FindAccountFromClientID(iClientID));

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkBan(wstring wscCharname, bool bBan)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	if(iClientID != -1)
		acc = Players.FindAccountFromClientID(iClientID);
	else {
		if(!(acc = HkGetAccountByCharname(wscCharname)))
			return HKE_CHAR_DOES_NOT_EXIST;
	}

	wstring wscID = HkGetAccountID(acc);
	flstr *flStr = CreateWString(wscID.c_str());
	Players.BanAccount(*flStr, bBan);
	FreeWString(flStr);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkBeam(wstring wscCharname, wstring wscBasename)
{
	if(set_iBeamCmd == 3)
		return HkKillBeam(wscCharname, wscBasename);

	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	string scBasename = wstos(wscBasename);
	// check if ship in space
	uint iShip = 0;
	pub::Player::GetShip(iClientID, iShip);
	if(!iShip)
		return HKE_PLAYER_NOT_IN_SPACE;

	// get base id
	uint iBaseID;

	if(pub::GetBaseID(iBaseID, scBasename.c_str()) == -4)
	{
		string scBaseShortcut = IniGetS(set_scCfgCommandsFile, "names", wstos(wscBasename), "");
		if(!scBaseShortcut.length())
			return HKE_INVALID_BASENAME;

		if(pub::GetBaseID(iBaseID, scBaseShortcut.c_str()) == -4)
			return HKE_INVALID_BASENAME;
	}

	if(set_iBeamCmd != 1)
	{
		HkSaveChar(wscCharname); // maybe fixes crashes ?
		pub::Player::ForceLand(iClientID, iBaseID);
		HkSaveChar(wscCharname); // maybe fixes crashes ?
		if(set_iBeamCmd == 2)
			HkMsgAndKick(iClientID, L"To prevent a server crash", 1500); //fix crashes
	}
	else
	{
		uint iSysID;
		pub::Player::GetSystem(iClientID, iSysID);
		Universe::IBase* base = Universe::get_base(iBaseID);

		pub::Player::ForceLand(iClientID, iBaseID); // beam

		// if not in the same system, emulate F1 charload
		if(base->iSystemID != iSysID)
		{
			Server.BaseEnter(iBaseID,iClientID);
			Server.BaseExit(iBaseID,iClientID);
			wstring wscCharFileName;
			HkGetCharFileName(ARG_CLIENTID(iClientID),wscCharFileName);
			wscCharFileName += L".fl";
			CHARACTER_ID cID;
			strcpy(cID.charfilename,wstos(wscCharFileName.substr(0,14)).c_str());
			Server.CharacterSelect(cID, iClientID);
		}
	}

	return HKE_OK;
}

HK_ERROR HkBeamInSys(wstring wscCharname, Vector vOffsetVector, Matrix mOrientation)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iShip, iSystemID;
	pub::Player::GetShip(iClientID, iShip);
	pub::Player::GetSystem(iClientID, iSystemID);

	Quaternion qRotation = HkMatrixToQuaternion(mOrientation);
	
	LAUNCH_PACKET* ltest = new LAUNCH_PACKET;
	ltest->iShip = iShip;
	ltest->iDunno[0] = 0;
	ltest->iDunno[1] = 0xFFFFFFFF;
	ltest->fRotate[0] = qRotation.w;
	ltest->fRotate[1] = qRotation.x;
	ltest->fRotate[2] = qRotation.y;
	ltest->fRotate[3] = qRotation.z;
	ltest->fPos[0] = vOffsetVector.x;
	ltest->fPos[1] = vOffsetVector.y;
	ltest->fPos[2] = vOffsetVector.z;

	char* ClientOffset = (char*)hModRemoteClient + ADDR_RMCLIENT_CLIENT;
	char* SendLaunch = (char*)hModRemoteClient + ADDR_RMCLIENT_LAUNCH;

	__asm {
		mov ecx, ClientOffset
		mov ecx, [ecx]
		push [ltest]
		push [iClientID]
		call SendLaunch
	}

	pub::SpaceObj::Relocate(iShip, iSystemID, vOffsetVector, mOrientation);

	delete ltest;

	return HKE_OK;
}

HK_ERROR HkInstantDock(wstring wscCharname, uint iDockObj)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if(!iShip)
		return HKE_PLAYER_NOT_IN_SPACE;

	uint iSystemID, iSystemID2;
	pub::SpaceObj::GetSystem(iShip, iSystemID);
	pub::SpaceObj::GetSystem(iDockObj, iSystemID2);
	if(iSystemID!=iSystemID2)
		return HKE_SYSTEM_NO_MATCH;

	try {
		pub::SpaceObj::InstantDock(iShip, iDockObj, 1);
	} catch(...) { return HKE_OBJECT_NO_DOCK; }

	return HKE_OK;
}

vector< list<CARGO_INFO> > vRestoreKBeamCargo;
vector<uint> vRestoreKBeamClientIDs;
bool bSupressDeathMsg = false;
HK_ERROR HkKillBeam(wstring wscCharname, wstring wscBasename)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	string scBasename = wstos(wscBasename);
	// check if ship in space
	uint iBase = 0;
	pub::Player::GetBase(iClientID, iBase);
	if(iBase)
		return HKE_PLAYER_NOT_IN_SPACE;

	// get base id
	uint iBaseID;

	if(pub::GetBaseID(iBaseID, scBasename.c_str()) == -4)
	{
		string scBaseShortcut = IniGetS(set_scCfgCommandsFile, "names", wstos(wscBasename), "");
		if(!scBaseShortcut.length())
			return HKE_INVALID_BASENAME;

		if(pub::GetBaseID(iBaseID, scBaseShortcut.c_str()) == -4)
			return HKE_INVALID_BASENAME;
		
		wscBasename = stows(scBaseShortcut);
	}

	list<CARGO_INFO> lstBeforeCargo;
	HkEnumCargo(ARG_CLIENTID(iClientID), lstBeforeCargo, 0);
	vRestoreKBeamClientIDs.push_back(iClientID);
	vRestoreKBeamCargo.push_back(lstBeforeCargo);

	Players[iClientID].iPrevBaseID = iBaseID;
	bSupressDeathMsg = true;
	HkKill(ARG_CLIENTID(iClientID));

	PrintUserCmdText(iClientID, L"You will respawn at " + HkGetWStringFromIDS(Universe::get_base(iBaseID)->iBaseIDS) + L".");
	PrintUserCmdText(iClientID, L"If you do not click \"RESPAWN\" your cargo will be lost!");
	return HKE_OK;
}	

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkSaveChar(wstring wscCharname)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	void *pJmp = (char*)hModServer + 0x7EFA8;
	char szNop[2] = { '\x90', '\x90' };
	char szTestAlAl[2] = { '\x74', '\x44' };
	WriteProcMem(pJmp, szNop, sizeof(szNop)); // nop the SinglePlayer() check
	pub::Save(iClientID, 1);
	WriteProcMem(pJmp, szTestAlAl, sizeof(szTestAlAl)); // restore

	return HKE_OK;
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

HK_ERROR HkEnumCargo(wstring wscCharname, list<CARGO_INFO> &lstCargo, int *iRemainingHoldSize)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1 || HkIsInCharSelectMenu(iClientID))
		return HKE_PLAYER_NOT_LOGGED_IN;

	lstCargo.clear();

	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);

	EQ_ITEM *eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM *eq;
	eq = eqLst->next;
	while(eq != eqLst)
	{
		CARGO_INFO ci = {eq->sID, eq->iCount, eq->iGoodID, eq->bMission, eq->bMounted};
		lstCargo.push_back(ci);

		eq = eq->next;
	}

	if(iRemainingHoldSize)
	{
		float fRemHold;
		pub::Player::GetRemainingHoldSize(iClientID, fRemHold);
		*iRemainingHoldSize = (int)fRemHold;
	}
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkRemoveCargo(wstring wscCharname, uint iID, int iCount)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1 || HkIsInCharSelectMenu(iClientID))
		return HKE_PLAYER_NOT_LOGGED_IN;

	list <CARGO_INFO> lstCargo;
	HkEnumCargo(wscCharname, lstCargo, 0);
	foreach(lstCargo, CARGO_INFO, it)
	{
		if((*it).iID == iID && ((*it).iCount < iCount || iCount == -1))
			iCount = (*it).iCount; // trying to remove more than actually there, thus fix
	}

	pub::Player::RemoveCargo(iClientID, iID, iCount);

	// anti-cheat related
/*	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);
	EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;
	const EquipDesc *ed = edlList->find_equipment_item(iID);
	if(ed)
	{
		ed->get_id();
		edlList->remove_equipment_item(
	} */


	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, int iCount, bool bMission)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	uint iClientIDAcc = 0;
	if(iClientID == -1) {
		acc = HkGetAccountByCharname(wscCharname);
		if(!acc)
			return HKE_CHAR_DOES_NOT_EXIST;
		iClientIDAcc = HkGetClientIdFromAccount(acc);
	} else
		iClientIDAcc = iClientID;

	if((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
		return HKE_NO_CHAR_SELECTED;
	else if((iClientID != -1) && !HkIsInCharSelectMenu(iClientID))
	{ // player logged in
		// anti-cheat related
		char *szClassPtr;
		memcpy(&szClassPtr, &Players, 4);
		szClassPtr += 0x418 * (iClientID - 1);
		EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;

		// add
		const GoodInfo *gi;
		if(!(gi = GoodList::find_by_id(iGoodID)))
			return HKE_INVALID_GOOD;

		//Check to make sure cargo will fit in hold
		float fRemainingHold;
		pub::Player::GetRemainingHoldSize(iClientID, fRemainingHold);
		Archetype::Equipment *eq = Archetype::GetEquipment(iGoodID);
		if(eq->fVolume*iCount > fRemainingHold)
			return HKE_CARGO_WONT_FIT;

		bool bMultiCount;
		memcpy(&bMultiCount, (char*)gi + 0x70, 1);

		if(bMultiCount) { // it's a good that can have multiple units(commodities, missile ammo, etc)
			// we need to do this, else server or client may crash
			list<CARGO_INFO> lstCargo;
			HkEnumCargo(wscCharname, lstCargo, 0);
			foreach(lstCargo, CARGO_INFO, it)
			{
				if(((*it).iArchID == iGoodID) && ((*it).bMission != bMission))
				{
					HkRemoveCargo(wscCharname, (*it).iID, (*it).iCount);
					iCount += (*it).iCount;
				}
			}

			pub::Player::AddCargo(iClientID, iGoodID, iCount, 1, bMission);
		} else {
			for(int i = 0; (i < iCount); i++)
				pub::Player::AddCargo(iClientID, iGoodID, 1, 1, bMission);
		}

		uint iBase = 0;
		pub::Player::GetBase(iClientID, iBase);
		if(iBase)
		{ // player docked on base
			///////////////////////////////////////////////////
			// fix, else we get anti-cheat msg when undocking
			// this DOES NOT disable anti-cheat-detection, we're
			// just making some adjustments so that we dont get kicked
			
			// fix "Ship or Equipment not sold on base" kick
			// get last equipid
			char *szLastEquipID = szClassPtr + 0x3C8;
			ushort sEquipID;
			memcpy(&sEquipID, szLastEquipID, 2);

			// add to check-list which is being compared to the users equip-list when saving char
			EquipDesc ed;
			memset(&ed, 0, sizeof(ed));
			ed.id = sEquipID;
			ed.count = iCount;
			ed.archid = iGoodID;
			try{ edlList->add_equipment_item(ed, true); } catch(...) { edlList->add_equipment_item(ed, false); }

			// fix "Ship Related" kick, update crc
			ulong lCRC;
			__asm
			{
				mov ecx, [szClassPtr]
				call [CRCAntiCheat]
				mov [lCRC], eax
			}
			memcpy(szClassPtr + 0x320, &lCRC, 4);
		}

		return HKE_OK;
	}
	else
	{ // player not logged in
		Archetype::Equipment *eq = Archetype::GetEquipment(iGoodID);
		if(!eq)
			return HKE_INVALID_GOOD;
		list<wstring> lstCharFile;
		HK_ERROR err = HkReadCharFile(wscCharname, lstCharFile);
		if(!HKHKSUCCESS(err))
			return err;
		wstring wscCharFile = L"";
		bool bAddedCargo = false;
		uint iShipArchID = 0;
		float fCargoHold = 0.0f, fSizeCargo = eq->fVolume*iCount;
		foreach(lstCharFile, wstring, line)
		{
			wstring wscNewLine = *line;
			if(!bAddedCargo)
			{
				wstring wscKey = Trim(line->substr(0,line->find(L"=")));
				if(wscKey == L"cargo")
				{
					int iFindEqual = line->find(L"=");
					if(iFindEqual == -1)
					{
						wscCharFile += wscNewLine + L"\n";
						continue;
					}
					int iFindComma = line->find(L",", iFindEqual);
					if(iFindComma == -1)
					{
						wscCharFile += wscNewLine + L"\n";
						continue;
					}
					uint iFileGoodID = ToUint(Trim(line->substr(iFindEqual+1,iFindComma)));
					uint iFileGoodCount = ToUint(Trim(line->substr(iFindComma+1,line->find(L",",iFindComma+1))));
					Archetype::Equipment *eq = Archetype::GetEquipment(iFileGoodID);
					if(eq)
						fSizeCargo += iFileGoodCount * eq->fVolume;
				}
				else if(wscKey == L"ship_archetype")
				{
					iShipArchID = ToUint(Trim(line->substr(line->find(L"=")+1, line->length())));
					Archetype::Ship *ship = Archetype::GetShip(iShipArchID);
					if(ship)
						fCargoHold = ship->fHoldSize;
				}
				else if(wscKey == L"last_base")
				{
					if(fSizeCargo > fCargoHold)
						return HKE_CARGO_WONT_FIT;
					wscCharFile = wscCharFile.substr(0, wscCharFile.length()-1);
					wscNewLine = L"cargo = " + stows(utos(iGoodID)) + L", " + stows(itos(iCount)) + L", , , 0\n\n" + wscNewLine;
					bAddedCargo = true;
				}
			}
			wscCharFile += wscNewLine + L"\n";
		}
		wscCharFile = wscCharFile.substr(0, wscCharFile.length()-1);
		err = HkWriteCharFile(wscCharname, wscCharFile);
		if(!HKHKSUCCESS(err))
			return err;

		if(HkIsInCharSelectMenu(wscCharname) || (iClientIDAcc != -1))
		{ // cargo fix in case player logs in with this account
			bool bFound = false;
			wscCharname = ToLower(wscCharname);
			foreach(ClientInfo[iClientIDAcc].lstCargoFix, CARGO_FIX, i)
			{
				if(i->wscCharname == wscCharname && i->iGoodID == iGoodID)
				{
					i->iCount += iCount;
					bFound = true;
				}
			}

			if(!bFound)
			{
				CARGO_FIX cf;
				cf.wscCharname = wscCharname;
				cf.iCount = iCount;
				cf.iGoodID = iGoodID;
				cf.bMission = bMission;
				ClientInfo[iClientIDAcc].lstCargoFix.push_back(cf);
			}
		}

		return HKE_OK;
	}
}

HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, CacheString *hardpoint, bool bMission)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	uint iClientIDAcc = 0;
	if(iClientID == -1) {
		acc = HkGetAccountByCharname(wscCharname);
		if(!acc)
			return HKE_CHAR_DOES_NOT_EXIST;
		iClientIDAcc = HkGetClientIdFromAccount(acc);
	} else
		iClientIDAcc = iClientID;

	if((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
		return HKE_NO_CHAR_SELECTED;
	else if((iClientID != -1) && !HkIsInCharSelectMenu(iClientID))
	{ // player logged in
		// anti-cheat related
		char *szClassPtr;
		memcpy(&szClassPtr, &Players, 4);
		szClassPtr += 0x418 * (iClientID - 1);
		EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;

		// add
		const GoodInfo *gi;
		if(!(gi = GoodList::find_by_id(iGoodID)))
			return HKE_INVALID_GOOD;

		int iOne = 1;
		int iMission = bMission;
		float fHealth = 1;
		CacheString **pHP = &hardpoint;
		__asm
		{
			push iOne
			push iMission
			push iOne
			push fHealth
			push iOne
			push pHP
			push iGoodID
			mov ecx, szClassPtr
			call AddCargoDocked
		}

		uint iBase = 0;
		pub::Player::GetBase(iClientID, iBase);
		if(iBase)
		{ // player docked on base
			///////////////////////////////////////////////////
			// fix, else we get anti-cheat msg when undocking
			// this DOES NOT disable anti-cheat-detection, we're
			// just making some adjustments so that we dont get kicked
			
			// fix "Ship or Equipment not sold on base" kick
			// get last equipid
			char *szLastEquipID = szClassPtr + 0x3C8;
			ushort sEquipID;
			memcpy(&sEquipID, szLastEquipID, 2);

			// add to check-list which is being compared to the users equip-list when saving char
			EquipDesc ed;
			memset(&ed, 0, sizeof(ed));
			ed.id = sEquipID;
			ed.count = 1;
			ed.archid = iGoodID;
			try{ edlList->add_equipment_item(ed, true); } catch(...) { edlList->add_equipment_item(ed, false); }

			// fix "Ship Related" kick, update crc
			ulong lCRC;
			__asm
			{
				mov ecx, [szClassPtr]
				call [CRCAntiCheat]
				mov [lCRC], eax
			}
			memcpy(szClassPtr + 0x320, &lCRC, 4);

			return HKE_OK;
		}
	}
	else
	{ // player not logged in
		Archetype::Equipment *eq = Archetype::GetEquipment(iGoodID);
		if(!eq)
			return HKE_INVALID_GOOD;
		list<wstring> lstCharFile;
		HK_ERROR err = HkReadCharFile(wscCharname, lstCharFile);
		if(!HKHKSUCCESS(err))
			return err;
		wstring wscCharFile = L"";
		bool bAddedCargo = false;
		uint iShipArchID = 0;
		foreach(lstCharFile, wstring, line)
		{
			wstring wscNewLine = *line;
			if(!bAddedCargo)
			{
				wstring wscKey = Trim(line->substr(0,line->find(L"=")));
				if(wscKey == L"last_base")
				{
					wscCharFile = wscCharFile.substr(0, wscCharFile.length()-1);
					wscNewLine = L"equip = " + stows(utos(iGoodID)) + L", " + stows((char*)hardpoint) + L", 1\n\n" + wscNewLine;
					bAddedCargo = true;
				}
			}
			wscCharFile += wscNewLine + L"\n";
		}
		wscCharFile = wscCharFile.substr(0, wscCharFile.length()-1);
		err = HkWriteCharFile(wscCharname, wscCharFile);
		if(!HKHKSUCCESS(err))
			return err;

		if(HkIsInCharSelectMenu(wscCharname) || (iClientIDAcc != -1))
		{ // cargo fix in case player logs in with this account
			bool bFound = false;
			wscCharname = ToLower(wscCharname);
			foreach(ClientInfo[iClientIDAcc].lstCargoFix, CARGO_FIX, i)
			{
				if(i->wscCharname == wscCharname && i->iGoodID == iGoodID)
				{
					i->iCount++;
					bFound = true;
				}
			}

			if(!bFound)
			{
				CARGO_FIX cf;
				cf.wscCharname = wscCharname;
				cf.iCount = 1;
				cf.iGoodID = iGoodID;
				cf.bMission = bMission;
				ClientInfo[iClientIDAcc].lstCargoFix.push_back(cf);
			}
		}

		return HKE_OK;
	}
}

HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, int iCount, bool bMission)
{
	uint iGoodID = ToInt(wscGood.c_str());
	if(!iGoodID)
		pub::GetGoodID(iGoodID, wstos(wscGood).c_str());
	if(!iGoodID)
		return HKE_INVALID_GOOD;

	return HkAddCargo(wscCharname, iGoodID, iCount, bMission);
}

HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, wstring wscHardpoint, bool bMission)
{
	uint iGoodID = ToInt(wscGood.c_str());
	if(!iGoodID)
		pub::GetGoodID(iGoodID, wstos(wscGood).c_str());
	if(!iGoodID)
		return HKE_INVALID_GOOD;
	
	CacheString *hardpoint = (CacheString*)StringAlloc(wstos(wscHardpoint).c_str(), false);

	return HkAddCargo(wscCharname, iGoodID, hardpoint, bMission);
}

HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, string scHardpoint, bool bMission)
{	
	CacheString *hardpoint = (CacheString*)StringAlloc(scHardpoint.c_str(), false);
	return HkAddCargo(wscCharname, iGoodID, hardpoint, bMission);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkRename(wstring wscCharname, wstring wscNewCharname, bool bOnlyDelete)
{
	HkSaveChar(wscCharname); //Fixes weird rename behavior

	HK_GET_CLIENTID(iClientID, wscCharname);

	if((iClientID == -1) && !HkGetAccountByCharname(wscCharname))
		return HKE_CHAR_DOES_NOT_EXIST;

	if(!bOnlyDelete && HkGetAccountByCharname(wscNewCharname))
		return HKE_CHARNAME_ALREADY_EXISTS;

	if(!bOnlyDelete && (wscNewCharname.length() > 23))
		return HKE_CHARNAME_TOO_LONG;
	
	if(!bOnlyDelete && !wscNewCharname.length())
		return HKE_CHARNAME_TOO_SHORT;

	CAccount *acc;
	wstring wscOldCharname;
	if(iClientID != -1) {
		acc = Players.FindAccountFromClientID(iClientID);
		wscOldCharname = Players.GetActiveCharacterName(iClientID);
	} else {
		wscOldCharname = wscCharname;
		acc = HkGetAccountByCharname(wscCharname);
	}

	wstring wscAccountDirname;
	if(!HKHKSUCCESS(HkGetAccountDirName(acc, wscAccountDirname)) || !wscAccountDirname.length())//Could not get account dir name
		return HKE_COULD_NOT_GET_PATH;
	wstring wscNewFilename;
	if(!bOnlyDelete && (!HKHKSUCCESS(HkGetCharFileName(wscNewCharname, wscNewFilename)) || !wscNewFilename.length()))//Could not get filename of new char
		return HKE_COULD_NOT_GET_PATH;
	wstring wscOldFilename;
	if(!HKHKSUCCESS(HkGetCharFileName(wscOldCharname, wscOldFilename)) || !wscOldFilename.length())
		return HKE_COULD_NOT_GET_PATH;

	string scNewCharfilePath = scAcctPath + wstos(wscAccountDirname) + "\\" + wstos(wscNewFilename) + ".fl";
	string scOldCharfilePath = scAcctPath + wstos(wscAccountDirname) + "\\" + wstos(wscOldFilename) + ".fl";
	if(!bOnlyDelete)
	{
		if(!flc_decode(scOldCharfilePath.c_str(), scNewCharfilePath.c_str()))
		{ // file wasn't encoded, thus simply rename it
			DeleteFile(scNewCharfilePath.c_str()); // just to get sure...
			CopyFile(scOldCharfilePath.c_str(), scNewCharfilePath.c_str(), FALSE);
		}
	}

	// delete old character
	flstr *str = CreateWString(wscOldCharname.c_str());
	HkLockAccountAccess(acc, true); // also kicks player on this account
	bool bRet = Players.DeleteCharacterFromName(*str);
	HkUnlockAccountAccess(acc);
	FreeWString(str);

	DeleteFile(scOldCharfilePath.c_str()); // doesn't delete when previously renamed, dunno why
	
	if(bOnlyDelete)
		return HKE_OK; // only delete char, thus we're finished

	wstring wscNewNameString = L"";
	for(uint i = 0; (i < wscNewCharname.length()); i++)
	{
		char cHiByte = wscNewCharname[i] >> 8;
		char cLoByte = wscNewCharname[i] & 0xFF;
		wchar_t wszBuf[8];
		swprintf(wszBuf, L"%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		wscNewNameString += wszBuf;
	}

	IniWrite(scNewCharfilePath, "Player", "Name", wstos(wscNewNameString));

	// we'll also need to add the new character into the flname searchtree in memory(which is usually build when
	// flserver starts). methods like FindAccountByCharacterName won't work else

	// insert into flname search tree
	struct TREENODE
	{
		TREENODE *pLeft;
		TREENODE *pParent;
		TREENODE *pRight;
		ulong l1;
		char *szFLName;
		uint lLength;
		ulong l2; // ??
		CAccount *acc;
	};

	char *pEBP;
	pEBP = (char*)&Players + 0x30;
	char *pEDI;
	memcpy(&pEDI, pEBP + 4, 4);
	TREENODE *leaf;
	memcpy(&leaf, pEBP + 8, 4);
	// edi+4 is where the root node is
	TREENODE *node;
	memcpy(&node, pEDI + 4, 4);

	bool bLeft = true;
	TREENODE *lastparent = (TREENODE*)(pEDI + 4);
	while(node != leaf)
	{
		lastparent = node;
		int iRet = strcmp(node->szFLName, wstos(wscNewFilename + L".fl").c_str());
		if(iRet >= 0) { // leftnode?
			node = node->pLeft;
			bLeft = true;
		} else { // rightnode?
			node = node->pRight;
			bLeft = false;
		}
	}

	// we need to use fl's new operator, else free might fail later when char gets deleted
	typedef void* (*_flnew)(unsigned int);
	_flnew flnew = (_flnew) SRV_ADDR(ADDR_FLNEW);

	TREENODE *tn = (TREENODE *)flnew(sizeof(TREENODE));
	memset(tn, 0, sizeof(TREENODE));
	tn->pLeft = leaf;
	tn->pParent = lastparent;
	tn->pRight = leaf;
	tn->szFLName = new char[32];
	strcpy(tn->szFLName, wstos(wscNewFilename + L".fl").c_str());
	tn->lLength = (uint)strlen(tn->szFLName);
	tn->l2 = 0x1F; // seems to be always 0x1F
	tn->acc = acc;
	if(bLeft)
		lastparent->pLeft = tn;
	else
		lastparent->pRight = tn;

	// increment character counter
		uint iNumberOfChars;
		memcpy(&iNumberOfChars, (char*)acc + 0x2C, 4);
		iNumberOfChars++;
		memcpy((char*)acc + 0x2C, &iNumberOfChars, 4);

	// finally we need to add the new char to the CAccount character list
	// else it will not be shown in the accounts list in the flserver window
	struct LISTNODE
	{
		LISTNODE *next;
		LISTNODE *prev;
		u_long   l1;
		wchar_t *wszCharname;
		ulong lArray[32];
	};
	LISTNODE *lnHead;
	memcpy(&lnHead, (char*)acc + 0x28, 4);

	LISTNODE *lnCur = lnHead->next;
	while(lnCur->next != lnHead)
		lnCur = lnCur->next;

	LISTNODE *ln = (LISTNODE *)flnew(sizeof(LISTNODE));
	ln->next = lnHead;
	ln->prev = lnCur;
	ln->wszCharname = new wchar_t[32];
	wcscpy(ln->wszCharname, wscNewCharname.c_str());
	lnCur->next = ln;

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkMsgAndKick(uint iClientID, wstring wscReason, uint iIntervall)
{
	if(!ClientInfo[iClientID].tmKickTime)
	{
		wstring wscMsg = ReplaceStr(set_wscKickMsg, L"%reason", XMLText(wscReason));
		HkFMsg(iClientID, wscMsg);
		ClientInfo[iClientID].tmKickTime = timeInMS() + iIntervall;
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkKill(wstring wscCharname)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if(!iShip)
		return HKE_PLAYER_NOT_IN_SPACE;

	bSupressDeathMsg = true;
	pub::SpaceObj::SetRelativeHealth(iShip, 0.0f);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkRemoveCargoByGoodID(uint iClientID, uint iGoodID, int iCount)
{
	ushort sID = 0;
	HkGetSIDFromGoodID(iClientID, sID, iGoodID);
	if(sID)
	{
		pub::Player::RemoveCargo(iClientID, sID, iCount);
		return true;
	}
	return false;
}

struct REMOVED_ITEM
{
	REMOVED_ITEM(uint good, uint count) {iGoodID = good; iCount = count; }
	REMOVED_ITEM(uint good) {iGoodID = good; iCount = 0; }
	uint iGoodID;
	uint iCount;
	bool operator==(REMOVED_ITEM ri) { return ri.iGoodID==iGoodID; }
	bool operator>=(REMOVED_ITEM ri) { return ri.iGoodID>=iGoodID; }
	bool operator<=(REMOVED_ITEM ri) { return ri.iGoodID<=iGoodID; }
	bool operator>(REMOVED_ITEM ri) { return ri.iGoodID>iGoodID; }
	bool operator<(REMOVED_ITEM ri) { return ri.iGoodID<iGoodID; }
};
struct TEMP_ITEM
{
	TEMP_ITEM(uint good, uint count) {iID = good; iCount = count; }
	ushort iID;
	uint iCount;
};

void HkRemoveDeathPenaltyItems(uint iClientID)
{
	BinaryTree<REMOVED_ITEM> *btRemovedItems = new BinaryTree<REMOVED_ITEM>();
	list<TEMP_ITEM> lstDelayRemoveItems;

	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);

	EQ_ITEM *eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM *eq;
	eq = eqLst->next;
	while(eq != eqLst)
	{
		DEATH_ITEM diFind = DEATH_ITEM(eq->iGoodID);
		DEATH_ITEM *diFound = set_btDeathItems->Find(&diFind);
		if(diFound)
		{
			float fRand = (float)rand()/(float)RAND_MAX;
			if(diFound->fChance >= fRand)
			{
				//pub::Player::RemoveCargo(iClientID, eq->sID, eq->iCount);  //BAD, modifies item data structure
				REMOVED_ITEM *riFind = new REMOVED_ITEM(eq->iGoodID, eq->iCount);
				lstDelayRemoveItems.push_back(TEMP_ITEM(eq->sID, eq->iCount));
				REMOVED_ITEM *riFound = btRemovedItems->Find(riFind);
				if(riFound)
				{
					riFound->iCount += eq->iCount;
					delete riFind;
				}
				else
				{
					btRemovedItems->Add(riFind);
				}
			}
		}

		eq = eq->next;
	}

	foreach(lstDelayRemoveItems, TEMP_ITEM, tempItem)
	{
		pub::Player::RemoveCargo(iClientID, tempItem->iID, tempItem->iCount);
	}

	if(btRemovedItems->Count())
	{
 		BinaryTreeIterator<REMOVED_ITEM> *btiItemIter = new BinaryTreeIterator<REMOVED_ITEM>(btRemovedItems);
		if(btRemovedItems->Count() == 1)
		{
			REMOVED_ITEM *ri = btiItemIter->First();
			const GoodInfo *gi = GoodList::find_by_id(ri->iGoodID);
			if(gi && gi->iIDS)
			{
				PrintUserCmdText(iClientID, L"Death penalty: removed %u %s.", ri->iCount, (HkGetWStringFromIDS(gi->iIDS) + (ri->iCount>1 ? L"s" : L"")).c_str());
			}
		}
		else if(btRemovedItems->Count() == 2)
		{
			REMOVED_ITEM *ri1 = btiItemIter->First();
			REMOVED_ITEM *ri2 = btiItemIter->Next();
			const GoodInfo *gi1 = GoodList::find_by_id(ri1->iGoodID);
			const GoodInfo *gi2 = GoodList::find_by_id(ri2->iGoodID);
			if(gi1 && gi1->iIDS && gi2 && gi2->iIDS)
			{
				PrintUserCmdText(iClientID, L"Death penalty: removed %u %s and %u %s.", ri1->iCount, (HkGetWStringFromIDS(gi1->iIDS) + (ri1->iCount>1 ? L"s" : L"")).c_str(), ri2->iCount, stows(HkGetStringFromIDS(gi2->iIDS) + (ri2->iCount>1 ? "s" : "")).c_str());
			}
			else if(gi1 && gi1->iIDS)
			{
				PrintUserCmdText(iClientID, L"Death penalty: removed %u %s.", ri1->iCount, (HkGetWStringFromIDS(gi1->iIDS) + (ri1->iCount>1 ? L"s" : L"")).c_str());
			}
			else if(gi2 && gi2->iIDS)
			{
				PrintUserCmdText(iClientID, L"Death penalty: removed %u %s.", ri2->iCount, (HkGetWStringFromIDS(gi2->iIDS) + (ri2->iCount>1 ? L"s" : L"")).c_str());
			}
		}
		else
		{
			list<wstring> lstItems;
			btiItemIter->First();
			for(long i=0; i<btRemovedItems->Count(); i++)
			{
				const GoodInfo *gi = GoodList::find_by_id(btiItemIter->Curr()->iGoodID);
				if(gi && gi->iIDS)
				{
					lstItems.push_back(stows(itos(btiItemIter->Curr()->iCount)) + L" " + HkGetWStringFromIDS(gi->iIDS) + (btiItemIter->Curr()->iCount>1 ? L"s" : L""));
				}
				btiItemIter->Next();
			}
			wstring wscItems = L"";
			list<wstring>::iterator wscItem = lstItems.begin();
			list<wstring>::iterator wscItemEnd = lstItems.end();
			--wscItemEnd;
			for(;wscItem != wscItemEnd; wscItem++)
			{
				wscItems += *wscItem + L", ";
			}
			wscItems += L"and " + lstItems.back();
			PrintUserCmdText(iClientID, L"Death penalty: removed %s.", wscItems.c_str());
		}
		delete btiItemIter;
	}

	delete btRemovedItems;
}

void HkPenalizeDeath(wstring wscCharname, uint iKillerID, bool bSecondTime)
{
	if(!set_fDeathPenalty)
		return;

	HK_GET_CLIENTID_NORETURN(iClientID, wscCharname);

	if(ClientInfo[iClientID].bDeathPenaltyOnEnter)
	{
		if(iClientID != -1)
		{
			UINT_WRAP uwShip = UINT_WRAP(Players[iClientID].iShipArchID);
			UINT_WRAP uwSystem = UINT_WRAP(Players[iClientID].iSystemID);
			if(!set_btNoDeathPenalty->Find(&uwShip) && !set_btNoDPSystems->Find(&uwSystem))
			{
				int iCash;
				HkGetCash(wscCharname, iCash);
				int iOwed = ClientInfo[iClientID].iDeathPenaltyCredits;
				if(iKillerID && set_fDeathPenaltyKiller)
				{
					int iGive = (int)(iOwed * set_fDeathPenaltyKiller);
					HkAddCash(ARG_CLIENTID(iKillerID), iGive);
					PrintUserCmdText(iKillerID, L"Death penalty: given " + ToMoneyStr(iGive) + L" credits from %s's death penalty.", Players.GetActiveCharacterName(iClientID));
				}
				if(iOwed/2 > iCash) //remove items from player's cargo from penalty list
				{
					if(bSecondTime)
					{
						HkRemoveDeathPenaltyItems(iClientID);
						ClientInfo[iClientID].bDeathPenaltyOnEnter = false;
					}
				}
				else
				{
					PrintUserCmdText(iClientID, L"Death penalty: charged " + ToMoneyStr(iOwed) + L" credits.");
					HkAddCash(wscCharname, -iOwed);
					ClientInfo[iClientID].bDeathPenaltyOnEnter = false;
				}
			}
			else //Ship is on no death penalty list
			{
				ClientInfo[iClientID].bDeathPenaltyOnEnter = false;
			}
		}
		else
		{
			flstr *str = CreateWString(wscCharname.c_str());
			CAccount *acc = Players.FindAccountFromCharacterName(*str);
			FreeWString(str);
			wstring wscDir;
			HkGetAccountDirName(acc, wscDir);
			string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
			wstring wscFilename;
			HkGetCharFileName(wscCharname, wscFilename);
			IniWrite(scUserFile, "general_" + wstos(wscFilename), "deathpenalty", "yes");

			ClientInfo[iClientID].bDeathPenaltyOnEnter = false;
		}
	}
}

struct REMOVED_ITEM_PERCENT
{
	REMOVED_ITEM_PERCENT(uint good, uint count, float chance) {iGoodID = good; iCount = count; fChance = chance; }
	REMOVED_ITEM_PERCENT(uint good) {iGoodID = good; iCount = 0; fChance = 0.0f; }
	uint iGoodID;
	uint iCount;
	float fChance;
	bool operator==(REMOVED_ITEM_PERCENT ri) { return ri.iGoodID==iGoodID; }
	bool operator>=(REMOVED_ITEM_PERCENT ri) { return ri.iGoodID>=iGoodID; }
	bool operator<=(REMOVED_ITEM_PERCENT ri) { return ri.iGoodID<=iGoodID; }
	bool operator>(REMOVED_ITEM_PERCENT ri) { return ri.iGoodID>iGoodID; }
	bool operator<(REMOVED_ITEM_PERCENT ri) { return ri.iGoodID<iGoodID; }
};

list<wstring> HkGetDeathPenaltyItems(uint iClientID)
{
	BinaryTree<REMOVED_ITEM_PERCENT> *btRemovedItems = new BinaryTree<REMOVED_ITEM_PERCENT>();

	char *szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (iClientID - 1);

	EQ_ITEM *eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM *eq;
	eq = eqLst->next;
	while(eq != eqLst)
	{
		DEATH_ITEM diFind = DEATH_ITEM(eq->iGoodID);
		DEATH_ITEM *diFound = set_btDeathItems->Find(&diFind);
		if(diFound)
		{
			REMOVED_ITEM_PERCENT *riFind = new REMOVED_ITEM_PERCENT(eq->iGoodID, eq->iCount, diFound->fChance);
			REMOVED_ITEM_PERCENT *riFound = btRemovedItems->Find(riFind);
			if(riFound)
			{
				riFound->iCount += eq->iCount;
				delete riFind;
			}
			else
			{
				btRemovedItems->Add(riFind);
			}
		}

		eq = eq->next;
	}

	list<wstring> lstItems;
	if(btRemovedItems->Count())
	{
 		BinaryTreeIterator<REMOVED_ITEM_PERCENT> *btiItemIter = new BinaryTreeIterator<REMOVED_ITEM_PERCENT>(btRemovedItems);
		btiItemIter->First();
		for(long i=0; i<btRemovedItems->Count(); i++)
		{
			const GoodInfo *gi = GoodList::find_by_id(btiItemIter->Curr()->iGoodID);
			if(gi && gi->iIDS)
			{
				lstItems.push_back(HkGetWStringFromIDS(gi->iIDS) + (btiItemIter->Curr()->iCount>1 ? L"s " : L" ") + L"(" + ftows(btiItemIter->Curr()->fChance*100) + L"% chance)");
			}
			btiItemIter->Next();
		}
		delete btiItemIter;
	}

	delete btRemovedItems;

	return lstItems;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetReservedSlot(wstring wscCharname, bool &bResult)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	if(iClientID != -1)
		acc = Players.FindAccountFromClientID(iClientID);
	else
		acc = HkGetAccountByCharname(wscCharname);

	if(!acc)
		return HKE_CHAR_DOES_NOT_EXIST;

	wstring wscDir; 
	HkGetAccountDirName(acc, wscDir); 
	string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	bResult = IniGetB(scUserFile, "Settings", "ReservedSlot", false);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkSetReservedSlot(wstring wscCharname, bool bReservedSlot)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	CAccount *acc;
	if(iClientID != -1)
		acc = Players.FindAccountFromClientID(iClientID);
	else
		acc = HkGetAccountByCharname(wscCharname);

	if(!acc)
		return HKE_CHAR_DOES_NOT_EXIST;

	wstring wscDir; 
	HkGetAccountDirName(acc, wscDir); 
	string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	if(bReservedSlot)
		IniWrite(scUserFile, "Settings", "ReservedSlot", "yes");
	else
		IniWrite(scUserFile, "Settings", "ReservedSlot", "no");
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkPlayerAutoBuy(uint iClientID, uint iBaseID)
{
	if(!ClientInfo[iClientID].lstAutoBuyItems.size())
		return; //don't waste CPU time

	// player cargo
	int iRemHoldSize;
	list<CARGO_INFO> lstCargoTemp, lstCargo;
	HkEnumCargo(ARG_CLIENTID(iClientID), lstCargoTemp, &iRemHoldSize);
	//consolidate multiple items into one entry, skip mounted items
	foreach(lstCargoTemp, CARGO_INFO, cargo)
	{
		if(!cargo->bMounted)
		{
			bool bFoundItem = false;
			foreach(lstCargo, CARGO_INFO, cargo2)
			{
				if(cargo2->iArchID == cargo->iArchID)
				{
					cargo2->iCount++;
					bFoundItem = true;
					break;
				}
			}
			if(!bFoundItem)
			{
				lstCargo.push_back(*cargo);
			}
		}
	}

	uint iNanobotsID = 2911012559;
	uint iShieldBatID = 2596081674;
	Archetype::Ship *ship = Archetype::GetShip(Players[iClientID].iShipArchID);
	list<AUTOBUY_CARTITEM> lstItems = ClientInfo[iClientID].lstAutoBuyItems;
	foreach(lstItems, AUTOBUY_CARTITEM, item)
	{
		if(item->iArchID == iNanobotsID)
		{
			if((uint)item->iCount > ship->iMaxNanobots)
			{
				PrintUserCmdText(iClientID, L"Warning: the number of nanobots in your autobuy template is greater than allowed; skipping purchase.");
				item->iCount = 0;
				continue;
			}
		}
		else if(item->iArchID == iShieldBatID)
		{
			if((uint)item->iCount > ship->iMaxShieldBats)
			{
				PrintUserCmdText(iClientID, L"Warning: the number of shield batteries in your autobuy template is greater than allowed; skipping purchase.");
				item->iCount = 0;
				continue;
			}
		}
		foreach(lstCargo, CARGO_INFO, cargo)
		{
			if(cargo->iArchID == item->iArchID)
			{
				if((uint)cargo->iCount < item->iCount)
					item->iCount -= cargo->iCount;
				else
					item->iCount = 0;
			}
		}
	}

	// search base in base-info list
	BASE_INFO *bi = 0;
	foreach(lstBases, BASE_INFO, it3)
	{
		if(it3->iBaseID == iBaseID)
		{
			bi = &(*it3);
			break;
		}
	}

	if(!bi)
		return; // base not found

	int iCash;
	HkGetCash(ARG_CLIENTID(iClientID), iCash);

	foreach(lstItems, AUTOBUY_CARTITEM, it4)
	{
		if(!(*it4).iCount || !Arch2Good((*it4).iArchID))
			continue;

		// check if good is available and if player has the neccessary rep
		bool bGoodAvailable = false;
		foreach(bi->lstMarketMisc, DATA_MARKETITEM, itmi)
		{
			if(itmi->iArchID == it4->iArchID)
			{
				int iSolarRep;
				pub::SpaceObj::GetSolarRep(bi->iObjectID, iSolarRep);
				uint iBaseRep;
				Reputation::Vibe::GetAffiliation(iSolarRep, iBaseRep, true);
				if(iBaseRep == -1)
					continue; // rep can't be determined yet(space object not created yet?)

				// get player rep
				int iRepID;
				pub::Player::GetRep(iClientID, iRepID);

				// check if rep is sufficient
				float fPlayerRep;
				pub::Reputation::GetGroupFeelingsTowards(iRepID, iBaseRep, fPlayerRep);
				if(fPlayerRep < itmi->fRep)
					break; // bad rep, not allowed to buy
				bGoodAvailable = true;
				break;
			}
		}
		if(!bGoodAvailable)
		{
			foreach(bi->lstMarketCommodities, DATA_MARKETITEM, itco)
			{
				if(itco->iArchID == it4->iArchID)
				{
					// get base rep
					int iSolarRep;
					pub::SpaceObj::GetSolarRep(bi->iObjectID, iSolarRep);
					uint iBaseRep;
					Reputation::Vibe::GetAffiliation(iSolarRep, iBaseRep, true);
					if(iBaseRep == -1)
						continue; // rep can't be determined yet(space object not created yet?)

					// get player rep
					int iRepID;
					pub::Player::GetRep(iClientID, iRepID);

					// check if rep is sufficient
					float fPlayerRep;
					pub::Reputation::GetGroupFeelingsTowards(iRepID, iBaseRep, fPlayerRep);
					if(fPlayerRep < itco->fRep)
						break; // bad rep, not allowed to buy
					bGoodAvailable = true;
					break;
				}
			}
		}

		if(!bGoodAvailable)
			continue; // base does not sell this item or bad rep
		
		float fPrice;
		if(pub::Market::GetPrice(iBaseID, (*it4).iArchID, fPrice) == -1)
			continue; // good not available

		Archetype::Equipment *eq = Archetype::GetEquipment((*it4).iArchID);
		if(iRemHoldSize < (eq->fVolume * (*it4).iCount))
		{
			uint iNewCount = iRemHoldSize / (uint)eq->fVolume;
			if(!iNewCount) {
				continue;
			} else
				(*it4).iCount = iNewCount;
		}

		int iCost = ((int)fPrice * (*it4).iCount);
		if(iCash < iCost)
			PrintUserCmdText(iClientID, L"Auto-Buy(%s): FAILED! Insufficient Credits", HkGetWStringFromIDS(eq->iIDSName).c_str());
		else {
			HkAddCash(ARG_CLIENTID(iClientID), -iCost);
			iCash -= iCost;
			iRemHoldSize -= ((int)eq->fVolume * (*it4).iCount);
			HkAddCargo(ARG_CLIENTID(iClientID), (*it4).iArchID, (*it4).iCount, false);
			PrintUserCmdText(iClientID, L"Auto-Buy(%s): Bought %u unit(s), cost: $%s", HkGetWStringFromIDS(eq->iIDSName).c_str(), (*it4).iCount, ToMoneyStr(iCost).c_str());
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResetRep(wstring wscCharname)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	if(!set_lstNewCharRep.size())
		return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;

	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	foreach(set_lstNewCharRep, NEW_CHAR_REP, rep)
	{
		pub::Reputation::SetReputation(iPlayerRep, rep->iRepGroupID, rep->fRep);
	}

	return HKE_OK;
}

HK_ERROR HkSetRep(wstring wscCharname, wstring wscRepGroup, float fValue)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if(iRepGroupID == -1)
		return HKE_INVALID_REP_GROUP;

	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fValue);
	return HKE_OK;
}

HK_ERROR HkSetRepRelative(wstring wscCharname, wstring wscRepGroup, float fValue, bool bUseDestruction)
{
	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if(iRepGroupID == -1)
		return HKE_INVALID_REP_GROUP;

	return HkSetRepRelative(wscCharname, iRepGroupID, fValue, bUseDestruction);
}
HK_ERROR HkSetRepRelative(wstring wscCharname, uint iRepGroupID, float fValue, bool bUseDestruction)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	REP_CHANGE_EFFECT rceFind = REP_CHANGE_EFFECT(iRepGroupID);
	REP_CHANGE_EFFECT *rceFound = set_btEmpathy->Find(&rceFind);
	if(rceFound)
	{
		if(bUseDestruction)
			fValue = rceFound->fObjectDestructionEvent;
		BinaryTreeIterator<EMPATHY_RATE> *btiRates = new BinaryTreeIterator<EMPATHY_RATE>(rceFound->btEmpathyRates);
		btiRates->First();
		float fExistingRep, fNewRep;
		for(long i=0; i<rceFound->btEmpathyRates->Count(); i++)
		{
			pub::Reputation::GetGroupFeelingsTowards(iPlayerRep, btiRates->Curr()->iGroupID, fExistingRep);
			fNewRep = fExistingRep + fValue*btiRates->Curr()->fRate;
			if(fNewRep > set_fMaxRepValue)
				fNewRep = set_fMaxRepValue;
			else if(fNewRep < set_fMinRepValue)
				fNewRep = set_fMinRepValue;
			pub::Reputation::SetReputation(iPlayerRep, btiRates->Curr()->iGroupID, fNewRep);
			btiRates->Next();
		}
		delete btiRates;
		pub::Reputation::GetGroupFeelingsTowards(iPlayerRep, iRepGroupID, fExistingRep);
		fNewRep = fExistingRep + fValue;
		if(fNewRep > set_fMaxRepValue)
			fNewRep = set_fMaxRepValue;
		else if(fNewRep < set_fMinRepValue)
			fNewRep = set_fMinRepValue;
		pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fNewRep);
	}
	else
		return HKE_INVALID_REP_GROUP;

	return HKE_OK;
}

HK_ERROR HkSetAffiliation(wstring wscCharname, wstring wscRepGroup)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if(iRepGroupID == -1)
		return HKE_INVALID_REP_GROUP;

	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	pub::Reputation::SetAffiliation(iPlayerRep, iRepGroupID);
	return HKE_OK;
}

HK_ERROR HkGetAffiliation(wstring wscCharname, wstring &wscRepGroup)
{
	list<wstring> lstLines;
	HK_ERROR hErr = HkReadCharFile(wscCharname, lstLines);
	if(!HKHKSUCCESS(hErr))
		return hErr;
	
	foreach(lstLines, wstring, str)
	{
		if(!ToLower((*str)).find(L"rep_group")) //Line contains affiliation
		{
			wscRepGroup = (*str).substr((*str).find(L"=")+1);
			wscRepGroup = Trim(wscRepGroup);
			return HKE_OK;
		}
	}
	return HKE_UNKNOWN_ERROR;

	//Bad; can get repgroup id but not the nickname from it
	/*HK_GET_CLIENTID(iClientID, wscCharname);
	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;
	uint iRepGroupID;
	uint iGroupName;
	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	Reputation::Vibe::GetAffiliation(iPlayerRep, iRepGroupID, false);
	pub::Reputation::GetGroupName(iRepGroupID, iGroupName);
	TString<16> nick;
	Reputation::get_nickname(nick, iGroupName);
	wscRepGroup = stows(nick.data);*/
	/*FmtStr fsOut1 = FmtStr(0 , FmtStr::Val()), fsOut2 = FmtStr(0 , FmtStr::Val());
	pub::Reputation::GetName(iPlayerRep, fsOut1, fsOut2);
	wscRepGroup = stows(fsOut1.szBuf);
	return HKE_OK;*/
}

HK_ERROR HkGetRep(wstring wscCharname, wstring wscRepGroup, float &fValue)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if(iRepGroupID == -1)
		return HKE_INVALID_REP_GROUP;
	
	int iPlayerRep;
	pub::Player::GetRep(iClientID, iPlayerRep);
	pub::Reputation::GetGroupFeelingsTowards(iPlayerRep, iRepGroupID, fValue);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetGroupMembers(wstring wscCharname, list<GROUP_MEMBER> &lstMembers)
{
	lstMembers.clear();
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	// hey, at least it works!
	vector<uint> vMembers;
	char szBuf[1024] = "";
	pub::Player::GetGroupMembers(iClientID, *((vector<uint>*)szBuf));
	vMembers = *((vector<uint>*)szBuf);

	for(uint i = 0 ; (i < vMembers.size()); i++)
	{
		GROUP_MEMBER gm;
		gm.iClientID = vMembers[i];
		gm.wscCharname = Players.GetActiveCharacterName(vMembers[i]);
		lstMembers.push_back(gm);
	}

	return HKE_OK;
}

HK_ERROR HkGetGroupID(wstring wscCharname, uint &iGroupID)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	iGroupID = Players.GetGroupID(iClientID);
	return HKE_OK;
}

HK_ERROR HkAddToGroup(wstring wscCharname, uint iGroupID)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iCurrentGroupID = Players.GetGroupID(iClientID);
	if(iCurrentGroupID==iGroupID)
		return HKE_INVALID_GROUP_ID;

	CPlayerGroup *group = CPlayerGroup::FromGroupID(iGroupID);
	if(!group)
		return HKE_INVALID_GROUP_ID;
	group->AddMember(iClientID);
	return HKE_OK;
}

HK_ERROR HkInviteToGroup(wstring wscCharname, wstring wscCharFrom) 
{
	HK_GET_CLIENTID(iClientID, wscCharFrom);
	HK_GET_CLIENTID2(iClientIDTarget, wscCharname);
	wscCharname = Players.GetActiveCharacterName(iClientIDTarget);

	// check if logged in
	if(iClientIDTarget == -1 || iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	wstring wscXML = L"<TEXT>/i " + XMLText(wscCharname) + L"</TEXT>";
	char szBuf[0xFFFF];
	uint iRet;
	HK_ERROR eXML = HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
	if(!HKHKSUCCESS(eXML))
	{
		return eXML;
	}

	CHAT_ID cID;
	cID.iID = iClientID;
	CHAT_ID cIDTo;
	cIDTo.iID = 0x00010001;
	Server.SubmitChat(cID, iRet, szBuf, cIDTo, -1);

	/*CPlayerGroup *group = CPlayerGroup::FromGroupID(iGroupID);
	if(!group)
		return HKE_INVALID_GROUP_ID;
	group->AddInvite(iClientID);*/
	return HKE_OK;
}

HK_ERROR HkRemoveFromGroup(wstring wscCharname, uint iGroupID)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	CPlayerGroup *group = CPlayerGroup::FromGroupID(iGroupID);
	if(!group)
		return HKE_INVALID_GROUP_ID;
	group->DelMember(iClientID);
	return HKE_OK;
}

HK_ERROR HkSendGroupCash(uint iGroupID, int iAmount)
{
	CPlayerGroup *group = CPlayerGroup::FromGroupID(iGroupID);
	if(!group)
		return HKE_INVALID_GROUP_ID;
	group->RewardMembers(iAmount);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkReadCharFile(wstring wscCharname, list<wstring> &lstOutput)
{
	lstOutput.clear();
	HK_GET_CLIENTID(iClientID, wscCharname);

	wstring wscDir;
	CAccount *acc;
	if(iClientID != -1) {
		acc = Players.FindAccountFromClientID(iClientID);
		const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
		if(!wszCharname)
			return HKE_NO_CHAR_SELECTED;

		if(!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
	} else {
		if(!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
	}

	wstring wscFile;
	if(!HKHKSUCCESS(HkGetCharFileName(wscCharname, wscFile)) || !wscDir.length() || !wscFile.length())
		return HKE_COULD_NOT_GET_PATH;
	string scCharFile  = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	string scFileToRead;
	bool bDeleteAfter;
	if(HkIsEncoded(scCharFile)) {
		string scCharFileNew = scCharFile + ".ini";
		if(!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return HKE_COULD_NOT_DECODE_CHARFILE;
		scFileToRead = scCharFileNew;
		bDeleteAfter = true;
	} else {
		scFileToRead = scCharFile;
		bDeleteAfter = false;
	}

	ifstream ifs;
	ifs.open(scFileToRead.c_str(), ios_base::in);
	if(!ifs.is_open())
		return HKE_UNKNOWN_ERROR;

	string scLine;
	while(getline(ifs, scLine))
		lstOutput.push_back(stows(scLine));
	ifs.close();
	if(bDeleteAfter)
		DeleteFile(scFileToRead.c_str());
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkWriteCharFile(wstring wscCharname, wstring wscData)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	wstring wscDir;
	CAccount *acc;
	if(iClientID != -1) {
		acc = Players.FindAccountFromClientID(iClientID);
		const wchar_t *wszCharname = Players.GetActiveCharacterName(iClientID);
		if(!wszCharname)
			return HKE_NO_CHAR_SELECTED;

		if(!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
	} else {
		if(!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
			return HKE_CHAR_DOES_NOT_EXIST;
	}

	wstring wscFile;
	if(!HKHKSUCCESS(HkGetCharFileName(wscCharname, wscFile)) || !wscDir.length() || !wscFile.length())
		return HKE_COULD_NOT_GET_PATH;
	string scCharFile  = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	string scFileToWrite;
	bool bEncode;
	if(HkIsEncoded(scCharFile)) {
		scFileToWrite = scCharFile + ".ini";
		bEncode = true;
	} else {
		scFileToWrite = scCharFile;
		bEncode = false;
	}

	ofstream ofs;
	ofs.open(scFileToWrite.c_str(), ios_base::out);
	if(!ofs.is_open())
		return HKE_UNKNOWN_ERROR;

	size_t iPos;
	while((iPos = wscData.find(L"\\n")) != -1)
	{
		wstring wscLine = wscData.substr(0, iPos);
		ofs << wstos(wscLine) << endl;
		wscData.erase(0, iPos + 2);
	}

	if(wscData.length())
		ofs << wstos(wscData);

	ofs.close();
	if(bEncode)
	{
		flc_encode(scFileToWrite.c_str(), scCharFile.c_str());
		DeleteFile(scFileToWrite.c_str());
	}
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetTimePlayed(wstring wscCharname, uint &iSeconds)
{
	list<wstring> lstCharFile;
	HK_ERROR err = HkReadCharFile(wscCharname, lstCharFile);
	if(!HKHKSUCCESS(err))
		return err;
	list<wstring>::reverse_iterator line;
	for(line = lstCharFile.rbegin(); line!=lstCharFile.rend(); line++)
	{
		uint iPos = line->find(L"total_time_played");
		if(!iPos)
		{
			iPos = line->find(L"=");
			iSeconds = _wtol(Trim(line->substr(iPos+1)).c_str());
			return HKE_OK;
		}
	}
	return HKE_UNKNOWN_ERROR;
}
			
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkCloak(uint iClientID)
{
	ClientInfo[iClientID].bIsCloaking = true;
	ClientInfo[iClientID].bWantsCloak = false;
	ClientInfo[iClientID].bCloaked = false;
	ClientInfo[iClientID].tmCloakTime = timeInMS();

	XActivateEquip ActivateEq;
	ActivateEq.bActivate = true;
	ActivateEq.l1 = ClientInfo[iClientID].iShip;
	ActivateEq.sID = ClientInfo[iClientID].iCloakSlot;
	Server.ActivateEquip(iClientID,ActivateEq);

	PrintUserCmdText(iClientID, L" Cloaking");
	
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID2 = HkGetClientIdFromPD(pPD);
		HkUnMarkObject(iClientID2, ClientInfo[iClientID].iShip);
	}

	return HKE_OK;
}

HK_ERROR HkUnCloak(uint iClientID)
{
	ClientInfo[iClientID].bIsCloaking = false;
	ClientInfo[iClientID].bWantsCloak = false;
	ClientInfo[iClientID].bCloaked = false;
	ClientInfo[iClientID].tmCloakTime = timeInMS();

	XActivateEquip ActivateEq;
	ActivateEq.bActivate = false;
	ActivateEq.l1 = ClientInfo[iClientID].iShip;
	ActivateEq.sID = ClientInfo[iClientID].iCloakSlot;
	Server.ActivateEquip(iClientID,ActivateEq);

	PrintUserCmdText(iClientID, L" Uncloaking");

	return HKE_OK;
}

HK_ERROR HkInitCloakSettings(uint iClientID)
{
	ClientInfo[iClientID].bCanCloak = false;

	wstring wscCharname = Players.GetActiveCharacterName(iClientID);

    list<CARGO_INFO> lstEquipment;
	if (HKHKSUCCESS(HkEnumCargo(wscCharname,lstEquipment,0)))
	{
		foreach(lstEquipment, CARGO_INFO, it)
		{
			if((*it).bMounted)
			{
				// check for mounted cloak device
				foreach(set_lstCloakDevices,INISECTIONVALUE,it2)
				{
					uint iArchIDCloak = CreateID((*it2).scKey.c_str());
					if ((*it).iArchID == iArchIDCloak)
					{
						ClientInfo[iClientID].iCloakingTime = atoi(Trim(GetParam((*it2).scValue, ',', 0)).c_str());
						ClientInfo[iClientID].iCloakCooldown = atoi(Trim(GetParam((*it2).scValue, ',', 1)).c_str());
						ClientInfo[iClientID].iCloakWarmup = atoi(Trim(GetParam((*it2).scValue, ',', 2)).c_str());
						ClientInfo[iClientID].bCanCloak = true;
						ClientInfo[iClientID].bIsCloaking = false;
						ClientInfo[iClientID].bWantsCloak = false;
						ClientInfo[iClientID].bCloaked = false;
						ClientInfo[iClientID].iCloakSlot = (*it).iID;
						ClientInfo[iClientID].tmCloakTime = timeInMS();
						break;
					}
				}
			}
		}
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

char HkMarkObject(uint iClientID, uint iObject)
{
	if(!iObject)
		return 1;

	uint iClientIDMark = HkGetClientIDByShip(iObject);
	if(iClientIDMark && (ClientInfo[iClientIDMark].bCloaked || ClientInfo[iClientIDMark].bIsCloaking))
		return 2;

	uint iSystemID, iObjectSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	pub::SpaceObj::GetSystem(iObject, iObjectSystemID);
	if(iSystemID == iObjectSystemID)
	{
		for(uint i=0; i<ClientInfo[iClientID].vMarkedObjs.size(); i++)
		{
			if(ClientInfo[iClientID].vMarkedObjs[i] == iObject)
				return 3; //already marked
		}
	}
	else
	{
		for(uint j=0; j<ClientInfo[iClientID].vDelayedSystemMarkedObjs.size(); j++)
		{
			if(ClientInfo[iClientID].vDelayedSystemMarkedObjs[j] == iObject)
				return 3; //already marked
		}
		ClientInfo[iClientID].vDelayedSystemMarkedObjs.push_back(iObject);
		return 0;
	}

	pub::Player::MarkObj(iClientID, iObject, 1);
	for(uint i=0; i<ClientInfo[iClientID].vAutoMarkedObjs.size(); i++) //remove from automarked vector
	{
		if(ClientInfo[iClientID].vAutoMarkedObjs[i] == iObject)
		{
			if(i != ClientInfo[iClientID].vAutoMarkedObjs.size()-1)
			{
				ClientInfo[iClientID].vAutoMarkedObjs[i] = ClientInfo[iClientID].vAutoMarkedObjs[ClientInfo[iClientID].vAutoMarkedObjs.size()-1];
			}
			ClientInfo[iClientID].vAutoMarkedObjs.pop_back();
			break;
		}
	}
	ClientInfo[iClientID].vMarkedObjs.push_back(iObject);
	return 0;
}

void HkUnMarkObject(uint iClientID, uint iObject)
{
	if(!iObject)
		return;

	for(uint i=0; i<ClientInfo[iClientID].vMarkedObjs.size(); i++)
	{
		if(ClientInfo[iClientID].vMarkedObjs[i] == iObject)
		{
			if(i != ClientInfo[iClientID].vMarkedObjs.size()-1)
			{
				ClientInfo[iClientID].vMarkedObjs[i] = ClientInfo[iClientID].vMarkedObjs[ClientInfo[iClientID].vMarkedObjs.size()-1];
			}
			ClientInfo[iClientID].vMarkedObjs.pop_back();
			pub::Player::MarkObj(iClientID, iObject, 0);
			return;
		}
	}

	for(uint j=0; j<ClientInfo[iClientID].vAutoMarkedObjs.size(); j++)
	{
		if(ClientInfo[iClientID].vAutoMarkedObjs[j] == iObject)
		{
			if(j != ClientInfo[iClientID].vAutoMarkedObjs.size()-1)
			{
				ClientInfo[iClientID].vAutoMarkedObjs[j] = ClientInfo[iClientID].vAutoMarkedObjs[ClientInfo[iClientID].vAutoMarkedObjs.size()-1];
			}
			ClientInfo[iClientID].vAutoMarkedObjs.pop_back();
			pub::Player::MarkObj(iClientID, iObject, 0);
			return;
		}
	}

	for(uint k=0; k<ClientInfo[iClientID].vDelayedSystemMarkedObjs.size(); k++)
	{
		if(ClientInfo[iClientID].vDelayedSystemMarkedObjs[k] == iObject)
		{
			if(k != ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1)
			{
				ClientInfo[iClientID].vDelayedSystemMarkedObjs[k] = ClientInfo[iClientID].vDelayedSystemMarkedObjs[ClientInfo[iClientID].vDelayedSystemMarkedObjs.size()-1];
			}
			ClientInfo[iClientID].vDelayedSystemMarkedObjs.pop_back();
			return;
		}
	}
}

void HkUnMarkAllObjects(uint iClientID)
{
	for(uint i=0; i<ClientInfo[iClientID].vMarkedObjs.size(); i++)
	{
		pub::Player::MarkObj(iClientID, (ClientInfo[iClientID].vMarkedObjs[i]), 0);
	}
	ClientInfo[iClientID].vMarkedObjs.clear();
	for(uint i=0; i<ClientInfo[iClientID].vAutoMarkedObjs.size(); i++)
	{
		pub::Player::MarkObj(iClientID, (ClientInfo[iClientID].vAutoMarkedObjs[i]), 0);
	}
	ClientInfo[iClientID].vAutoMarkedObjs.clear();
	ClientInfo[iClientID].vDelayedSystemMarkedObjs.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsOkayToDock(uint iClientID, uint iTargetClientID)
{
	if(ClientInfo[iTargetClientID].iMaxPlayersDocked == -1)
		return true;

	uint iNumDocked = 0;
	foreach(ClientInfo[iTargetClientID].lstPlayersDocked, uint, iDockedClientID)
	{
		uint iShip;
		pub::Player::GetShip(*iDockedClientID, iShip);
		if(!iShip)
			iNumDocked++;
	}
	if(iNumDocked >= (uint)ClientInfo[iTargetClientID].iMaxPlayersDocked)
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkNewShipBought(uint iClientID)
{
	//Erase autobuy settings
	if(ClientInfo[iClientID].lstAutoBuyItems.size())
	{
		CAccount *acc = Players.FindAccountFromClientID(iClientID);
		wstring wscDir;
		HkGetAccountDirName(acc, wscDir);
		string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
		wstring wscFilename;
		HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
		string scSection = "autobuy_" + wstos(wscFilename);
		ClientInfo[iClientID].lstAutoBuyItems.clear();
		IniDelSection(scUserFile, scSection);
		PrintUserCmdText(iClientID, L"Notice: your autobuy settings have been erased. Use the autobuy command again with your new ship.");
	}

	//Unmount items that aren't allowed to be mounted on the new ship
	list<CARGO_INFO> lstCargo;
	HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, 0);
	foreach(lstCargo, CARGO_INFO, cargo)
	{
		if(cargo->bMounted)
		{
			MOUNT_RESTRICTION mrFind = MOUNT_RESTRICTION(cargo->iArchID);
			MOUNT_RESTRICTION *mrFound = set_btMRestrict->Find(&mrFind);
			if(mrFound)
			/*Check to make sure ship can mount good*/
			{
				uint iShipArchID;
				pub::Player::GetShipID(iClientID, iShipArchID);
				UINT_WRAP uw = UINT_WRAP(iShipArchID);
				if(!mrFound->btShipArchIDs->Find(&uw))
				{
					//remove and then add cargo to fix client mounted disagreement
					pub::Player::RemoveCargo(iClientID, cargo->iID, cargo->iCount);
					pub::Player::AddCargo(iClientID, cargo->iArchID, cargo->iCount, 1, false);
				}
			}
		}
		else
		{
			set<uint>::iterator mount = set_setAutoMount.find(cargo->iArchID);
			if(mount != set_setAutoMount.end())
			{
				pub::Player::RemoveCargo(iClientID, cargo->iID, cargo->iCount);
				HkAddCargo(ARG_CLIENTID(iClientID), cargo->iArchID, "internal", false);
			}
		}
	}

	//Check if player has over the max number of nanobots or shield batteries, since FL doesn't
	Archetype::Ship *ship = Archetype::GetShip(Players[iClientID].iShipArchID);
	if(ship)
	{
		uint iNanobotsID = 2911012559;
		uint iShieldBatID = 2596081674;
		int iNumNanobots = 0, iNumShieldBat = 0;
		ushort iNanobots, iShieldBat;
		list<CARGO_INFO> lstCargo;
		HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, 0);
		foreach(lstCargo, CARGO_INFO, cargo)
		{
			if(cargo->iArchID == iNanobotsID)
			{
				iNumNanobots = cargo->iCount;
				iNanobots = cargo->iID;
				if(iNumShieldBat)
					break;
			}
			else if(cargo->iArchID == iShieldBatID)
			{
				iNumShieldBat = cargo->iCount;
				iShieldBat = cargo->iID;
				if(iNumNanobots)
					break;
			}
		}
		iNumNanobots -= ship->iMaxNanobots;
		iNumShieldBat -= ship->iMaxShieldBats;
		if(iNumNanobots > 0)
		{
			pub::Player::RemoveCargo(iClientID, iNanobots, iNumNanobots);
			uint iBaseID;
			pub::Player::GetBase(iClientID, iBaseID);
			float fPrice = 0;
			pub::Market::GetPrice(iBaseID, iNanobotsID, fPrice);
			if(fPrice)
				HkAddCash(ARG_CLIENTID(iClientID), (int)fPrice * iNumNanobots);
		}
		if(iNumShieldBat > 0)
		{
			pub::Player::RemoveCargo(iClientID, iShieldBat, iNumShieldBat);
			uint iBaseID;
			pub::Player::GetBase(iClientID, iBaseID);
			float fPrice = 0;
			pub::Market::GetPrice(iBaseID, iShieldBat, fPrice);
			if(fPrice)
				HkAddCash(ARG_CLIENTID(iClientID), (int)fPrice * iNumShieldBat);
		}
	}
}