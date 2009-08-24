#include <map>
#ifndef _COMMON_
#define _COMMON_

#define OBJECT_DATA_SIZE 2048

typedef char CacheString;

IMPORT unsigned int __cdecl Arch2Good(unsigned int);
IMPORT unsigned int __cdecl Good2Arch(unsigned int);
IMPORT bool __cdecl GetUserDataPath(char * const);
IMPORT unsigned int __cdecl CreateID(char const *); //Makes hash of nickname
IMPORT class GoodInfoList * __cdecl GoodList_get(void);
IMPORT char *  StringAlloc(char const *,bool);
IMPORT int MAX_PLAYER_AMMO;

class IMPORT BinaryRDLWriter
{
	char szBuf[1024];

public:
	BinaryRDLWriter(class BinaryRDLWriter const &);
	BinaryRDLWriter(void);
	class BinaryRDLWriter & operator=(BinaryRDLWriter const &);
	virtual bool write_buffer(class RenderDisplayList const &,char *,unsigned int,unsigned int &);
	virtual bool write_file(class RenderDisplayList const &,char const *);
};

class IMPORT BinaryRDLReader
{
	char szBuf[1024];

public:
	BinaryRDLReader(class BinaryRDLReader const &);
	BinaryRDLReader(void);
	virtual bool extract_text_from_buffer(unsigned short *,unsigned int,unsigned int &,char const *,unsigned int);
	class BinaryRDLReader & operator=(class BinaryRDLReader const &);
	virtual bool read_buffer(class RenderDisplayList &,char const *,unsigned int);
	virtual bool read_file(class RenderDisplayList &,char const *);
};

class IMPORT TextNode
{
	char szBuf[1024];
public:
	TextNode(unsigned short const *,int);
};

template <int size> struct TString
{
	int len;
	char data[size+1];
	
	TString():len(0) { data[0] = 0; }
};

namespace BehaviorTypes
{
	IMPORT int  get_behavior_id(char const *);
	IMPORT char const *  get_behavior_name(int);
};

namespace Universe {
	struct IBase
	{
		uint iDunno1;
		uint iBaseID;
		uint iDunno2;
		uint iBaseIDS;
		uint iDunno3;
		uint iSystemID;
		uint l2[4];
		uint iSpaceObjID;
	};
	IMPORT struct IBase * __cdecl GetFirstBase(void);
	IMPORT struct IBase * __cdecl GetNextBase(void);
	IMPORT struct IBase * __cdecl get_base(unsigned int);
	IMPORT unsigned int __cdecl get_base_id(char const *);
}

namespace Reputation
{
	struct Relation;

	namespace Vibe
	{
		IMPORT int AdjustAttitude(int const &,int const &,float);
		IMPORT int Alloc(int const &);
		IMPORT int AttitudeTowards(int const &,int const &,float &);
		IMPORT int AttitudeTowardsSymmetrical(int const &,int const &,float &,float &);
		IMPORT unsigned int Count(void);
		IMPORT int EnsureExists(int const &);
		IMPORT int Free(int const &);
		IMPORT void FreeDynamic(void);
		IMPORT  int  Get(int const &,unsigned int &,unsigned int &,unsigned char &,struct Relation * const,struct FmtStr &,struct FmtStr &,unsigned short const * &);
		IMPORT  int  GetAffiliation(int const &,unsigned int &,bool);
		IMPORT  unsigned int  GetClientID(int);
		IMPORT  int  GetGroupFeelingsTowards(int const &,unsigned int const &,float &);
		IMPORT  int  GetInfocard(int const &,unsigned int &);
		IMPORT  int  GetName(int const &,struct FmtStr &,struct FmtStr &,unsigned short const * &);
		IMPORT  int  GetRank(int const &,unsigned int &);
		IMPORT  int  Set(int const &,unsigned int,unsigned int,unsigned char,struct Reputation::Relation const * const,struct FmtStr const &,struct FmtStr const &,unsigned short const *);
		IMPORT  int  SetAffiliation(int const &,unsigned int,bool);
		IMPORT  int  SetAttitudeTowardsPlayer(int,float);
		IMPORT  int  SetClientID(int,unsigned int);
		IMPORT  int  SetGroupFeelingsTowards(int const &,unsigned int const &,float);
		IMPORT  int  SetInfocard(int const &,unsigned int);
		IMPORT  int  SetName(int const &,struct FmtStr const &,struct FmtStr const &,unsigned short const *);
		IMPORT  int  SetRank(int const &,unsigned int);
		IMPORT  bool Verify(int);
	};

	IMPORT  void  FreeFeelings(void);
	IMPORT  std::map<unsigned int,unsigned int> *  GetChangedAffiliationClientMap(void);
	IMPORT  bool  IsStoryFaction(unsigned int);
	IMPORT  void  LoadFeelings(class INI_Reader &);
	IMPORT  void  Save(struct ISave *);
	IMPORT  void  Shutdown(void);
	IMPORT  bool  Startup(char const *);
	IMPORT  int  affect_relation(unsigned int const &,unsigned int const &,float);
	IMPORT  void  enumerate(struct RepGroupCB *);
	IMPORT  int  get_feelings_towards(unsigned int,unsigned int,float &);
	IMPORT  unsigned int  get_id(struct TString<16> const &);
	IMPORT  unsigned int  get_info_card(unsigned int);
	IMPORT  unsigned int  get_name(unsigned int);
	IMPORT  int  get_nickname(struct TString<16> &,unsigned int const &);
	IMPORT  unsigned int  get_short_name(unsigned int);
	IMPORT  unsigned int  group_count(void);
	IMPORT  void  set(struct TString<16> const &,struct Relation * const,unsigned int);
};


namespace pub
{
	namespace AI
	{
		enum OP_TYPE
		{
			M_NONE = -1,
			M_NULL = 0,
			M_BUZZ = 1,
			M_GOTO = 2,
			M_TRAIL = 3,
			M_FLEE = 4,
			M_EVADE = 5,
			M_IDLE = 6,
			M_DOCK = 7,
			M_LAUNCH = 8,
			M_INSTANTTRADELANE = 9,
			M_FORMATION = 10,
			M_LARGE_SHIP_MOVE = 11,
			M_CRUISE = 12,
			M_STRAFE = 13,
			M_GUIDE = 14,
			M_FACE = 15,
			M_LOOT = 16,
			M_FOLLOW = 17,
			M_DRASTICEVADE = 18,
			M_FREEFLIGHT = 19,
			M_DELAY = 20
		};
		enum ScanResponse;
		enum DirectivePriority;
		struct DirectiveCallback;

		class IMPORT BaseOp
		{
		public:
			BaseOp(BaseOp const &);
			BaseOp(OP_TYPE);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		struct IMPORT ContentCallback
		{
			virtual void combat_drift_event(void);
			virtual void combat_state_notification(enum CSNType,unsigned int,unsigned int);
			virtual void follow_event(unsigned int,unsigned int,enum FollowStatusType,float);
			virtual enum CreationType  get_creation_type(void);
			virtual bool information_request(enum InformationType,int,char *);
			virtual void lead_object_event(unsigned int,unsigned int,enum LeadProgressType,float,class Vector const &);
			virtual bool request_flee_destination(unsigned int,enum FleeReason,enum FleeDestinationType &,class Vector &,unsigned int &,float &);
			virtual void scan_state_change(bool);
			virtual void ship_in_sights_event(unsigned int);
			virtual void tether_object_event(unsigned int,unsigned int,enum TetherStatusType,class Vector const &);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveCancelOp
		{
		public:
			DirectiveCancelOp(class DirectiveCancelOp const &);
			DirectiveCancelOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveDelayOp
		{
		public:
			DirectiveDelayOp(class DirectiveDelayOp const &);
			DirectiveDelayOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveDockOp
		{
		public:
			DirectiveDockOp(class DirectiveDockOp const &);
			DirectiveDockOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveDrasticEvadeOp
		{
		public:
			DirectiveDrasticEvadeOp(class DirectiveDrasticEvadeOp const &);
			DirectiveDrasticEvadeOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveEvadeOp
		{
		public:
			DirectiveEvadeOp(class DirectiveEvadeOp const &);
			DirectiveEvadeOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveFaceOp
		{
		public:
			DirectiveFaceOp(class DirectiveFaceOp const &);
			DirectiveFaceOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveFollowOp
		{
		public:
			DirectiveFollowOp(class DirectiveFollowOp const &);
			DirectiveFollowOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveFormationOp
		{
		public:
			DirectiveFormationOp(class DirectiveFormationOp const &);
			DirectiveFormationOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveGotoOp
		{
		public:
			DirectiveGotoOp(class DirectiveGotoOp const &);
			DirectiveGotoOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveGuideOp
		{
		public:
			DirectiveGuideOp(class DirectiveGuideOp const &);
			DirectiveGuideOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveIdleOp
		{
		public:
			DirectiveIdleOp(class DirectiveIdleOp const &);
			DirectiveIdleOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveInstantTradelaneOp
		{
		public:
			DirectiveInstantTradelaneOp(class DirectiveInstantTradelaneOp const &);
			DirectiveInstantTradelaneOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveLaunchOp
		{
		public:
			DirectiveLaunchOp(class DirectiveLaunchOp const &);
			DirectiveLaunchOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveRamOp
		{
		public:
			DirectiveRamOp(class DirectiveRamOp const &);
			DirectiveRamOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveStrafeOp
		{
		public:
			DirectiveStrafeOp(class DirectiveStrafeOp const &);
			DirectiveStrafeOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveTractorBeamOp
		{
		public:
			DirectiveTractorBeamOp(class DirectiveTractorBeamOp const &);
			DirectiveTractorBeamOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveTrailOp
		{
		public:
			DirectiveTrailOp(class DirectiveTrailOp const &);
			DirectiveTrailOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveWaitForPlayerManeuverOp
		{
		public:
			DirectiveWaitForPlayerManeuverOp(class DirectiveWaitForPlayerManeuverOp const &);
			DirectiveWaitForPlayerManeuverOp(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		class IMPORT DirectiveWaitForShip
		{
		public:
			DirectiveWaitForShip(class DirectiveWaitForShip const &);
			DirectiveWaitForShip(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		struct IMPORT GSplinePath
		{
			GSplinePath(void);
			struct GSplinePath & operator=(struct GSplinePath const &);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		struct IMPORT SetFlagParams
		{
			SetFlagParams(struct SetFlagParams const &);
			SetFlagParams(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		struct IMPORT SetPersonalityParams
		{
			SetPersonalityParams(struct SetPersonalityParams const &);
			SetPersonalityParams(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

		struct IMPORT SetZoneBehaviorParams
		{
			SetZoneBehaviorParams(struct SetZoneBehaviorParams const &);
			SetZoneBehaviorParams(void);
			virtual bool validate(void);

		public:
			unsigned char data[OBJECT_DATA_SIZE];
		};

	};

	namespace StateGraph
	{
		IMPORT  int  get_state_graph(int,enum Type);
		IMPORT  int  get_state_graph(char const *,enum Type);
		IMPORT  struct IStateGraph const *  get_state_graph_internal(int);
		IMPORT  void  refresh_state_graph(void);
	};

};

struct IMPORT CEqObj
{
	struct IMPORT DockAnimInfo
	{
		DockAnimInfo(void);
		struct DockAnimInfo & operator=(struct DockAnimInfo const &);

	public:
		unsigned char data[OBJECT_DATA_SIZE];
	};

	CEqObj(struct CEqObj const &);
	virtual bool add_item(struct EquipDesc const &);
	float attitude_towards(struct CEqObj const *)const ;
	void attitude_towards_symmetrical(float &,struct CEqObj const *,float &)const ;
	void clear_arch_groups(void);
	virtual void clear_equip_and_cargo(void);
	virtual void connect(struct INotify *);
	virtual void connect(struct IObjDB *);
	virtual void destroy_sub_objs(struct DamageList const &,bool);
	virtual void disable_controllers(void);
	virtual void disconnect(struct INotify *);
	virtual void disconnect(struct IObjDB *);
	virtual void disconnect(struct IObjRW *);
	virtual void enable_controllers(void);
	virtual int enumerate_sub_objs(void)const ;
	virtual void flush_animations(void);
	virtual bool get_activate_state(class std::vector<bool,class std::allocator<bool> > &);
	unsigned int get_base(void)const ;
	unsigned int get_base_name(void)const ;
	float get_cloak_percentage(void)const ;
	unsigned int const & get_dock_target(void)const ;
	virtual void get_equip_desc_list(struct EquipDescVector &)const ;
	bool get_explosion_dmg_bounding_sphere(float &,class Vector &)const ;
	float get_max_power(void)const ;
	virtual unsigned int get_name(void)const ;
	float get_power(void)const ;
	float get_power_ratio(void)const ;
	virtual bool get_sub_obj_center_of_mass(unsigned short,class Vector &)const ;
	virtual bool get_sub_obj_hit_pts(unsigned short,float &)const ;
	virtual bool get_sub_obj_max_hit_pts(unsigned short,float &)const ;
	virtual bool get_sub_obj_relative_health(unsigned short,float &)const ;
	virtual bool get_sub_obj_velocity(unsigned short,class Vector &)const ;
	virtual float get_total_hit_pts(void)const ;
	virtual float get_total_max_hit_pts(void)const ;
	virtual float get_total_relative_health(void)const ;
	int get_vibe(void)const ;
	virtual unsigned short inst_to_subobj_id(long)const ;
	bool is_base(void)const ;
	int is_cloaked(void)const ;
	bool is_control_excluded(unsigned int)const ;
	bool is_damaged_obj_attached(struct CacheString const &)const ;
	bool is_dock(void)const ;
	virtual bool is_targetable(void)const ;
	bool launch_pos(class Vector &,class Matrix &,int)const ;
	virtual void load_equip_and_cargo(struct EquipDescVector const &,bool);
	void set_control_exclusion(unsigned int);
	void set_power(float);
	virtual bool set_sub_obj_hit_pts(unsigned short,float);
	virtual long sub_obj_id_to_inst(unsigned short)const ;
	bool sync_cargo(class EquipDescList const &);
	virtual void unmake_physical(void);
	virtual int update(float,unsigned int);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

struct IMPORT FmtStr
{
	char szBuf[0xFFFF];

public:
	struct Val
	{
	char szBuf[0xFFFF];

	private: 
		int InitFromVoid(void const *);
	public:
		Val(unsigned short,void *,unsigned char);
		Val(void const *);
		Val(void const *,int &);
		unsigned int flatten(void *,unsigned int)const;
		unsigned int get_flattened_size(void)const;
		static void __cdecl operator delete(void *);
		static void * __cdecl operator new(unsigned int); // jmp to malloc
		~Val(void);
	};
 
	FmtStr(struct FmtStr const &);
	FmtStr(unsigned int, struct Val const *);
	struct FmtStr const & operator=(struct FmtStr const &);
	bool operator==(struct FmtStr const &)const ;
	void append(struct FmtStr::Val *);
	void append_base(unsigned int const &);
	void append_fmt_str(struct FmtStr const &);
	void append_good(unsigned int const &);
	void append_installation(int);
	void append_int(unsigned int);
	void append_loot(int);
//	void append_nav_marker(struct FmtStr::NavMarker const &);
	void append_rep_group(unsigned int const &);
	void append_rep_instance(int const &);
	void append_spaceobj_id(unsigned int const &);
	void append_string(unsigned int);
	void append_system(unsigned int const &);
	int append_void(void *);
	void append_zone_id(unsigned int const &);
	void destroy(void);
	int flatten(void *,unsigned int)const;
	int unflatten(void *,unsigned int);
	void begin_mad_lib(unsigned int);
	void end_mad_lib(void);
	~FmtStr(void);
};

struct IMPORT DamageEntry
{
	char szBuf[1024];

public:
	enum SubObjFate {};
	DamageEntry(void);
	static char const * __cdecl FateToString(enum  SubObjFate);
	bool operator!=(struct DamageEntry const &)const;
	bool operator<(struct DamageEntry const &)const;
	struct DamageEntry & operator=(struct DamageEntry const &);
	bool operator==(struct DamageEntry const &)const;
	bool operator>(struct DamageEntry const &)const;
};

struct IMPORT DamageList
{
	char szBuf[1024];
public:
	DamageList(struct DamageList const &);
	DamageList(void);
	~DamageList(void);

	static char const * __cdecl DmgCauseToString(enum  DamageCause);
	void add_damage_entry(unsigned short,float,enum  DamageEntry::SubObjFate);
	enum  DamageCause get_cause(void)const;
	float get_hit_pts_left(unsigned short)const;
	unsigned int get_inflictor_id(void)const;
	unsigned int get_inflictor_owner_player(void)const;
	bool is_destroyed(void)const;
	bool is_inflictor_a_player(void)const;
	struct DamageList & operator=(struct DamageList const &);
	void set_cause(enum  DamageCause);
	void set_destroyed(bool);
	void set_inflictor_id(unsigned int);
	void set_inflictor_owner_player(unsigned int);

};

class IMPORT RenderDisplayList
{
	char szBuf[1024];
	public:
		RenderDisplayList(void);
		~RenderDisplayList(void);
};

struct IMPORT XMLReader
{
	char szBuf[1024];
public:
	virtual bool read_buffer(class RenderDisplayList &,char const *,unsigned int);
	virtual bool read_file(class RenderDisplayList &,char const *);
};

struct GoodInfo
{
	uint i1;
	uint iLen;
	float iDunno[16];
	uint ArchID;
	uint type; // == 0 when commodity
	uint i3; //hash/CRC thing
	float iDunno2[13];
	uint iIDS;
};

namespace GoodList 
{
	IMPORT struct GoodInfo const * __cdecl find_by_id(unsigned int);
	IMPORT struct GoodInfo const * __cdecl find_by_archetype(unsigned int);
};

class IMPORT GoodInfoList
{
public:
	GoodInfoList(class GoodInfoList const &);
	GoodInfoList(void);
	~GoodInfoList(void);
	class GoodInfoList & operator=(class GoodInfoList const &);
	void destroy(void);
	struct GoodInfo const * find_by_archetype(unsigned int)const ;
	struct GoodInfo const * find_by_id(unsigned int)const ;
	struct GoodInfo const * find_by_name(char const *)const ;
	struct GoodInfo const * find_by_ship_arch(unsigned int)const ;
	class std::list<struct GoodInfo *,class std::allocator<struct GoodInfo *> > const * get_list(void)const ;
	void load(char const *);

private:
	void read_Good_block(class INI_Reader *,struct GoodInfo *);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

namespace Archetype
{
 	enum AClassType
	{
		ROOT = 0,
		EQ_OBJ = 1,
		SOLAR = 2,
		SHIP = 3,
		ASTEROID = 4,
		DYNAMIC_ASTEROID = 5,
		EQUIPMENT = 6,
		ATTACHED_EQUIPMENT = 7,
		LOOT_CRATE = 8,
		CARGO_POD = 9,
		COMMODITY = 10,
		POWER = 11,
		ENGINE = 12,
		SHIELD = 13,
		SHIELD_GENERATOR = 14,
		THRUSTER = 15,
		LAUNCHER = 16,
		GUN = 17,
		MINE_DROPPER = 18,
		COUNTER_MEASURE_DROPPER = 19,
		SCANNER = 20,
		LIGHT = 21,
		TRACTOR = 22,
		ATTACHED_FX_EQUIP = 23,
		INTERNAL_FX_EQUIP = 24,
		REPAIR_DROID = 25,
		REPAIR_KIT = 26,
		SHIELD_BATTERY = 27,
		CLOAKING_DEVICE = 28,
		TRADE_LANE_EQUIP = 29, 
		PROJECTILE = 30,
		MUNITION = 31,
		MINE = 32,
		COUNTER_MEASURE = 33,
		ARMOR = 34,
	};
	
	struct IMPORT Ship
	{
		uint	iDunno1[2];
		uint	iShipArchID;
		uint	iDunno2;
		uint	iType;
		uint	iIDSName;
		uint	iIDSInfo;
		float	fHitPts;
		float	fMass;
		uint	iDunno3[2];
		float	fRotationIntertia[3];
		uint	iDunno4[45];
		uint	iIDSInfo1;
		uint	iIDSInfo2;
		uint	iIDSInfo3;
		uint	iShipClass;
		uint	iNumExhaustNozzles;
		float	fHoldSize;
		float	fLinearDrag;
		float	fAngularDrag[3];
		float	fSteeringTorque[3];
		float	fNudgeForce;
		float	fStrafeForce;
		uint	iDunno5[6];
		uint	iMaxNanobots;
		uint	iMaxShieldBats;
	};

	struct Equipment
	{
		virtual enum AClassType  get_class_type(void)const ;

		uint iVFTable;
		uint i2;
		uint iEquipID;
		char *szName;
		uint i5;
		uint iIDSName;
		uint iIDSInfo;
		float fMaxHP;
		float fMass;
		uint iDunno;
		float fExplosionResistance;
		uint iArray[12];
		float fVolume;
		uint iArray2[6];
		float fChildImpulse;
		float fParentImpulse;
		float fHullDamage; //Only for ammo entries
		uint iArray3[4];
		float fMuzzleVelocity;
		uint iAmmoArchID; // 39
		uint iArray4[67];
		uint iGunTypeFlags;
		uint iDunno2;
		uint iGunType;
		uint iArray5[50];
//		uint i[152];
	};

	struct Munition
	{
		uint iDunno;
		//uint iDunno[513];
	};

	IMPORT struct Ship * __cdecl GetShip(unsigned int);
	IMPORT struct Archetype::Equipment * __cdecl GetEquipment(unsigned int);

	IMPORT int GetArchMaxHitPts(unsigned int);
}

struct EquipDesc
{
	public: 
		ushort dunno; // 00
		ushort id; // 02
		ulong archid; // 04
		ulong hardpoint; // 08
		bool equipped; //0Ch
		ulong status; //10h
		ulong count; //14h
		bool temporary; //18h
		ulong owner; //01Ch

//		unsigned short get_id(void)const;
};

/*struct IMPORT EquipDesc
{
	EquipDesc(struct EquipDesc const &);
	EquipDesc(void);
	struct EquipDesc & operator=(struct EquipDesc const &);
	bool operator==(struct EquipDesc const &)const ;
	bool operator!=(struct EquipDesc const &)const ;
	bool operator<(struct EquipDesc const &)const ;
	bool operator>(struct EquipDesc const &)const ;

	static struct IMPORT CacheString const CARGO_BAY_HP_NAME;
	unsigned int get_arch_id(void)const ;
	float get_cargo_space_occupied(void)const ;
	int get_count(void)const ;
	struct CacheString const & get_hardpoint(void)const ;
	unsigned short get_id(void)const ;
	int get_owner(void)const ;
	float get_status(void)const ;
	bool get_temporary(void)const ;
	bool is_equipped(void)const ;
	bool is_internal(void)const ;
	void make_internal(void);
	void set_arch_id(unsigned int);
	void set_count(int);
	void set_equipped(bool);
	void set_hardpoint(struct CacheString const &);
	void set_id(unsigned short);
	void set_owner(int);
	void set_status(float);
	void set_temporary(bool);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};*/

struct EquipStatus
{
	uint iDunno;
};
struct EquipmentVal
{
	uint iDunno;
};
class IMPORT EquipDescList
{
	public: 
		EquipDescList(struct EquipDescVector const &);
		EquipDescList(class EquipDescList const &);
		EquipDescList(void);
		~EquipDescList(void);
		class EquipDescList & operator=(class EquipDescList const &);
		int add_equipment_item(struct EquipDesc const &,bool);
		struct EquipDesc const * find_equipment_item(unsigned short)const;
		struct EquipDesc const * find_matching_cargo(unsigned int,int,float)const;
		int remove_equipment_item(unsigned short,int);
};
struct IMPORT EquipDescVector
{
	EquipDescVector(struct EquipDescVector const &);
	EquipDescVector(class EquipDescList const &);
	EquipDescVector(void);
	~EquipDescVector(void);
	struct EquipDescVector & operator=(struct EquipDescVector const &);
	int add_equipment_item(struct EquipDesc const &,bool);
	void append(struct EquipDescVector const &);
	struct EquipDesc * traverse_equipment_type(unsigned int,struct EquipDesc const *);

public:
	uint iDunno;
	EquipDesc *start;
	EquipDesc *end;
};

class IMPORT CEquip
{
public:
	CEquip(class CEquip const &);
	CEquip(unsigned int,struct CEqObj *,unsigned short,struct Equipment const *,bool);
	virtual ~CEquip(void);
	virtual bool Activate(bool);
	virtual bool CanDelete(void)const ;
	void ConnectClientEquip(struct INotify *);
	void ConnectServerEquip(struct INotify *);
	virtual void Destroy(void);
	virtual bool DisableController(void);
	virtual bool EnableController(void);
	Archetype::Equipment const * EquipArch(void)const ;
	virtual bool GetConnectionPosition(class Vector *,class Matrix *)const ;
	virtual bool GetEquipDesc(struct EquipDesc &)const ;
	virtual float GetHitPoints(void)const ;
	unsigned short GetID(void)const ;
	virtual float GetMaxHitPoints(void)const ;
	struct CEqObj * GetOwner(void)const ;
	virtual float GetRelativeHealth(void)const ;
	virtual void GetStatus(struct EquipStatus &)const ;
	unsigned int GetType(void)const ;
	char const * IdentifyEquipment(void)const ;
	virtual bool IsActive(void)const ;
	virtual bool IsControllerEnabled(void)const ;
	virtual bool IsDestroyed(void)const ;
	virtual bool IsDisabled(void)const ;
	virtual bool IsFunctioning(void)const ;
	virtual bool IsLootable(void)const ;
	virtual bool IsTemporary(void)const ;
	//void Notify(enum INotify::Event,void *);
	virtual void NotifyArchGroupDestroyed(unsigned short);
	void NotifyDisconnecting(struct INotify *);
	virtual void SetHitPoints(float);
	virtual bool Update(float,unsigned int);

	static void * operator new(unsigned int);
	static void operator delete(void *);


private:
	CEquip(void);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

struct IMPORT CObject
{
	enum Class;

	CObject(struct CObject const &);
	unsigned int AddRef(void);
	static struct CObject *  Alloc(enum Class);
	static int  Count(enum Class);
	static struct CObject *  Find(unsigned int const &,enum Class);
	static struct CObject *  Find(long,enum Class);
	static struct CObject *  FindFirst(enum Class);
	static struct CObject *  FindNext(void);
	static unsigned int const  NO_ACTIONS;
	static unsigned int const  NO_BEHAVIOR;
	static unsigned int const  NO_COUNTERMEASURE_AI;
	static unsigned int const  NO_DYNAMICS;
	static unsigned int const  NO_SCANNER;
	unsigned int Release(void);
	static void  ReleaseAll(void);
	static unsigned int const  UPDATE_DEFAULT;
	static unsigned int const  UPDATE_DUMB;
	void add_impulse(class Vector const &,class Vector const &);
	void add_impulse(class Vector const &);
	//void advise(struct CBase *,bool);
	virtual void beam_object(class Vector const &,class Matrix const &,bool);
	virtual void __stdcall destroy_instance(long);
	virtual void disable_controllers(void);
	void enable_collisions_r(bool);
	virtual void enable_controllers(void);
	bool flag_part_as_shield(unsigned int);
	virtual class Vector  get_angular_velocity(void)const ;
	//struct Archetype::Root * get_archetype(void)const ;
	virtual class Vector  get_center_of_mass(void)const ;
	//void get_intruder_set_r(struct CheapSet<struct CObject *,struct std::less<struct CObject *> > &);
	virtual float get_mass(void)const ;
	class Vector  get_moment_of_inertia(void)const ;
	virtual float get_physical_radius_r(class Vector &)const ;
	virtual bool get_surface_extents(class Vector &,class Vector &)const ;
	virtual class Vector  get_velocity(void)const ;
	float hierarchy_radius(class Vector &)const ;
	float hierarchy_radius(void)const ;
	unsigned int inst_to_part(long)const ;
	bool is_shield_part(unsigned int)const ;
	//virtual void open(struct Archetype::Root *);
	long part_to_inst(unsigned int)const ;
	//virtual void remake_physical(struct PhySys::CreateParms const &,float);
	virtual void unmake_physical(void);
	virtual int update(float,unsigned int);

	static void * operator new(unsigned int);
	static void operator delete(void *);


protected:
	CObject(enum Class);
	virtual ~CObject(void);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

enum HpAttachmentType;

struct MarketGoodInfo
{
	u_long lArray[16];
};

class IMPORT BaseDataList
{
public:
	class BaseData * get_base_data(unsigned int)const;
};

class BaseDataList * __cdecl BaseDataList_get(void);

class IMPORT BaseData
{
public: 
	class std::map<unsigned int,struct MarketGoodInfo,struct std::less<unsigned int>,class std::allocator<struct MarketGoodInfo> > const * get_market(void)const;
};

class IMPORT CSimple
{
public:
	float get_hit_pts(void)const;
};

namespace Archetype
{
	class IMPORT Gun 
	{
	public: 
//		virtual enum  HpAttachmentType get_hp_type(void)const;
		int get_number_of_hp_types(void)const;
		enum HpAttachmentType get_hp_type_by_index(int)const;
	};

	IMPORT unsigned short __cdecl LargeIDToSmallID(unsigned int);
	IMPORT unsigned int __cdecl SmallIDToLargeID(unsigned short);
}

struct IMPORT CShip
{
public:
	CShip(struct CShip const &);
	CShip(void);
	virtual ~CShip(void);
	static float const  JETTISON_CLEARANCE_TIME;
	void VerifyTarget(void);
	enum FORMATION_RTYPE  add_follow_follower(struct IObjRW *);
	enum FORMATION_RTYPE  add_formation_follower(struct IObjRW *);
	void airlock_in(class Vector const &,class Matrix const &);
	bool any_thrusters_on(void)const ;
	bool check_formation_follower_status(float);
	float close_bay(void);
	virtual void disable_controllers(void);
	virtual void enable_controllers(void);
	virtual void flush_animations(void);
	bool generate_follow_offset(struct IObjInspect const *,class Transform &)const ;
	int get_ammo_capacity_remaining(unsigned int)const ;
	int get_ammo_capacity_remaining(struct Archetype::Equipment const *)const ;
	float get_angular_distance_travelled(float,float,float)const ;
	float get_angular_distance_travelled(float,float,float,float)const ;
	class Vector  get_angular_drag(void)const ;
	float get_angular_speed_XY(float,float)const ;
	float get_angular_speed_Z(float)const ;
	class Vector const & get_axis_throttle(void)const ;
	enum BayState  get_bay_state(void)const ;
	float get_cargo_hold_remaining(void)const ;
	bool get_farthest_active_gun_range(float &)const ;
	struct IObjRW * get_follow_leader(void);
	class Vector const & get_follow_offset(void);
	int get_formation_follower_count(void);
	int get_formation_followers(struct IObjRW * * const,unsigned int);
	unsigned int get_group_name(void)const ;
	struct ShipGunStats const & get_gun_stats(void)const ;
	float get_initial_speed_to_coast_distance(float,bool)const ;
	float get_linear_drag(void)const ;
	float get_max_bank_angle(void)const ;
	float get_max_thrust_power(void)const ;
	virtual unsigned int get_name(void)const ;
	class Vector  get_nudge_vec(void)const ;
	unsigned short const * get_pilot_name(void)const ;
	float get_projected_axis_throttle_XY(float)const ;
	float get_projected_axis_throttle_Z(float)const ;
	long get_renderable_subtarget(void)const ;
	int get_repair_kit_capacity_remaining(void)const ;
	int get_shield_battery_capacity_remaining(void)const ;
	int get_space_for_cargo_type(struct Archetype::Equipment const *)const ;
	int get_status(char *,int)const ;
	enum StrafeDir  get_strafe_dir(void)const ;
	unsigned short get_sub_target(void)const ;
	struct IObjRW * get_target(void)const ;
	int get_targeted_enemy_count(void);
	bool get_tgt_lead_fire_pos(class Vector &)const ;
	bool get_tgt_lead_fire_pos(struct IObjInspect const *,class Vector &)const ;
	float get_throttle(void)const ;
	float get_thrust_power(void)const ;
	float get_thrust_power_ratio(void)const ;
	float get_time_to_accelerate_angularly(float,float,float)const ;
	float get_time_to_accelerate_angularly(float,float,float,float)const ;
	unsigned int get_tradelane_entrance_ring(void);
	unsigned int get_tradelane_exit_ring(void);
	float get_tradelane_speed(void);
	int go_tradelane(struct IObjInspect const *,struct IObjInspect const *,struct IObjRW *,bool,float);
	virtual void init(struct CreateParms const &);
	bool is_cruise_active(void)const ;
	bool is_enemy(struct IObjInspect *);
	bool is_jumping(void)const ;
	bool is_launching(void)const ;
	bool is_player(void)const ;
	bool is_using_tradelane(void)const ;
	void jump_in(struct IObjRW *,class Vector const &,class Matrix const &,float);
	void launch(struct IObjRW *,struct CEqObj *,int);
	bool leaving_system_update(unsigned int &);
	struct IObjRW * next_enemy(void);
	struct IObjRW * next_target(bool);
	void notify_set_target(struct CShip *,bool);
	float open_bay(void);
	struct IObjRW * previous_enemy(void);
	enum FORMATION_RTYPE  remove_follow_follower(struct IObjRW *);
	enum FORMATION_RTYPE  remove_formation_follower(struct IObjRW *);
	void request_exit_tradelane(void);
	void set_angular_drag_factor(float);
	void set_axis_throttle(class Vector const &);
	void set_follow_leader(struct IObjRW *);
	void set_follow_offset(class Vector const &);
	void set_gun_stats_dirty(void);
	void set_nudge_vec(class Vector const &);
	void set_strafe_dir(enum StrafeDir);
	void set_sub_target(unsigned short);
	void set_target(struct IObjRW *);
	void set_throttle(float);
	void set_thrust_power(float);
	void set_tradelane_speed(float);
	struct Archetype::Ship const * shiparch(void)const ;
	void stop_tradelane(void);
	virtual int update(float,unsigned int);
	void use_tradelane(bool);
};

class IMPORT INI_Reader
{
	char szBuf[1024];

public:
	INI_Reader(class INI_Reader const &);
	INI_Reader(void);
	~INI_Reader(void);
	class INI_Reader & operator=(class INI_Reader const &);
	void close(void);
	float get_value_float(unsigned int);
	int get_value_int(unsigned int);
	char const * get_value_ptr(void);
	char const * get_value_string(unsigned int);
	char const * get_value_string(void);
	unsigned short const * get_value_wstring(void);
	bool is_header(char const *);
	bool is_number(char const *)const ;
	bool is_value(char const *);
	bool is_value_empty(unsigned int);
	bool open(char const *,bool);
	bool read_header(void);
	bool read_value(void);
	void reset(void);
};

struct IMPORT IObject
{
	enum ThrustEquipType;
};

struct IMPORT IObjInspect
{
	struct CargoEnumerator;
	struct SubtargetEnumerator;
};

struct IMPORT IObjInspectImpl
{
public:
	virtual class Vector const & get_position(void)const ;
	virtual class Vector  get_velocity(void)const ;
	virtual class Vector  get_angular_velocity(void)const ;
	virtual class Matrix const & get_orientation(void)const ;
	virtual class Transform const & get_transform(void)const ;
	virtual class Vector  get_center_of_mass(void)const ;
	virtual int get_sub_obj_center_of_mass(unsigned short,class Vector &)const ;
	virtual long get_index(void)const ;
	virtual unsigned int const  get_id(void)const ;
	virtual int get_good_id(unsigned int &)const ;
	virtual int get_archetype_extents(class Vector &,class Vector &)const ;
	virtual int get_physical_radius(float &,class Vector &)const ;
	virtual float get_mass(void)const ;
	virtual bool is_targetable(void)const ;
	virtual bool is_dying(void)const ;
	virtual int get_status(float &,float &)const ;
	virtual int get_status(float &)const ;
	virtual int get_shield_status(float &,float &,bool &)const ;
	virtual int get_shield_status(float &,bool &)const ;
	virtual int get_throttle(float &)const ;
	virtual int get_axis_throttle(class Vector &)const ;
	virtual int get_nudge_vec(class Vector &)const ;
	virtual int get_strafe_dir(enum StrafeDir &)const ;
	virtual int is_cruise_active(bool &)const ; // why in different order ?! msvc i hate you! both is_cruise_active dont work properly for players(always return false when engine kill was used during cruise) 
	virtual int is_cruise_active(bool &,bool &)const ;
	virtual int are_thrusters_active(bool &)const ;
	virtual int get_attitude_towards(float &,struct IObjInspect const *)const ;
	virtual int get_attitude_towards_symmetrical(float &,struct IObjInspect const *,float &)const ;
	virtual int get_reputation(float &,unsigned int const &)const ;
	virtual int get_target(struct IObjRW * &)const ;
	virtual int get_subtarget(unsigned short &)const ;
	virtual int get_subtarget_center_of_mass(class Vector &)const ;
	virtual int get_rank(unsigned int &)const ;
	virtual int get_affiliation(unsigned int &)const ;
	virtual int get_type(unsigned int &)const ;
	virtual int get_base(unsigned int &)const ;
	virtual int get_dock_target(unsigned int &)const ;
	virtual int get_power(float &)const ;
	virtual int get_zone_props(unsigned long &)const ;
	virtual float get_scanner_interference(void)const ;
	virtual int get_hold_left(float &)const ;
	virtual int enumerate_cargo(struct IObjInspect::CargoEnumerator *)const ;
	virtual int get_data(void const * &)const ;
	virtual int get_formation_offset(class Vector &)const ;
	virtual int get_formation_leader(struct IObjRW * &)const ;
	virtual int get_follow_offset(class Vector &)const ;
	virtual int get_follow_leader(struct IObjRW * &)const ;
	virtual bool is_player(void)const ;
	virtual int get_hardpoint(char const *,class Vector *,class Matrix *)const ;
	virtual bool has_dock_hardpoints(void)const ;
	virtual int get_dock_hardpoints(int,enum TERMINAL_TYPE *,class Transform *,class Transform *,class Transform *,float *)const ;
	virtual float get_time_to_accelerate(float,float,float,enum IObject::ThrustEquipType)const ;
	virtual float get_distance_travelled(float,float,float,enum IObject::ThrustEquipType)const ;
	virtual float get_projected_throttle(float,enum IObject::ThrustEquipType)const ;
	virtual float get_speed(float,enum IObject::ThrustEquipType)const ;
	virtual float get_initial_speed_to_coast_distance(float,bool)const ;
	virtual float get_time_to_accelerate_angularly(float,float,float)const ;
	virtual float get_time_to_accelerate_angularly(float,float,float,float)const ;
	virtual float get_angular_distance_travelled(float,float,float)const ;
	virtual float get_angular_distance_travelled(float,float,float,float)const ;
	virtual float get_angular_speed_XY(float,float)const ;
	virtual float get_angular_speed_Z(float)const ;
	virtual float get_projected_axis_throttle_XY(float)const ;
	virtual float get_projected_axis_throttle_Z(float)const ;
	virtual float get_max_bank_angle(void)const ;
	virtual int get_scanlist(struct ScanList const * &,unsigned int,bool)const ;
	virtual int get_tgt_lead_fire_pos(unsigned short const &,class Vector &)const ;
	virtual int is_pointing_at(bool &,unsigned short const &,float)const ;
	virtual int can_point_at(bool &,unsigned short const &,class Vector const &,float)const ;
	virtual int find_equipment(unsigned short * const,unsigned int,unsigned int)const ;
	virtual int get_equipment_status(struct EquipStatus &,unsigned short const &)const ;
	virtual int get_equipment_val(struct EquipmentVal &,unsigned short const &,enum EquipmentValType,float)const ;
	virtual int scan_cargo(struct IObjRW *,struct EquipDescVector &)const ;
	virtual int enumerate_subtargets(struct IObjInspect::SubtargetEnumerator *)const ;
	virtual int get_lane_direction(class Transform const &,bool *)const ;
	virtual int get_ring_side(class Transform const &,bool *)const ;
	virtual int traverse_rings(unsigned int &,bool)const ;
	virtual int is_using_tradelane(bool *)const ;
	virtual int get_lane_start(struct IObjInspect const *,struct IObjInspect const * &,class Vector &,class Vector &)const ;
	virtual bool generate_follow_offset(struct IObjInspect const *,class Transform &)const ;
	virtual int get_atmosphere_range(float &)const ;
	virtual int get_toughness(void)const ;
	virtual int get_behavior_id(void)const ;
	virtual int get_formation_followers(struct IObjRW * * const,unsigned int)const ;
	virtual struct CObject const * cobject(void)const ;

/*	IObjInspectImpl(struct IObjInspectImpl const &);
	IObjInspectImpl(void);
	virtual ~IObjInspectImpl(void);
	struct IObjInspectImpl & operator=(struct IObjInspectImpl const &);
*/
private:
	int find_equipment(class CEquip const * &,unsigned short const &)const ; 
	char szBuf[1024];
};

struct IObjRW : IObjInspectImpl
{
};

class IMPORT CEquipManager
{
public:
	CEquipManager(class CEquipManager const &);
	CEquipManager(void);
	~CEquipManager(void);
	class CEquipManager & operator=(class CEquipManager const &);
	bool AddNewEquipment(class CEquip *);
	int CleanUp(void);
	void Clear(void);
	class CExternalEquip * FindByHardpoint(struct CacheString const &);
	class CExternalEquip const * FindByHardpoint(struct CacheString const &)const ;
	class CEquip * FindByID(unsigned short);
	class CEquip const * FindByID(unsigned short)const ;
	class CEquip * FindFirst(unsigned int);
	class CEquip const * FindFirst(unsigned int)const ;
	bool HasDecayingCargo(void)const ;
	bool Init(struct CEqObj *);
	unsigned short InstToSubObjID(long)const ;
	class CEquip * ReverseTraverse(class CEquipTraverser &);
	class CEquip const * ReverseTraverse(class CEquipTraverser &)const ;
	int Size(void)const ;
	class CEquip * Traverse(class CEquipTraverser &);
	class CEquip const * Traverse(class CEquipTraverser &)const ;
	bool VerifyListSync(class EquipDescList const &)const ;

private:
	int CleanUp(class std::list<class CEquip *,class std::allocator<class CEquip *> > &);
	static void  Clear(class std::list<class CEquip *,class std::allocator<class CEquip *> > &);
};

class IMPORT CEquipTraverser
{
public:
	CEquipTraverser(int);
	CEquipTraverser(int,bool);
	CEquipTraverser(void);
	class CEquipTraverser & operator=(class CEquipTraverser const &);
	void Restart(void);
};


#endif
