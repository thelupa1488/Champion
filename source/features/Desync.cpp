#include "../hooks/hooks.hpp"
#include "../helpers/math.hpp"
#include "../helpers/input.hpp"
#include "../config/options.hpp"
extern float next_lby;
extern float side;


int GetBestHeadAngle(float yaw)
{
	float Back, Right, Left;

	Vector src3D, dst3D, forward, right, up, src, dst;
	trace_t tr;
	Ray_t ray, ray2, ray3, ray4, ray5;
	CTraceFilter filter;

	QAngle engineViewAngles;


	engineViewAngles.pitch = 0;
	engineViewAngles.yaw = yaw;

	Math::AngleVectors(engineViewAngles, forward, right, up);

	filter.pSkip = g_LocalPlayer;
	src3D = g_LocalPlayer->GetEyePos();
	dst3D = src3D + (forward * 384);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	Back = (tr.endpos - tr.startpos).Length();

	ray2.Init(src3D + right * 35, dst3D + right * 35);

	g_EngineTrace->TraceRay(ray2, MASK_SHOT, &filter, &tr);

	Right = (tr.endpos - tr.startpos).Length();

	ray3.Init(src3D - right * 35, dst3D - right * 35);

	g_EngineTrace->TraceRay(ray3, MASK_SHOT, &filter, &tr);

	Left = (tr.endpos - tr.startpos).Length();

	static int result = 0;

	if (Left > Right)
	{
		result = -1;
	}
	else if (Right > Left)
	{
		result = 1;
	}

	return result;
}

float AngleDiff(float destAngle, float srcAngle) {
	float delta;

	delta = fmodf(destAngle - srcAngle, 360.0f);
	if (destAngle > srcAngle) {
		if (delta >= 180)
			delta -= 360;
	}
	else {
		if (delta <= -180)
			delta += 360;
	}
	return delta;
}

void updatelby(CCSGOPlayerAnimState* animstate)
{
	if (animstate->speed_2d > 0.1f || std::fabsf(animstate->flUpVelocity)) {
		next_lby = g_GlobalVars->curtime + 0.22f;
	}
	else if (g_GlobalVars->curtime > next_lby) {
		if (std::fabsf(AngleDiff(animstate->m_flGoalFeetYaw, animstate->m_flEyeYaw)) > 35.0f) {
			next_lby = g_GlobalVars->curtime + 1.1f;
		}
	}
}

void Desync(CUserCmd* cmd, bool& send_packet) {
	if (cmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_USE) ||
		g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP
		|| !g_LocalPlayer->IsAlive())
		return;
	if (*g_GameRules && (*g_GameRules)->IsFreezePeriod())
		return;
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
	if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && weapon->m_flNextPrimaryAttack() >= g_GlobalVars->curtime)
		return;

	auto weapon_data = weapon->GetCSWeaponData();

	if (weapon_data->iWeaponType == WEAPONTYPE_GRENADE) {
		if (!weapon->m_bPinPulled()) {
			float throwTime = weapon->m_fThrowTime();
			if (throwTime > 0.f)
				return;
		}

		if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2)) {
			if (weapon->m_fThrowTime() > 0.f)
				return;
		}
	}

	static bool broke_lby = false;
	extern QAngle vangle;
	auto sideauto = GetBestHeadAngle(vangle.yaw);

	if (!g_Options.misc_desync_ad) {
		if (InputSys::Get().WasKeyPressed(g_Options.misc_desync_bind)) {
			side = -side;
		}
	}
	else
		side = sideauto;

	if (g_Options.misc_desync == 1) {
		float minimal_move = g_LocalPlayer->m_fFlags() & IN_DUCK ? 3.0f : 1.0f;

		if (!send_packet) {
			cmd->viewangles.yaw += 120.f * side;
		}

		static bool flip = 1;
		flip = !flip;

		cmd->sidemove += flip ? minimal_move : -minimal_move;
	}
	else if (g_Options.misc_desync == 2) {
		if (next_lby >= g_GlobalVars->curtime) {
			if (!broke_lby && send_packet)
				return;

			broke_lby = false;
			send_packet = false;
			cmd->viewangles.yaw += 120.0f * side;
		}
		else {
			broke_lby = true;
			send_packet = false;
			cmd->viewangles.yaw += 120.0f * -side;
		}
	}
	else if (g_Options.misc_desync == 3)
	{
		static bool switchaa = false;
		switchaa = !switchaa;

		if (!send_packet)
			cmd->viewangles.yaw += switchaa ? 180.f : 0.f;
	}
	Math::FixAngles(cmd->viewangles);
	//MoveFix(cmd);
};
