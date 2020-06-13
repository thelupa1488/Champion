#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"

namespace Globals {
	CUserCmd* cmd = nullptr;
	bool bSendPacket = true;
	bool got_w2s_matrix = false;
	DWORD w2s_offset;
}



bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash ||
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{
	//index: 160
	//ref: "CNewParticleEffect::DrawModel"
	//sig: 8B 80 ? ? ? ? FF D0 84 C0 74 6F 8B 4D A4
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165)(this);
}

const char* C_BaseEntity::GetWeaponName()
{
	return CallVFunction<const char*(__thiscall*)(C_BaseEntity*)>(this, 385)(this);
}

bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}

bool C_BaseEntity::IsPlayer()
{
	return GetClientClass()->m_ClassID == ClassId_CCSPlayer;
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	/*static auto fnGetWpnData
		= reinterpret_cast<CCSWeaponInfo * (__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B")
			);
	return fnGetWpnData(this);*/

	return g_WeaponSystem->GetWpnData(this->m_iItemDefinitionIndex());

}

bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

float ServerTime(C_BasePlayer* owner) {

	static CUserCmd* lastcmd;
	static int fixtickbase;


	if (!lastcmd || lastcmd->hasbeenpredicted)
		fixtickbase = owner->m_nTickBase();
	else
		fixtickbase++;

	lastcmd = Globals::cmd;

	return fixtickbase * g_GlobalVars->interval_per_tick;
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;
	if (IsReloading() || m_iClip1() <= 0)
		return false;

	if (owner->m_flNextAttack() > ServerTime(owner))
		return false;

	return m_flNextPrimaryAttack() <= ServerTime(owner);
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 482)(this);
}



float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 452)(this);
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 483)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd * *)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x298C);
}

AnimationLayer* C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer * *)((DWORD)this + 0x2980);
}

void C_BasePlayer::SetAbsOrigin(const Vector& origin)
{
	using SetAbsOriginFn = void(__thiscall*)(void*, const Vector & origin);
	static SetAbsOriginFn SetAbsOrigin = (SetAbsOriginFn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");

	SetAbsOrigin(this, origin);
}

void C_BasePlayer::SetAbsAngles(const QAngle& angles)
{
	using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle & angles);
	static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

	SetAbsAngles(this, angles);
}


AnimationLayer* C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}

int C_BaseEntity::GetSequenceActivity(int sequence)
{
	auto hdr = g_MdlInfo->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	// sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
	// sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
	// c_csplayer vfunc 242, follow calls to find the function.

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(this, hdr, sequence);
}

void C_BasePlayer::pre_think()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 315)(this);
}

void C_BasePlayer::think()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 138)(this);
}

CCSGOPlayerAnimState* C_BasePlayer::GetPlayerAnimState()
{
	return *(CCSGOPlayerAnimState * *)((DWORD)this + 0x3900);
}


void C_BasePlayer::run_pre_think()
{
	static auto fn = reinterpret_cast<bool(__thiscall*)(void*, int)>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87"));

	if (fn(this, 0))
		pre_think();
}

void C_BasePlayer::run_think()
{
	static auto fn = reinterpret_cast<void(__thiscall*)(int)>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6"));

	if (get_think_tick() != -1 && get_think_tick() > 0
		&& get_think_tick() < TIME_TO_TICKS(g_GlobalVars->curtime))
	{
		get_think_tick() = -1;
		fn(0);
		think();
	}
}


void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState* state, QAngle angle)
{
	static auto UpdateAnimState = Utils::PatternScan(
		GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");

	if (!UpdateAnimState)
		return;

	__asm {
		push 0
	}

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState* state)
{
	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState* state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}

Vector C_BasePlayer::GetEyePos()
{
	return m_vecOrigin() + m_vecViewOffset();

	

	//Vector vec;
	//CallVFunction<void(__thiscall*)(void*, Vector&)>(this, 283)(this, vec);
	//return vec;

}

player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}

bool C_BasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}

bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

std::optional<Vector> C_BasePlayer::get_hitbox_position(int id, matrix3x4_t* bones) const
{
	auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());

	if (studio_model) {
		auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(id);

		if (hitbox)
		{
			auto
				min = Vector{},
				max = Vector{};

			Math::VectorTransform(hitbox->bbmin, bones[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, bones[hitbox->bone], max);

			return (min + max) / 2.0f;
		}
	}

	return std::nullopt;
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, m_flSimulationTime())) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}


mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector& output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t* model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return boneMatrix[bone].at(3);
	}
	return Vector{};
}
#include "../features/aimbot.hpp"
bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox, bool smokecheck)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	if (g_Aimbot.IsLineGoesThroughSmoke(g_LocalPlayer->GetEyePos(), endpos) && smokecheck)
		return false;

	ray.Init(g_LocalPlayer->GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos, bool smokecheck)
{
	if (g_Aimbot.IsLineGoesThroughSmoke(GetEyePos(), pos) && smokecheck)
		return false;
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);

	UINT mask = MASK_SHOT | CONTENTS_GRATE;

	mask &= ~(CONTENTS_WINDOW);

	g_EngineTrace->TraceRay(ray, mask, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 223)(this);
}

void C_BasePlayer::InvalidateBoneCache()
{
	static auto addr = Utils::PatternScan(GetModuleHandleA("client.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	*(int*)((uintptr_t)this + 0xA30) = g_GlobalVars->framecount; //we'll skip occlusion checks now
	*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;

}

int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle& C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return *(QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 244)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA360);
}

bool C_BasePlayer::IsNotTarget()
{
	return !this || IsDormant() || !IsPlayer() || this == g_LocalPlayer || !this->IsAlive() || this->m_bGunGameImmunity();
}
void C_BasePlayer::StandardBlendingRules(c_studio_hdr* hdr, Vector* pos, Quaternion* q, float_t curtime, int32_t boneMask)
{
	typedef void(__thiscall* o_StandardBlendingRules)(void*, c_studio_hdr*, Vector*, Quaternion*, float_t, int32_t);
	CallVFunction<o_StandardBlendingRules>(this, 205)(this, hdr, pos, q, curtime, boneMask);
}

#include "../features/animfixed.h"

void ClearTargets(DWORD m_pIk)
{
	//??? valve pls
	int max = *(int*)((DWORD)m_pIk + 4080);

	int v59 = 0;

	if (max > 0)
	{
		DWORD v60 = (DWORD)((DWORD)m_pIk + 208);
		do
		{
			*(int*)(v60) = -9999;
			v60 += 340;
			++v59;
		} while (v59 < max);
	}
}

bool C_BasePlayer::HandleBoneSetup(matrix3x4_t* boneOut,  int32_t boneMask, float_t curtime)
{
	auto hdr = this->GetModelPtr();
	if (!hdr)
		return false;

	CBoneAccessor* accessor = this->GetBoneAccessor();
	if (!accessor)
		return false;

	matrix3x4_t* backup_matrix = this->GetBoneArrayForWrite();
	if (!backup_matrix)
		return false;

	Vector origin = this->m_vecOrigin();
	QAngle angles = this->m_angEyeAngles();

	Vector backup_origin = this->GetAbsOrigin();
	QAngle backup_angles = this->GetAbsAngles();

	std::array<float_t, 24> backup_poses;
	backup_poses = this->m_flPoseParameter();

	AnimationLayer backup_layers[15];
	std::memcpy(backup_layers, this->GetAnimOverlays(), (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));

	matrix3x4a_t parentTransform;
	Math::AngleMatrix(backup_angles, backup_origin, parentTransform);

	get_effects() |= 0x008;

	auto m_pIK = *(DWORD*)((DWORD)this + 0x2670);

	if (m_pIK)
	{
		ClearTargets(m_pIK);

		typedef void(__thiscall* Initfn)(DWORD, c_studio_hdr*, const QAngle&, const Vector&, float, int, int);
		static auto Init = (Initfn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D");//sub_10714F40(*(entity + 9836), v107, v62, v61, v104, v60, boneMask);
		Init(m_pIK, hdr, angles, origin, curtime, g_GlobalVars->framecount, boneMask);
	}

	//auto& anim_data = Animation::Get().GetPlayerAnimationInfo(this->EntIndex());

	this->SetAbsOrigin(origin);
	this->SetAbsAngles(angles);
	//this->m_flPoseParameter() = anim_data.m_flPoseParameters;
	//std::memcpy(this->GetAnimOverlays(), anim_data.m_AnimationLayer, (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));

	Vector* pos = (Vector*)(g_pMemAlloc->Alloc(sizeof(Vector[128])));
	Quaternion* q = (Quaternion*)(g_pMemAlloc->Alloc(sizeof(Quaternion[128])));
	std::memset(pos, 0xFF, sizeof(pos));
	std::memset(q, 0xFF, sizeof(q));

	accessor->SetBoneArrayForWrite(boneOut);


	this->StandardBlendingRules(hdr, pos, q, curtime, boneMask);


	byte* computed = (byte*)(g_pMemAlloc->Alloc(sizeof(byte[0x20])));
	std::memset(computed, 0, sizeof(byte[0x20]));
	//byte computed[0x100] = { 0 };

	/*if (m_pIK)
	{
		
		//m_pIk()->UpdateTargets();

		typedef void(__thiscall* fn)(DWORD aids, Vector*, Quaternion*, void*, byte*);

		static fn UpdateTargets = (fn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F0 81 EC ? ? ? ? 33 D2");
		static fn SolveDependencies = (fn)Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81");

		UpdateIKLocks(curtime);

		UpdateTargets(m_pIK, pos, q, this->GetBoneArrayForWrite(), &computed[0]);

		CalculateIKLocks(curtime);

		SolveDependencies(m_pIK, pos, q, this->GetBoneArrayForWrite(), &computed[0]);
	}*/

	this->SetAbsOrigin(backup_origin);
	this->SetAbsAngles(backup_angles);
	this->m_flPoseParameter() = backup_poses;
	std::memcpy(this->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * this->GetNumAnimOverlays()));
	get_effects() &= ~0x008;
	GetBoneAccessor()->SetBoneArrayForWrite(backup_matrix);
	
	return true;
}