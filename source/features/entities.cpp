#include "../helpers/math.hpp"

#include "entities.h"
#include "../config/options.hpp"
#include "aimbot.hpp"
namespace entities
{
	const auto point_scale = 0.5f;

	ConVar* game_type = nullptr;
	ConVar* game_mode = nullptr;

	std::mutex locker;
	std::mutex local_mutex;

	local_data_t m_local;
	std::list<tick_data_t> m_items(24);
	QAngle aim_angles;
	Vector top = Vector(0, 0, 1.f);
	Vector bot = Vector(0, 0, -1.f);
	Vector min, max, forward, origin, head_screen;

	Ray_t ray;
	CGameTrace tr;

	bool is_hitbox_has_multipoints(const int& hitbox)
	{
		switch (hitbox)
		{
		case HITBOX_HEAD:
		case HITBOX_NECK:
		case HITBOX_STOMACH:
		case HITBOX_UPPER_CHEST:
			return true;
		default:
			return false;
		}
	}

	bool is_hitbox_for_visible_check(const int& hitbox)
	{
		switch (hitbox) {
		case HITBOX_HEAD:
		case HITBOX_UPPER_CHEST:
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
			return true;
		default:
			return false;
		}
	}

	RECT GetBBox(C_BaseEntity* ent, Vector* pointsTransformed)
	{
		auto collideable = ent->GetCollideable();
		if (!collideable)
			return {};

		auto min = collideable->OBBMins();
		auto max = collideable->OBBMaxs();

		const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

		Vector points[] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		for (int i = 0; i < 8; i++)
			Math::VectorTransform(points[i], trans, pointsTransformed[i]);

		return {};
	}

	float damage_for_armor(const float& damage, const int& armor_value)
	{
		if (armor_value <= 0)
			return damage;

		float new_damage = damage * 0.5f;
		float armor = (damage - new_damage) * 0.5f;
		if (armor > static_cast<float>(armor_value))
		{
			armor = static_cast<float>(armor_value) * (1.f / 0.5f);
			new_damage = damage - armor;
		}

		return new_damage;
	}

	float curtime(const int& tick_base)
	{
		return g_GlobalVars->interval_per_tick * tick_base;
	}

	int get_health(C_BasePlayer* local, C_PlantedC4* bomb)
	{
		if (!local->IsAlive())
			return 0;

		float distance = local->m_vecOrigin().DistTo(bomb->m_vecOrigin());

		float d = ((distance - 75.68f) / 789.2f);
		float damage = 450.7f * exp(-d * d);

		const auto f = damage_for_armor(damage, local->m_ArmorValue());
		damage = std::max(int(ceilf(f)), 0);

		int result = local->m_iHealth() - damage;

		return result <= 0 ? 0 : result;
	}

	float get_bomb_time(C_PlantedC4* bomb, const int& tick_base)
	{
		if (!bomb->m_bBombTicking())
			return 0;

		const auto bomb_time = bomb->m_flC4Blow() - curtime(tick_base);

		return bomb_time >= 0.f ? bomb_time : 0;
	}

	float get_defuse_time(C_PlantedC4* bomb, const int& tick_base)
	{
		static float defuse_time = -1;

		if (!bomb->m_hBombDefuser())
			defuse_time = -1;
		else if (defuse_time == -1)
			defuse_time = curtime(tick_base) + bomb->m_flDefuseLength();

		if (defuse_time > -1 && bomb->m_hBombDefuser())
			return defuse_time - curtime(tick_base);

		return 0;
	}

	C_PlantedC4* get_bomb()
	{
		//if (!interfaces::game_rules_proxy || !interfaces::game_rules_proxy->m_bBombPlanted())
			//return nullptr;

		C_BaseEntity* entity;
		for (auto i = 1; i <= g_EntityList->GetMaxEntities(); ++i)
		{
			entity = C_BaseEntity::GetEntityByIndex(i);
			if (entity && !entity->IsDormant() && entity->IsPlantedC4())
				return reinterpret_cast<C_PlantedC4*>(entity);
		}

		return nullptr;
	}

	void destroy()
	{
		local_mutex.lock();
		{
			m_local = local_data_t{};
		}
		local_mutex.unlock();

		locker.lock();
		{
			m_items.clear();
		}
		locker.unlock();
	}

	bool is_matchmaking()
	{
		if (!game_type)
			game_type = g_CVar->FindVar("game_type");

		if (!game_mode)
			game_mode = g_CVar->FindVar("game_mode");

		if (game_type->GetInt() != 0) //classic
			return false;

		if (game_mode->GetInt() != 0 && game_mode->GetInt() != 1 && game_mode->GetInt() != 2) //casual, mm, wingman
			return false;
		
		return true;
	}

	void fetch_hp(local_data_t& tick_data)
	{
		tick_data.is_matchmaking = is_matchmaking();
		if (!tick_data.is_matchmaking || !tick_data.local)
			return;
		
		tick_data.hp = tick_data.is_alive ? tick_data.health : 0;

		const auto bomb = get_bomb();
		if (bomb && bomb->m_bBombTicking())
		{
			tick_data.bomb_time = get_bomb_time(bomb, tick_data.tick_base);
			tick_data.defuse_time = get_defuse_time(bomb, tick_data.tick_base);

			tick_data.hp = get_health(tick_data.local, bomb);
		}
	}

	void set_local(C_BasePlayer* local, local_data_t& tick_data, const int& tick_base)
	{
		tick_data.local = local;
		tick_data.angles = local->m_angEyeAngles();
		tick_data.is_alive = local->IsAlive();
		tick_data.is_flashed = local->IsFlashed();
		tick_data.is_scoped = local->m_bIsScoped();

		tick_data.tick_base = tick_base;

		tick_data.health = local->m_iHealth();
		tick_data.armor_value = local->m_ArmorValue();

		tick_data.has_sniper = tick_data.is_alive && local->m_hActiveWeapon() && local->m_hActiveWeapon()->IsSniper();

		tick_data.world_pos = local->m_vecOrigin();
		Math::WorldToScreen(tick_data.world_pos, tick_data.local_pos);

		tick_data.hp = 0;
		tick_data.bomb_time = 0.f;
		tick_data.defuse_time = 0.f;

		fetch_hp(tick_data);
	}

	void fetch_targets(CUserCmd* cmd)
	{
		C_BasePlayer* local = g_LocalPlayer;
		if (!local)
		{
			destroy();
			return;
		}
		
		if (!local->IsAlive() && local->m_hObserverTarget())
		{
			const auto observer = (C_BasePlayer*)C_BasePlayer::get_entity_from_handle(local->m_hObserverTarget());
			if (observer && observer->IsPlayer())
				local = observer;
		}

		local_mutex.lock();
		set_local(local, m_local, g_LocalPlayer->m_nTickBase());
		local_mutex.unlock();

		tick_data_t tick_data;
		tick_data.tick_count = cmd->tick_count;

		const auto eye_pos = local->GetEyePos();

		bool in_smoke;
		bool is_visible;

		C_BasePlayer* player;
		for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
		{
			player = C_BasePlayer::GetPlayerByIndex(i);
			if (!player || player == local)
				continue;

			//if (settings::misc::radar && !player->m_bSpotted())
			//	player->m_bSpotted() = true;

			if (!player->IsPlayer() || player->IsDormant() || player->m_iHealth() <= 0 || !player->IsAlive())
				continue;

			//if (player->m_fFlags() & FL_FROZEN || player->m_bGunGameImmunity())
			//	continue;

			const auto is_enemy = player->m_iTeamNum() != local->m_iTeamNum();

			auto studio_model = g_MdlInfo->GetStudiomodel(player->GetModel());
			if (!studio_model)
				continue;

			if (!player->m_hActiveWeapon() || !player->m_hActiveWeapon()->GetCSWeaponData())
				continue;

			/*
#ifdef _DEBUG
			console::print("=== entity ===");
			console::print(player->m_vecOrigin());
			console::print("flVelocity %.2f", player->m_vecVelocity().Length2D());
			console::print("flSimulationTime %.2f", player->m_flSimulationTime());
#endif
			*/
			player_data_t player_data;
			player_data.index = player->EntIndex();
			player_data.world_pos = player->m_vecOrigin();
			player_data.eye_pos = player->GetEyePos();

			player_data.name = std::string(player->GetPlayerInfo().szName).substr(0, 12);

			player_data.weapon = player->m_hActiveWeapon()->GetCSWeaponData()->szWeaponName;

			player_data.is_dormant = false;
			player_data.is_scoped = player->m_bIsScoped();

			player_data.is_enemy = true;
			player_data.m_iHealth = player->m_iHealth();
			player_data.m_ArmorValue = player->m_ArmorValue();
			player_data.m_flSimulationTime = player->m_flSimulationTime();

			const auto tick_offset = player->m_vecVelocity() * g_GlobalVars->interval_per_tick;

			in_smoke = true;
			is_visible = false;

			const auto on_screen = Math::WorldToScreen(player->m_vecOrigin(), origin);

			const auto old_origin = player->GetAbsOrigin();
			player->SetAbsOrigin(player_data.world_pos);
			player->InvalidateBoneCache();

			if (!player->SetupBones(player_data.bone_matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
			{
				player->SetAbsOrigin(old_origin);
				player->InvalidateBoneCache();

				continue;
			}



			CTraceFilterSkipTwoEntities filter(local, player);
			for (int k = HITBOX_HEAD; k < HITBOX_MAX; k++)
			{
				const auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(k);
				if (!hitbox)
					continue;

				const auto bone = player_data.bone_matrix[hitbox->bone];
				float mod = hitbox->m_flRadius != -1.f ? hitbox->m_flRadius : 0.f;

				Math::VectorTransform(hitbox->bbmax + mod, bone, max);
				Math::VectorTransform(hitbox->bbmin - mod, bone, min);

				player_data.hitboxes[k][0] = (min + max) * 0.5f;

				Math::VectorAngles(player_data.hitboxes[k][0] - eye_pos, aim_angles);
				aim_angles.Normalize();

				if (is_hitbox_has_multipoints(k))
				{
					Math::AngleVectors(aim_angles, forward);

					Vector right = forward.Cross(Vector(0, 0, 1));
					Vector left = Vector(-right.x, -right.y, right.z);

					const auto index = k == HITBOX_HEAD ? 2 : 1;
					const auto custom_scale = k == HITBOX_HEAD ? 0.7f : point_scale;

					if (k == HITBOX_HEAD)
						player_data.hitboxes[k][1] = player_data.hitboxes[k][0] + top * (mod * custom_scale);

					player_data.hitboxes[k][index] = player_data.hitboxes[k][0] + right * (mod * custom_scale);
					player_data.hitboxes[k][index + 1] = player_data.hitboxes[k][0] + left * (mod * custom_scale);
				}
								
				if (!g_Options.esp_enabled || !on_screen || !is_hitbox_for_visible_check(k))
					continue;

				if (!is_visible)
				{
					ray.Init(eye_pos, player_data.hitboxes[k][0]);
					g_EngineTrace->TraceRay(ray, CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_DEBRIS | CONTENTS_HITBOX, &filter, &tr);

					is_visible = tr.endpos.DistTo(eye_pos) == player_data.hitboxes[k][0].DistTo(eye_pos);
				}

				if (is_visible && in_smoke && !g_Aimbot.IsLineGoesThroughSmoke(eye_pos, player_data.hitboxes[k][0]))
					in_smoke = false;
			}

			player->SetAbsOrigin(old_origin);
			player->InvalidateBoneCache();

			Math::WorldToScreen(player_data.hitboxes[0][0], head_screen);
			head_screen.z += 15.f;

			const auto height = fabs(head_screen.y - origin.y);
			const auto width = height / 1.65f;

			player_data.in_smoke = in_smoke;
			player_data.is_visible = is_visible;

			player_data.box = GetBBox(player, player_data.points);
			player_data.origin = origin;
			player_data.offset = tick_offset;
			player_data.angles = player->m_angEyeAngles();

			tick_data.players[i] = player_data;
		}

		locker.lock();
		{
			m_items.push_front(tick_data);
			m_items.resize(13);
		}
		locker.unlock();
	}
}