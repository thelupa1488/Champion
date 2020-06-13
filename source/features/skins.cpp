#include "skins.h"
#include "../helpers/jsonconfig.h"
player_death_event player_death_listener;
static bool UpdateGlove;

void LoadStatrack()
{
	config::load("stattrack.json", "others", false, [](Json::Value root)
		{
			g_Options.statrack_items.clear();

			Json::Value it = root["stattrack"];
			if (it.isNull())
				return;

			for (auto item : it)
			{
				auto skin_data = &g_Options.statrack_items[item["definition_index"].asInt()];
				Option::Load(item["definition_index"], skin_data->definition_index);
				Option::Load(item["stat_track.counter_new"], skin_data->statrack_new.counter);
			}
		});
}

void SaveStatrack()
{
	config::save("stattrack.json", "others", false, []()
		{
			Json::Value config;

			Json::Value it;
			for (auto m_item : g_Options.statrack_items)
			{
				Json::Value item;
				item["definition_index"] = m_item.first;
				item["stat_track.counter_new"] = m_item.second.statrack_new.counter;
				it.append(item);
			}

			config["stattrack"] = it;

			return config;
		});
}

static auto erase_override_if_exists_by_index(const int definition_index) -> void
{
	if (k_weapon_info.count(definition_index))
	{
		auto& icon_override_map = g_Options.skins.m_icon_overrides;
		const auto& original_item = k_weapon_info.at(definition_index);

		if (!original_item.icon)
			return;

		const auto override_entry = icon_override_map.find(original_item.icon);
		if (override_entry != end(icon_override_map))
			icon_override_map.erase(override_entry); // Remove the leftover override
	}
}

static auto apply_config_on_attributable_item(C_BaseAttributableItem* item, const item_setting* config,
	const unsigned xuid_low) -> void
{
	if (!config->enabled) {
		return;
	}
	item->m_Item().m_iItemIDHigh() = -1;
	item->m_Item().m_iAccountID() = xuid_low;
	if (config->custom_name[0])
		strcpy_s(item->m_Item().m_iCustomName(), config->custom_name);
	if (config->paint_kit_index)
		item->m_nFallbackPaintKit() = config->paint_kit_index;
	if (config->seed)
		item->m_nFallbackSeed() = config->seed;
	if (config->stat_trak) {
		item->m_nFallbackStatTrak() = g_Options.statrack_items[config->definition_index].statrack_new.counter;
		item->m_Item().m_iEntityQuality() = 9;
	}
	else {
		item->m_Item().m_iEntityQuality() = is_knife(config->definition_index) ? 3 : 0;
	}
	item->m_flFallbackWear() = config->wear;
	auto& definition_index = item->m_Item().m_iItemDefinitionIndex();
	auto& icon_override_map = g_Options.skins.m_icon_overrides;

	if (config->definition_override_index && config->definition_override_index != definition_index && k_weapon_info.count(config->definition_override_index))
	{
		const auto old_definition_index = definition_index;
		definition_index = config->definition_override_index;
		const auto& replacement_item = k_weapon_info.at(config->definition_override_index);

		item->m_nModelIndex() = g_MdlInfo->GetModelIndex(replacement_item.model);
		item->SetModelIndex(g_MdlInfo->GetModelIndex(replacement_item.model));
		item->GetClientNetworkable()->PreDataUpdate(0);
		if (old_definition_index && k_weapon_info.count(old_definition_index))
		{
			const auto& original_item = k_weapon_info.at(old_definition_index);

			if (original_item.icon && replacement_item.icon)
				icon_override_map[original_item.icon] = replacement_item.icon;
		}
	}
	else
	{
		erase_override_if_exists_by_index(definition_index);
	}
}

template <typename T>
T* get_entity_from_handle(CBaseHandle h)
{
	if (h == INVALID_EHANDLE_INDEX)
		return nullptr;

	return static_cast<T*>(g_EntityList->GetClientEntityFromHandle(h));
}

static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto clazz = g_CHLClient->GetAllClasses();
	// Please, if you gonna paste it into a cheat use classids here. I use names because they
	// won't change in the foreseeable future and i dont need high speed, but chances are
	// you already have classids, so use them instead, they are faster.
	while (strcmp(clazz->m_pNetworkName, "CEconWearable"))
		clazz = clazz->m_pNext;
	return clazz->m_pCreateFn;
}

static auto make_glove(int entry, int serial) -> C_BaseAttributableItem*
{
	static auto create_wearable_fn = get_wearable_create_fn();

	create_wearable_fn(entry, serial);

	const auto glove = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntity(entry));
	assert(glove);
	{
		static auto set_abs_origin_addr = Utils::PatternScan(GetModuleHandle(L"client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8");
		const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
		static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
		set_abs_origin_fn(glove, new_pos);
	}

	return glove;
}

void Skins::Listener()
{
	g_GameEvents->AddListener(&player_death_listener, "player_death", false);
}

void player_death_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (strstr(p_event->GetName(), "player_spawn"))
	{
		if (g_EngineClient->GetPlayerForUserID(p_event->GetInt("userid")) == g_EngineClient->GetLocalPlayer()) {
			UpdateGlove = true;
		}
	}

	if (strstr(p_event->GetName(), "player_death")) {

		if (g_EngineClient->GetPlayerForUserID(p_event->GetInt("attacker")) == g_EngineClient->GetLocalPlayer()) 
		{
			auto& weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon) {
				auto& skin_data = g_Options.skins.m_items[weapon->m_Item().m_iItemDefinitionIndex()];
				auto& stat_track = g_Options.statrack_items[weapon->m_Item().m_iItemDefinitionIndex()];
				if (skin_data.enabled && skin_data.stat_trak) {

					stat_track.statrack_new.counter++;
					weapon->m_nFallbackStatTrak() = stat_track.statrack_new.counter;
					weapon->GetClientNetworkable()->PostDataUpdate(0);
					weapon->GetClientNetworkable()->OnDataChanged(0);
				}
			}

			SaveStatrack();

			const auto icon_override = g_Options.skins.get_icon_override(p_event->GetString("weapon"));

			if (icon_override)
				p_event->SetString("weapon", icon_override);
		}
	}
}

static bool handle_glove_config(player_info_t player_info)
{
	const auto local_index = g_EngineClient->GetLocalPlayer();
	const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(local_index));

	const auto wearables = local->m_hMyWearables();
	const auto glove_config = &g_Options.skins.m_items[local->m_iTeamNum() == team::team_ct ? GLOVE_CT_SIDE : GLOVE_T_SIDE];
	static auto glove_handle = CBaseHandle(0);

	auto glove = get_entity_from_handle<C_BaseAttributableItem>(wearables[0]);
	if (!glove) // There is no glove
	{
		// Try to get our last created glove
		const auto our_glove = get_entity_from_handle<C_BaseAttributableItem>(glove_handle);
		if (our_glove) // Our glove still exists
		{
			wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (!local->IsAlive())
	{
		// We are dead but we have a glove, destroy it
		if (glove)
		{
			glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
			glove->GetClientNetworkable()->Release();
		}

		return false;
	}

	if (glove_config && glove_config->definition_override_index)
	{
		// We don't have a glove, but we should
		if (!glove || !UpdateGlove)
		{
			const auto entry = g_EntityList->GetHighestEntityIndex() + 1;
			const auto serial = rand() % 0x1000;

			glove = make_glove(entry, serial);
			UpdateGlove = false;

			const auto wearable_handle = reinterpret_cast<CBaseHandle*>(&wearables[0]);
			*wearable_handle = entry | serial << 16;

			// Let's store it in case we somehow lose it.
			glove_handle = wearables[0];
		}

		// Thanks, Beakers
		glove->SetModelIndex(-1);

		apply_config_on_attributable_item(glove, glove_config, player_info.xuid_low);
	}

	return true;
}

void Skins::OnFrameStageNotify(bool frame_end)
{
	const auto local_index = g_EngineClient->GetLocalPlayer();
	const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(local_index));

	if (!local)
		return;

	player_info_t player_info;

	if (!g_EngineClient->GetPlayerInfo(local_index, &player_info))
		return;

	if (frame_end)
	{
		handle_glove_config(player_info);
	}
	else
	{
		auto weapons = local->m_hMyWeapons();
		for (int i = 0; weapons[i].IsValid(); i++)
		{
			C_BaseAttributableItem* weapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(weapons[i]);
			if (!weapon)
				continue;
			auto& definition_index = weapon->m_Item().m_iItemDefinitionIndex();

			const auto knife_index = local->m_iTeamNum() == team::team_ct ? WEAPON_KNIFE : WEAPON_KNIFE_T;
			const auto defined_index = is_knife(definition_index) ? knife_index : definition_index;

			const auto active_conf = &g_Options.skins.m_items[defined_index];
			apply_config_on_attributable_item(weapon, active_conf, player_info.xuid_low);
		}
	}

	const auto view_model_handle = local->m_hViewModel();
	if (!view_model_handle.IsValid())
		return;
	const auto view_model = static_cast<C_BaseViewModel*>(g_EntityList->GetClientEntityFromHandle(view_model_handle));
	if (!view_model)
		return;
	const auto view_model_weapon_handle = view_model->m_hWeapon();
	if (!view_model_weapon_handle.IsValid())
		return;
	const auto view_model_weapon = static_cast<C_BaseCombatWeapon*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
	if (!view_model_weapon)
		return;
	if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex()))
	{
		const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
		auto override_model_index = g_MdlInfo->GetModelIndex(override_model);

		view_model->m_nModelIndex() = override_model_index;
		auto world_model_handle = view_model_weapon->m_hWeaponWorldModel();
		if (!world_model_handle.IsValid())
			return;
		const auto world_model = static_cast<C_BaseWeaponWorldModel*>(g_EntityList->GetClientEntityFromHandle(world_model_handle));
		if (!world_model)
			return;
		world_model->m_nModelIndex() = override_model_index + 1;
	}
}