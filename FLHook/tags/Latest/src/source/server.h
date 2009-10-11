#ifndef _SERVER_
#define _SERVER_
#include <vector>
#include "common.h"

#define OBJECT_DATA_SIZE 2048

struct CHAT_ID
{
	int iID;
};

enum DestroyType
{
	DT_TEST = 2,
};

enum DOCK_HOST_RESPONSE
{
	DH_Test = 1
};

struct XSetTarget
{
   uint iShipID;
   ushort sSlotID;
   ushort sDunno2;
   uint iTargetSpaceID;
   ushort sDunno3;
};

namespace pub
{ 
	struct CargoEnumerator;
	namespace Player
	{
		IMPORT int __cdecl InspectCash(unsigned int const &,int &);
		IMPORT int __cdecl AdjustCash(unsigned int const &,int);
		IMPORT int __cdecl ForceLand(unsigned int,unsigned int);
		IMPORT int __cdecl PopUpDialog(unsigned int,struct FmtStr const &,struct FmtStr const &,unsigned int);
		IMPORT int __cdecl DisplayMissionMessage(unsigned int const &,struct FmtStr const &,enum  MissionMessageType,bool);
		IMPORT int __cdecl GetLocation(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetSystem(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetBase(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetShip(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetShipID(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetMsnID(unsigned int,unsigned int &);
		IMPORT int __cdecl EnumerateCargo(unsigned int const &,struct pub::CargoEnumerator &);
		IMPORT int __cdecl RemoveCargo(unsigned int const &,unsigned short,unsigned int);
		IMPORT int __cdecl AddCargo(unsigned int const &,unsigned int const &,unsigned int,float,bool);
		IMPORT int __cdecl GetRemainingHoldSize(unsigned int const &,float &);
		IMPORT int __cdecl GetMoneyNeededToNextRank(unsigned int const &,int &);
		IMPORT int __cdecl GetNumKills(unsigned int const &,int &);
		IMPORT int __cdecl SetNumKills(unsigned int const &,int);
		IMPORT int __cdecl ReturnPlayerStats(unsigned int,unsigned char *,int);
		IMPORT int __cdecl GetRep(unsigned int const &,int &);
		IMPORT int __cdecl GetGroupMembers(unsigned int,std::vector<unsigned int> &);
		IMPORT int __cdecl RevertCamera(unsigned int);
		IMPORT int __cdecl SetCamera(unsigned int,class Transform const &,float,float);
		IMPORT int __cdecl MarkObj(unsigned int,unsigned int,int);
		IMPORT int __cdecl SendNNMessage(unsigned int,unsigned int); //Use GetNicknameId for msg entries in voices_recognizable.ini
		IMPORT int __cdecl GetRelativeHealth(unsigned int const &,float &);
		IMPORT int __cdecl SetInitialOrnt(unsigned int const &,class Matrix const &);
		IMPORT int __cdecl SetInitialPos(unsigned int const &,class Vector const &);
		IMPORT int __cdecl GetAssetValue(unsigned int const &,float &);
		IMPORT int __cdecl SetShipAndLoadout(unsigned int const &,unsigned int,struct EquipDescVector const &);
	}

	namespace GF
	{
		IMPORT unsigned int __cdecl FindBase(char const *);
	}

	namespace SpaceObj
	{
		struct ShipInfo
		{
			 uint iFlag; // 4
			 uint iSystem; //u System
			 uint iShipArchetype; //u ship f.e. "Titan"
			 Vector vPos; //u position
			 Vector vUnknown4; //u all 0
			 Vector vUnknown7; //u all 0
			 // struct start?!
			 uint iUnknown10; //u identical for formation followers?!
			 uint iUnknown11; // identical for formation followers?! / 0
			 float fLookDirection; // identical for formation followers?!
			 uint iUnknown13; // 0
			 float fUnknown14; // 1.0
			 uint iUnknown15; // unknown id, or float pos // 0
			 uint iUnknown16; // pos?
			 uint iUnknown17; // unknown id, or float pos // 0
			 uint iUnknown18; // pos?
			 uint iUnknown19; //u 0
			 uint iLoadout; //u fc_c_co_elite2_loadout01
			 uint iUnknown20; //u 063A0880 mem address? to eq?!
			 uint iUnknown21; // 0
			 uint iUnknown22; // 0
			 uint iUnknown23; // 0
			 uint iUnknown24; // 0

			 // struct start
			 uint iLook1; //u li_newscaster_head_gen_hat
			 uint iLook2; //u pl_female1_journeyman_body
			 uint iUnknown25; //u 0
			 uint iUnknown26; //u 0
			 uint iComm; // comm_br_darcy_female
			 uint iUnknown27; // unknown id
			 uint iUnknown28; // unknown id
			 uint iFlag2; // 4
			 uint iUnknown29; // 00860000
			 uint iUnknown30; // 1/00000018
			 uint iUnknown31; // 00860000/00000010
			 uint iUnknown32; // 00860000/008601B8
			 uint iUnknown33; //u 1
			 // struct end

			 uint iPilotInfo; //u 2/3/4/F
			 uint iPilotVoice; //u pilot_f_leg_f01a
			 uint iUnknown35; //u 0
			 uint iUnknown36; //u -1
			 uint iUnknown37; //u 0
			 uint iUnknown38; // 0
			 uint iLevel; //u
		}; 
		//struct SolarInfo;
		IMPORT int __cdecl GetHealth(unsigned int const &,float &,float &);
		IMPORT int __cdecl GetArchetypeID(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetGoodID(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetSolarArchetypeNickname(char *,int,unsigned int);
		IMPORT int __cdecl GetType(unsigned int,unsigned int &);
		IMPORT int __cdecl GetSolarArchetypeID(unsigned int,unsigned int &);
		IMPORT int __cdecl GetSolarParent(unsigned int const &,unsigned int &);
		IMPORT int __cdecl Destroy(unsigned int,enum  DestroyType);
		IMPORT int __cdecl SetRelativeHealth(unsigned int const &,float);
		IMPORT int __cdecl GetRelativeHealth(unsigned int const &,float &);
		IMPORT int __cdecl GetLocation(unsigned int,class Vector &,class Matrix &);
		IMPORT int __cdecl Relocate(unsigned int const &,unsigned int const &,class Vector const &,class Matrix const &);
		IMPORT int __cdecl GetMotion(unsigned int,class Vector &,class Vector &);
		IMPORT int __cdecl GetRep(unsigned int,int &);
		IMPORT int __cdecl GetSolarRep(unsigned int,int &);
		IMPORT int __cdecl GetMass(unsigned int const &,float &);
		IMPORT int __cdecl DrainShields(unsigned int);
		IMPORT int __cdecl InstantDock(unsigned int const &,unsigned int const &,int);
		IMPORT int __cdecl GetSystem(unsigned int,unsigned int &);		
		IMPORT int __cdecl JumpIn(unsigned int const &,unsigned int const &);
		IMPORT int __cdecl GetDockingTarget(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetTarget(unsigned int const &,unsigned int &);
		IMPORT int __cdecl Create(unsigned int &,struct ShipInfo const &);
		IMPORT int __cdecl CreateSolar(unsigned int &,struct SolarInfo const &);
		IMPORT int __cdecl SetInvincible2(unsigned int,bool,bool,float);
		IMPORT int __cdecl SetInvincible(unsigned int,bool,bool,float);
		IMPORT int __cdecl Dock(unsigned int const &,unsigned int const &,int,enum DOCK_HOST_RESPONSE);
		IMPORT int __cdecl ExistsAndAlive(unsigned int);
		IMPORT int __cdecl GetShieldHealth(unsigned int const &,float &,float &,bool &);
		IMPORT int __cdecl GetRadius(unsigned int const &,float &,class Vector &);
		IMPORT int __cdecl GetJumpTarget(unsigned int const &,unsigned int &,unsigned int &);
		IMPORT int __cdecl GetTradelaneNextAndPrev(unsigned int const &,unsigned int &,unsigned int &);
		IMPORT int __cdecl AddImpulse(unsigned int const &,class Vector const &,class Vector const &);
		IMPORT int __cdecl LightFuse(unsigned int const &,char const *,float);

	}

	namespace System
	{
		IMPORT int __cdecl Find(unsigned int const &,char const *,unsigned int &);
		IMPORT int __cdecl GetName(unsigned int,unsigned int &);
		IMPORT int __cdecl GetNestedProperties(unsigned int const &,class Vector const &,unsigned long &);
		IMPORT int __cdecl LoadSystem(unsigned int const &);
		IMPORT int __cdecl ScanObjects(unsigned int const &,unsigned int * const,unsigned int,class Vector const &,float,unsigned int,unsigned int &);
	}

	namespace Market
	{
		IMPORT int __cdecl GetPrice(unsigned int,unsigned int,float &);
		IMPORT int __cdecl GetNominalPrice(unsigned int,float &);
		IMPORT int __cdecl GetCommoditiesForSale(unsigned int,unsigned int * const,int *);
	}

	namespace Reputation
	{
		IMPORT int __cdecl GetAttitude(int const &,int const &,float &);
		IMPORT int __cdecl GetGroupFeelingsTowards(int const &,unsigned int const &,float &);
		IMPORT int __cdecl GetReputation(int &,char const *);
		IMPORT int __cdecl GetGroupName(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetReputationGroup(unsigned int &,char const *);
		IMPORT int __cdecl GetAffiliation(int const &,unsigned int &);
		IMPORT int __cdecl SetAffiliation(int const &,unsigned int const &);
		IMPORT int __cdecl SetAttitude(int const &,int const &,float);
		IMPORT int __cdecl SetReputation(int const &,unsigned int const &,float);
		IMPORT int __cdecl SetReputation(unsigned int const &,unsigned int const &,float);
		IMPORT int __cdecl GetName(int const &,struct FmtStr &,struct FmtStr &);
		IMPORT int __cdecl GetShortGroupName(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetGroupName(unsigned int const &,unsigned int &);
		IMPORT int __cdecl GetReputation(unsigned int const &,unsigned int const &,float &);
	}

	namespace AI
	{
		enum OP_RTYPE;

		IMPORT  OP_RTYPE __cdecl SubmitDirective(unsigned int,class BaseOp *);
		IMPORT  OP_RTYPE __cdecl SubmitState(unsigned int,class BaseOp *);
		IMPORT  bool __cdecl enable_all_maneuvers(unsigned int);
		IMPORT  bool __cdecl enable_maneuver(unsigned int,int,bool);
		IMPORT  int __cdecl get_behavior_id(unsigned int);
		IMPORT  enum ScanResponse __cdecl get_scan_response(unsigned int,unsigned int,unsigned int);
		IMPORT  int __cdecl get_state_graph_id(unsigned int);
		IMPORT  bool __cdecl lock_maneuvers(unsigned int,bool);
		IMPORT  void __cdecl refresh_state_graph(unsigned int);
		IMPORT  int __cdecl remove_forced_target(unsigned int,unsigned int);
		IMPORT  OP_RTYPE  __cdecl set_directive_priority(unsigned int,enum DirectivePriority);
		IMPORT  bool __cdecl set_player_enemy_clamp(unsigned int,int,int);
		IMPORT  int __cdecl submit_forced_target(unsigned int,unsigned int);
		IMPORT  enum FORMATION_RTYPE __cdecl update_formation_state(unsigned int,unsigned int,class Vector const &);
	};

	namespace Controller
	{
		struct CreateParms
		{
			uint iID;
			uint iClientID;
		};
		enum PRIORITY;
		IMPORT  unsigned int __cdecl Create(char const *,char const *,struct CreateParms const *,enum PRIORITY);
		IMPORT  void __cdecl Destroy(unsigned int);
		IMPORT  int __cdecl SetHeartbeatInterval(unsigned int const &,float);
		IMPORT  int __cdecl _SendMessage(unsigned int const &,int,void const *);
	};

	IMPORT int Save(unsigned int,unsigned int);
	IMPORT int GetBaseID(unsigned int &,char const *);
	IMPORT int GetSystemID(unsigned int &,char const *);
	IMPORT unsigned int __cdecl GetBaseNickname(char *,unsigned int,unsigned int const &);
	IMPORT int __cdecl GetBaseStridName(unsigned int &,unsigned int const &);
	IMPORT unsigned int __cdecl GetSystemNickname(char *,unsigned int,unsigned int const &);
	IMPORT int __cdecl GetShipID(unsigned int &,char const *);
	IMPORT int __cdecl GetGoodID(unsigned int &,char const *);
	IMPORT int __cdecl GetEquipmentID(unsigned int &,char const *);
	IMPORT int __cdecl GetLoadoutID(unsigned int &,char const *);
	IMPORT int __cdecl GetLoadout(struct EquipDescVector &,unsigned int const &);
	IMPORT int __cdecl IsCommodity(unsigned int const &,bool &);
	IMPORT int __cdecl GetBases(unsigned int const &,unsigned int * const,unsigned int,unsigned int &);
	IMPORT unsigned int __cdecl GetNicknameId(char const *);
	IMPORT int __cdecl GetSystemGateConnection(unsigned int const &,unsigned int &);
	IMPORT int __cdecl GetMaxHitPoints(unsigned int const &,int &);
	IMPORT int __cdecl GetFullHealth(unsigned int const &,unsigned int &);
}

struct SSPMunitionCollisionInfo
{
	uint iProjectileArchID;
	DWORD dw2;
	DWORD dwTargetShip;
	short s1;
};

struct SSPObjCollisionInfo
{
	DWORD dwTargetShip;
	ulong iHitSubObj;
};

struct XActivateEquip
{
	long	l1;
	ushort	sID;
	bool	bActivate;
};

struct XActivateCruise
{
	long	l1;
	bool	bActivate;
};

struct XActivateThrusters
{
	long	l1;
	bool	bActivate;
};

struct SGFGoodSellInfo
{
	long	l1;
	uint	iArchID;
	int		iCount;
};

struct SGFGoodBuyInfo
{
	uint iBaseID;
	ulong lNull;
	uint iGoodID;
	int iCount;
};

struct XFireWeaponInfo
{
	uint iDunno1;
	Vector vDirection;
	uint *iDunno2;
	/*short *sArray1;
	short *sArray2;
	short *s3;*/
	ushort *iDunno3;
	uint *iDunno4;
	uint *iDunno5; //Same as iDunno4
};

struct SSPObjUpdateInfo
{
	float f[9];
	uint i;
};

struct IMPORT IServerImpl
{
public:
	virtual void AbortMission(unsigned int,unsigned int);
	virtual void AcceptTrade(unsigned int,bool);
	virtual void ActivateCruise(unsigned int,struct XActivateCruise const &);
	virtual void ActivateEquip(unsigned int,struct XActivateEquip const &);
	virtual void ActivateThrusters(unsigned int,struct XActivateThrusters const &);
	virtual void AddTradeEquip(unsigned int,struct EquipDesc const &);
	virtual void BaseEnter(unsigned int,unsigned int);
	virtual void BaseExit(unsigned int,unsigned int);
	virtual void BaseInfoRequest(unsigned int,unsigned int,bool);
	virtual void CharacterInfoReq(unsigned int,bool);
	virtual void CharacterSelect(struct CHARACTER_ID const &,unsigned int);
	virtual void CharacterSkipAutosave(unsigned int);
	virtual void CommComplete(unsigned int,unsigned int,unsigned int,enum  CommResult);
	virtual void Connect(char const *,unsigned short *);
	virtual void CreateNewCharacter(struct SCreateCharacterInfo const &,unsigned int);
	virtual void DelTradeEquip(unsigned int,struct EquipDesc const &);
	virtual void DestroyCharacter(struct CHARACTER_ID const &,unsigned int);
	virtual void DisConnect(unsigned int,enum  EFLConnection);
	virtual void Dock(unsigned int const &,unsigned int const &);
	virtual void DumpPacketStats(char const *);
	virtual void ElapseTime(float);
	virtual void FireWeapon(unsigned int,struct XFireWeaponInfo const &);
	virtual void GFGoodBuy(struct SGFGoodBuyInfo const &,unsigned int);
	virtual void GFGoodSell(struct SGFGoodSellInfo const &,unsigned int);
	virtual void GFGoodVaporized(struct SGFGoodVaporizedInfo const &,unsigned int);
	virtual void GFObjSelect(unsigned int,unsigned int);
	virtual unsigned int GetServerID(void);
	virtual char const * GetServerSig(void);
	void GetServerStats(struct ServerStats &);
	virtual void GoTradelane(unsigned int,struct XGoTradelane const &);
	virtual void Hail(unsigned int,unsigned int,unsigned int);
	virtual void InitiateTrade(unsigned int,unsigned int);
	virtual void InterfaceItemUsed(unsigned int,unsigned int);
	virtual void JettisonCargo(unsigned int,struct XJettisonCargo const &);
	virtual void JumpInComplete(unsigned int,unsigned int);
	virtual void LaunchComplete(unsigned int,unsigned int);
	virtual void LocationEnter(unsigned int,unsigned int);
	virtual void LocationExit(unsigned int,unsigned int);
	virtual void LocationInfoRequest(unsigned int,unsigned int,bool);
	virtual void Login(struct SLoginInfo const &,unsigned int);
	virtual void MineAsteroid(unsigned int,class Vector const &,unsigned int,unsigned int,unsigned int,unsigned int);
	virtual void MissionResponse(unsigned int,unsigned long,bool,unsigned int);
	virtual void MissionSaveB(unsigned int,unsigned long);
	virtual void NewCharacterInfoReq(unsigned int);
	virtual void OnConnect(unsigned int);
	virtual void PlayerLaunch(unsigned int,unsigned int);
	virtual void PushToServer(class CDAPacket *);
	virtual void PopUpDialog(unsigned int,unsigned int);
	virtual void RTCDone(unsigned int,unsigned int);
	virtual void ReqAddItem(unsigned int,char const *,int,float,bool,unsigned int);
	virtual void ReqCargo(class EquipDescList const &,unsigned int);
	virtual void ReqChangeCash(int,unsigned int);
	virtual void ReqCollisionGroups(class std::list<struct CollisionGroupDesc,class std::allocator<struct CollisionGroupDesc> > const &,unsigned int);
	virtual void ReqDifficultyScale(float,unsigned int);
	virtual void ReqEquipment(class EquipDescList const &,unsigned int);
	virtual void ReqHullStatus(float,unsigned int);
	virtual void ReqModifyItem(unsigned short,char const *,int,float,bool,unsigned int);
	virtual void ReqRemoveItem(unsigned short,int,unsigned int);
	virtual void ReqSetCash(int,unsigned int);
	virtual void ReqShipArch(unsigned int,unsigned int);
	virtual void RequestBestPath(unsigned int,unsigned char *,int);
	virtual void RequestCancel(int,unsigned int,unsigned int,unsigned long,unsigned int);
	virtual void RequestCreateShip(unsigned int);
	virtual void RequestEvent(int,unsigned int,unsigned int,unsigned int,unsigned long,unsigned int);
	virtual void RequestGroupPositions(unsigned int,unsigned char *,int);
	virtual void RequestPlayerStats(unsigned int,unsigned char *,int);
	virtual void RequestRankLevel(unsigned int,unsigned char *,int);
	virtual void RequestTrade(unsigned int,unsigned int);
	virtual void SPBadLandsObjCollision(struct SSPBadLandsObjCollisionInfo const &,unsigned int);
	virtual void SPMunitionCollision(struct SSPMunitionCollisionInfo const &,unsigned int);
	virtual void SPObjCollision(struct SSPObjCollisionInfo const &,unsigned int);
	virtual void SPObjUpdate(struct SSPObjUpdateInfo const &,unsigned int);
	virtual void SPRequestInvincibility(unsigned int,bool,enum  InvincibilityReason,unsigned int);
	virtual void SPRequestUseItem(struct SSPUseItem const &,unsigned int);
	virtual void SPScanCargo(unsigned int const &,unsigned int const &,unsigned int);
	virtual void SaveGame(struct CHARACTER_ID const &,unsigned short const *,unsigned int);
	virtual void SetActiveConnection(enum  EFLConnection);
	virtual void SetInterfaceState(unsigned int,unsigned char *,int);
	virtual void SetManeuver(unsigned int,struct XSetManeuver const &);
	virtual void SetMissionLog(unsigned int,unsigned char *,int);
	virtual void SetTarget(unsigned int,struct XSetTarget const &);
	virtual void SetTradeMoney(unsigned int,unsigned long);
	virtual void SetVisitedState(unsigned int,unsigned char *,int);
	virtual void SetWeaponGroup(unsigned int,unsigned char *,int);
	virtual void Shutdown(void);
	virtual bool Startup(struct SStartupInfo const &);
	virtual void StopTradeRequest(unsigned int);
	virtual void StopTradelane(unsigned int,unsigned int,unsigned int,unsigned int);
	virtual void SubmitChat(struct CHAT_ID,unsigned long,void const *, struct CHAT_ID,int);
	virtual void SystemSwitchOutComplete(unsigned int,unsigned int);
	virtual void TerminateTrade(unsigned int,int);
	virtual void TractorObjects(unsigned int,struct XTractorObjects const &);
	virtual void TradeResponse(unsigned char const *,int,unsigned int);
	virtual int Update(void);
};

//?PopUpDialog@Player@pub@@YAHIABUFmtStr@@0I@Z
//?PopUpDialog@Player@pub@@YAHIABUFmtStr@@II@Z

class IMPORT CAccount
{
public:
	void ForceLogout(void);
	void DeleteCharacterFromID(std::basic_string<char> &);
	void InitFromFolder(char const *);
};

namespace BaseGroupMessage
{
	enum Type;
};

class IMPORT CPlayerGroup
{
public:
	 CPlayerGroup(class CPlayerGroup const &);
	 CPlayerGroup(void);
	 virtual ~CPlayerGroup(void);
	 class CPlayerGroup & operator=(class CPlayerGroup const &);
	 bool AddInvite(unsigned int);
	 bool AddMember(unsigned int);
	 bool DelInvite(unsigned int);
	 bool DelMember(unsigned int);
	 void DeliverChat(unsigned long,void const *);
	 static class CPlayerGroup * FromGroupID(unsigned int);
	 unsigned int GetID(void);
	 unsigned int GetInviteCount(void);
	 unsigned int GetMember(int)const ;
	 unsigned int GetMemberCount(void);
	 unsigned int GetMissionID(void);
	 unsigned int GetMissionSetBy(void);
	 void HandleClientLogout(unsigned int);
	 bool IsFull(void);
	 bool IsInvited(unsigned int);
	 bool IsMember(unsigned int);
	 void RewardMembers(int);
	 void SendChat(int,unsigned short const *,...);
	 void SendGroup(enum BaseGroupMessage::Type,unsigned int);
	 void SetMissionID(unsigned int,unsigned int);
	 void SetMissionMessage(struct CSetMissionMessage &);
	 void SetMissionObjectives(struct CMissionObjectives &);
	 void StoreMemberList(std::vector<unsigned int> &);

protected:
	 static unsigned int  s_uiGroupID;
};

struct PlayerData
{
	uint iArray1[153];
	uint iShipArchID;
	uint iArray2[58];
	float fPosX;
	float fPosY;
	float fPosZ;
	uint iArray3[33];
	uint iSystemID;
	uint iSpaceObjID;
	uint iDunno;
	uint iBaseID;
	uint iPrevBaseID;
	uint iArray5[9];
};

class IMPORT PlayerDB
{
public:
	struct PlayerData & operator[](unsigned int const &);
	bool BanAccount(flstr &,bool);
	class CAccount* FindAccountFromCharacterName(flstr &);
	struct PlayerData * traverse_active(struct PlayerData *)const;
	unsigned short const * GetActiveCharacterName(unsigned int)const;
	class CAccount * FindAccountFromClientID(unsigned int);
	bool GetAccountAdminRights(flstr &);
	unsigned int GetMaxPlayerCount(void);
	unsigned int GetGroupID(unsigned int);
	unsigned char login(struct SLoginInfo const &,unsigned int);
	bool DeleteCharacterFromName(std::basic_string<unsigned short,struct ci_wchar_traits> &);
	void DeleteCharacterFromID(std::basic_string<char> &);
	void logout(unsigned int);
	void LockAccountAccess(std::basic_string<unsigned short,struct ci_wchar_traits> &);
	void UnlockAccountAccess(std::basic_string<unsigned short,struct ci_wchar_traits> &);
private:
	unsigned char load_user_data(struct SLoginInfo const &,unsigned int);
public:
	unsigned char data[OBJECT_DATA_SIZE];
};

#endif