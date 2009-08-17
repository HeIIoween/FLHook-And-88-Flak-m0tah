#include "hook.h"

FARPROC fpOldLaunchPos;
bool g_bInPlayerLaunch = false;
uint g_iClientID = 0;
Vector g_Vlaunch;
Matrix g_Mlaunch;

bool __stdcall LaunchPosHook(uint iSpaceID, struct CEqObj &p1, Vector &p2, Matrix &p3, int iDock) 
{
	bool iRet = p1.launch_pos(p2,p3,iDock);
	if(g_bInPlayerLaunch)
	{
		p2 = g_Vlaunch;
		p3 = g_Mlaunch;
	}

   return iRet;
}
__declspec(naked) void _HkCb_LaunchPos()
{
	__asm
   {
	  push ecx //4
	  push [esp+8+8] //8
	  push [esp+12+4] //12
	  push [esp+16+0] //16
	  push ecx
	  push [ecx+176]
	  call LaunchPosHook   
	  pop ecx
	  ret 0x0C
   }
}

vector<uint> vMarkSpaceObjProc;
int __cdecl SpaceObjCreate(unsigned int &iShip, struct pub::SpaceObj::ShipInfo const & shipinfo)
{
	int iRet = pub::SpaceObj::Create(iShip, shipinfo);
	
	try{
		if(set_fAutoMarkRadius > 0.0f) //automarking enabled
		{
			vMarkSpaceObjProc.push_back(iShip);
		}

		map<uint,float>::iterator repair = set_mapShipRepair.find(shipinfo.iShipArchetype);
		if(repair != set_mapShipRepair.end())
		{
			SHIP_REPAIR sr;
			sr.iObjID = iShip;
			sr.fIncreaseHealth = repair->second;
			g_lstRepairShips.push_back(sr);
		}

		if(set_mapItemRepair.size())
		{
			EquipDescVector edv;
			pub::GetLoadout(edv, shipinfo.iLoadout);
			EquipDesc *ed = edv.start;
			while(ed != edv.end)
			{
				map<uint,float>::iterator repair = set_mapItemRepair.find(ed->archid);
				if(repair != set_mapItemRepair.end())
				{
					SHIP_REPAIR sr;
					sr.iObjID = iShip;
					sr.fIncreaseHealth = repair->second;
					g_lstRepairShips.push_back(sr);
					break;
				}

				ed = (EquipDesc*)((char*)ed + sizeof(EquipDesc));
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	//pub::AI::enable_all_maneuvers(iShip);

	return iRet;
}

vector<DOCK_INFO> vDelayedNPCDocks;
//list<NPC_JUMP> lstJumpNPCs;
//iCancel == 0 for first dock, called again with == -1 to cancel dock in progress or signify dock completed for base dock
int __cdecl SpaceObjDock(unsigned int const &iShip, unsigned int const & iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response)
{
	//PrintUniverseText(L"Dock! %u %u %i", iShip, iDockTarget, iCancel);
	
	try{
		//Check to make sure player has permission to dock - formation docks aren't covered under RequestEvent
		if(!iCancel)
		{
			uint iClientID = HkGetClientIDByShip(iShip);
			float fRelativeHealth;
			pub::SpaceObj::GetRelativeHealth(iDockTarget, fRelativeHealth);
			if(fRelativeHealth < set_fBaseDockDmg)
			{
				if(iClientID)
				{
					pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("dock_disallowed"));
					PrintUserCmdText(iClientID, L"The docking ports are damaged");
				}
				else
				{
					DOCK_INFO dock = {iShip, iDockTarget, response};
					vDelayedNPCDocks.push_back(dock);
				}
				return 0;
			}
			if(iClientID)
			{
				if(!ClientInfo[iClientID].bCheckedDock)
				{
					ClientInfo[iClientID].lstRemCargo.clear();
					DOCK_RESTRICTION jrFind = DOCK_RESTRICTION(iDockTarget);
					DOCK_RESTRICTION *jrFound = set_btJRestrict->Find(&jrFind);
					if(jrFound)
					{
						list<CARGO_INFO> lstCargo;
						bool bPresent = false;
						HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, 0);
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
										ClientInfo[iClientID].lstRemCargo.push_back(cm);
										pub::Player::RemoveCargo(iClientID, cargo->iID, -jrFound->iCount);
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
							pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
							PrintUserCmdText(iClientID, jrFound->wscDeniedMsg);
							return 0; //block dock
						}
					}
				}
				else
					ClientInfo[iClientID].bCheckedDock = false;
			}
			/*else //NPC docking
			{
				uint iType;
				pub::SpaceObj::GetType(iDockTarget, iType);
				if(iType==64 || iType==2048) //docking at jump hole/gate
				{
					uint iJumpGate, iJumpSystemID;
					pub::SpaceObj::GetJumpTarget(iDockTarget, iJumpSystemID, iJumpGate);
					NPC_JUMP nj;
					nj.iShip = iShip;
					nj.iJumpGate = iJumpGate;
					nj.iSystemID = iJumpSystemID;
					nj.iNumTimes = 0;
					lstJumpNPCs.push_back(nj);
				}
			}*/
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	return pub::SpaceObj::Dock(iShip, iDockTarget, iCancel, response);
}

#pragma optimize("", off) //Do not remove, will cause crashes because VC++'s optimization does weird things
struct ITEM_DROP
{
	uint iSpaceObjID;
	Archetype::Equipment *eqLoot; //External look of loot (crate, for example)
	float fDunno1;
	Archetype::Equipment *eqEquip; //Actual equipment dropped
	float fDunno2;
	uint iNum;
};
bool __cdecl SpawnItem(void *ecx, void *ret1, void *ret2, uint iShip, uint iDunno2, ITEM_DROP *id, uint iDunno3)
{
	//PrintUniverseText(L"Spawned %u " + HkGetWStringFromIDS(id->eqEquip->iIDSName), id->iNum);
	try{
		map<uint,uint>::iterator item = set_mapNoSpaceItems.find(id->eqEquip->iEquipID);
		if(item != set_mapNoSpaceItems.end())
		{
			float fHealth;
			pub::SpaceObj::GetRelativeHealth(iShip, fHealth);
			if(fHealth)
			{
				uint iClientID = HkGetClientIDByShip(iShip);
				if(iClientID)
				{
					HkAddCargo(ARG_CLIENTID(iClientID), id->eqEquip->iEquipID, id->iNum, false);
					pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("objective_failed"));
				}
			}
			return false;
		}
		item = set_mapAutoMark.find(id->eqEquip->iEquipID);
		if(item != set_mapAutoMark.end())
		{
			mstime tmTimeNow = timeInMS();
			DELAY_MARK dm;
			dm.time = tmTimeNow;
			dm.iObj = id->iSpaceObjID;
			g_lstDelayedMarks.push_back(dm);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	return true;
}
#pragma warning( disable : 4414 ) //disable "short jump to function converted to near" warning
__declspec(naked) void _HkCb_SpawnItem()
{
	__asm
	{
		push ecx
		call SpawnItem
		pop ecx
		test al, al
		pop eax
		jnz _SpawnItemOrig
		ret 16
	}
}
#pragma warning( default : 4414 )
__declspec(naked) void _SpawnItemOrig()
{
	__asm
	{
		//push byte 0xFF
		//push 0x6D61337
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		jmp eax
	}
}
#pragma optimize("", on) 

/*int __cdecl SpaceObjLightFuse(unsigned int const &iShip ,char const *sz ,float fDunno)
{
	PrintUniverseText(L"LightFuse %f", fDunno);
	
	return pub::SpaceObj::LightFuse(iShip, sz, fDunno);
}*/

/*int __cdecl SolarSpaceObjCreate(unsigned int & iShip, struct pub::SpaceObj::SolarInfo const & solarinfo)
{
	int iRet = pub::SpaceObj::CreateSolar(iShip, solarinfo);
	ConPrint(L"Solar created!\n");
	return iRet;
}*/

/*int __cdecl SpaceObjDestroy(unsigned int iShip, enum DestroyType dType)
{
	list<NPC_JUMP>::iterator itNJ;
	for(itNJ = lstJumpNPCs.begin(); itNJ!=lstJumpNPCs.end(); itNJ++)
	{
		if(itNJ->iShip == iShip)
		{
			if(!itNJ->iNumTimes)
			{
				Matrix Mjump;
				Vector Vjump;
				pub::SpaceObj::GetLocation(itNJ->iJumpGate, Vjump, Mjump);
				Vjump.x -= Mjump.data[0][2]*750;
				Vjump.y -= Mjump.data[1][2]*750;
				Vjump.z -= Mjump.data[2][2]*750;
				PrintUniverseText(L"NPCs relocated");
				pub::SpaceObj::Relocate(itNJ->iShip, itNJ->iSystemID, Vjump, Mjump);
			}
			else if(itNJ->iNumTimes > 2)
			{
				pub::SpaceObj::Destroy(iShip, dType);
				lstJumpNPCs.erase(itNJ);
				return 0;
			}
			itNJ->iNumTimes++;
			return -1;
		}
	}

	int iRet = pub::SpaceObj::Destroy(iShip, dType);
	return iRet;
}*/

unsigned int __cdecl ControllerCreate(char const *szDLLPath, char const *szControllerType, struct pub::Controller::CreateParms const *create, enum pub::Controller::PRIORITY iDunno)
{
	uint iControllerID;
	if(!strcmp(szControllerType, "Mission_13"))
	{
		iControllerID = ClientInfo[create->iClientID].iControllerID;
		if(!iControllerID)
		{
			iControllerID = pub::Controller::Create(szDLLPath, szControllerType, create, iDunno);
			ClientInfo[create->iClientID].iControllerID = iControllerID;
			ConPrint(L"Create %s %u | %u %u\n", stows(szControllerType).c_str(), iControllerID, create->iID, create->iClientID);
		}
	}
	else
	{
		iControllerID = pub::Controller::Create(szDLLPath, szControllerType, create, iDunno);
	}
	return iControllerID;
}

int __cdecl ControllerSend(unsigned int const &iControllerID, int iMsgType, void const *msgData)
{
	//PrintUniverseText(L"%u %i %u", iControllerID, iMsgType, msgData);
	//ConPrint(L"%u %i %u\n", iControllerID, iMsgType, msgData);

	return pub::Controller::_SendMessage(iControllerID, iMsgType, msgData);
}

void __cdecl ControllerDestroy(unsigned int iControllerID)
{
	ConPrint(L"Destroy %u\n", iControllerID);
	return pub::Controller::Destroy(iControllerID);
}