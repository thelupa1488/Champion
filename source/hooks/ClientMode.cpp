#include "ClientMode.h"
void AutoBlock(CUserCmd* cmd);
matrix3x4_t fakelag[128];

bool sendpacket = false;

std::vector<std::string> msgs =
{
"Champion - Best Legit Cheat",
"Download Champion and will be happy!",
"Ur cheat missing, download Champion",
"YouGame.Biz | THE LUPA",
};

bool gitchance(float hitchance) {

	if (1 > hitchance)
		return true;

	float inaccuracy = g_LocalPlayer->m_hActiveWeapon()->GetInaccuracy();
	if (inaccuracy == 0) inaccuracy = 0.0000001;
	inaccuracy = 1 / inaccuracy;
	return inaccuracy > hitchance;
}
bool IsPlayerBehind(C_BasePlayer* player)
{
	Vector toTarget = (g_LocalPlayer->m_vecOrigin() - player->m_vecOrigin()).Normalized();
	Vector playerViewAngles;
	Math::AngleVectors(player->m_angEyeAngles(), playerViewAngles);
	if (toTarget.Dot(playerViewAngles) > -0.5f)
		return false;
	else
		return true;
}

void knifebot(CUserCmd* pCmd) {
	if (g_Options.kbot)
	{
		C_BaseCombatWeapon* pLocalWeapon = g_LocalPlayer->m_hActiveWeapon();
		if (pLocalWeapon && pLocalWeapon->GetCSWeaponData())
		{
			if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE)
			{

				for (int EntIndex = 1; EntIndex < g_EngineClient->GetMaxClients(); EntIndex++)
				{
					C_BasePlayer* Entity = (C_BasePlayer*)g_EntityList->GetClientEntity(EntIndex);

					if (!Entity)
						continue;

					if (!Entity->IsPlayer())
						continue;

					if (!Entity->IsAlive())
						continue;

					if (Entity == g_LocalPlayer)
						continue;

					float Distance = g_LocalPlayer->m_vecOrigin().DistTo(Entity->m_vecOrigin());

					if (Distance > 68)
						continue;

					if (Entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
						continue;

					Vector OrignWorld = Entity->m_vecOrigin();
					Vector OrignScreen;

					if (!Math::WorldToScreen(OrignWorld, OrignScreen))
						continue;

					static int cur_attack = 0;

					if (Distance > 60.f || Entity->m_iHealth() < 33)
						pCmd->buttons |= IN_ATTACK;
					else
					{
						if ((g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick) - pLocalWeapon->m_flNextPrimaryAttack() > 0)
						{
							if (!IsPlayerBehind(Entity)) {
								if (Entity->m_ArmorValue() > 0)
								{
									if (Entity->m_iHealth() > 61)
										pCmd->buttons |= IN_ATTACK;
									else
										pCmd->buttons |= IN_ATTACK2;
								}
								else
								{
									if (Entity->m_iHealth() < 33)
										pCmd->buttons |= IN_ATTACK;
									else
										pCmd->buttons |= IN_ATTACK2;
								}
							}
							else
								pCmd->buttons |= IN_ATTACK2;
						}

					}
				}
			}
		}
	}
}

QAngle vangle = QAngle();
float damage = 0.f;

#include <chrono>

void ChatSpammer() //ebat ELITNO suuuka
{
	if (g_Options.misc_chat_spammer == 1)
	{
		static DWORD g_dTimer = 0;
		if (GetTickCount() > g_dTimer + 850)
		{
			g_EngineClient->ExecuteClientCmd(u8"say ﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽﷽ ﷽﷽");
			g_dTimer = GetTickCount();
		}
	}

	if (g_Options.misc_chat_spammer == 2)
	{
		long curTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		static long timestamp = curTime;

		if ((curTime - timestamp) < 850)
			return;

		if (g_Options.misc_chat_spammer)
		{
			if (msgs.empty())
				return;

			std::srand(time(NULL));

			std::string str;

			std::rotate(msgs.begin(), msgs.begin() + 1, msgs.end());

			str.append("say ");
			str.append(msgs[0].data());

			g_EngineClient->ExecuteClientCmd(str.c_str());
		}
		timestamp = curTime;
	}

}

bool didSwitch = false;

void AutoWeapons(CUserCmd* cmd)
{
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) return;

	if (weapon->m_iClip1() <= 0 && !weapon->IsKnife() && cmd->buttons & IN_ATTACK)
	{
		auto type = g_LocalPlayer->m_hActiveWeapon();

		if ((weapon->m_iItemDefinitionIndex() == WEAPONTYPE_MACHINEGUN || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_RIFLE || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SHOTGUN ||
			weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SNIPER_RIFLE || weapon->m_iItemDefinitionIndex() == WEAPONTYPE_SUBMACHINEGUN) && !didSwitch) {
			g_EngineClient->ExecuteClientCmd("slot2");
			didSwitch = true;
		}
	}
	else
		didSwitch = false;
}

void SlideWalk(CUserCmd* cmd)
{
	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
		return;

	if (cmd->forwardmove > 0)
	{
		cmd->buttons |= IN_BACK;
		cmd->buttons &= ~IN_FORWARD;
	}

	if (cmd->forwardmove < 0)
	{
		cmd->buttons |= IN_FORWARD;
		cmd->buttons &= ~IN_BACK;
	}

	if (cmd->sidemove < 0)
	{
		cmd->buttons |= IN_MOVERIGHT;
		cmd->buttons &= ~IN_MOVELEFT;
	}

	if (cmd->sidemove > 0)
	{
		cmd->buttons |= IN_MOVELEFT;
		cmd->buttons &= ~IN_MOVERIGHT;
	}
}
void Desync(CUserCmd* cmd, bool& send_packet);

float anglereal = 0.f;
float anglefake = 0.f;
void fix_movement(CUserCmd* cmd, QAngle& wishangle)
{
	Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	Math::AngleVectors(wishangle, view_fwd, view_right, view_up);
	Math::AngleVectors(cmd->viewangles, cmd_fwd, cmd_right, cmd_up);

	const auto v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	const auto v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	const auto v12 = sqrtf(view_up.z * view_up.z);

	const Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	const Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	const Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	const auto v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	const auto v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	const auto v18 = sqrtf(cmd_up.z * cmd_up.z);

	const Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	const Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	const Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	const auto v22 = norm_view_fwd.x * cmd->forwardmove;
	const auto v26 = norm_view_fwd.y * cmd->forwardmove;
	const auto v28 = norm_view_fwd.z * cmd->forwardmove;
	const auto v24 = norm_view_right.x * cmd->sidemove;
	const auto v23 = norm_view_right.y * cmd->sidemove;
	const auto v25 = norm_view_right.z * cmd->sidemove;
	const auto v30 = norm_view_up.x * cmd->upmove;
	const auto v27 = norm_view_up.z * cmd->upmove;
	const auto v29 = norm_view_up.y * cmd->upmove;

	cmd->forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
		+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
		+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	cmd->sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
		+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
		+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
	cmd->upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
		+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
		+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

	const auto ratio = 2.f - fmaxf(fabsf(cmd->sidemove), fabsf(cmd->forwardmove)) / 450.f;
	cmd->forwardmove *= ratio;
	cmd->sidemove *= ratio;

	wishangle = cmd->viewangles;
}


float autozeus(CUserCmd* cmd)
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon)
		return 0;
	const auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return 0;

	if (!(weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER))
		return 0;

	Ray_t ray;
	CGameTrace tr;
	CTraceFilterWorldAndPropsOnly filter;

	for (int i = 1; i < g_GlobalVars->maxClients; i++) {
		auto player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!player || !player->IsAlive() || player->IsDormant() || player == g_LocalPlayer || player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			continue;
		auto hitbox = player->GetHitboxPos(HITBOX_CHEST);

		const auto distance = g_LocalPlayer->GetEyePos().DistTo(hitbox);
		if (distance > weapon_data->flRange)
			continue;
		damage = weapon_data->iDamage * pow(weapon_data->flRangeModifier, distance / 500);
		if (damage < 120.f && damage < player->m_iHealth())
			continue;
		return damage;
	}
}

bool is_delayed = false;
float shot_delay_time;
bool shot_delay = false;
#include "../features/autowall.hpp"
void TriggerBot(CUserCmd* usercmd) {
	//if (!(g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER))
	if (g_Options.onkey && !GetAsyncKeyState(g_Options.trigbind)) {

		is_delayed = false;
		shot_delay = false;
		return;
	}
		
	std::vector<int> hitgroups;
	if (!(g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)) {
		if (g_Options.trighead)
			hitgroups.push_back(HITGROUP_HEAD);
		if (g_Options.trigchest)
			hitgroups.push_back(HITGROUP_CHEST);
		if (g_Options.trigstomach)
			hitgroups.push_back(HITGROUP_STOMACH);
		if (g_Options.trigleftarm)
			hitgroups.push_back(HITGROUP_LEFTARM);
		if (g_Options.trigrightarm)
			hitgroups.push_back(HITGROUP_RIGHTARM);
		if (g_Options.trigleftleg)
			hitgroups.push_back(HITGROUP_LEFTLEG);
		if (g_Options.trigrightleg)
			hitgroups.push_back(HITGROUP_RIGHTLEG);
	}
	else {
		hitgroups.push_back(HITGROUP_CHEST);
		hitgroups.push_back(HITGROUP_STOMACH);
	}
	Vector rem, forward,
		src = g_LocalPlayer->GetEyePos();

	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;

	QAngle viewangles = usercmd->viewangles;

	auto settings = g_Options.aimbot[g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex()];

	QAngle rec;

	//rec.pitch = g_LocalPlayer->m_aimPunchAngle().pitch * (settings.rcs_x / 2) / 50;

	//viewangles += g_LocalPlayer->m_aimPunchAngle() * ((settings.rcs_x + settings.rcs_y) / 2) / 50;

	Math::AngleVectors(viewangles, forward);

	forward *= g_LocalPlayer->m_hActiveWeapon().Get()->GetCSWeaponData()->flRange;

	rem = src + forward;

	ray.Init(src, rem);

	UINT mask = MASK_SHOT | CONTENTS_GRATE;

	mask &= ~(CONTENTS_WINDOW);

	g_EngineTrace->TraceRay(ray, mask, &filter, &tr);

	if (!tr.hit_entity)
		return;
	bool dh = false;
	for (auto& hitgroupsorted : hitgroups) {
		if (tr.hitgroup == hitgroupsorted)
			dh = true;
	}
	auto player = reinterpret_cast<C_BasePlayer*>(tr.hit_entity);
	if (!player || player->IsDormant() || !player->IsAlive() || !player->IsPlayer())
		return;

	if (player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return;

	if (g_Aimbot.IsLineGoesThroughSmoke(g_LocalPlayer->GetEyePos(), tr.endpos) && g_Options.trigchecksmoke)
		return;
	if (g_LocalPlayer->IsFlashed() && g_Options.trigcheckflash)
		return;

	if (dh && gitchance(g_Options.trighitchance))
	{
		if (!is_delayed && !shot_delay && g_Options.trigdelay > 0) {
			is_delayed = true;
			shot_delay = true;
			shot_delay_time = GetTickCount() + g_Options.trigdelay;
		}

		if (shot_delay && shot_delay_time <= GetTickCount()) {
			shot_delay = false;
		}

		if (!shot_delay) 
		{
			is_delayed = false;
			shot_delay = false;
			usercmd->buttons |= IN_ATTACK;
		}
	}
	else
	{
		shot_delay_time = 0.f;
		is_delayed = false;
		shot_delay = false;
	}
}

int max_choke_ticks() {

	int maxticks = (*g_GameRules && (*g_GameRules)->IsValveDS()) ? 11 : 15;
	static int max_choke_ticks = 0;
	static int latency_ticks = 0;
	float fl_latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
	int latency = TIME_TO_TICKS(fl_latency);;
	if (g_ClientState->chokedcommands <= 0)
		latency_ticks = latency;
	else latency_ticks = std::max(latency, latency_ticks);

	if (fl_latency >= g_GlobalVars->interval_per_tick)
		max_choke_ticks = 11 - latency_ticks;
	else max_choke_ticks = 11;
	return max_choke_ticks;
}
void updatelby(CCSGOPlayerAnimState* animstate);

float GetFullLatency() {
	return g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) + g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
}
void AutoRunBoost(CUserCmd* cmd)
{
	if (g_Options.misc_arb && GetAsyncKeyState(g_Options.misc_arb_bind)) { 
		for (int i = 1; i <= g_EngineClient->GetMaxClients(); ++i)
		{
			// Bla bla bla validity checks.
			C_BaseEntity* pEntity;
			C_BasePlayer* pLocal; 
			const auto pLocalEntity = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
			pEntity = (C_BaseEntity*)g_EntityList->GetClientEntity(i);
			if (!pEntity
				|| !pLocal->IsAlive()
				|| pEntity->IsDormant())
				continue;

			if (pEntity == pLocalEntity)
				continue;

			C_BaseEntity* GroundEntity = (C_BaseEntity*)g_EntityList->GetClientEntityFromHandle(pLocalEntity->m_hGroundEntity());

			// Check if there's a player under us.
			if (GroundEntity && GroundEntity == pEntity)
			{
				// Get the target's speed.
				auto Velocity = pLocal->m_vecVelocity();
				const auto Speed = Velocity.Length2D();

				if (Speed > 0.0f)
				{
					// Get the angles direction based on the target's speed.
					Vector Direction;
					QAngle Directions;
					Math::VectorAngles(Velocity, Directions);

					QAngle* ViewAngles;
					Vector ViewAngle;
					g_EngineClient->GetViewAngles(ViewAngles);

					// Cut down on our viewangles.
					Direction.y = ViewAngle.y - Direction.y;

					// Transform into vector again.
					Vector Forward;
					Math::VectorAngles(Forward, Directions);

					// Calculate the new direction based on the target's speed.
					Vector NewDirection = Forward * Speed;

					// Move accordingly.
					cmd->forwardmove = NewDirection.x;
					cmd->sidemove = NewDirection.y;
				}
			}
		}
	}
}
void Clantag() {

	static bool restore = false;
	static float lastchangetime = 0.0;
	if (g_Options.misc_clantag && g_LocalPlayer) {

		if (g_GlobalVars->curtime + (GetFullLatency() / 2) - lastchangetime >= 0.35f)
		{
			
			std::string Name;

			if (g_Options.misc_clantag_custom && g_Options.misc_clantag_name && g_Options.misc_clantag_name[0] && g_Options.misc_clantag_animated)
				Name = g_Options.misc_clantag_name;
			else
				if(!g_Options.misc_clantag_custom)
					Name = _xor_("Champion ").c_str();
				else
					Name = _xor_(" ").c_str();

			Utils::SetClantag(Name.substr(0, int(g_GlobalVars->curtime + (GetFullLatency() / 2)) % Name.length()).c_str());
			lastchangetime = (g_GlobalVars->curtime + (GetFullLatency() / 2));
			restore = true;
		}
	}
	else if (restore) {
		restore = false;
		Utils::SetClantag("");
	}

}
#define Square(x) ((x)*(x))
#define FastSqrt(x)	(sqrt)(x)

void MinWalk(CUserCmd* get_cmd, float get_speed) {

	if (!GetAsyncKeyState(g_Options.misc_minwalk_bind))
		return;

	if (g_LocalPlayer->m_vecVelocity().Length() > (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->flMaxPlayerSpeed / (g_LocalPlayer->m_hActiveWeapon()->IsSniper() ? 3.65f : 2.f)))
	{
		get_cmd->buttons |= IN_WALK;
		get_cmd->forwardmove = 0;
		get_cmd->sidemove = 0;
		get_cmd->upmove = 0;
	}
}

void Fakelag(CUserCmd* cmd, bool& send_packet) {
	if (!g_Options.fakelag_enabled)
		return;
	
	if (g_Options.fakelag_unducking &&
		g_LocalPlayer->m_flDuckAmount() > 0.05f && g_LocalPlayer->m_flDuckAmount() < 0.95f) {
		send_packet = !(g_ClientState->chokedcommands < max_choke_ticks());
		return;
	}

	if (g_Options.fakelag_factor <= 0)
		return;

	int choke_factor = g_Options.misc_desync ? std::min(max_choke_ticks(), g_Options.fakelag_factor) : g_Options.fakelag_factor;

	auto LegitPeek = [choke_factor](CUserCmd* cmd, bool& send_packet) {
		static bool m_bIsPeeking = false;
		if (m_bIsPeeking) {
			send_packet = !(g_ClientState->chokedcommands < choke_factor);
			if (send_packet)
				m_bIsPeeking = false;
			return;
		}

		auto speed = g_LocalPlayer->m_vecVelocity().Length();
		if (speed <= 70.0f)
			return;

		auto collidable = g_LocalPlayer->GetCollideable();

		Vector min, max;
		min = collidable->OBBMins();
		max = collidable->OBBMaxs();

		min += g_LocalPlayer->m_vecOrigin();
		max += g_LocalPlayer->m_vecOrigin();

		Vector center = (min + max) * 0.5f;

		for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
			auto player = C_BasePlayer::GetPlayerByIndex(i);
			if (!player || !player->IsAlive() || player->IsDormant())
				continue;
			if (player == g_LocalPlayer || g_LocalPlayer->m_iTeamNum() == player->m_iTeamNum())
				continue;

			auto weapon = player->m_hActiveWeapon().Get();
			if (!weapon || weapon->m_iClip1() <= 0)
				continue;

			auto weapon_data = weapon->GetCSWeaponData();
			if (!weapon_data || weapon_data->iWeaponType <= WEAPONTYPE_KNIFE || weapon_data->iWeaponType >= WEAPONTYPE_C4)
				continue;

			auto eye_pos = player->GetEyePos();

			Vector direction;
			Math::AngleVectors(player->m_angEyeAngles(), direction);
			direction.NormalizeInPlace();

			Vector hit_point;
			bool hit = Math::IntersectionBoundingBox(eye_pos, direction, min, max, &hit_point);
			if (hit && eye_pos.DistTo(hit_point) <= weapon_data->flRange) {
				Ray_t ray;
				trace_t tr;
				CTraceFilterSkipEntity filter((C_BasePlayer*)player);
				ray.Init(eye_pos, hit_point);

				g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);
				if (tr.contents & CONTENTS_WINDOW) { // skip windows
																							// at this moment, we dont care about local player
					filter.pSkip = tr.hit_entity;
					ray.Init(tr.endpos, hit_point);
					g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);
				}

				if (tr.fraction == 1.0f || tr.hit_entity == g_LocalPlayer) {
					m_bIsPeeking = true;
					break;
				}
			}
		}
	};

	auto speed = g_LocalPlayer->m_vecVelocity().Length();
	bool standing = speed <= 1.0f;
	if (!g_Options.fakelag_standing && standing) {
		return;
	}

	if (!g_Options.fakelag_moving && !standing) {
		return;
	}

	enum FakelagMode {
		FakelagStatic = 0,
		FakelagSwitch,
		FakelagAdaptive,
		FakelagRandom,
		FakelagLegitPeek
	};

	float UnitsPerTick = 0.0f;

	int WishTicks = 0;
	int AdaptiveTicks = 2;
	static int LastRandomNumber = 5;
	static int randomSeed = 12345;

	switch (g_Options.fakelag_mode)
	{
	case FakelagSwitch:
		// apply same logic as static fakelag
		if (cmd->command_number % 30 > 15)
			break;
	case FakelagStatic:
		send_packet = !(g_ClientState->chokedcommands < choke_factor);
		break;
	case FakelagAdaptive:
		if (standing) {
			send_packet = !(g_ClientState->chokedcommands < choke_factor);
			break;
		}

		UnitsPerTick = g_LocalPlayer->m_vecVelocity().Length() * g_GlobalVars->interval_per_tick;
		while ((WishTicks * UnitsPerTick) <= 68.0f)
		{
			if (((AdaptiveTicks - 1) * UnitsPerTick) > 68.0f)
			{
				++WishTicks;
				break;
			}
			if ((AdaptiveTicks * UnitsPerTick) > 68.0f)
			{
				WishTicks += 2;
				break;
			}
			if (((AdaptiveTicks + 1) * UnitsPerTick) > 68.0f)
			{
				WishTicks += 3;
				break;
			}
			if (((AdaptiveTicks + 2) * UnitsPerTick) > 68.0f)
			{
				WishTicks += 4;
				break;
			}
			AdaptiveTicks += 5;
			WishTicks += 5;
			if (AdaptiveTicks > 16)
				break;
		}

		send_packet = !(g_ClientState->chokedcommands < WishTicks);
		break;
	case FakelagRandom:
		if (g_ClientState->chokedcommands < LastRandomNumber) {
			send_packet = false;
		}
		else {
			randomSeed = 0x41C64E6D * randomSeed + 12345;
			LastRandomNumber = (randomSeed / 0x10000 & 0x7FFFu) % choke_factor;
			send_packet = true;
		}
		break;
	case FakelagLegitPeek:
		LegitPeek(cmd, send_packet);
		break;
	}

	if (choke_factor < g_ClientState->chokedcommands)
		send_packet = true;

};

#include "../features/prediction.h"

/*void fakeduck(CUserCmd* pCmd, bool& bSendPacket) {
	int fakelag_limit = 14;
	int choked_goal = fakelag_limit / 2;

	

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
	{
		bSendPacket = g_ClientState->m_NetChannel->m_nChokedPackets >= fakelag_limit;

		if (g_ClientState->chokedcommands <= 7)
			pCmd->buttons &= ~IN_DUCK;
		else
			pCmd->buttons |= IN_DUCK;
	}
}*/

void CorrectMouse(CUserCmd* cmd) {
	static ConVar* m_yaw = m_yaw = g_CVar->FindVar("m_yaw");
	static ConVar* m_pitch = m_pitch = g_CVar->FindVar("m_pitch");
	static ConVar* sensitivity = sensitivity = g_CVar->FindVar("sensitivity");

	static QAngle m_angOldViewangles = g_ClientState->viewangles;

	float delta_x = std::remainderf(cmd->viewangles.pitch - m_angOldViewangles.pitch, 360.0f);
	float delta_y = std::remainderf(cmd->viewangles.yaw - m_angOldViewangles.yaw, 360.0f);

	if (delta_x != 0.0f) {
		float mouse_y = -((delta_x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		short mousedy;
		if (mouse_y <= 32767.0f) {
			if (mouse_y >= -32768.0f) {
				if (mouse_y >= 1.0f || mouse_y < 0.0f) {
					if (mouse_y <= -1.0f || mouse_y > 0.0f)
						mousedy = static_cast<short>(mouse_y);
					else
						mousedy = -1;
				}
				else {
					mousedy = 1;
				}
			}
			else {
				mousedy = 0x8000u;
			}
		}
		else {
			mousedy = 0x7FFF;
		}

		cmd->mousedy = mousedy;
	}

	if (delta_y != 0.0f) {
		float mouse_x = -((delta_y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		short mousedx;
		if (mouse_x <= 32767.0f) {
			if (mouse_x >= -32768.0f) {
				if (mouse_x >= 1.0f || mouse_x < 0.0f) {
					if (mouse_x <= -1.0f || mouse_x > 0.0f)
						mousedx = static_cast<short>(mouse_x);
					else
						mousedx = -1;
				}
				else {
					mousedx = 1;
				}
			}
			else {
				mousedx = 0x8000u;
			}
		}
		else {
			mousedx = 0x7FFF;
		}

		cmd->mousedx = mousedx;
	}
}

void Slowwalk(CUserCmd* cmd) {
	if (g_Options.misc_minwalk)
	{
		if (g_LocalPlayer->m_hActiveWeapon()) {
			if (g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08)
				MinWalk(cmd, 70);
			else if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_RIFLE || g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_PISTOL)
				MinWalk(cmd, 67);
			else if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->iWeaponType == WEAPONTYPE_SUBMACHINEGUN)
				MinWalk(cmd, 76);
			else if (g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP)
				MinWalk(cmd, 30);
			else
				MinWalk(cmd, 36);
		}
	}
}
static DWORD delay = 0;
void NameSteal()
{
	if (g_Options.namestealer) 
	{
		if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
		{
			const auto pLocalEntity = C_BasePlayer::GetPlayerByIndex(g_EngineClient->GetLocalPlayer());
			static auto start_t = clock();
			const auto timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
			if (timeSoFar < .5)
				return;
			std::vector<std::string> allName;

			if (pLocalEntity)
			{
				for (auto i = 1; i < g_EngineClient->GetMaxClients(); i++)
				{
					const auto pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
					if (!pBaseEntity) continue;
					player_info_t pInfo;
					g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
					if (pInfo.ishltv) continue;
					allName.emplace_back(pInfo.szName);
				}

				const int random_index = rand() % allName.size();
				char buffer[128];
				sprintf_s(buffer, "%s ", allName[random_index].c_str());
				const char* result = buffer;


				Utils::SetName(result);
				start_t = clock();

				if (GetTickCount() - delay > 4000)
				{
					delay = GetTickCount();
					Utils::SetName("\n\xAD\xAD\xAD");
				}
			}
		}
	}
}
bool changeName(bool reconnect, const char* newName, float delay) noexcept
{
	static auto exploitInitialized{ false };

	static auto name{ g_CVar->FindVar("name") };

	if (reconnect) 
	{
		exploitInitialized = false;
		return false;
	}

	if (!exploitInitialized && g_EngineClient->IsInGame()) 
	{
		if (player_info_s playerInfo; g_EngineClient->GetPlayerInfo(g_LocalPlayer->EntIndex(), &playerInfo) && (!strcmp(playerInfo.szName, "?empty") || !strcmp(playerInfo.szName, "\n\xAD\xAD\xAD"))) {
			exploitInitialized = true;
		}
		else 
		{
			name->m_fnChangeCallbacks.m_Size = 0;
			name->SetValue("\n\xAD\xAD\xAD");
			return false;
		}
	}

	static auto nextChangeTime{ 0.0f };
	if (nextChangeTime <= g_GlobalVars->realtime) {
		name->SetValue(newName);
		nextChangeTime = g_GlobalVars->realtime + delay;
		return true;
	}
	return false;
}

void nosmoke()
{
	static bool set = true;

	static std::vector<const char*> vistasmoke_wireframe =
	{
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	if (!g_Options.nosmoke)
	{
		if (set)
		{
			for (auto material_name : vistasmoke_wireframe)
			{
				IMaterial* mat = g_MatSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);
				mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			}
			set = false;
		}
		return;
	}

	set = true;
	for (auto material_name : vistasmoke_wireframe)
	{
		IMaterial* mat = g_MatSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER);
		mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
	}
	//static auto shit = *(DWORD*)(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0") + 0x8);
	//if(shit)
	//	if (g_Options.nosmoke)
	//		*(int*)(shit) = 0;
}

bool __stdcall Hooks::hkCreateMove(float input_sample_frametime, CUserCmd* cmd)
{
	static auto original = clientmode_hook.get_original<decltype(&hkCreateMove)>(index::CreateMove);
	clientmode_hook.hook_index(index::ClientModeCreateMove, hkCreateMove);
	original(input_sample_frametime, cmd);
	bool* send_packet = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(_AddressOfReturnAddress()) + 0x14);
	auto oCreateMove = clientmode_hook.get_original< CreateMoveClientMode >(index::ClientModeCreateMove);
	bool result = oCreateMove(g_ClientMode, input_sample_frametime, cmd);

	uintptr_t* FPointer; __asm { MOV FPointer, EBP }
	byte* SendPacket = (byte*)(*FPointer - 0x1C);

	if (!cmd || !cmd->command_number)
		return result;
	if (!g_LocalPlayer)								return original;
		
	Globals::cmd = cmd;
	
	if (g_Options.misc_bhop_human)
		HumanBhop::Get().Human_Bhop(cmd);

	if (original)
		g_Prediction->SetLocalViewangles(Vector(cmd->viewangles.pitch, cmd->viewangles.yaw, cmd->viewangles.roll));

	if (g_Options.misc_bhop)						BunnyHop::OnCreateMove(cmd);
	if (g_Options.misc_autostrafer)					BunnyHop::AutoStrafe(cmd, cmd->viewangles);
	if (g_Options.misc_chat_spammer > 0) ChatSpammer();

	bool bSendPacket = *SendPacket;

	sendpacket = bSendPacket;
	
	static float SpawnTime = 0.0f;
	if (g_LocalPlayer->m_flSpawnTime() != SpawnTime) {
		g_AnimState.pBaseEntity = g_LocalPlayer;
		g_LocalPlayer->ResetAnimationState(&g_AnimState);
		SpawnTime = g_LocalPlayer->m_flSpawnTime();
	}

	AutoRunBoost(cmd);
	Clantag();
	NameSteal();
	auto unpred_flags = g_LocalPlayer->m_fFlags();
	auto oldangles = cmd->viewangles;

	g_Options.dista = cmd->random_seed;

	//entities::fetch_targets(cmd);

	EnginePred::BeginPred(cmd);
	{
		AutoBlock(cmd);

		if (cmd->command_number % 2 == 1 && bSendPacket && !(g_Options.fakeduck && GetAsyncKeyState(g_Options.fakeduck_bind)) && g_Options.misc_desync)
			bSendPacket = false;

		if (g_Options.edgejump && GetAsyncKeyState(g_Options.edgejump_bind))
		{
			if ((unpred_flags & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
				cmd->buttons |= IN_JUMP;
		}
		QAngle angleold = cmd->viewangles;
		if (g_LocalPlayer->m_hActiveWeapon()) 
		{	
			g_Aimbot.OnMove(cmd);

			if (g_Options.slidewalk)
				SlideWalk(cmd);

			if (g_Options.autoreload)
				AutoWeapons(cmd);

			g_Backtrack.OnMove(cmd);
			if (g_Options.trigenable || g_LocalPlayer->m_hActiveWeapon()->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)
				TriggerBot(cmd);

			if (g_Options.kbot)
				knifebot(cmd);
		}

		//if (g_Options.edgejump)
		//	Walkbot(oldangles, cmd);

		if (g_Options.misc_desync && g_ClientState->chokedcommands >= max_choke_ticks()) {
			bSendPacket = true;
			cmd->viewangles = g_ClientState->viewangles;
		}

		if (g_Options.misc_desync && std::fabsf(g_LocalPlayer->m_flSpawnTime() - g_GlobalVars->curtime) > 1.0f)
			Desync(cmd, bSendPacket);

		CorrectMouse(cmd);

		auto anim_state = g_LocalPlayer->GetPlayerAnimState();
		if (anim_state) {
			CCSGOPlayerAnimState anim_state_backup = *anim_state;
			*anim_state = g_AnimState;
			g_LocalPlayer->GetVAngles() = cmd->viewangles;
			g_LocalPlayer->UpdateClientSideAnimation();

			updatelby(anim_state);

			g_AnimState = *anim_state;
			*anim_state = anim_state_backup;
		}
		if (bSendPacket) {
			anglereal = g_AnimState.m_flGoalFeetYaw;
			if(anim_state)
				anglefake = anim_state->m_flGoalFeetYaw;
			vangle = cmd->viewangles;
		}

		Slowwalk(cmd);

		fix_movement(cmd, angleold);
	}
	EnginePred::EndPred();

	//if (g_Options.fakeduck && GetAsyncKeyState(g_Options.fakeduck_bind) && !g_EngineClient->IsVoiceRecording())
	//	fakeduck(cmd, bSendPacket);

	//if (g_Options.fakeduck && GetAsyncKeyState(g_Options.fakeduck_bind));
	//	cmd->buttons |= IN_BULLRUSH;

	if (!g_EngineClient->IsVoiceRecording() && !(g_Options.fakeduck && GetAsyncKeyState(g_Options.fakeduck_bind)))
		Fakelag(cmd, bSendPacket);

	nosmoke();

	if (g_LocalPlayer && InputSys::Get().IsKeyDown(VK_TAB) && g_Options.misc_rankreveal)
		Utils::RankRevealAll();

	if (g_Options.antiuntrusted)
	{
		Math::ClampAngles(cmd->viewangles);
	}
	
	//if ()
		//g_InputSystem->EnableInput();
	
	Math::FixAngles(cmd->viewangles);
	std::clamp(cmd->sidemove, -450.f, 450.f);
	std::clamp(cmd->forwardmove, -450.f, 450.f);
	std::clamp(cmd->upmove, -320.f, 320.f);
	Globals::bSendPacket = bSendPacket;
	*SendPacket = bSendPacket;

	return false;
}


void __fastcall Hooks::hkOverrideView(void* _this, int edx, CViewSetup* vsView)
{
	static auto override_view_o = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

	if (g_EngineClient->IsInGame() && g_LocalPlayer && vsView) {

		auto r_modelAmbientMin = g_CVar->FindVar("r_modelAmbientMin");
		auto mat_force_tonemap_scale = g_CVar->FindVar("mat_force_tonemap_scale");
		auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");

		g_CVar->FindVar("weapon_debug_spread_show")->SetValue(g_Options.crosshair && !g_LocalPlayer->m_bIsScoped() ? 3 : 0);

		if (!g_Unload)
		{
			//r_modelAmbientMin->SetValue(g_Options.other_nightmode ? 1.f : 0.0f);
			//mat_force_tonemap_scale->SetValue(g_Options.other_nightmode ? g_Options.other_nightmode_size : 1.0f);

			if (g_Options.other_mat_ambient_light_rainbow)
			{
				static float rainbow; rainbow += 0.005f; if (rainbow > 1.f) rainbow = 0.f;
				auto rainbow_col = Color::FromHSB(rainbow, 1.0f, 1.0f);
				mat_ambient_light_r->SetValue(rainbow_col.r() / 255.0f);
				mat_ambient_light_g->SetValue(rainbow_col.g() / 255.0f);
				mat_ambient_light_b->SetValue(rainbow_col.b() / 255.0f);
			}
			else
			{
				if (mat_ambient_light_r->GetFloat() != 0.f)			mat_ambient_light_r->SetValue(0.f);
				if (mat_ambient_light_g->GetFloat() != 0.f)			mat_ambient_light_g->SetValue(0.f);
				if (mat_ambient_light_b->GetFloat() != 0.f)			mat_ambient_light_b->SetValue(0.f);
			}
		}
		else {
			if (r_modelAmbientMin->GetFloat() != 0.f)			r_modelAmbientMin->SetValue(0.f);
			if (mat_force_tonemap_scale->GetFloat() != 1.f)		mat_force_tonemap_scale->SetValue(1.f);
			//	if (mat_postprocess_enable->GetInt() != 0)		mat_postprocess_enable->SetValue(0);
			if (mat_ambient_light_r->GetFloat() != 0.f)			mat_ambient_light_r->SetValue(0.f);
			if (mat_ambient_light_g->GetFloat() != 0.f)			mat_ambient_light_g->SetValue(0.f);
			if (mat_ambient_light_b->GetFloat() != 0.f)			mat_ambient_light_b->SetValue(0.f);
		}
		/*if (g_Options.other_drawfov)
		{
			QAngle viewPunch = g_LocalPlayer->m_viewPunchAngle();
			QAngle aimPunch = g_LocalPlayer->m_aimPunchAngle();

			vsView->angles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
			vsView->angles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
			vsView->angles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		}*/

		if (g_Options.nozoom)
	        g_CVar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(0);
		else
			g_CVar->FindVar("zoom_sensitivity_ratio_mouse")->SetValue(1);

		if (!Globals::got_w2s_matrix)
		{
			Globals::got_w2s_matrix = true;
			Globals::w2s_offset = (reinterpret_cast<DWORD>(&g_EngineClient->WorldToScreenMatrix()) + 0x40);
		}

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.misc_viewmodel_fov);

		Visuals::Get().ThirdPerson();

		if (!g_LocalPlayer->m_bIsScoped())
			vsView->fov = g_Options.misc_fov;
		else if (g_LocalPlayer->m_bIsScoped() && g_Options.nozoom && g_LocalPlayer->m_hActiveWeapon() &&
			g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_SG556 &&
			g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_AUG)
			vsView->fov = g_Options.misc_fov;

		if (g_Options.fakeduck && GetAsyncKeyState(g_Options.fakeduck_bind) && g_LocalPlayer->IsAlive() && g_LocalPlayer->m_fFlags() & FL_ONGROUND)
			vsView->origin.z = g_LocalPlayer->m_vecOrigin().z + 64;

		if (g_Options.esp_grenade_prediction && g_LocalPlayer->m_hActiveWeapon())
			GrenadePrediction::Get().View(vsView);
	}
	override_view_o(g_ClientMode, edx, vsView);
}
#include "../features/glow.hpp"
int __fastcall Hooks::hkDoPostScreenEffects(void* _this, int edx, int a1)
{
	static auto do_pose_effects_o = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

	if (g_LocalPlayer && g_Options.glow_enabled)
		Glow::Get().Run();

	return do_pose_effects_o(g_ClientMode, edx, a1);
}