#include "c_cs_player_.h"

#include <future>



void C_BasePlayer_::hook()
{
	//static uint32_t dummy[1] = { reinterpret_cast<uint32_t>(C_BasePlayer::get_vtable()) };
	hk.HookManual<isplayer_t>(C_BasePlayer::get_vtable(), 157, (isplayer_t)is_player);
	_eye_angles = hk.HookManual<eye_angles_t>(C_BasePlayer::get_vtable(),169, (eye_angles_t)eye_angles);
	_do_extra_bone_processing = hk.HookManual<do_extra_bone_processing_t>(C_BasePlayer::get_vtable(), 197, (do_extra_bone_processing_t)do_extra_bone_processing);
	_standard_blending_rules = hk.HookManual<standard_blending_rules_t>(C_BasePlayer::get_vtable(), 205, (standard_blending_rules_t)standard_blending_rules);
	_accumulate_layers = hk.HookManual<accumulate_layers_t>(C_BasePlayer::get_vtable(), 206, (accumulate_layers_t)hk_accumulate_layers);
}

void C_BasePlayer_::apply_to_player(C_BasePlayer* player)
{
	//hk->patch_pointer(reinterpret_cast<uintptr_t*>(player));
}
void __fastcall C_BasePlayer_::hk_accumulate_layers(C_BasePlayer* player, uintptr_t /*edx*/, void** bone_setup, Vector* pos, float time, Quaternion* q)
{
	if(!player)
		_accumulate_layers(player, bone_setup, pos, time, q);

	*reinterpret_cast<BYTE*> (player + 2600) &= ~0xA;
	_accumulate_layers(player, bone_setup, pos, time, q);
	*reinterpret_cast<BYTE*> (player + 2600) |= 0xA;
}

QAngle* C_BasePlayer_::eye_angles(C_BasePlayer* player, uint32_t)
{
	static auto return_to_fire_bullet = Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "8B 0D ? ? ? ? F3 0F 7E 00 8B 40 08 89 45 E4");
	static auto return_to_set_first_person_viewangles = Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "8B 5D 0C 8B 08 89 0B 8B 48 04 89 4B 04 B9");

	if (player == g_LocalPlayer
		&& g_EngineClient->IsInGame()
		&& _ReturnAddress() != return_to_fire_bullet
		&& _ReturnAddress() != return_to_set_first_person_viewangles
		&& g_Options.rage_enabled
		)
	{
		auto& angle = animation_system.local_animation.eye_angles;

		if (player->GetPlayerAnimState()
			&& player->GetPlayerAnimState()->m_bInHitGroundAnimation
			&& !animation_system.in_jump)
			angle.pitch = 0.f;

		return &angle;
	}

	return _eye_angles(player);
}

void __fastcall C_BasePlayer_::build_transformations(C_BasePlayer* player, uint32_t, c_studio_hdr* hdr, Vector* pos, Quaternion* q,
	const matrix3x4_t& transform, const int32_t mask, byte* computed)
{
	// backup bone flags.
	const auto backup_bone_flags = hdr->bone_flags;

	// stop procedural animations.
	for (auto i = 0; i < hdr->bone_flags.Count(); i++)
		hdr->bone_flags.Element(i) &= ~BONE_ALWAYS_PROCEDURAL;

	_build_transformations(player, hdr, pos, q, transform, mask, computed);

	// restore bone flags.
	hdr->bone_flags = backup_bone_flags;
}
void prepare_animation(C_BasePlayer* local);
int C_BasePlayer_::proxy_is_player(C_BasePlayer* player, void* return_address, void* eax)
{
	static const auto return_to_should_skip_animframe = Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "84 C0 75 02 5F C3 8B 0D");

	if (return_address != return_to_should_skip_animframe)
		return 2;

	auto local = g_LocalPlayer;

	//if (player == g_LocalPlayer && player->IsAlive())
	//	prepare_animation(player);

	if (!local || !local->IsAlive() || player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return 2;

	return !(player->get_effects() & 8);
}

void C_BasePlayer_::standard_blending_rules(C_BasePlayer* player, uint32_t, studiohdr_t* hdr, Vector* pos, Quaternion* q, const float time, int mask)
{
	if (player->m_iTeamNum() != g_LocalPlayer->m_iTeamNum() || player == g_LocalPlayer)
		mask = BONE_USED_BY_SERVER;

	if (player == g_LocalPlayer)
		mask |= BONE_USED_BY_BONE_MERGE;
	
	_standard_blending_rules(player, hdr, pos, q, time, mask);
}

void C_BasePlayer_::do_extra_bone_processing(C_BasePlayer* player, uint32_t, studiohdr_t* hdr, Vector* pos, Quaternion* q,
	const matrix3x4_t& matrix, uint8_t* bone_computed, void* context)
{	
	
	if (player->get_effects() & 8)
		return;

	const auto state = player->GetPlayerAnimState();

	if (!state)
		return _do_extra_bone_processing(player, hdr, pos, q, matrix, bone_computed, context);

	const auto val = reinterpret_cast<float*> (reinterpret_cast<uintptr_t> (state) + 292);
	const auto backup = *val;
	auto backup_onground = false;


	if (state)
	{
		backup_onground = state->m_bOnGround;
		state->m_bOnGround = false;

		if (g_LocalPlayer->m_vecVelocity().Length2D() < 0.1f)
			*val = 0.f;
	}

	_do_extra_bone_processing(player, hdr, pos, q, matrix, bone_computed, context);

	if (state)
	{
		*val = backup;
		state->m_bOnGround = backup_onground;
	}
}

void C_BasePlayer_::update_client_side_animation(C_BasePlayer* player, uint32_t)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !player ||  player != g_LocalPlayer)
		return _update_client_side_animation(player);

	if (animation_system.enable_bones)
		_update_client_side_animation(player);
}

// ReSharper disable once CppDeclaratorNeverUsed
static uint32_t is_player_retn_address = 0;

bool __declspec(naked) is_player(void* eax, void* edx)
{
	_asm
	{
		push eax

		mov eax, [esp + 4]
		mov is_player_retn_address, eax

		push is_player_retn_address
		push ecx
		call C_BasePlayer_::proxy_is_player

		cmp eax, 1
		je _retn1

		cmp eax, 0
		je _retn0

		mov al, 1
		retn

		_retn0:
			mov al, 0
			retn

		_retn1:
			pop eax
			mov eax, is_player_retn_address
			add eax, 0x6B
			push eax
			retn
	}
}
