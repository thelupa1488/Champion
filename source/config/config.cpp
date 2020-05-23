#include "config.hpp"

CConfig* Config = new CConfig();

void CConfig::loadskins()
{
	g_ClientState->ForceFullUpdate();

	config::load("skins.json", "others", true, [](Json::Value root)
		{

			Json::Value items = root["skins"];
			if (items.isNull())
				return;

			for (auto item : items)
			{
				auto skin_data = &g_Options.skins.m_items[item["definition_index"].asInt()];

				Option::Load(item["enabled"], skin_data->enabled);

				Option::Load(item["definition_vector_index"], skin_data->definition_vector_index);

				Option::Load(item["definition_index"], skin_data->definition_index);
				Option::Load(item["definition_override_index"], skin_data->definition_override_index);
				Option::Load(item["definition_override_vector_index"], skin_data->definition_override_vector_index);
				Option::Load(item["paint_kit_index"], skin_data->paint_kit_index);
				Option::Load(item["paint_kit_vector_index"], skin_data->paint_kit_vector_index);
				Option::Load(item["seed"], skin_data->seed);
				Option::Load(item["stat_track.enabled"], skin_data->stat_trak);
				Option::Load(item["wear"], skin_data->wear);
				strcpy_s(skin_data->custom_name, sizeof(skin_data->custom_name), item["nametag"].asCString());
			}

		});
}

void CConfig::saveskins()
{
	config::save("skins.json", "others", true, []()
		{
			Json::Value config;

			Json::Value items;
			for (auto m_item : g_Options.skins.m_items)
			{
				Json::Value item;

				item["enabled"] = m_item.second.enabled;
				item["definition_vector_index"] = m_item.second.definition_vector_index;
				item["definition_index"] = m_item.second.definition_index;
				item["definition_override_index"] = m_item.second.definition_override_index;
				item["definition_override_vector_index"] = m_item.second.definition_override_vector_index;
				item["paint_kit_index"] = m_item.second.paint_kit_index;
				item["paint_kit_vector_index"] = m_item.second.paint_kit_vector_index;
				item["seed"] = m_item.second.seed;
				item["stat_track.enabled"] = m_item.second.stat_trak;
				item["wear"] = m_item.second.wear;
				item["nametag"] = m_item.second.custom_name;


				items.append(item);
			}

			config["skins"] = items;

			return config;
		});
}

void CConfig::load(std::string name)
{
	config::load(name, "", true, [](Json::Value root)
		{

			Option::Load(root["esp.enabled"], g_Options.esp_enabled);
			Option::Load(root["esp.visible_only"], g_Options.esp_visible_only);
			Option::Load(root["esp.enemy_only"], g_Options.esp_enemy_only);
			Option::Load(root["esp.player_box"], g_Options.esp_player_box, 0);
			Option::Load(root["esp.player_names"], g_Options.esp_player_names);
			Option::Load(root["esp.player_skeleton"], g_Options.esp_player_skeleton);
			Option::Load(root["esp.player_skeleton_bt"], g_Options.esp_player_skeleton_bt);
			Option::Load(root["esp.health"], g_Options.esp_player_health);
			Option::Load(root["esp.armour"], g_Options.esp_player_armour);
			Option::Load(root["esp.weapons"], g_Options.esp_player_weapons);
			Option::Load(root["esp.ammo"], g_Options.show_ammo);
			Option::Load(root["esp.items"], g_Options.esp_items);
			Option::Load(root["esp.distance_dz"], g_Options.esp_distance_dz);

			Option::Load(root["esp.flags_armor"], g_Options.esp_flags_armor);
			Option::Load(root["esp.flags_defusing"], g_Options.esp_flags_defusing);
			Option::Load(root["esp.flags_c4"], g_Options.esp_flags_c4);
			Option::Load(root["esp.flags_hostage"], g_Options.esp_flags_hostage);
			Option::Load(root["esp.flags_kit"], g_Options.esp_flags_kit);
			Option::Load(root["esp.flags_money"], g_Options.esp_flags_money);
			Option::Load(root["esp.flags_scoped"], g_Options.esp_flags_scoped);


			Option::Load(root["esp.optimise"], g_Options.esp_optimise);

			Option::Load(root["esp.sounds"], g_Options.esp_sounds);
			Option::Load(root["esp.sounds_enemy_only"], g_Options.esp_sounds_enemy_only);
			Option::Load(root["esp.sounds_radius"], g_Options.esp_sounds_radius);
			Option::Load(root["esp.sounds_time"], g_Options.esp_sounds_time);
			Option::Load(root["esp.color_esp_sounds"], g_Options.color_esp_sounds);

			Option::Load(root["esp.visible_color"], g_Options.color_esp_visible, 1.f);
			Option::Load(root["esp.occluded_color"], g_Options.color_esp_occluded, 1.f);
			Option::Load(root["esp.skeleton_color"], g_Options.color_esp_skeleton);

			Option::Load(root["glow.enabled"], g_Options.glow_enabled);
			Option::Load(root["glow.enemy_only"], g_Options.glow_enemy_only);
			Option::Load(root["glow.style"], g_Options.glow_style);
			Option::Load(root["glow.color"], g_Options.color_glow, 1.f);

			Option::Load(root["visuals.drawfov"], g_Options.other_drawfov);
			Option::Load(root["visuals.no_hands"], g_Options.other_no_hands);
			Option::Load(root["visuals.grenade_prediction"], g_Options.esp_grenade_prediction);
			Option::Load(root["visuals.angle_lines"], g_Options.esp_angle_lines);

			Option::Load(root["visuals.speclist"], g_Options.esp_speclist);
			Option::Load(root["visuals.damageindicator"], g_Options.esp_damageindicator);
			Option::Load(root["visuals.damageindicator_color"], g_Options.esp_damageindicator_color);

			Option::Load(root["visuals.thirdperson"], g_Options.misc_thirdperson);
			Option::Load(root["visuals.thirdperson_dist"], g_Options.misc_thirdperson_dist);
			Option::Load(root["visuals.thirdperson_bind"], g_Options.misc_thirdperson_bind);


			Option::Load(root["visuals.defuse_kit"], g_Options.esp_defuse_kit);
			Option::Load(root["visuals.crosshair"], g_Options.crosshair);
			Option::Load(root["visuals.planted_c4"], g_Options.esp_planted_c4);
			Option::Load(root["visuals.defusing_c4"], g_Options.esp_defusing_c4);
			Option::Load(root["visuals.dropped_weapons"], g_Options.esp_dropped_weapons);

			Option::Load(root["visuals.zeusrange"], g_Options.zeusrange);
			Option::Load(root["visuals.rainbow_world"], g_Options.other_mat_ambient_light_rainbow);
			Option::Load(root["visuals.nozoom"], g_Options.nozoom);

			Option::Load(root["chams.enabled"], g_Options.chams_player_enabled, false); //old chams
			Option::Load(root["chams.visible_only"], g_Options.chams_player_visibleonly);
			Option::Load(root["chams.wireframe"], g_Options.chams_player_wireframe);
			Option::Load(root["chams.flat"], g_Options.chams_player_flat, false);
			Option::Load(root["chams.enemies_only"], g_Options.chams_player_enemies_only, false);
			//Option::Load(root["chams.player_fake"], g_Options.chams_player_fake, false);

			Option::Load(root["chams_arms_color"], g_Options.chams_arms_color); //old chams
			Option::Load(root["chams_arms_enabled"], g_Options.chams_arms_enabled);
			Option::Load(root["chams_arms_mat"], g_Options.chams_arms_mat);
			Option::Load(root["chams_arms_wireframe"], g_Options.chams_arms_wireframe);

			Option::Load(root["chams_sleeves_color"], g_Options.chams_sleeves_color); //old chams
			Option::Load(root["chams_sleeves_enabled"], g_Options.chams_sleeves_enabled);
			Option::Load(root["chams_sleeves_mat"], g_Options.chams_sleeves_mat);
			Option::Load(root["chams_sleeves_wireframe"], g_Options.chams_sleeves_wireframe);

			Option::Load(root["chams_weapon_color"], g_Options.chams_weapon_color); //old chams
			Option::Load(root["chams_weapon_enabled"], g_Options.chams_weapon_enabled);
			Option::Load(root["chams_weapon_mat"], g_Options.chams_weapon_mat);
			Option::Load(root["chams_weapon_wireframe"], g_Options.chams_weapon_wireframe);
			Option::Load(root["namestealer"], g_Options.namestealer);

			Option::Load(root["chams.visible_color"], g_Options.color_chams_player_visible, 0.f);
			Option::Load(root["chams.occluded_color"], g_Options.color_chams_player_occluded, 0.f);
			//Option::Load(root["chams.real_color"], g_Options.color_chams_real, 0.f);

			Option::Load(root["misc.bhop"], g_Options.misc_bhop);
			Option::Load(root["misc.human_bhop"], g_Options.misc_bhop_human);
			Option::Load(root["misc.human_bhop_hit_chance"], g_Options.misc_bhop_hit_chance);
			Option::Load(root["misc.human_bhop_restricted_limit"], g_Options.misc_bhops_restricted_limit);
			Option::Load(root["misc.human_bhops_max_hit"], g_Options.misc_max_bhops_hit);
			Option::Load(root["misc.auto_strafe"], g_Options.misc_autostrafer);
			Option::Load(root["misc.watermark"], g_Options.misc_watermark);
			Option::Load(root["misc.backdrop"], g_Options.misc_backdrop);
			Option::Load(root["misc.viewmodel_fov"], g_Options.misc_viewmodel_fov, 68);
			Option::Load(root["misc.knife_bot"], g_Options.kbot);
			Option::Load(root["misc.misc_fov"], g_Options.misc_fov, 90);
			Option::Load(root["misc.hitmarker_sound"], g_Options.hitsound);
			Option::Load(root["misc.clantag"], g_Options.misc_clantag);
			Option::Load(root["misc.clantag.custom"], g_Options.misc_clantag_custom);
			Option::Load(root["misc.clantag.animated"], g_Options.misc_clantag_animated);
			Option::Load(root["misc.rankreveal"], g_Options.misc_rankreveal);
			Option::Load(root["misc.antiobs"], g_Options.misc_antiobs);
			Option::Load(root["misc.radar"], g_Options.misc_radar);
			Option::Load(root["misc.edgejump"], g_Options.edgejump);
			Option::Load(root["misc.edgejump_bind"], g_Options.edgejump_bind);

			Option::Load(root["misc.flash"], g_Options.flash);
			Option::Load(root["misc.retrack"], g_Options.retrack);

			Option::Load(root["misc.desync.enabled"], g_Options.misc_desync);
			Option::Load(root["misc.desync.bind"], g_Options.misc_desync_bind);
			Option::Load(root["misc.desync.ad"], g_Options.misc_desync_ad);

			Option::Load(root["autoblock"], g_Options.autoblock);
			Option::Load(root["autoblock_bind"], g_Options.autoblock_bind);

			Option::Load(root["misc.resolver"], g_Options.resolver);
			Option::Load(root["misc.slidewalk"], g_Options.slidewalk);
			Option::Load(root["misc.chatspammer"], g_Options.misc_chat_spammer);
			Option::Load(root["misc.antiuntrusted"], g_Options.antiuntrusted);
			Option::Load(root["misc.fakeprime"], g_Options.misc_fake_prime);

			Option::Load(root["misc.fakeduck"], g_Options.fakeduck);
			Option::Load(root["misc.fakeduck_bind"], g_Options.fakeduck_bind);

			Option::Load(root["misc.autoaccept"], g_Options.misc_autoaccept);

			Option::Load(root["misc.fakelag.enabled"], g_Options.fakelag_enabled);
			Option::Load(root["misc.fakelag.factor"], g_Options.fakelag_factor);
			Option::Load(root["misc.fakelag.mode"], g_Options.fakelag_mode);
			Option::Load(root["misc.fakelag.moving"], g_Options.fakelag_moving);
			Option::Load(root["misc.fakelag.unducking"], g_Options.fakelag_unducking);
			Option::Load(root["misc.fakelag.standing"], g_Options.fakelag_standing);

			Option::Load(root["misc.autorunboost"], g_Options.misc_arb);
			Option::Load(root["misc.autorunboost.bind"], g_Options.misc_arb_bind);

			if (root.isMember("misc.clantag.name"))
				strcpy_s(g_Options.misc_clantag_name, sizeof(g_Options.misc_clantag_name), root["misc.clantag.name"].asCString());

			g_Options.aimbot.clear();

			Json::Value aimbot_items = root["legitbot.items"];
			for (Json::Value::iterator it = aimbot_items.begin(); it != aimbot_items.end(); ++it)
			{
				Json::Value settings = *it;

				aimbot_settings data;
				Option::Load(settings["enabled"], data.enabled);
				Option::Load(settings["deathmatch"], data.deathmatch);
				Option::Load(settings["autofire_key"], data.autofire_key);
				Option::Load(settings["on_key"], data.on_key);
				Option::Load(settings["key"], data.key);
				Option::Load(settings["priority"], data.priority);
				Option::Load(settings["antiaimlock"], data.antiaimlock);
				Option::Load(settings["autofire"], data.autofire);
				Option::Load(settings["autopistol"], data.autopistol);
				Option::Load(settings["check_smoke"], data.check_smoke);
				Option::Load(settings["check_flash"], data.check_flash);
				Option::Load(settings["check_jump"], data.check_jump, false);
				Option::Load(settings["smooth_type"], data.smooth_type);
				Option::Load(settings["backtrack.enabled"], data.backtrack.enabled);
				Option::Load(settings["backtrack.time"], data.backtrack.ticks, 6);
				if (data.backtrack.ticks < 1 || data.backtrack.ticks > 13)
					data.backtrack.ticks = 6;

				Option::Load(settings["only_in_zoom"], data.only_in_zoom, true);
				Option::Load(settings["silent.enabled"], data.silent, false);
				Option::Load(settings["silent.fov"], data.silent_fov, 0.f);
				Option::Load(settings["silent.chance"], data.silent_chance, 0.f);

				Option::Load(settings["hitbox"], data.hitbox, 0);
				Option::Load(settings["aim_type"], data.aim_type, 0);

				Option::Load(settings["fov_type"], data.fov_type);
				Option::Load(settings["fov"], data.fov);
				Option::Load(settings["smooth"], data.smooth, 1);
				Option::Load(settings["shot_delay"], data.shot_delay);
				Option::Load(settings["kill_delay"], data.kill_delay, 600);

				Option::Load(settings["autowall.enabled"], data.autowall);
				Option::Load(settings["autowall.min_damage"], data.min_damage, 1);

				Option::Load(settings["rcs"], data.rcs);
				Option::Load(settings["rcs.humanize"], data.humanize);
				Option::Load(settings["rcs.curve"], data.curviness);
				Option::Load(settings["rcs.first_bullet"], data.rcs_start);
				Option::Load(settings["rcs.standalone"], data.rcs_type, 0);
				Option::Load(settings["rcs.fov_enabled"], data.rcs_fov_enabled);
				Option::Load(settings["rcs.smooth_enabled"], data.rcs_smooth_enabled);
				Option::Load(settings["rcs.rcs_fov"], data.rcs_fov);
				Option::Load(settings["rcs.smooth"], data.rcs_smooth);
				Option::Load(settings["rcs_value_x"], data.rcs_x, 100);
				Option::Load(settings["rcs_value_y"], data.rcs_y, 100);

				g_Options.aimbot[it.key().asInt()] = data;
			}

			//g_Options.skins.m_items.clear();
			/*
			settings::aimbot::m_groups.clear();

			for (auto group : root["aimbot.groups"])
			{
				std::vector<int> weapons = { };

				for (auto& weapon_index : group["weapons"])
				{
					weapons.emplace_back(weapon_index.asInt());
				}

				settings::aimbot::m_groups.emplace_back(aimbot_group{ group["name"].asString(), weapons });
			}*/
		});



}

void CConfig::save(std::string name)
{
	config::save(name, "", true, []()
		{
			Json::Value config;

			config["esp.enabled"] = g_Options.esp_enabled;
			config["esp.enemy_only"] = g_Options.esp_enemy_only;
			config["esp.visible_only"] = g_Options.esp_visible_only;
			config["esp.player_box"] = g_Options.esp_player_box;
			config["esp.player_names"] = g_Options.esp_player_names;
			config["esp.player_skeleton"] = g_Options.esp_player_skeleton;
			config["esp.player_skeleton_bt"] = g_Options.esp_player_skeleton_bt;

			config["esp.health"] = g_Options.esp_player_health;
			config["esp.armour"] = g_Options.esp_player_armour;
			config["esp.weapons"] = g_Options.esp_player_weapons;
			config["esp.ammo"] = g_Options.show_ammo;
			config["esp.items"] = g_Options.esp_items;
			config["esp.distance_dz"] = g_Options.esp_distance_dz;


			config["esp.flags_armor"] = g_Options.esp_flags_armor;
			config["esp.flags_defusing"] = g_Options.esp_flags_defusing;
			config["esp.flags_c4"] = g_Options.esp_flags_c4;
			config["esp.flags_hostage"] = g_Options.esp_flags_hostage;
			config["esp.flags_kit"] = g_Options.esp_flags_kit;
			config["esp.flags_money"] = g_Options.esp_flags_money;
			config["esp.flags_scoped"] = g_Options.esp_flags_scoped;

			config["esp.optimise"] = g_Options.esp_optimise;

			config["esp.sounds"] = g_Options.esp_sounds;
			config["esp.sounds_enemy_only"] = g_Options.esp_sounds_enemy_only;
			config["esp.sounds_radius"] = g_Options.esp_sounds_radius;
			config["esp.sounds_time"] = g_Options.esp_sounds_time;
			Option::Save(config["esp.color_esp_sounds"], g_Options.color_esp_sounds);

			Option::Save(config["esp.visible_color"], g_Options.color_esp_visible);
			Option::Save(config["esp.occluded_color"], g_Options.color_esp_occluded);
			Option::Save(config["esp.skeleton_color"], g_Options.color_esp_skeleton);

			config["glow.enabled"] = g_Options.glow_enabled;
			config["glow.enemy_only"] = g_Options.glow_enemy_only;
			config["glow.style"] = g_Options.glow_style;
			Option::Save(config["glow.color"], g_Options.color_glow);


			config["visuals.grenade_prediction"] = g_Options.esp_grenade_prediction;
			config["visuals.drawfov"] = g_Options.other_drawfov;
			config["visuals.no_hands"] = g_Options.other_no_hands;

			config["visuals.speclist"] = g_Options.esp_speclist;
			config["visuals.damageindicator"] = g_Options.esp_damageindicator; //old chams
			config["visuals.damageindicator_color"] = g_Options.esp_damageindicator_color;
			config["visuals.angle_lines"] = g_Options.esp_angle_lines;

			config["visuals.thirdperson"] = g_Options.misc_thirdperson;
			config["visuals.thirdperson_dist"] = g_Options.misc_thirdperson_dist;
			config["visuals.thirdperson_bind"] = g_Options.misc_thirdperson_bind; //old chams

			config["visuals.defuse_kit"] = g_Options.esp_defuse_kit;
			config["visuals.crosshair"] = g_Options.crosshair;
			config["visuals.planted_c4"] = g_Options.esp_planted_c4;
			config["visuals.defusing_c4"] = g_Options.esp_defusing_c4;
			config["visuals.dropped_weapons"] = g_Options.esp_dropped_weapons;

			config["visuals.nightmode"] = g_Options.other_nightmode;
			config["visuals.nightmode_size"] = g_Options.other_nightmode_size; //old chams
			config["visuals.ambient_light_r"] = g_Options.other_mat_ambient_light_r;
			config["visuals.ambient_light_g"] = g_Options.other_mat_ambient_light_g;
			config["visuals.ambient_light_b"] = g_Options.other_mat_ambient_light_b;
			config["visuals.ambient_light_rainbow"] = g_Options.other_mat_ambient_light_rainbow;

			config["nozoom"] = g_Options.nozoom;

			config["chams.enabled"] = g_Options.chams_player_enabled; //old chams
			config["chams.visible_only"] = g_Options.chams_player_visibleonly;
			config["chams.wireframe"] = g_Options.chams_player_wireframe;
			config["chams.flat"] = g_Options.chams_player_flat;
			config["chams.enemies_only"] = g_Options.chams_player_enemies_only;
			//config["chams.player_fake"] = g_Options.chams_player_fake;


			config["chams_arms_enabled"] = g_Options.chams_arms_enabled;
			config["chams_arms_mat"] = g_Options.chams_arms_mat;
			config["chams_arms_wireframe"] = g_Options.chams_arms_wireframe;

			config["chams_sleeves_enabled"] = g_Options.chams_sleeves_enabled;
			config["chams_sleeves_mat"] = g_Options.chams_sleeves_mat;
			config["chams_sleeves_wireframe"] = g_Options.chams_sleeves_wireframe;

			config["chams_weapon_enabled"] = g_Options.chams_weapon_enabled;
			config["chams_weapon_mat"] = g_Options.chams_weapon_mat;
			config["chams_weapon_wireframe"] = g_Options.chams_weapon_wireframe;
			config["namestealer"] = g_Options.namestealer;



			Option::Save(config["chams_arms_color"], g_Options.chams_arms_color);
			Option::Save(config["chams_weapon_color"], g_Options.chams_weapon_color);
			Option::Save(config["chams_sleeves_color"], g_Options.chams_sleeves_color);


			Option::Save(config["chams.visible_color"], g_Options.color_chams_player_visible);
			Option::Save(config["chams.occluded_color"], g_Options.color_chams_player_occluded);
			//Option::Save(config["chams.real_color"], g_Options.color_chams_real);

			config["misc.bhop"] = g_Options.misc_bhop;
			config["misc.human_bhop"] = g_Options.misc_bhop_human;
			config["misc.human_bhop_hit_chance"] = g_Options.misc_bhop_hit_chance;
			config["misc.human_bhop_restricted_limit"] = g_Options.misc_bhops_restricted_limit;
			config["misc.human_bhops_max_hit"] = g_Options.misc_max_bhops_hit;
			config["misc.auto_strafe"] = g_Options.misc_autostrafer;
			config["misc.watermark"] = g_Options.misc_watermark;
			config["misc.backdrop"] = g_Options.misc_backdrop;
			config["misc.viewmodel_fov"] = g_Options.misc_viewmodel_fov;
			config["misc.knife_bot"] = g_Options.kbot;
			config["misc.misc_fov"] = g_Options.misc_fov;
			config["misc.hitmarker_sound"] = g_Options.hitsound;

			config["misc.edgejump"] = g_Options.edgejump;
			config["misc.edgejump_bind"] = g_Options.edgejump_bind;
			config["misc.rankreveal"] = g_Options.misc_rankreveal;
			config["misc.antiobs"] = g_Options.misc_antiobs;
			config["misc.radar"] = g_Options.misc_radar;
			config["misc.clantag"] = g_Options.misc_clantag;
			config["misc.clantag.custom"] = g_Options.misc_clantag_custom;
			config["misc.clantag.animated"] = g_Options.misc_clantag_animated;

			config["misc.flash"] = g_Options.flash;
			config["misc.retrack"] = g_Options.retrack;

			config["misc.desync.enabled"] = g_Options.misc_desync;
			config["misc.desync.bind"] = g_Options.misc_desync_bind;
			config["misc.desync.ad"] = g_Options.misc_desync_ad;

			config["misc.autoblock"] = g_Options.autoblock;
			config["misc.autoblock_bind"] = g_Options.autoblock_bind;

			config["misc.resolver"] = g_Options.resolver;
			config["misc.slidewalk"] = g_Options.slidewalk;
			config["misc.antiuntrusted"] = g_Options.antiuntrusted;
			config["misc.fakeprime"] = g_Options.misc_fake_prime;


			config["misc.fakeduck"] = g_Options.fakeduck;
			config["misc.fakeduck_bind"] = g_Options.fakeduck_bind;

			config["misc.autoaccept"] = g_Options.misc_autoaccept;

			config["misc.fakelag.enabled"] = g_Options.fakelag_enabled;
			config["misc.fakelag.factor"] = g_Options.fakelag_factor;
			config["misc.fakelag.mode"] = g_Options.fakelag_mode;
			config["misc.fakelag.moving"] = g_Options.fakelag_moving;
			config["misc.fakelag.unducking"] = g_Options.fakelag_unducking;
			config["misc.fakelag.standing"] = g_Options.fakelag_standing;

			config["misc.autorunboost"] = g_Options.misc_arb;
			config["misc.autorunboost.bind"] = g_Options.misc_arb_bind;

			config["misc.clantag.name"] = g_Options.misc_clantag_name;

			Json::Value aimbot_items;
			for (auto data : g_Options.aimbot)
			{
				Json::Value act;
				const auto aimbot_data = data.second;
				act["autopistol"] = aimbot_data.autopistol;
				act["autofire"] = aimbot_data.autofire;
				act["on_key"] = aimbot_data.on_key;
				act["deathmatch"] = aimbot_data.deathmatch;

				act["key"] = aimbot_data.key;
				act["autowall.enabled"] = aimbot_data.autowall;
				act["autowall.min_damage"] = aimbot_data.min_damage;
				act["backtrack.enabled"] = aimbot_data.backtrack.enabled;
				act["backtrack.time"] = aimbot_data.backtrack.ticks;
				act["autofire_key"] = aimbot_data.autofire_key;
				act["check_flash"] = aimbot_data.check_flash;
				act["antiaimlock"] = aimbot_data.antiaimlock;
				act["priority"] = aimbot_data.priority;

				act["check_smoke"] = aimbot_data.check_smoke;
				act["check_jump"] = aimbot_data.check_jump;
				act["enabled"] = aimbot_data.enabled;
				act["fov"] = aimbot_data.fov;
				act["smooth_type"] = aimbot_data.smooth_type;
				act["fov_type"] = aimbot_data.fov_type;
				act["silent.enabled"] = aimbot_data.silent;
				act["silent.fov"] = aimbot_data.silent_fov;
				act["silent.chance"] = aimbot_data.silent_chance;

				act["aim_type"] = aimbot_data.aim_type;
				act["hitbox"] = aimbot_data.hitbox;

				act["kill_delay"] = aimbot_data.kill_delay;
				act["only_in_zoom"] = aimbot_data.only_in_zoom;
				act["shot_delay"] = aimbot_data.shot_delay;
				act["smooth"] = aimbot_data.smooth;

				act["rcs"] = aimbot_data.rcs;
				act["rcs.humanize"] = aimbot_data.humanize;
				act["rcs.curve"] = aimbot_data.curviness;
				act["rcs.first_bullet"] = aimbot_data.rcs_start;
				act["rcs.standalone"] = aimbot_data.rcs_type;
				act["rcs.fov_enabled"] = aimbot_data.rcs_fov_enabled;
				act["rcs.smooth_enabled"] = aimbot_data.rcs_smooth_enabled;
				act["rcs.rcs_fov"] = aimbot_data.rcs_fov;
				act["rcs.smooth"] = aimbot_data.rcs_smooth;
				act["rcs_value_x"] = aimbot_data.rcs_x;
				act["rcs_value_y"] = aimbot_data.rcs_y;

				aimbot_items[data.first] = act;
			}

			config["legitbot.items"] = aimbot_items;

			/*
			Json::Value aimbot_groups;
			for (auto group : g_Options.aimbot)
			{
				Json::Value act;
				act["name"] = group.name;

				Json::Value weapons;
				for (auto& weapon : group.weapons)
					weapons.append(weapon);

				act["weapons"] = weapons;

				aimbot_groups.append(act);
			}

			config["aimbot.groups"] = aimbot_groups;
			*/
			return config;
		});
}
