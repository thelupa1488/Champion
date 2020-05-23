#include "AnimationSystem.h"
//#include "c_resolver.h"

c_animation_system::animation::animation(C_BasePlayer* player)
{
	const auto weapon = g_LocalPlayer->m_hActiveWeapon();

	this->player = player;
	index = player->EntIndex();
	dormant = player->IsDormant();
	velocity = player->m_vecVelocity();
	origin = player->m_vecOrigin();
	abs_origin = player->GetAbsOrigin();
	obb_mins = player->GetCollideable()->OBBMins();
	obb_maxs = player->GetCollideable()->OBBMaxs();
	layers = *player->GetAnimOverlays();
	poses = player->m_flPoseParameter();
	if ((has_anim_state = player->GetPlayerAnimState()))
		anim_state = *player->GetPlayerAnimState();
	anim_time = player->m_flOldSimulationTime() + g_GlobalVars->interval_per_tick;
	sim_time = player->m_flSimulationTime();
	interp_time = 0.f;
	last_shot_time = weapon ? weapon->GetLastShotTime() : 0.f;
	duck = player->m_flDuckAmount();
	lby = player->m_flLowerBodyYawTarget();
	eye_angles = player->m_angEyeAngles();
	abs_ang = player->GetAbsAngles();
	flags = player->m_fFlags();
	eflags = player->get_eflags();
	effects = player->get_effects();
	lag = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());

	// animations are off when we enter pvs, we do not want to shoot yet.
	valid = lag >= 0 && lag <= 17;

	// clamp it so we don't interpolate too far : )
	lag = std::clamp(lag, 0, 17);
}

c_animation_system::animation::animation(C_BasePlayer* player, QAngle last_reliable_angle) : animation(player)
{
	this->last_reliable_angle = last_reliable_angle;
}

void c_animation_system::animation::restore(C_BasePlayer* player) const
{
	player->m_vecVelocity() = velocity;
	player->m_fFlags() = flags;
	player->get_eflags() = eflags;
	player->m_flDuckAmount() = duck;
	*player->GetAnimOverlays() = layers;
	player->m_flLowerBodyYawTarget() = lby;
	player->m_vecOrigin() = origin;
	player->SetOrigin(abs_origin);
}

void c_animation_system::animation::apply(C_BasePlayer* player) const
{
	player->m_flPoseParameter() = poses;
	player->m_angEyeAngles() = eye_angles;
	player->m_vecVelocity() = player->get_abs_velocity() = velocity;
	player->m_flLowerBodyYawTarget() = lby;
	player->m_flDuckAmount() = duck;
	player->m_fFlags() = flags;
	player->m_vecOrigin() = origin;
	player->SetOrigin(origin);
	if (player->GetPlayerAnimState() && has_anim_state)
		*player->GetPlayerAnimState() = anim_state;
}

void c_animation_system::animation::build_server_bones(C_BasePlayer* player)
{
	const auto backup_occlusion_flags = *(int*)((uintptr_t)player + 0xA28);
	const auto backup_occlusion_framecount = *(int*)((uintptr_t)player + 0xA30);

	// skip occlusion checks in c_cs_player::setup_bones
	if (player != g_LocalPlayer) {
		*(int*)((uintptr_t)player + 0xA28) = 0;
		*(int*)((uintptr_t)player + 0xA30) = 0;
	}
	// clear bone accessor
	player->get_readable_bones() = player->get_writable_bones() = 0;

	// invalidate bone cache
	player->get_most_recent_model_bone_counter() = 0;
	player->get_last_bone_setup_time() = -FLT_MAX;
	// stop interpolation
	player->get_effects() |= 8;
	
	// change bone accessor
	const auto backup_bone_array = player->get_bone_array_for_write();
	if (player != g_LocalPlayer)
		player->get_bone_array_for_write() = bones;

	// build bones
	player->SetupBones(nullptr, -1, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime);

	player->get_bone_array_for_write() = backup_bone_array;

	// restore original occlusion
	if (player != g_LocalPlayer) {
		*(int*)((uintptr_t)player + 0xA28) = backup_occlusion_flags;
		*(int*)((uintptr_t)player + 0xA30) = backup_occlusion_framecount;
	}

	// start interpolation again
	player->get_effects() &= ~8;
}

void c_animation_system::animation_info::update_animations(animation* record, animation* from)
{
	if (!from)
	{
		// set velocity and layers.
		record->velocity = player->m_vecVelocity();

		// fix feet spin.
		record->anim_state.m_flFeetYawRate = 0.f;

		// resolve player.
		//c_resolver::resolve(record);

		// apply record.
		record->apply(player);

		// run update.
		return Get().update_player(player);
	}

	const auto new_velocity = player->m_vecVelocity();

	// restore old record.
	*player->GetAnimOverlays() = from->layers;
	player->SetOrigin(record->origin);
	player->set_abs_angles(from->abs_ang);
	player->m_vecVelocity() = from->velocity;

	// setup velocity.
	record->velocity = new_velocity;

	// did the player shoot?
	const auto shot = record->last_shot_time > from->sim_time && record->last_shot_time <= record->sim_time;


	record->shot = shot;

	// setup extrapolation parameters.
	auto old_origin = from->origin;
	auto old_flags = from->flags;

	for (auto i = 0; i < record->lag; i++)
	{
		// move time forward.
		const auto time = from->sim_time + TICKS_TO_TIME(i + 1);
		const auto lerp = 1.f - (record->sim_time - time) / (record->sim_time - from->sim_time);

		if(!std::isfinite(lerp))
			continue;
		// lerp eye angles.
		auto eye_angles = Math::interpolate(Vector(from->eye_angles.pitch, from->eye_angles.yaw, from->eye_angles.roll ), Vector(record->eye_angles.pitch, record->eye_angles.yaw, record->eye_angles.roll), lerp);
		
		if (!eye_angles.IsValid())
			continue;

		Math::Normalize3(eye_angles);
		player->m_angEyeAngles() = QAngle(eye_angles.x, eye_angles.y, eye_angles.z);

		// lerp duck amount.
		player->m_flDuckAmount() = Math::interpolate(from->duck, record->duck, lerp);

		// resolve player.
		if (record->lag - 1 == i)
		{
			player->m_vecVelocity() = new_velocity;
			player->m_fFlags() = record->flags;

			//if (record->lag > 1)
				//c_resolver::resolve(record);
		}
		else // compute velocity and flags.
		{
			//c_trace_system::extrapolate(player, old_origin, player->m_vecVelocity(), player->m_fFlags(), old_flags & FL_ONGROUND);
			old_flags = player->m_fFlags();
		}

		// correct shot desync.
		if (shot)
		{
			player->m_angEyeAngles() = record->last_reliable_angle;

			if (record->last_shot_time <= time)
				player->m_angEyeAngles() = record->eye_angles;
		}

		// instant approach.
		if (player->m_vecVelocity().Length2D() < .1f && fabsf(player->m_vecVelocity().z) < 100.f && record->lag > 1)
			animation_system.last_process_state = player->GetPlayerAnimState();

		// fix feet spin.
		player->GetPlayerAnimState()->m_flFeetYawRate = 0.f;

		// backup simtime.
		const auto backup_simtime = player->m_flSimulationTime();

		// set new simtime.
		player->m_flSimulationTime() = time;

		// run update.
		Get().update_player(player);

		// restore old simtime.
		player->m_flSimulationTime() = backup_simtime;
	}
}

void updatelby(CCSGOPlayerAnimState* animstate);

void c_animation_system::post_player_update()
{
	if (!g_EngineClient->IsInGame())
		return;

	auto local = g_LocalPlayer;

	// erase outdated entries
	for (auto it = animation_infos.begin(); it != animation_infos.end();) {
		auto player = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(it->first));

		if (!player || player != it->second.player || !player->IsAlive() || (player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
			|| !local || !local->IsAlive())
		{
			if (player)
				player->m_bClientSideAnimation() = true;
			it = animation_infos.erase(it);
		}
		else
			it = next(it);
	}

	if (!local || !local->IsAlive() || !local->m_hActiveWeapon())
		return;
	
	// create new entries and reset old ones
	for (int i = 1; i < 65; i++) 
	{
		auto player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!player)
			continue;

		if (!player->IsAlive() || player->IsDormant() || (player == g_LocalPlayer) || (player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) || !player->m_hActiveWeapon())
			continue;

		if (!(player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) && !(player == g_LocalPlayer))
			player->m_bClientSideAnimation() = true;		

		if (animation_infos.find(player->GetRefEHandle().ToInt()) == animation_infos.end())
			animation_infos.insert_or_assign(player->GetRefEHandle().ToInt(), animation_info(player, {}));
	}

	// run post update
	for (auto& info : animation_infos)
	{

		auto& animation = info.second;
		const auto player = animation.player;

		// erase frames out-of-range
		for (auto it = animation.frames.rbegin(); it != animation.frames.rend();) {
			if (g_GlobalVars->curtime - it->sim_time > 1.2f)
				it = decltype(it) { info.second.frames.erase(next(it).base()) };
			else
				it = next(it);
		}

		// have we already seen this update?
		if (player->m_flSimulationTime() == player->m_flOldSimulationTime())
			continue;

		// reset animstate
		if (animation.last_spawn_time != player->m_flSpawnTime())
		{
			const auto state = player->GetPlayerAnimState();
			if (state)
				player->ResetAnimationState(state);

			animation.last_spawn_time = player->m_flSpawnTime();
		}

		// grab weapon
		const auto weapon = g_LocalPlayer->m_hActiveWeapon();

		if (!weapon)
			return;

		// make a full backup of the player
		auto backup = c_animation_system::animation(player);
		backup.apply(player);

		// grab previous
		c_animation_system::animation* previous = nullptr;

		if (!animation.frames.empty() && !animation.frames.front().dormant
			&& TIME_TO_TICKS(player->m_flSimulationTime() - animation.frames.front().sim_time) <= 17)
			previous = &animation.frames.front();
		//Beep(300, 100);
		// update shot info
		const auto shot = weapon && previous && weapon->GetLastShotTime() > previous->sim_time
			&& weapon->GetLastShotTime() <= player->m_flSimulationTime();

		if (!shot)
			info.second.last_reliable_angle = player->m_angEyeAngles();

		// store server record
		auto& record = animation.frames.emplace_front(player, info.second.last_reliable_angle);

		// run full update
		animation.update_animations(&record, previous);


		// restore server layers
		*player->GetAnimOverlays() = backup.layers;

		// use uninterpolated data to generate our bone matrix
		record.build_server_bones(player);

		// restore correctly synced values
		backup.restore(player);
		
		// set record to latest animation
		animation.latest_animation = record;
	}
}

std::optional<c_animation_system::animation*> c_animation_system::get_latest_firing_animation(C_BasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid(it->sim_time, it->valid) && it->shot)
			return &*it;

	return std::nullopt;
}

void c_animation_system::update_player(C_BasePlayer* player)
{
	static auto& enable_bone_cache_invalidation = **reinterpret_cast<bool**>(
		reinterpret_cast<uint32_t>(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "C6 05 ? ? ? ? ? 89 47 70")) + 2);

	if (!player || player->IsDormant() || !player->IsAlive())
		return;
	// make a backup of globals
	const auto backup_frametime = g_GlobalVars->frametime;
	const auto backup_curtime = g_GlobalVars->curtime;

	// get player anim state
	const auto state = player->GetPlayerAnimState();

	if (!state)
		return;

	// fixes for networked players
	if (player != g_LocalPlayer)
	{
		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
		g_GlobalVars->curtime = player->m_flSimulationTime();
		player->get_eflags() &= ~0x1000;
	}

	// allow reanimating in the same frame
	if (state->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
		state->m_iLastClientSideAnimationUpdateFramecount -= 1;

	// make sure we keep track of the original invalidation state
	const auto old_invalidation = enable_bone_cache_invalidation;

	// notify the other hooks to instruct animations and pvs fix
	Get().enable_bones = player->m_bClientSideAnimation() = true;
	player->UpdateClientSideAnimation();
	Get().enable_bones = false;

	if (player != g_LocalPlayer)
		player->m_bClientSideAnimation() = false;

	Get().last_process_state = nullptr;

	// invalidate physics.
	if (player != g_LocalPlayer)
		player->invalidate_physics_recursive(angles_changed
			| animation_changed
			| sequence_changed);

	// we don't want to enable cache invalidation by accident
	enable_bone_cache_invalidation = old_invalidation;

	// restore globals
	g_GlobalVars->frametime = backup_frametime;
	g_GlobalVars->curtime = backup_curtime;
}

void c_animation_system::update_simple_local_player(C_BasePlayer* player, CUserCmd* cmd)
{
	player->m_flPoseParameter() = animation_system.local_animation.poses;
	*player->GetAnimOverlays() = animation_system.server_layers;
	animation_system.local_animation.eye_angles = cmd->viewangles;
	Math::Normalize3(animation_system.local_animation.eye_angles);

	player->GetPlayerAnimState()->m_flFeetYawRate = 0.f;
	animation_system.update_player(player);
	animation_system.local_animation.abs_ang.yaw = player->GetPlayerAnimState()->m_flGoalFeetYaw;
	animation_system.local_animation.layers = *player->GetAnimOverlays();
	animation_system.local_animation.poses = player->m_flPoseParameter();
}
 
c_animation_system::animation_info* c_animation_system::get_animation_info(C_BasePlayer* player)
{
	auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end())
		return nullptr;

	return &info->second;
}

std::optional<c_animation_system::animation*> c_animation_system::get_latest_animation(C_BasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid())
			return &*it;

	return std::nullopt;
}

std::optional<c_animation_system::animation*> c_animation_system::get_oldest_animation(C_BasePlayer* player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.rbegin(); it != info->second.frames.rend(); it = next(it))
		if (it->is_valid())
			return &*it;

	return std::nullopt;
}

std::optional<c_animation_system::animation*> c_animation_system::get_uncrouched_animation(C_BasePlayer * player)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid() && fabsf(it->duck) < .0001f)
			return &*it;

	return std::nullopt;
}

std::optional<std::pair<c_animation_system::animation*, c_animation_system::animation*>> c_animation_system::get_intermediate_animations(
	C_BasePlayer* player, const float range)
{
	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return std::nullopt;

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid(range * .2f) && it + 1 != info->second.frames.end() && !(it + 1)->is_valid(range * .2f))
			return std::make_pair(&*(it + 1), &*it);

	return std::nullopt;
}

std::vector<c_animation_system::animation*> c_animation_system::get_valid_animations(C_BasePlayer* player, const float range)
{
	std::vector<animation*> result;

	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end() || info->second.frames.empty())
		return result;

	result.reserve(static_cast<int>(std::ceil(range * .2f / g_GlobalVars->interval_per_tick)));

	for (auto it = info->second.frames.begin(); it != info->second.frames.end(); it = next(it))
		if (it->is_valid(range * .2f))
			result.push_back(&*it);

	return result;
}

std::optional<c_animation_system::animation*> c_animation_system::get_lastest_animation_unsafe(C_BasePlayer* player)
{
	const auto latest = get_latest_animation(player);

	if (latest.has_value())
		return latest;

	const auto info = animation_infos.find(player->GetRefEHandle().ToInt());

	if (info == animation_infos.end())
		return std::nullopt;

	return &info->second.latest_animation;
}
