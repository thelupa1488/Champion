#include "hooks.hpp"
#include "../helpers/item_definitions.hpp"
#include "../config/options.hpp"

#include "../features/skins.h"

using SFHudDeathNoticeAndBotStatus_FireGameEvent_o = void __fastcall(void* thisptr, void*, IGameEvent* event);
using Post_o = void(__thiscall*)(IClientNetworkable * thisptr, int update_type);
vfunc_hook notice_hook;
vfunc_hook postdata_hook;
auto __fastcall SFHudDeathNoticeAndBotStatus_FireGameEvent(void* thisptr, void*, IGameEvent* event) -> void
{
	static auto m_original = notice_hook.get_original<decltype(&SFHudDeathNoticeAndBotStatus_FireGameEvent)>(1);
	// Filter to only the events we're interested in.
	if ((event->GetName()) == ("player_death")
		&& g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")) == g_EngineClient->GetLocalPlayer())
		if (const auto icon_override = g_Options.skins.get_icon_override(event->GetString("weapon")))
			event->SetString("weapon", icon_override);

	m_original(thisptr, nullptr, event);
}

template <typename Fn = void*>
Fn get_vfunc(void* class_base, const std::size_t index)
{
	return (*reinterpret_cast<Fn * *>(class_base))[index];
}


void __stdcall hooked(int update_type)
{
	Skins::OnFrameStageNotify(true);

	postdata_hook.get_original<Post_o>(7)(static_cast<IClientNetworkable*>(g_LocalPlayer), update_type);
}



void applyskinhooks() {
	static bool was_hooked = false;
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer)
	{
		was_hooked = false;
		return;
	}

	if (was_hooked)
		return;

	was_hooked = true;

	const auto network = static_cast<IClientNetworkable*>(g_LocalPlayer);
	postdata_hook.setup(network);
	postdata_hook.hook_index(7, hooked);
}

static auto random_sequence(const int low, const int high) -> int
{
	return rand() % (high - low + 1) + low;
}
static auto fix_animation(const char* model, const int sequence) -> int
{
	enum ESequence
	{
		SEQUENCE_DEFAULT_DRAW = 0,
		SEQUENCE_DEFAULT_IDLE1 = 1,
		SEQUENCE_DEFAULT_IDLE2 = 2,
		SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
		SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
		SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
		SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
		SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
		SEQUENCE_DEFAULT_LOOKAT01 = 12,
		SEQUENCE_BUTTERFLY_DRAW = 0,
		SEQUENCE_BUTTERFLY_DRAW2 = 1,
		SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
		SEQUENCE_BUTTERFLY_LOOKAT03 = 15,
		SEQUENCE_FALCHION_IDLE1 = 1,
		SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
		SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
		SEQUENCE_FALCHION_LOOKAT01 = 12,
		SEQUENCE_FALCHION_LOOKAT02 = 13,
		SEQUENCE_DAGGERS_IDLE1 = 1,
		SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
		SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
		SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
		SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,
		SEQUENCE_BOWIE_IDLE1 = 1,
	};
	if (strstr(model, "models/weapons/v_knife_butterfly.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_DRAW:
			return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
		default:
			return sequence + 1;
		}
	}
	else if (strstr(model, "models/weapons/v_knife_falchion_advanced.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_FALCHION_IDLE1;
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence - 1;
		}
	}
	else if (strstr(model, "models/weapons/v_knife_push.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_DAGGERS_IDLE1;
		case SEQUENCE_DEFAULT_LIGHT_MISS1:
		case SEQUENCE_DEFAULT_LIGHT_MISS2:
			return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
		case SEQUENCE_DEFAULT_HEAVY_HIT1:
		case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		case SEQUENCE_DEFAULT_LOOKAT01:
			return sequence + 3;
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence + 2;
		}
	}
	else if (strstr(model, "models/weapons/v_knife_survival_bowie.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_BOWIE_IDLE1;
		default:
			return sequence;
		}
	}
	else if (strstr(model, "models/weapons/v_knife_ursus.mdl") || strstr(model, "models/weapons/v_knife_canis.mdl") || strstr(model, "models/weapons/v_knife_outdoor.mdl") || strstr(model, "models/weapons/v_knife_cord.mdl") || strstr(model, "models/weapons/v_knife_skeleton.mdl")) {
		switch (sequence )
		{
		case SEQUENCE_DEFAULT_DRAW:
			return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		default:
			return sequence + 1;
		}
	}

	else if (strstr(model, "models/weapons/v_knife_widowmaker.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(14, 15);
		default:
			return sequence;
		}
	}

	else if (strstr(model, "models/weapons/v_knife_stiletto.mdl")) {
		switch (sequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return random_sequence(12, 13);
		default:
			return sequence;
		}
	}
	else {
		return sequence;
	}
}
void Hooks::hkRecvProxy(const CRecvProxyData* pData, void* entity, void* output)
{
	//applyskinhooks();
	static auto original_fn = sequence_hook->get_original_function();
	const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
	if (local && local->IsAlive())
	{
		//FrameStageInSeq();
		
		const auto proxy_data = const_cast<CRecvProxyData*>(pData);
		const auto view_model = static_cast<C_BaseViewModel*>(entity);
		if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid())
		{
			const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
			if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()))
			{
				const auto view_model_weapon_handle = view_model->m_hWeapon();
				if (view_model_weapon_handle.IsValid())
				{
					const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
					if (view_model_weapon)
					{
						if (k_weapon_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex()))
						{
							auto original_sequence = proxy_data->m_Value.m_Int;
							const auto override_model = k_weapon_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
							proxy_data->m_Value.m_Int = fix_animation(override_model, proxy_data->m_Value.m_Int);
						}
					}
				}
			}
		}
	}
	original_fn(pData, entity, output);
}