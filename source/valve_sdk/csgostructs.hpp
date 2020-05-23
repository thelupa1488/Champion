#pragma once
#include "misc/BoneAccessor.h"
#include "sdk.hpp"
#include <array>
#include "../helpers/utils.hpp"

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
    } 

#define NETVAR_OFFSET(type, name, table, netvar, offset)             \
    type& name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name + offset);                 \
    } 

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
    }

#define NETPROP(name, table, netvar) static RecvProp* name() \
{ \
	static auto prop_ptr = NetvarSys::Get().GetNetvarProp(table,netvar); \
	return prop_ptr; \
}

#define OFFSET(type, name, offset)             \
    type& name##() const {                                          \
        return *(type*)((uintptr_t)this + offset);                 \
    } 

struct datamap_t;
class AnimationLayer;
class CBasePlayerAnimState;
class CCSGOPlayerAnimState;
class C_BaseEntity;


enum TeamID 
{
	TEAM_UNASSIGNED,
	TEAM_SPECTATOR,
	TEAM_TERRORIST,
	TEAM_COUNTER_TERRORIST,
};

enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

namespace Globals {
	extern CUserCmd* cmd;
	extern bool bSendPacket;
	extern bool got_w2s_matrix;
	extern DWORD w2s_offset;
}

class C_BaseEntity;

// Created with ReClass.NET by KN4CK3R
class CHudTexture
{
public:
	char szShortName[64];    //0x0000
	char szTextureFile[64];  //0x0040
	bool bRenderUsingFont;   //0x0080
	bool bPrecached;         //0x0081
	int8_t cCharacterInFont; //0x0082
	uint8_t pad_0083[1];     //0x0083
	uint32_t hFont;          //0x0084
	int32_t iTextureId;      //0x0088
	float afTexCoords[4];    //0x008C
	uint8_t pad_009C[16];    //0x009C
};

class C_EconItemView
{
private:
	using str_32 = char[32];
public:
	NETVAR(int32_t, m_bInitialized, "DT_BaseAttributableItem", "m_bInitialized");
	NETVAR(short, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	NETVAR(int32_t, m_iEntityLevel, "DT_BaseAttributableItem", "m_iEntityLevel");
	NETVAR(int32_t, m_iAccountID, "DT_BaseAttributableItem", "m_iAccountID");
	NETVAR(int32_t, m_iItemIDLow, "DT_BaseAttributableItem", "m_iItemIDLow");
	NETVAR(int32_t, m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh");
	NETVAR(int32_t, m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality");
	NETVAR(str_32, m_iCustomName, "DT_BaseAttributableItem", "m_szCustomName");
};

class C_BaseEntity : public IClientEntity
{
public:
	datamap_t* GetDataDescMap() {
		typedef datamap_t* (__thiscall * o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>(this, 15)(this);
	}

	datamap_t* GetPredDescMap() {
		typedef datamap_t* (__thiscall * o_GetPredDescMap)(void*);
		return CallVFunction<o_GetPredDescMap>(this, 17)(this);
	}
	static __forceinline C_BaseEntity* GetEntityByIndex(int index) {
		return static_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(index));
	}
	static __forceinline C_BaseEntity* get_entity_from_handle(CBaseHandle h) {
		return static_cast<C_BaseEntity*>(g_EntityList->GetClientEntityFromHandle(h));
	}

	NETVAR_OFFSET(int, GetIndex_, "DT_BaseEntity", "m_bIsAutoaimTarget", +0x4);

	int GetClassId(const char* className)
	{
		static DWORD serverGameDLL = **(DWORD * *)(Utils::PatternScan(GetModuleHandle(L"engine.dll"), "8B 0D ? ? ? ? 56 57 8B 01 FF 50 28 8B 3D ? ? ? ? 8B F0 68") + 0x2);
		ClientClass* serverclass = CallVFunction<ClientClass*(__thiscall*)(PVOID)>((void*)serverGameDLL, 10)((void*)serverGameDLL);
		int id = 0;
		while (serverclass)
		{
			if (!strcmp(serverclass->m_pNetworkName, className))
				return id;
			serverclass = serverclass->m_pNext, id++;
		}
		return -1;
	}

	int GetSequenceActivity(int sequence);

	NETVAR(int32_t, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex");
	NETVAR(int32_t, m_iTeamNum, "DT_BaseEntity", "m_iTeamNum");
	NETVAR(Vector, m_vecOrigin, "DT_BaseEntity", "m_vecOrigin");
	NETVAR(Vector, m_vecAngles, "DT_BaseEntity", "m_vecAngles");
	NETVAR(bool, m_bShouldGlow, "DT_DynamicProp", "m_bShouldGlow");
	NETVAR(CHandle<C_BasePlayer>, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity");
	NETVAR(bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted");
	NETVAR(float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow");
	NETVAR(float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown");
	NETVAR(CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser");
	NETVAR(int32_t, m_nBombSite, "DT_PlantedC4", "m_nBombSite");

	const matrix3x4_t& m_rgflCoordinateFrame()
	{
		static auto _m_rgflCoordinateFrame = NetvarSys::Get().GetOffset("DT_BaseEntity", "m_CollisionGroup") - 0x30;
		return *(matrix3x4_t*)((uintptr_t)this + _m_rgflCoordinateFrame);
	}

	bool IsPlayer();
	bool IsLoot();
	bool IsWeapon();
	bool IsPlantedC4();
	const char* GetWeaponName();
	bool IsDefuseKit();
	//bool isSpotted();
};

class C_PlantedC4 : public C_BaseEntity
{
public:
	NETVAR(bool, m_bBombTicking, "DT_PlantedC4", "m_bBombTicking");
	NETVAR(bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused");
	NETVAR(float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow");
	NETVAR(float, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength");
	NETVAR(float, m_flDefuseLength, "DT_PlantedC4", "m_flDefuseLength");
	NETVAR(float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown");
	NETVAR(CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser");
};

class C_BaseAttributableItem : public C_BaseEntity
{
public:
	NETVAR(uint64_t, m_OriginalOwnerXuid, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidLow, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	NETVAR(int32_t, m_OriginalOwnerXuidHigh, "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
	NETVAR(int32_t, m_nFallbackStatTrak, "DT_BaseAttributableItem", "m_nFallbackStatTrak");
	NETVAR(int32_t, m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit");
	NETVAR(int32_t, m_nFallbackSeed, "DT_BaseAttributableItem", "m_nFallbackSeed");
	NETVAR(float_t, m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear");

	NETVAR(C_EconItemView, m_Item2, "DT_BaseAttributableItem", "m_Item");

	C_EconItemView& m_Item()
	{
		// Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
		// A real fix for this requires a rewrite of the netvar manager
		return *(C_EconItemView*)this;
	}
	void SetModelIndex(const int index)
	{
		return CallVFunction<void(__thiscall*)(C_BaseEntity*, int)>(this, 75)(this, index);
	}
	void SetGloveModelIndex(int modelIndex);

};

class C_BaseWeaponWorldModel : public C_BaseEntity
{
public:
	NETVAR(int32_t, m_nModelIndex, "DT_BaseWeaponWorldModel", "m_nModelIndex");
};

class C_BaseCombatWeapon : public C_BaseAttributableItem
{
public:
	NETVAR(float_t, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(float_t, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(int32_t, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1");
	NETVAR(int32_t, m_iPrimaryReserveAmmoCount, "DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
	NETVAR(float, GetFireReadyTime, "DT_WeaponCSBase", "m_flPostponeFireReadyTime");
	NETVAR(float, GetLastShotTime, "DT_WeaponCSBase", "m_fLastShotTime");
	NETVAR(int32_t, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2");
	NETVAR(float_t, m_flRecoilIndex, "DT_WeaponCSBase", "m_flRecoilIndex");
	NETVAR(int32_t, m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex");
	NETVAR(int32_t, m_iWorldModelIndex, "DT_BaseCombatWeapon", "m_iWorldModelIndex");
	OFFSET(short, m_iItemDefinitionIndex, 0x2FAA);
	NETVAR(int32_t, m_iWorldDroppedModelIndex, "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex");
	NETVAR(bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled");
	NETVAR(bool, m_bBurstMode, "DT_BaseCombatWeapon", "m_bBurstMode");
	NETVAR(float, ThrowStrength, "DT_BaseCSGrenade", "m_flThrowStrength");
	NETVAR(float_t, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime");
	NETVAR(float_t, m_flPostponeFireReadyTime, "DT_BaseCombatWeapon", "m_flPostponeFireReadyTime");
	NETVAR(CHandle<C_BaseWeaponWorldModel>, m_hWeaponWorldModel, "DT_BaseCombatWeapon", "m_hWeaponWorldModel");
	
	CCSWeaponInfo* GetCSWeaponData();
	bool HasBullets();
	bool CanFire();
	bool IsGrenade();
	bool IsKnife();
	bool IsReloading();
	bool IsRifle();
	bool IsPistol();
	bool IsSniper();
	bool IsGun();
	float GetInaccuracy();
	float GetSpread();
	void UpdateAccuracyPenalty();
	CUtlVector<IRefCounted*>& m_CustomMaterials();
	bool* m_bCustomMaterialInitialized();
	NETVAR(float, throw_time, "DT_BaseCSGrenade", "m_fThrowTime");
	NETVAR(float, get_accuracy_penalty, "DT_WeaponCSBase", "m_fAccuracyPenalty");
	NETVAR(float, get_recoil_index, "DT_WeaponCSBase", "m_flRecoilIndex");
	NETVAR(int32_t, get_zoom_level, "DT_WeaponCSBaseGun", "m_zoomLevel");
	NETVAR(float, throw_strength, "DT_BaseCSGrenade", "m_flThrowStrength");
	NETVAR(bool, pin_pulled, "DT_BaseCSGrenade", "m_bPinPulled");
	bool is_grenade() {
		return this->m_Item().m_iItemDefinitionIndex() >= 43 && this->m_Item().m_iItemDefinitionIndex() <= 48;
	}

	bool is_being_thrown() {
		if (this->is_grenade()) {
			if (!this->pin_pulled()) {
				float throwtime = this->throw_time();
				if ((throwtime > 0) && (throwtime < g_GlobalVars->curtime))
					return true;
			}
		}

		return false;
	}
};
#include <optional>

class BoneBitList;

class IKContext
{
public:
	static IKContext* CreateIKContext()
	{
		auto ik = reinterpret_cast<IKContext*>(g_pMemAlloc->Alloc(sizeof(IKContext)));
		Construct(ik);
		return ik;

	}

	static void Construct(IKContext* ik)
	{
		typedef void(__thiscall* Fn)(IKContext* player);
		static Fn fn = nullptr;
		if (!fn) fn = reinterpret_cast<Fn>(Utils::PatternScan(GetModuleHandleA("server.dll"), "53 8B D9 F6 C3"));
		fn(ik);
	}

	IKContext()
	{
		Construct(this);
	}



	void Init(const studiohdr_t* pStudioHdr, const QAngle& absAngles, const Vector& adjOrigin, const float curTime, int ikCounter, int boneMask)
	{
		typedef void(__thiscall* Fn)(IKContext* pIK, const studiohdr_t* pStudioHdr, const QAngle& absAngles, const Vector& adjOrigin, const float curTime, int ikCounter, int boneMask);
		static Fn fn = nullptr;
		if (!fn) fn = reinterpret_cast<Fn>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"),"55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D"));
		return fn(this, pStudioHdr, absAngles, adjOrigin, curTime, ikCounter, boneMask);
	}


	OFFSET(matrix3x4_t, m_rootxform, 0x1030);
	OFFSET(float, m_iFrameCounter, 0x1060);
	OFFSET(float, m_flTime, 0x1064);
	OFFSET(int, m_iBoneMask, 0x1068);

private:
	static constexpr auto size = 0x1070;
	char _pad[size];
}; //0x1070


class C_BasePlayer : public C_BaseEntity
{
public:
	static __forceinline C_BasePlayer* GetPlayerByUserId(int id)
	{
		return static_cast<C_BasePlayer*>(GetEntityByIndex(g_EngineClient->GetPlayerForUserID(id)));
	}
	static __forceinline C_BasePlayer* GetPlayerByIndex(int i)
	{
		return static_cast<C_BasePlayer*>(GetEntityByIndex(i));
	}

	static uint32_t* get_vtable()
	{
		static const auto table = reinterpret_cast<uint32_t>(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"),
			"55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C")) + 0x47;
		return *reinterpret_cast<uint32_t * *>(table);
	}

	std::optional<Vector> get_hitbox_position(int id, matrix3x4_t* bones) const;
	void pre_think();
	void think();
	NETVAR(bool, m_bHasDefuser, "DT_CSPlayer", "m_bHasDefuser");
	NETVAR(bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity");
	NETVAR(int32_t, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired");
	NETVAR(QAngle, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles[0]");
	NETVAR(Vector, get_mins, "DT_BaseEntity", "m_vecMins");
	
	NETVAR(Vector, get_maxs, "DT_BaseEntity", "m_vecMaxs");
	NETPROP(m_angEyeAnglesProp, "DT_CSPlayer", "m_angEyeAngles[0]");
	NETVAR(int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue");
	NETVAR(bool, m_bHasHeavyArmor, "DT_CSPlayer", "m_bHasHeavyArmor");
	NETVAR(bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet");
	NETVAR(bool, m_bIsDefusing, "DT_CSPlayer", "m_bIsDefusing");
	NETVAR(bool, m_bIsGrabbingHostage, "DT_CSPlayer", "m_bIsGrabbingHostage");
	
	NETVAR(CHandle<C_BasePlayer>, m_hGroundEntity, "DT_CSPlayer", "m_hGroundEntity");
	NETVAR(CHandle<C_BasePlayer> , m_hCarriedHostage, "DT_CSPlayer", "m_hCarriedHostage");
	NETVAR(bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped");
	char* GetArmorName()
	{
		if (this->m_ArmorValue() > 0)
		{
			if (this->m_bHasHelmet())
				return "HK";
			else
				return "K";
		}
		return "";
	}

	void SetLocalViewangles(Vector& angle) {
		//  (*(*player + 1468))(player, &cmd->viewangles);
		using fnSetLocalViewAngles = void(__thiscall*)(void*, Vector&);
		CallVFunction< fnSetLocalViewAngles >(this, 369)(this, angle);
	}

	NETVAR(float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget");
	NETVAR(float, m_flNextAttack, "DT_BaseCombatCharacter", "m_flNextAttack");
	NETVAR(int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth");
	NETVAR(int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState");
	NETVAR(int32_t, m_fFlags, "DT_BasePlayer", "m_fFlags");
	NETVAR(int32_t, m_nTickBase, "DT_BasePlayer", "m_nTickBase");
	NETVAR(Vector, m_vecViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]");
	NETVAR(QAngle, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle");
	NETVAR(QAngle, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle");
	NETVAR(Vector, get_punch_angle_vel, "DT_CSPlayer", "m_aimPunchAngleVel");
	NETVAR(CHandle<C_BaseViewModel>, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]");
	NETVAR(Vector, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]");
	NETVAR(float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed");
	NETVAR(CHandle<C_BasePlayer>, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget");
	NETVAR(float, m_flFlashMaxAlpha, "DT_CSPlayer", "m_flFlashMaxAlpha");
	NETVAR(int32_t, m_nHitboxSet, "DT_BaseAnimating", "m_nHitboxSet");
	NETVAR(CHandle<C_BaseCombatWeapon>, m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon");
	NETVAR(int32_t, m_iAccount, "DT_CSPlayer", "m_iAccount");
	NETVAR(float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration");
	NETVAR(float, m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime");
	NETVAR(float, m_flCycle, "DT_ServerAnimationData", "m_flCycle");
	NETVAR(int, m_nSequence, "DT_BaseViewModel", "m_nSequence");
	PNETVAR(char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName");
	NETPROP(m_flLowerBodyYawTargetProp, "DT_CSPlayer", "m_flLowerBodyYawTarget");

	void UpdateIKLocks(float currentTime)
	{
		typedef void(__thiscall* oUpdateIKLocks)(PVOID, float currentTime);
		CallVFunction<oUpdateIKLocks>(this, 191)(this, currentTime);
	}

	void CalculateIKLocks(float currentTime)
	{
		typedef void(__thiscall* oCalculateIKLocks)(PVOID, float currentTime);
		CallVFunction<oCalculateIKLocks>(this, 192)(this, currentTime);
	}

	void StandardBlendingRules(c_studio_hdr* hdr, Vector* pos, Quaternion* q, float_t curtime, int32_t boneMask);
	//void BuildTransformations(c_studio_hdr* hdr, Vector* pos, Quaternion* q, const matrix3x4a_t& cameraTransform, int32_t boneMask, byte* computed);

	bool HandleBoneSetup(matrix3x4_t* boneOut,int32_t boneMask, float_t curtime);

	void SetAbsOrigin(const Vector& origin);
	void SetAbsAngles(const QAngle& angles);

	NETVAR(int32_t, m_iFOV, "DT_BasePlayer", "m_iFOV");
	NETVAR(int32_t, m_iDefaultFOV, "DT_BasePlayer", "m_iDefaultFOV");

	bool IsEnemy() {
		return g_LocalPlayer->m_iTeamNum() != m_iTeamNum();
	}

	bool IsLocalPlayer() {
		return g_LocalPlayer->EntIndex() == EntIndex();
	}

	player_info_t get_info() const
	{
		player_info_t info;
		g_EngineClient->GetPlayerInfo(this->EntIndex(), &info);
		return info;
	}

	void run_pre_think();

	void run_think();

	CBoneAccessor* GetBoneAccessor()
	{
		return (CBoneAccessor*)((uintptr_t)this + 0x2924);
	}

	c_studio_hdr* GetModelPtr()
	{
		return *(c_studio_hdr**)((uintptr_t)this + 0x294C);
	}


	float& get_last_bone_setup_time() {
		return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0x2924);
	}
	c_studio_hdr* get_model_hdr() {
		return *reinterpret_cast<c_studio_hdr**>(reinterpret_cast<uintptr_t>(this) + 0x294C);
	}

	matrix3x4_t**& get_bone_cache() {
		return *reinterpret_cast<matrix3x4_t***>(reinterpret_cast<uintptr_t>(this) + 0x2910);
	}
	void invalidate_physics_recursive(int32_t flags) {
		static const auto invalidate_physics_recursive = reinterpret_cast<void(__thiscall*)(C_BaseEntity*, int32_t)>(
			Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56"));
		invalidate_physics_recursive(this, flags);
	}

	matrix3x4_t*& get_bone_array_for_write() {
		return *reinterpret_cast<matrix3x4_t**>(reinterpret_cast<uintptr_t>(this) + 0x26A8);
	}

	matrix3x4a_t* GetBoneArrayForWrite() {
		return (matrix3x4a_t*)((uintptr_t)this + 0x26A8);
	}

	uint32_t& get_readable_bones() {
		return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + 0x26AC);
	}
	uint32_t& get_writable_bones() {
		return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + 0x26B0);
	}


	int32_t& get_think_tick() {
		return *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + 0x40);
	}

	uint32_t& get_most_recent_model_bone_counter() {
		return *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + 0x2690);
	}

	float& m_flOldSimulationTime() {

		static int name = NetvarSys::Get().GetOffset("DT_BaseEntity", "m_flSimulationTime");    
        return *(float*)((uintptr_t)this + (name + 4));
	}

	void SetOrigin(Vector origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector & origin);
		static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

		SetAbsOrigin(this, origin);
	}


	void set_abs_angles(QAngle angle) {
		static auto set_abs_angles_fn = reinterpret_cast<void(__thiscall*)(C_BaseEntity*, const QAngle&)>(
			Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8"));
		set_abs_angles_fn(this, angle);
	}

	int C_BasePlayer::GetFOV() {
		if (m_iFOV() != 0)
			return m_iFOV();
		return m_iDefaultFOV();
	}

	//NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");
	Vector& GetAbsOrigin() {
		typedef Vector& (__thiscall * oGetAbsAngles)(PVOID);
		return CallVFunction < oGetAbsAngles >(this, 10)(this);
	}

	QAngle& GetAbsAngles() {
		typedef QAngle& (__thiscall * oGetAbsAngles)(PVOID);
		return CallVFunction < oGetAbsAngles >(this, 11)(this);
	}

	NETVAR(QAngle, m_angAbsAngles, "DT_BaseEntity", "m_angAbsAngles");
	NETVAR(Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin");
	NETVAR(float, m_flDuckSpeed, "DT_BaseEntity", "m_flDuckSpeed");
	NETVAR(float, m_flDuckAmount, "DT_BaseEntity", "m_flDuckAmount");
	std::array<float, 24> m_flPoseParameter() const {
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_BaseAnimating", "m_flPoseParameter");
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}


	PNETVAR(CHandle<C_BaseCombatWeapon>, m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons");
	PNETVAR(CHandle<C_BaseAttributableItem>, m_hMyWearables, "DT_BaseCombatCharacter", "m_hMyWearables");

	CUserCmd*& m_pCurrentCommand();

	/*gladiator v2*/
	void InvalidateBoneCache();
	int GetNumAnimOverlays();
	AnimationLayer* GetAnimOverlays();
	AnimationLayer* GetAnimOverlay(int i);
	
	CCSGOPlayerAnimState* GetPlayerAnimState();

	static void UpdateAnimationState(CCSGOPlayerAnimState* state, QAngle angle);
	static void ResetAnimationState(CCSGOPlayerAnimState* state);
	void CreateAnimationState(CCSGOPlayerAnimState* state);

	float_t& m_surfaceFriction()
	{
		static unsigned int _m_surfaceFriction = Utils::FindInDataMap(GetPredDescMap(), "m_surfaceFriction");
		return *(float_t*)((uintptr_t)this + _m_surfaceFriction);
	}
	Vector& m_vecBaseVelocity()
	{
		static unsigned int _m_vecBaseVelocity = Utils::FindInDataMap(GetPredDescMap(), "m_vecBaseVelocity");
		return *(Vector*)((uintptr_t)this + _m_vecBaseVelocity);
	}

	float_t& m_flMaxspeed()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap(GetPredDescMap(), "m_flMaxspeed");
		return *(float_t*)((uintptr_t)this + _m_flMaxspeed);
	}


	Vector& get_abs_velocity()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap(GetPredDescMap(), "m_vecAbsVelocity");
		return *(Vector*)((uintptr_t)this + _m_flMaxspeed);
	}

	int& get_effects()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap(GetPredDescMap(), "m_fEffects");
		return *(int*)((uintptr_t)this + _m_flMaxspeed);
	}

	int& get_eflags()
	{
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap(GetPredDescMap(), "m_iEFlags");
		return *(int*)((uintptr_t)this + _m_flMaxspeed);
	}

	IKContext* m_pIk()
	{
		return (IKContext*)((uintptr_t)this + 1040);
	}

	float* m_flEncodedController()
	{
		return (float*)((uintptr_t)this + 0x0A54);
	}

	

	Vector        GetEyePos();
	player_info_t GetPlayerInfo();
	bool          IsAlive();
	bool		  IsFlashed();
	bool          HasC4();
	Vector        GetHitboxPos(int hitbox_id);
	mstudiobbox_t* GetHitbox(int hitbox_id);
	bool          GetHitboxPos(int hitbox, Vector& output);
	Vector        GetBonePos(int bone);
	bool          CanSeePlayer(C_BasePlayer* player, int hitbox, bool smokecheck = false);
	bool          CanSeePlayer(C_BasePlayer* player, const Vector& pos, bool smokecheck = false);
	void UpdateClientSideAnimation();
	NETVAR(bool, m_bClientSideAnimation, "DT_BaseAnimating", "m_bClientSideAnimation");
	
	int m_nMoveType();
	QAngle& GetVAngles();
	float_t m_flSpawnTime();
	bool IsNotTarget();

};



class C_BaseViewModel : public C_BaseEntity
{
public:
	NETVAR(int32_t, m_nViewModelIndex, "DT_BaseViewModel", "m_nViewModelIndex");
	NETVAR(CHandle<C_BaseCombatWeapon>, m_hWeapon, "DT_BaseViewModel", "m_hWeapon");
	NETVAR(CHandle<C_BasePlayer>, m_hOwner, "DT_BaseViewModel", "m_hOwner");
	NETPROP(m_nSequence, "DT_BaseViewModel", "m_nSequence");
	void SendViewModelMatchingSequence(int sequence);
};

typedef std::array<float, 24> pose_paramater;

class AnimationLayer
{
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void* m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class CCSGOPlayerAnimState
{
public:
	void* pThis;
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
}; //Size=0x344

class DT_CSPlayerResource
{
public:
	PNETVAR(int32_t, m_nActiveCoinRank, "DT_CSPlayerResource", "m_nActiveCoinRank");
	PNETVAR(int32_t, m_nMusicID, "DT_CSPlayerResource", "m_nMusicID");
	PNETVAR(int32_t, m_nPersonaDataPublicLevel, "DT_CSPlayerResource", "m_nPersonaDataPublicLevel");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsLeader, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsTeacher, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher");
	PNETVAR(int32_t, m_nPersonaDataPublicCommendsFriendly, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly");
	PNETVAR(int32_t, m_iCompetitiveRanking, "DT_CSPlayerResource", "m_iCompetitiveRanking");
	PNETVAR(int32_t, m_iCompetitiveWins, "DT_CSPlayerResource", "m_iCompetitiveWins");
	PNETVAR(int32_t, m_iPlayerVIP, "DT_CSPlayerResource", "m_iPlayerVIP");
	PNETVAR(int32_t, m_iMVPs, "DT_CSPlayerResource", "m_iMVPs");
	PNETVAR(int32_t, m_iScore, "DT_CSPlayerResource", "m_iScore");
};

class CPlayerResource {};

class C_CSPlayerResource : public CPlayerResource
{
public:
	int GetPing(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iPing");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetKills(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iKills");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetAssists(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iAssists");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetDeaths(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iDeaths");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	bool GetConnected(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_bConnected");
		return *(bool*)((uintptr_t)this + _m_flPoseParameter + index);
	}

	int GetTeam(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iTeam");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetPendingTeam(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iPendingTeam");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	bool GetAlive(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_bAlive");
		return *(bool*)((uintptr_t)this + _m_flPoseParameter + index);
	}

	int GetHealth(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iHealth");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetPlayerC4()
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iPlayerC4");
		return *(int*)((uintptr_t)this + _m_flPoseParameter);
	}

	int GetMVPs(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iMVPs");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetArmor(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iArmor");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetScore(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iScore");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetCompetitiveRanking(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iCompetitiveRanking");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetCompetitiveWins(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iCompetitiveWins");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int GetCompTeammateColor(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_iCompTeammateColor");
		return *(int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	const char* GetClan(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_szClan");
		return (const char*)((uintptr_t)this + _m_flPoseParameter + index * 16);
	}

	int* GetActiveCoinRank(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_nActiveCoinRank");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetMusicID(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_nMusicID");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetPersonaDataPublicCommendsLeader(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetPersonaDataPublicCommendsTeacher(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}

	int* GetPersonaDataPublicCommendsFriendly(int index)
	{
		static int _m_flPoseParameter = NetvarSys::Get().GetOffset("DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly");
		return (int*)((uintptr_t)this + _m_flPoseParameter + index * 4);
	}
};