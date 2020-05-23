#pragma once
#include "hooks.hpp"
#include "../config/options.hpp"
#include "../helpers/math.hpp"
#include "../features/bhop.hpp"
#include "../features/aimbot.hpp"
#include "../helpers/input.hpp"
#include "../features/backtrack.hpp"
#include "../features/grenade_prediction.hpp"
#include "../features/visuals.hpp"
#include "../features/humanbhop.h"
#include "../helpers/xorstr.h"
static CCSGOPlayerAnimState g_AnimState;
static CBaseHandle* m_ulEntHandle;
class ClientMode : public Singleton<ClientMode>
{
public:
	static void Hook();
	
private:
	inline static vfunc_hook hook;
	static void __fastcall hkOverrideView(void* _this, int, CViewSetup* vsView);
	static int  __fastcall hkDoPostScreenEffects(void* _this, int, int a1);
	static bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd);
	typedef bool(__stdcall* CreateMove_t)(float, CUserCmd*);
	typedef int(__thiscall* DoPostScreenEffects_t)(void*, int, int);
	typedef void(__thiscall* OverrideView_t)(void*, CViewSetup*);
	inline static	CreateMove_t create_move_o;
	inline static	OverrideView_t override_view_o;
	inline static DoPostScreenEffects_t do_pose_effects_o;
};