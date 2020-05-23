#include "aimbot.hpp"
#include "autowall.hpp"
#include "../helpers/math.hpp"
#include "../helpers/input.hpp"
float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}
//--------------------------------------------------------------------------------
void Aimbot::Cleanup() {
	shot_delay_time = 0;
	shot_delay = false;
	silent_enabled = false;
	CurrentPunch = { 0,0,0 };
	RCSLastPunch = { 0,0,0 };
	is_delayed = false;
	kill_delay_time = 0;
	kill_delay = false;
	target = NULL;
}
bool Aimbot::IsRcs() {
	return g_LocalPlayer->m_iShotsFired() >= settings.rcs_start;
}
//--------------------------------------------------------------------------------
float GetRealDistanceFOV(float distance, QAngle angle, QAngle viewangles) {
	Vector aimingAt;
	Math::AngleVectors(viewangles, aimingAt);
	aimingAt *= distance;
	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;
	return aimingAt.DistTo(aimAt) / 5;
}
//void FixMouseDeltas(CUserCmd* cmd, const QAngle& angle, const QAngle& oldAngle)
//{
//	QAngle delta = angle - oldAngle;
//	float sens = g_CVar->FindVar(("sensitivity"))->GetFloat();
//	float m_pitch = g_CVar->FindVar(("m_pitch"))->GetFloat();
//	float m_yaw = g_CVar->FindVar(("m_yaw"))->GetFloat();
//	float zoomMultiplier = g_CVar->FindVar("zoom_sensitivity_ratio_mouse")->GetFloat();
//
//	Math::FixAngles(delta);
//
//	cmd->mousedx = -delta.yaw / (m_yaw * sens * zoomMultiplier);
//	cmd->mousedy = delta.pitch / (m_pitch * sens * zoomMultiplier);
//}
//--------------------------------------------------------------------------------
float Aimbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle) {
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}
//--------------------------------------------------------------------------------
bool Aimbot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) {
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}
//--------------------------------------------------------------------------------
bool Aimbot::IsEnabled(CUserCmd* pCmd) {
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return false;
	}

	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon) {
		return false;
	}

	auto weaponData = pWeapon->GetCSWeaponData();

	if (!weaponData)
		return false;

	auto weapontype = weaponData->iWeaponType;
	settings = g_Options.aimbot[pWeapon->m_Item().m_iItemDefinitionIndex()];
	if (!settings.enabled) {
		return false;
	}

	if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) && settings.only_in_zoom && !g_LocalPlayer->m_bIsScoped()) {
		return false;
	}

	if (!pWeapon->HasBullets() || pWeapon->IsReloading()) {
		return false;
	}

	if (settings.antiaimlock && !pWeapon->CanFire())
		return false;

	return !settings.on_key || InputSys::Get().IsKeyDown(settings.key);

}
//--------------------------------------------------------------------------------
float Aimbot::GetSmooth() {
	float smooth = IsRcs() && settings.rcs_smooth_enabled ? settings.rcs_smooth : settings.smooth;
	return smooth;
}
//--------------------------------------------------------------------------------
void Aimbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle) {
	auto smooth_value = GetSmooth();
	if (smooth_value <= 1) {
		return;
	}

	QAngle delta = aimAngle - currentAngle;
	Math::FixAngles(delta);

	// ничего не трогать и не менять
	if (settings.smooth_type == 1) {
		float deltaLength = fmaxf(sqrtf((delta.pitch * delta.pitch) + (delta.yaw * delta.yaw)), 0.01f);
		delta *= (1.0f / deltaLength);

		RandomSeed(g_GlobalVars->tickcount);
		float randomize = RandomFloat(-0.1f, 0.1f);
		smooth_value = fminf((g_GlobalVars->interval_per_tick * 64.0f) / (randomize + smooth_value * 0.15f), deltaLength);
	}
	else {
		smooth_value = (g_GlobalVars->interval_per_tick * 64.0f) / smooth_value;
	}

	delta *= smooth_value;
	angle = currentAngle + delta;
	Math::FixAngles(angle);
}

void SmoothAngle(QAngle currentAngle, QAngle& aimAngle, QAngle angle, float factor)
{
	auto smooth_value = factor;
	if (smooth_value <= 1) {
		return;
	}

	QAngle delta = aimAngle - currentAngle;
	Math::FixAngles(delta);

	// ничего не трогать и не менять
	
	smooth_value = (g_GlobalVars->interval_per_tick * 64.0f) / smooth_value;
	

	delta *= smooth_value;
	angle = currentAngle + delta;
	Math::FixAngles(angle);
}
//--------------------------------------------------------------------------------
void Aimbot::RCS(QAngle curang,QAngle& angle, C_BasePlayer* target, bool should_run) {
	if (!settings.rcs) {
		RCSLastPunch.Init();
		return;
	}

	if (settings.rcs_x == 0 && settings.rcs_y == 0) {
		RCSLastPunch.Init();
		return;
	}

	QAngle punch = g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (weapon && weapon->m_flNextPrimaryAttack() > g_GlobalVars->curtime) {
		auto delta_angles = punch - RCSLastPunch;
		auto delta = weapon->m_flNextPrimaryAttack() - g_GlobalVars->curtime;
		if (delta >= g_GlobalVars->interval_per_tick)
			punch = RCSLastPunch + delta_angles / static_cast<float>(TIME_TO_TICKS(delta));
	}
	
	CurrentPunch = punch;
	if (settings.rcs_type == 0 && !should_run)
		punch -= { RCSLastPunch.pitch, RCSLastPunch.yaw, 0.f };
		
	RCSLastPunch = CurrentPunch;
	if (!IsRcs()) {
		return;
	}

	float pithcmult = settings.rcs_x;

	if (g_LocalPlayer->m_iShotsFired() < 4 + settings.rcs_start)
		pithcmult += 10;
	punch.pitch *= (pithcmult / 100.0f);
	punch.yaw *= (settings.rcs_y / 100.0f);
	
	//SmoothAngle(curang -= punch , curang, angle, 10);
	angle -= punch;
	Math::FixAngles(angle);
}
//--------------------------------------------------------------------------------
float Aimbot::GetFov() {
	if (IsRcs() && settings.rcs && settings.rcs_fov_enabled) return settings.rcs_fov;
	if (!silent_enabled) return settings.fov;
	return settings.silent_fov > settings.fov ? settings.silent_fov : settings.fov;
}
//--------------------------------------------------------------------------------
C_BasePlayer* Aimbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone) {
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();
	if (target && !kill_delay && settings.kill_delay > 0 && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
		kill_delay = true;
		kill_delay_time = (int)GetTickCount() + settings.kill_delay;
	}
	if (kill_delay) {
		if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
		else return NULL;
	}

	C_BasePlayer* player;
	target = NULL;
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	int health;
	float fov;
	float damage;
	float distance;
	int fromBone = settings.aim_type == 1 ? 0 : settings.hitbox;
	int toBone = settings.aim_type == 1 ? 7 : settings.hitbox;
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i) {
		damage = 0.f;
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		if (!settings.deathmatch && player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) {
			continue;
		}
		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			//Math::VectorAngles(eVecTarget - pVecTarget, ang);
			

			ang = Math::CalcAngle(g_LocalPlayer->GetEyePos(), eVecTarget);

			Math::FixAngles(ang);

			distance = g_LocalPlayer->GetEyePos().DistTo(eVecTarget);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(distance, cmd->viewangles + RCSLastPunch , ang);
			else
				fov = GetFovToPlayer(cmd->viewangles + RCSLastPunch, ang);

			if (fov > GetFov())
				continue;

			if (!g_LocalPlayer->CanSeePlayer(player, eVecTarget)) {

				if (!settings.autowall)
					continue;

				damage = g_AutoWall.CanHit(eVecTarget);
				if (damage < settings.min_damage)
					continue;

			}
			if ((settings.priority == 1 || settings.priority == 2) && damage == 0.f)
				damage = g_AutoWall.CanHit(eVecTarget);

			health = player->m_iHealth() - damage;
			if (settings.check_smoke && IsLineGoesThroughSmoke(pVecTarget, eVecTarget))
				continue;

			bool OnGround = (g_LocalPlayer->m_fFlags() & FL_ONGROUND);
			if (settings.check_jump && !OnGround)
				continue;

			if (settings.aim_type == 1 && bestBoneFov < fov) {
				continue;
			}
			bestBoneFov = fov;
			if (
				(settings.priority == 0 && bestFov > fov) ||
				(settings.priority == 1 && bestHealth > health) ||
				(settings.priority == 2 && bestDamage < damage) ||
				(settings.priority == 3 && distance < bestDistance)
				) {
				bestBone = bone;
				target = player;
				bestFov = fov;
				bestHealth = health;
				bestDamage = damage;
				bestDistance = distance;
			}
		}
	}
	return target;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsNotSilent(float fov) {
	return IsRcs() || !silent_enabled || (silent_enabled && fov > settings.silent_fov);
}
//--------------------------------------------------------------------------------

bool IsShooting() {

	return !g_Options.aimbot[g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex()].on_key || InputSys::Get().IsKeyDown(g_Options.aimbot[g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex()].key);
}

QAngle calculate_angle(const Vector& source, const Vector& destination, const QAngle& viewAngles) {
	Vector delta = source - destination;
	auto radians_to_degrees = [](float radians) { return radians * 180 / static_cast<float>(M_PI); };
	QAngle angles;
	angles.pitch = radians_to_degrees(atanf(delta.z / std::hypotf(delta.x, delta.y))) - viewAngles.pitch;
	angles.yaw = radians_to_degrees(atanf(delta.y / delta.x)) - viewAngles.yaw;
	angles.roll = 0.0f;

	if (delta.x >= 0.0)
		angles.yaw += 180.0f;

	angles.Normalize();
	return angles;
}
#include "backtrack.hpp"

void Aimbot::OnMove(CUserCmd* pCmd) 
{
	if (!IsEnabled(pCmd)) {
		if (g_LocalPlayer && g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && settings.enabled && settings.rcs_type == 0) {
			auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
			if (pWeapon && (pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsRifle())) {
				RCS(pCmd->viewangles, pCmd->viewangles, target, false);
				Math::FixAngles(pCmd->viewangles);
				g_EngineClient->SetViewAngles(&pCmd->viewangles);
			}
		}
		else {
			RCSLastPunch = { 0, 0, 0 };
		}
		is_delayed = false;
		shot_delay = false;
		kill_delay = false;
		silent_enabled = settings.silent && settings.silent_fov > 0 && (settings.silent_chance == 100 ?
			true : ((0.1f + (rand() % 101)) <= settings.silent_chance));
		target = NULL;
		return;
	}

	QAngle oldAngle;
	g_EngineClient->GetViewAngles(&oldAngle);

	RandomSeed(pCmd->command_number);

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	bool should_do_rcs = false;
	QAngle angles = pCmd->viewangles;
	QAngle current = angles;

	float fov = 180.f;
	if (!(settings.check_flash && g_LocalPlayer->IsFlashed())) {
		int bestBone = -1;
		if (GetClosestPlayer(pCmd, bestBone)) 
		{
			Vector eVecTarget;
			eVecTarget = target->GetHitboxPos(bestBone);
			angles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), eVecTarget);

			if (settings.smooth > 2 && settings.humanize) {
				float dist = Math::ClampYaw(angles.yaw - oldAngle.yaw);
				if (dist > 180.0f) dist = 360.0f - dist;
				eVecTarget += Vector(0, 0, settings.curviness * dist);

				angles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), eVecTarget);
			}
			
			Math::FixAngles(angles);

			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(g_LocalPlayer->GetEyePos().DistTo(eVecTarget), angles, pCmd->viewangles);
			else
				fov = GetFovToPlayer(pCmd->viewangles, angles);

			should_do_rcs = true;

			if (!settings.silent && !is_delayed && !shot_delay && settings.shot_delay > 0) {
				is_delayed = true;
				shot_delay = true;
				shot_delay_time = GetTickCount() + settings.shot_delay;
			}

			if (shot_delay && shot_delay_time <= GetTickCount()) {
				shot_delay = false;
			}

			if (shot_delay) {
				pCmd->buttons &= ~IN_ATTACK;
			}

			if (settings.autofire) {
				if (!settings.autofire_key || InputSys::Get().IsKeyDown(settings.autofire_key)) {
					if (!weapon_data->bFullAuto) {
						if (pCmd->command_number % 2 == 0) {
							pCmd->buttons &= ~IN_ATTACK;
						}
						else {
							pCmd->buttons |= IN_ATTACK;
						}
					}
					else {
						pCmd->buttons |= IN_ATTACK;
					}
				}
			}
		}
	}
	
	if (IsNotSilent(fov) && (should_do_rcs || settings.rcs_type == 0)) {
		RCS(current,angles, target, should_do_rcs);
	}
	if (target && IsNotSilent(fov)) {
		Smooth(current, angles, angles);
	}
	

	Math::FixAngles(angles);
	pCmd->viewangles = angles;

	if (IsNotSilent(fov)) {
		g_EngineClient->SetViewAngles(&angles);
		//FixMouseDeltas(pCmd, angles, oldAngle);
	}

	silent_enabled = false;
	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && settings.autopistol) {
		float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
		if (next_shot > 0) {
			pCmd->buttons &= ~IN_ATTACK;
		}
	}
}

Aimbot g_Aimbot;
