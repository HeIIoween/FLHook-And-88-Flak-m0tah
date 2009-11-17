#include "hook.h"

uint		iDmgTo = 0;
uint		iDmgToSpaceID = 0;
IObjRW		*objDmgTo = 0;

bool g_gNonGunHitsBase = false;
float g_LastHitPts;

/**************************************************************************************************************
Called when a torp/missile/mine/wasp hits a ship
return 0 -> pass on to server.dll
return 1 -> suppress
**************************************************************************************************************/

FARPROC fpOldMissileTorpHit;

int __stdcall HkCB_MissileTorpHit(char *ECX, char *p1, DamageList *dmg)
{
	try {
		// get client id
		char *szP;
		memcpy(&szP, ECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);

		iDmgTo = iClientID;
		if(iClientID) 
		{ // a player was hit

			uint iInflictorShip;
			memcpy(&iInflictorShip, p1 + 4, 4);
			uint iClientIDInflictor = HkGetClientIDByShip(iInflictorShip);
			if(!iClientIDInflictor)
				return 0; // hit by npc

			if(!AllowPlayerDamage(iClientIDInflictor, iClientID))
				return 1;

			if(set_bChangeCruiseDisruptorBehaviour)
			{
				if(((dmg->get_cause() == 6) || (dmg->get_cause() == 0x15)) && !ClientInfo[iClientID].bCruiseActivated)
					dmg->set_cause((enum DamageCause)0xC0); // change to sth else, so client won't recognize it as a disruptor
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	return 0;
}

__declspec(naked) void _HookMissileTorpHit()
{
	__asm 
	{
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call HkCB_MissileTorpHit
		pop ecx
		cmp eax, 1
		jnz go_ahead
		mov edx, [esp] ; suppress
		add esp, 0Ch
		jmp edx

go_ahead:
		jmp [fpOldMissileTorpHit]
	}
}

void Clear_DmgTo()
{
	iDmgToSpaceID = 0;
	iDmgTo = 0;
	objDmgTo = 0;
}

/**************************************************************************************************************
Called when ship was damaged
however you can't figure out here, which ship is being damaged, that's why i use the iDmgTo variable...
**************************************************************************************************************/

bool bTakeOverDmgEntry = false;
float fTakeOverHealth = 0.0f;
ushort iTakeOverSubObj = 0;
uint iTakeOverClientID = 0;
uint iTakeOverSpaceObj = 0;
DamageCause dcTakeOver = DC_HOOK;
void __stdcall HkCb_AddDmgEntry(DamageList *dmgList, unsigned short p1, float p2, enum DamageEntry::SubObjFate p3)
{
	bool bAddDmgEntry = true;
	float fPrevHealth, fMaxHealth;
	
	try {
		if(bTakeOverDmgEntry)
		{
			p2 = fTakeOverHealth;
			if(iTakeOverSubObj)
				p1 = iTakeOverSubObj;
			dmgList->set_inflictor_owner_player(iTakeOverClientID);
			dmgList->set_inflictor_id(iTakeOverSpaceObj);
			dmgList->set_cause(dcTakeOver);
			dmgList->add_damage_entry(p1, p2, p3);
			bTakeOverDmgEntry = false;
			fTakeOverHealth = 0.0f;
			iTakeOverSubObj = 0;
			iTakeOverClientID = 0;
			iTakeOverSpaceObj = 0;
			dcTakeOver = DC_HOOK;
			return;
		}

		if(g_gNonGunHitsBase && (dmgList->get_cause() == 5))
		{
			float fDamage = g_LastHitPts - p2;
			p2 = g_LastHitPts - fDamage * set_fTorpMissileBaseDamageMultiplier;
			if(p2 < 0)
				p2 = 0;
		}
		//if(dmgList->is_inflictor_a_player()) 
			//ConPrint(L"p1=%u, p2=%f, DmgTo=%u, DmgToSpc=%u, iop=%u, id=%u",p1, p2, iDmgTo, iDmgToSpaceID, dmgList->get_inflictor_owner_player(), dmgList->get_inflictor_id());
		//if(dmgList->is_inflictor_a_player()) {PrintUniverseText(L"%f", p2);}

		if(!iDmgToSpaceID && iDmgTo)
			pub::Player::GetShip(iDmgTo, iDmgToSpaceID);
		
		if(iDmgToSpaceID)
			objDmgTo = GetIObjRW(iDmgToSpaceID);
		else
			objDmgTo = 0;

		/*if(objDmgTo)
		{
			uint *test = *((uint**)(objDmgTo)); //VFT
			if(!test) //VFT missing !?!
				Clear_DmgTo();
			else
			{
				test += 2; //Second entry in VFT
				if(!*test) //offset for some reason
					objDmgTo = (IObjRW*)(((char*)objDmgTo) + 4);

				objDmgTo->get_status(fPrevHealth, fMaxHealth);
			}
		}*/

		if(objDmgTo)
			objDmgTo->get_status(fPrevHealth, fMaxHealth);
		
		if(!dmgList->get_inflictor_owner_player() && !dmgList->get_inflictor_id())
		{
			FLOAT_WRAP fw = FLOAT_WRAP(p2);
			if(set_btSupressHealth->Find(&fw))
			{
				bAddDmgEntry = false;
			}
		}

		//Check if damage should be redirected to hull
		CEquip *equip = 0;
		if(objDmgTo && p1 != 1 && p1 != 65521)
		{
			CEquipManager *equipment = HkGetEquipMan(objDmgTo);
			if(equipment)
			{
				equip = equipment->FindByID(p1);
				if(equip)
				{
					set<uint>::iterator equipIter = set_setEquipReDam.find(equip->EquipArch()->iEquipID);
					if(equipIter != set_setEquipReDam.end())
					{
						float fHealth = equip->GetHitPoints();
						p2 = fPrevHealth - (fHealth - p2);
						p1 = 1;
					}
				}
			}
		}

		//Repair Gun
		if(g_bRepairPendHit)
		{
			if(dmgList->is_inflictor_a_player() && p2<=g_fRepairMaxHP)
			{
				dmgList->set_inflictor_owner_player(0); //NPCs don't mind you shooting at them when these are set to 0
				dmgList->set_inflictor_id(0);
				if(p1 != 65521) //Hull/equipment hit
				{
					if(p1 == 1) //Hull hit
					{
						p2 = g_fRepairDamage + g_fRepairBeforeHP;
						if(p2 > g_fRepairMaxHP)
						{
							p2 = g_fRepairMaxHP;
						}
					}
					else //Equipment hit
					{
						if(objDmgTo)
						{
							if(!equip)
							{
								CEquipManager *equipment = HkGetEquipMan(objDmgTo);
								if(equipment)
									equip = equipment->FindByID(p1);
							}
							if(!equip)
							{
								p1 = 1;
								p2 = g_fRepairDamage + g_fRepairBeforeHP;
								if(p2 > g_fRepairMaxHP)
								{
									p2 = g_fRepairMaxHP;
								}
							}
							float fMaxHP = equip->GetMaxHitPoints();
							p2 = g_fRepairDamage + equip->GetHitPoints();
							if(p2 > fMaxHP)
							{
								p2 = fMaxHP;
							}
						}
						else
						{
							p1 = 1;
							p2 = g_fRepairDamage + g_fRepairBeforeHP;
							if(p2 > g_fRepairMaxHP)
							{
								p2 = g_fRepairMaxHP;
							}
						}
					}
				}
				else //Shield hit
				{
					p2 += g_fRepairDamage;
					float hullHealth = (g_fRepairBeforeHP + g_fRepairDamage*0.5f)/g_fRepairMaxHP;
					if(hullHealth > 1.0f)
					{
						hullHealth = 1.0f;
					}
					pub::SpaceObj::SetRelativeHealth(g_iRepairShip, hullHealth);
				}
			}
			g_bRepairPendHit = false;
		}
	} catch(...) { AddLog("Exception in %s0", __FUNCTION__); }

	if(bAddDmgEntry)
	{
		dmgList->add_damage_entry(p1, p2, p3);

		try {
			if(objDmgTo && fPrevHealth)
			{
				uint iInflictor = dmgList->get_inflictor_id();
				if(iInflictor)
				{
					if(iDmgTo)
					{
						if(p1 == 1)
						{
							DAMAGE_INFO dmgInfo;
							dmgInfo.iInflictor = iInflictor;
							dmgInfo.iCause = dmgList->get_cause();
							dmgInfo.fDamage = fPrevHealth - p2;
							ClientInfo[iDmgTo].lstDmgRec.push_back(dmgInfo);
						}
					}
					else if(p1 == 1)
					{
						uint iType;
						//pub::SpaceObj::GetType(iDmgToSpaceID, iType);
						objDmgTo->get_type(iType);

						if(iType & set_iNPCDeathType)
						{
							DamageCause iCause = dmgList->get_cause();
							if(iCause != DC_HOOK)
							{
								DAMAGE_INFO dmgInfo;
								dmgInfo.iInflictor = iInflictor;
								dmgInfo.iCause = iCause;
								dmgInfo.fDamage = fPrevHealth - p2;
								if(p2 == 0 && iDmgToSpaceID / 1000000000)
								{
									lstSolarDestroyDelay.push_back(make_pair(iDmgToSpaceID, dmgInfo));
								}
								else
								{
									pair< map<uint, list<DAMAGE_INFO> >::iterator, bool> findSpaceObj = mapSpaceObjDmgRec.insert(make_pair(iDmgToSpaceID, list<DAMAGE_INFO>()));
									findSpaceObj.first->second.push_back(dmgInfo);
								}
							}
						}
						if(p2 == 0)
						{
							if(iType & (OBJ_DOCKING_RING | OBJ_STATION))
							{
								uint iClientIDKiller = HkGetClientIDByShip(iInflictor);
								BaseDestroyed(iDmgToSpaceID, iClientIDKiller);
							}
						}
					}
				}
			}
		} catch(...) {AddLog("Exception in %s1", __FUNCTION__); }
	}

	Clear_DmgTo();
}

__declspec(naked) void _HkCb_AddDmgEntry()
{
	__asm
	{
		push [esp+0Ch]
		push [esp+0Ch]
		push [esp+0Ch]
		push ecx
		call HkCb_AddDmgEntry
		mov eax, [esp]
		add esp, 10h
		jmp eax
	}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

FARPROC fpOldGeneralDmg;
FARPROC fpOldOtherDmg;

void __stdcall HkCb_GeneralDmg(char *szECX)
{
	try {
		char *szP;
		memcpy(&szP, szECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);
		uint iSpaceID;
		memcpy(&iSpaceID, szP + 0xB0, 4);

		iDmgTo = iClientID;
		iDmgToSpaceID = iSpaceID;
		objDmgTo = (IObjRW*)szECX;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

__declspec(naked) void _HkCb_GeneralDmg()
{
	__asm
	{
		push ecx
		push ecx
		call HkCb_GeneralDmg
		pop ecx
		jmp [fpOldGeneralDmg]
	}
}

__declspec(naked) void _HkCb_OtherDmg()
{
	__asm
	{
		push ecx
		push ecx
		call HkCb_GeneralDmg
		pop ecx
		jmp [fpOldOtherDmg]
	}
}


/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget)
{
	if(iClientID == iClientIDTarget)
		return true;
	if(iClientIDTarget)
	{
		// anti-dockkill check
		if((timeInMS() - ClientInfo[iClientIDTarget].tmSpawnTime) <= set_iAntiDockKill)
			return false; // target is protected
		if((timeInMS() - ClientInfo[iClientID].tmSpawnTime) <= set_iAntiDockKill)
			return false; // target may not shoot

		// no-pvp token check
		if(ClientInfo[iClientID].bNoPvp || ClientInfo[iClientIDTarget].bNoPvp)
			return false;

		// no-pvp check
		uint iSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);
		foreach(set_lstNoPVPSystems, uint, i)
		{
			if(iSystemID == (*i))
				return false; // no pvp
		}		
	}

	return true;
}

/**************************************************************************************************************
**************************************************************************************************************/

FARPROC fpOldNonGunWeaponHitsBase;

float fHealthBefore;
uint iHitObject;
uint iClientIDInflictor;

void __stdcall HkCb_NonGunWeaponHitsBaseBefore(char *ECX, char *p1, DamageList *dmg)
{
	CSimple *simple;
	memcpy(&simple, ECX + 0x10, 4);
	g_LastHitPts = simple->get_hit_pts();
/*	try {
		// get client id
		char *szP;
		memcpy(&szP, ECX + 0x10, 4);
		memcpy(&iHitObject, szP + 0xB0, 4);

		float fMaxHealth;
		pub::SpaceObj::GetHealth(iHitObject, fHealthBefore, fMaxHealth);

		uint iInflictorShip;
		memcpy(&iInflictorShip, p1 + 4, 4);
		iClientIDInflictor = HkGetClientIDByShip(iInflictorShip);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); } */
	g_gNonGunHitsBase = true;
}

void HkCb_NonGunWeaponHitsBaseAfter()
{
	g_gNonGunHitsBase = false;
/*	if(!fHealthBefore)
		return; // was already destroyed

	uint iType;
	pub::SpaceObj::GetType(iHitObject, iType);
	if(iType & 0x100) // 0x20 = docking ring, 0x02 = planet, 0x100 = base, 0x80 = tradelane
	{
		float fHealth;
		float fMaxHealth;
		pub::SpaceObj::GetHealth(iHitObject, fHealth, fMaxHealth);

		if(!fHealth)
			BaseDestroyed(iHitObject, iClientIDInflictor);
	} */
}

ulong lRetAddress;

__declspec(naked) void _HkCb_NonGunWeaponHitsBase()
{
	__asm
	{
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call HkCb_NonGunWeaponHitsBaseBefore
		pop ecx

		mov eax, [esp]
		mov [lRetAddress], eax
		lea eax, return_here
		mov [esp], eax
		jmp [fpOldNonGunWeaponHitsBase]
return_here:
		pushad
		call HkCb_NonGunWeaponHitsBaseAfter
		popad
		jmp [lRetAddress]
	}
}
