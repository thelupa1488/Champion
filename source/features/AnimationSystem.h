#pragma once

#include <deque>
#include <utility>
#include <mutex>
#include <optional>
#include <map>
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/SDK.hpp"
enum resolver_state {
	resolver_start,
	resolver_still,
	resolver_still2,
	resolver_stretch,
	resolver_stretch2
};

__forceinline float calculate_lerp()
{
	static auto cl_interp = g_CVar->FindVar(("cl_interp"));
	static auto cl_updaterate = g_CVar->FindVar(("cl_updaterate"));

	const auto update_rate = cl_updaterate->GetInt();
	const auto interp_ratio = cl_interp->GetFloat();

	auto lerp = interp_ratio / update_rate;

	if (lerp <= interp_ratio)
		lerp = interp_ratio;

	return lerp;
}

class c_animation_system : public Singleton<c_animation_system>
{
public:
	struct animation
	{
		animation() = default;
		explicit animation(C_BasePlayer* player);
		explicit animation(C_BasePlayer* player, QAngle last_reliable_angle);
		void restore(C_BasePlayer* player) const;
		void apply(C_BasePlayer* player) const;
		void build_server_bones(C_BasePlayer* player);
		__forceinline bool is_valid(const float range = .2f, const float max_unlag = .2f) const
		{
			if (!g_ClientState->m_NetChannel || !valid)
				return false;

			const auto correct = std::clamp(g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_INCOMING)
				+ g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING)
				+ calculate_lerp(), 0.f, max_unlag);

			return fabsf(correct - (g_GlobalVars ->curtime - sim_time)) <= range;
		}

		C_BasePlayer* player{};
		int32_t index{};

		bool valid{}, has_anim_state{};
		alignas(16) matrix3x4_t bones[128]{};

		bool dormant{};
		Vector velocity;
		Vector origin;
		Vector abs_origin;
		Vector obb_mins;
		Vector obb_maxs;
		AnimationLayer layers{};
		pose_paramater poses{};
		CCSGOPlayerAnimState anim_state{};
		float anim_time{};
		float sim_time{};
		float interp_time{};
		float duck{};
		float lby{};
		float last_shot_time{};
		QAngle last_reliable_angle{};
		QAngle eye_angles;
		QAngle abs_ang;
		int flags{};
		int eflags{};
		int effects{};
		int lag{};
		int shot{};

	};
private:
	struct animation_info {
		animation_info(C_BasePlayer* player, std::deque<animation> animations)
			: player(player), frames(std::move(animations)), last_spawn_time(0) { }

		void update_animations(animation* to, animation* from);

		C_BasePlayer* player{};
		std::deque<animation> frames{};

		// latest animation (might be invalid)
		animation latest_animation{};

		// last time this player spawned
		float last_spawn_time;

		// counter of how many shots we missed
		int32_t missed_due_to_spread{};
		
		// resolver data
		resolver_state brute_state{};
		//float brute_yaw_2{};
		float brute_yaw{};	
		QAngle last_reliable_angle{};
	};

	std::unordered_map<int, animation_info> animation_infos;

public:
	C_BasePlayer* entity_target = nullptr;

	void update_player(C_BasePlayer* player);
	void update_simple_local_player(C_BasePlayer* player, CUserCmd* cmd);
	void post_player_update();
	       
	animation_info* get_animation_info(C_BasePlayer* player);
	std::optional<animation*> get_latest_firing_animation(C_BasePlayer* player);
	std::optional<animation*> get_latest_animation(C_BasePlayer* player);
	std::optional<animation*> get_oldest_animation(C_BasePlayer* player);
	std::optional<animation*> get_uncrouched_animation(C_BasePlayer* player);
	std::optional<std::pair<animation*, animation*>> get_intermediate_animations(C_BasePlayer* player, float range = 1.f);
	std::vector<animation*> get_valid_animations(C_BasePlayer* player, float range = 1.f);
	std::optional<animation*> get_lastest_animation_unsafe(C_BasePlayer* player);

	animation local_animation;
	AnimationLayer server_layers{};
	bool in_jump, enable_bones = false;
	CCSGOPlayerAnimState* last_process_state{};
};

#define animation_system c_animation_system::Get()
#define lerp_ticks time_to_ticks(calculate_lerp())
