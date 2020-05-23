#pragma once
#include <set>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <limits>
#include "../valve_sdk/Misc/Color.hpp"
#include "../helpers/kit_parser.hpp"
#include "../helpers/item_definitions.hpp"

#define A( s ) #s
#define OPTION(type, var, val) type var = val

extern std::map<int, const char*> weapon_names;

class Sticker_t
{
public:
	int stickers_id = 0;
};

struct item_setting
{
	char name[32] = "Default";
	bool enabled = false;
	Sticker_t stickers[5];
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};

struct aimbot_settings {
	int autofire_key = 0;
	bool enabled = false;
	bool autofire = false;
	bool on_key = true;
	int key = 1;
	bool deathmatch = false;
	bool autopistol = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool check_jump = false;
	bool autowall = false;
	bool silent = false;
	bool antiaimlock = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool humanize = false;
	float curviness = false;
	struct {
		bool enabled = false;
		int ticks = 6;
	} backtrack;
	bool only_in_zoom = true;
	int aim_type = 1;
	int smooth_type;
	int priority = 0;
	int fov_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float silent_chance = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};
struct statrack_setting
{

	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};
struct funstr {
	OPTION(float, playback, 0.f);
	OPTION(float, weight, 0.f);
	OPTION(int, order, 0);
	OPTION(int, sequence, 0);
	OPTION(float, cycle, 0.f);
	OPTION(float, weightdelta, 0.f);
	OPTION(float, prevcycle, 0.f);
	OPTION(float, posparam, 0.f);
	OPTION(float, feetrate, 0.f);
	OPTION(bool, funenable, false);
};
struct skinInfo
{
	int seed = -1;
	int paintkit;
	int rarity;
	std::string tagName;
};


class Options
{
public:
		std::map<int, aimbot_settings> aimbot = {};
		std::map<int, funstr> funstuff = {};
		std::map<int, statrack_setting> statrack_items = { };
		OPTION(int, misc_minwalk_bind, 0);
		OPTION(bool, misc_minwalk, false);
		float color_esp_sounds[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		OPTION(bool, esp_sounds, false);
		OPTION(bool, esp_optimise, false);
		OPTION(bool, esp_sounds_enemy_only, false);
		OPTION(float, esp_sounds_time, 0.5f);
		OPTION(float, esp_sounds_radius, 15.0f);
		OPTION(bool, esp_flags_armor, false);
		OPTION(bool, esp_flags_kit, false);
		OPTION(bool, esp_flags_hostage, false);
		OPTION(bool, esp_flags_scoped, false);
		OPTION(bool, esp_flags_money, false);
		OPTION(bool, esp_flags_c4, false);
		OPTION(bool, esp_flags_defusing, false);
		OPTION(bool, esp_preview, false);
		OPTION(int, layer, 0);
		int dista = 0;
		int distb = 0;
		OPTION(bool, kbot, false);
		OPTION(bool, nosmoke, false);
		OPTION(int, misc_chat_spammer, false);
		OPTION(bool, nozoom, false);
		OPTION(bool, debug_showposes, false);
		OPTION(bool, debug_showactivities, false);
		OPTION(int, fakelags, 0);

		OPTION(bool, trigchecksmoke, false);
		OPTION(bool, trigger_autostop, false);
		OPTION(bool, trigger_autoscope, false);
		OPTION(float, trighitchance, 0.f);
		OPTION(float, trigdelay, 0.f);
		OPTION(bool, trigcheckflash, false);
		OPTION(bool, trighead, false);
		OPTION(bool, trigleftarm, false);
		OPTION(bool, trigrightarm, false);
		OPTION(bool, trigleftleg, false);
		OPTION(bool, trigrightleg, false);
		OPTION(bool, trigchest, false);
		OPTION(bool, trigstomach, false);

		OPTION(bool, trigenable, false);
		OPTION(bool, onkey, false);
		OPTION(int, trigbind, 0);

		OPTION(bool, baimlethal, false);
		OPTION(bool, baimfast, false);
		OPTION(bool, baimair, false);
		OPTION(bool, baimsw, false);
		OPTION(int, baimhp, 0);

		///
		OPTION(bool, rage_enabled, false);

		OPTION(float, flash, 255.f);

		OPTION(bool, aa_enabled, false);
		OPTION(bool, antiscreen, false);
		OPTION(bool, autoscope, false);
		OPTION(bool, resolver,false);
		OPTION(int, rage_mindmg, 55);
		OPTION(float , retrack, 15.f);
		OPTION(int, rage_mpsize, 0);
		OPTION(int, rage_hitchance, 50);
		OPTION(int, aa_x, 0);
		OPTION(int, aa_y, 0);
		///

		OPTION(bool, use_all_weapons, true);
		OPTION(bool, esp_angle_lines, false);
		OPTION(bool, esp_speclist, false);
		OPTION(bool, esp_damageindicator, false);
		OPTION(bool, hitsound, false);
		OPTION(bool, esp_damageindicator_color, false);
		OPTION(bool, show_ammo, false);
		//OPTION(bool, misc_autoaccept, false);
		OPTION(bool, edgejump, false);
		OPTION(int, edgejump_bind, 0);
		OPTION(bool, namestealer, false);
		OPTION(bool, fakeduck, false);
		OPTION(bool, slidewalk, false);
		OPTION(bool, panorama, false);
		OPTION(bool, invitespam, false);
		OPTION(bool, antiuntrusted, false);
		OPTION(int, fakeduck_bind, 0);
		OPTION(bool, misc_desync_ad, false);
		
		OPTION(bool, zoom, false);
		int zoomkey;

		OPTION(bool, misc_walkbot, false);

		OPTION(int, misc_desync, 0);
		OPTION(int, misc_desync_bind, 0);

		OPTION(bool, autoblock, false);
		OPTION(bool, misc_fake_prime, false);
		OPTION(bool, misc_backdrop, false);
		OPTION(bool, misc_autoaccept, false);
		OPTION(bool, misc_antiobs, false);
		OPTION(bool, misc_arb, false);
		OPTION(int, misc_arb_bind, 0);
		OPTION(int, autoblock_bind, 0);
		OPTION(bool, misc_clantag_custom, false);
		char misc_clantag_name[16];

		struct {
			std::map<int, item_setting> m_items;
			std::unordered_map<std::string, std::string> m_icon_overrides;
			auto get_icon_override(const std::string original) const -> const char*
			{
				return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
			}
		} skins;
		// 
		// ESP
		// 
		OPTION(bool, esp_enabled, false);
		OPTION(bool, esp_visible_only, false);
		OPTION(bool, esp_enemy_only, false);
		OPTION(bool, esp_farther, false);
		OPTION(int, esp_player_box, 0);
		OPTION(bool, esp_player_skeleton, false);
		OPTION(bool, esp_player_skeleton_bt, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false);
		OPTION(bool, esp_player_armour, false);
		OPTION(bool, esp_player_weapons, false);

		OPTION(bool, chams_arms_enabled, false);
		OPTION(bool, chams_arms_wireframe, false);
		OPTION(bool, chams_arms_reflect, false);
		OPTION(int, chams_arms_mat, false);
		float chams_arms_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, chams_sleeves_enabled, false);
		OPTION(bool, chams_sleeves_wireframe, false);
		OPTION(bool, chams_sleeves_reflect, false);
		OPTION(int, chams_sleeves_mat, false);
		float chams_sleeves_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, chams_weapon_enabled, false);
		OPTION(bool, chams_weapon_wireframe, false);
		OPTION(bool, chams_weapon_reflect, false);
		OPTION(int, chams_weapon_mat, false);
		float chams_weapon_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, crosshair, false);
		
		OPTION(bool, esp_defuse_kit, false);
		OPTION(bool, esp_planted_c4, false);
		OPTION(bool, esp_defusing_c4, false);
		OPTION(bool, esp_items, false);
		OPTION(float, esp_distance_dz, 1024.f);

		float color_esp_visible[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_esp_skeleton[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_esp_occluded[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		//
		// CHAMS
		//
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_player_enemies_only, false);
		OPTION(bool, chams_player_wireframe, false);
		OPTION(bool, chams_player_fake, false);
		OPTION(bool, chams_player_flat, false);
		OPTION(bool, chams_player_visibleonly, false);
		OPTION(bool, chams_player_glass, false);
		float color_chams_player_visible[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_chams_player_occluded[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float color_chams_real[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
		// 
		// GLOW
		// 
		OPTION(bool, glow_enabled, false);
		OPTION(int, glow_style, 0);
		OPTION(bool, glow_enemy_only, false);
		OPTION(bool, glow_visible_only, false);
		float color_glow[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		// 
		// OTHER
		// 
		OPTION(bool, other_drawfov, false);
		OPTION(bool, zeusrange, false);
		OPTION(bool, kniferange, false);
		OPTION(bool, ping, false);
		OPTION(float, other_drawfov_fov, 0.f);
		OPTION(bool, other_no_hands, false);
		OPTION(bool, esp_grenade_prediction, false);
		OPTION(bool, other_nightmode, false);
		OPTION(float, other_nightmode_size, 0.05f);
		OPTION(float, other_mat_ambient_light_r, 0.0f);
		OPTION(float, other_mat_ambient_light_g, 0.0f);
		OPTION(float, other_mat_ambient_light_b, 0.0f);
		OPTION(bool, other_mat_ambient_light_rainbow, false);

		//
		// MISC
		//
		OPTION(bool, misc_bhop, false);
		OPTION(bool, misc_bhop_human, false);
		OPTION(int,  misc_bhop_hit_chance, 0);
		OPTION(int,  misc_bhops_restricted_limit, 0);
		OPTION(int,  misc_max_bhops_hit, 0);

		OPTION(bool, autoreload, false);
		OPTION(bool, misc_autostrafer, false);
		OPTION(bool, misc_clantag, false);
		OPTION(bool, misc_clantag_animated , false);
		OPTION(bool, misc_thirdperson, false);
		OPTION(int, misc_thirdperson_bind, 0);
		OPTION(bool, misc_watermark, false);
		OPTION(bool, misc_rankreveal, false);
		OPTION(float, misc_thirdperson_dist, 50.f);
		OPTION(int, misc_viewmodel_fov, 68);
		OPTION(int, misc_fov, 90);	
		OPTION(bool, misc_radar, false);

		OPTION(bool, fakelag_enabled, false);
		OPTION(bool, fakelag_standing, false);
		OPTION(bool, fakelag_moving, false);
		OPTION(bool, fakelag_unducking, false);
		OPTION(int, fakelag_mode, 0);
		OPTION(int, fakelag_factor, 0);
		bool rage_autostop;
		bool rage_hitscan[7];
};

static char* tabs[] = { "LEGIT", "VISUALS", "MISC","SKINS" ,"CONFIG", "LIST" };

extern Options g_Options;
extern bool   g_Unload;
