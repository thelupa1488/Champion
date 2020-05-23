#pragma once

#include "hooks.hpp"
#include "../features/AnimationSystem.h"
#include "../config/options.hpp"
#include <rpcndr.h>
class C_BasePlayer_
{
	typedef QAngle*(__thiscall* eye_angles_t)(C_BasePlayer*);
	typedef void(__thiscall* build_transformations_t)(C_BasePlayer*, c_studio_hdr*, Vector*, Quaternion*, const matrix3x4_t&, int32_t, BYTE*);
	typedef void(__thiscall* standard_blending_rules_t)(C_BasePlayer*, studiohdr_t*, Vector*, Quaternion*, float, int);	
	typedef void(__thiscall* accumulate_layers_t)(C_BasePlayer*, void**, Vector*, float, Quaternion*);
	typedef void(__thiscall* do_extra_bone_processing_t)(C_BasePlayer*, studiohdr_t*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
	typedef void(__thiscall* update_client_side_animation_t)(C_BasePlayer*);
	typedef void(__thiscall* isplayer_t)();
public:
    static void hook();

	static void apply_to_player(C_BasePlayer* player);

	static int __stdcall proxy_is_player(C_BasePlayer* player, void* return_address, void* eax);
private:
	inline static vfunc_hook hk;

	inline static eye_angles_t _eye_angles;
	inline static build_transformations_t _build_transformations;
	inline static standard_blending_rules_t _standard_blending_rules;
	inline static do_extra_bone_processing_t _do_extra_bone_processing;
	inline static update_client_side_animation_t _update_client_side_animation;
	inline static accumulate_layers_t _accumulate_layers;
	
	static void __fastcall hk_accumulate_layers(C_BasePlayer* player, uintptr_t /*edx*/, void** bone_setup, Vector* pos, float time, Quaternion* q);
	static QAngle* __fastcall eye_angles(C_BasePlayer* player, uint32_t);
	static void __fastcall build_transformations(C_BasePlayer* player, uint32_t, c_studio_hdr* hdr, Vector* pos,
		Quaternion* q, const matrix3x4_t& transform, int32_t mask, BYTE* computed);
	static void __fastcall standard_blending_rules(C_BasePlayer* player, uint32_t, studiohdr_t* hdr, Vector* pos,
		Quaternion* q, float time, int mask);
	static void __fastcall do_extra_bone_processing(C_BasePlayer* player, uint32_t, studiohdr_t* hdr, Vector* pos, Quaternion* q,
		const matrix3x4_t& matrix, uint8_t* bone_computed, void* context);
	static void __fastcall update_client_side_animation(C_BasePlayer* player, uint32_t);
};

static bool is_player(void* eax, void* edx);
