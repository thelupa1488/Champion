#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../config/options.hpp"
#include "../hooks/hooks.hpp"
#include "../helpers/input.hpp"
#include "animfixed.h"

Chams::Chams()
{
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube", "Model textures");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat", "Model textures");
	materialGold = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
	materialGlass = g_MatSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
	materialGloss = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER);
	materialCrystal = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER);
	materialCrystalBlue = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", TEXTURE_GROUP_OTHER);
	materialSilver = g_MatSystem->FindMaterial("models/inventory_items/hydra_crystal/hydra_crystal_detail", TEXTURE_GROUP_OTHER);
	
		
}

Chams::~Chams()
{
}

void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba, bool modulate, bool gold, bool crystal, bool crystal2, bool silver)
{
	IMaterial* material = nullptr;

	if (modulate) {
		if (flat)
			material = materialFlat;
		else if (gold)
			material = materialGold;
		else if (crystal)
			material = materialCrystal;
		else if (glass)
			material = materialGlass;
		else if (crystal2)
			material = materialCrystalBlue;
		else if(silver)
			material = materialSilver;
		else
			material = materialRegular;
	}

	bool bFound = false;
	IMaterialVar* pMatVar = material->FindVar("$envmaptint", &bFound);
	//if (bFound)
	//	pMatVar->SetVecValue(rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);

	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	material->AlphaModulate(
		rgba.a() / 255.0f);

	material->IncrementReferenceCount();

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	if (is_player && g_Options.chams_player_enabled) {
		// 
		// Draw player Chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto clr_front = Color(g_Options.color_chams_player_visible);
			const auto clr_back = Color(g_Options.color_chams_player_occluded);

			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			//if (g_Options.chams_player_fake) {
			//	if (ent == g_LocalPlayer)
			//	{
			//		for (auto& i : Animation::Get().m_real_matrix)
			//		{
			//			i[0][3] += info.origin.x;
			//			i[1][3] += info.origin.y;
			//			i[2][3] += info.origin.z;
			//		}
			//		OverrideMaterial(
			//			false,
			//			false,
			//			false,
			//			false,
			//			Color(g_Options.color_chams_real));
			//		fnDME(g_MdlRender, 0, ctx, state, info, Animation::Get().m_real_matrix);
			//		for (auto& i : Animation::Get().m_real_matrix)
			//		{
			//			i[0][3] -= info.origin.x;
			//			i[1][3] -= info.origin.y;
			//			i[2][3] -= info.origin.z;
			//		}
			//	}
			//}

			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			

			if (!g_Options.chams_player_visibleonly) {
				OverrideMaterial(
					true,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_back);

				fnDME(g_MdlRender, 0, ctx, state, info, matrix);

				g_MdlRender->ForcedMaterialOverride(nullptr);

				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					clr_front);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					g_Options.chams_player_glass,
					clr_front);
				//g_RenderView->SetColorModulation(clr_front.r() / 255, clr_front.g() / 255, clr_front.b() / 255);
				//g_MdlRender->ForcedMaterialOverride(chamsmat);
			}
		}
	}
	
	else if (is_sleeve && g_Options.chams_sleeves_enabled) {
		
		OverrideMaterial(
			false,
			g_Options.chams_sleeves_mat == 1,
			g_Options.chams_sleeves_wireframe,
			g_Options.chams_sleeves_mat == 3,
			Color(g_Options.chams_sleeves_color),
			true,
			g_Options.chams_sleeves_mat == 4,
			g_Options.chams_sleeves_mat == 2,
			g_Options.chams_sleeves_mat == 5,
			g_Options.chams_sleeves_mat == 6
			);
		if (g_Options.chams_sleeves_reflect) {
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);

			g_MdlRender->ForcedMaterialOverride(nullptr);

			g_MdlRender->ForcedMaterialOverride(materialGloss);
		}
	}
	else if (!is_arm && !is_sleeve && is_weapon && g_Options.chams_weapon_enabled)
	{
		OverrideMaterial(
			false,
			g_Options.chams_weapon_mat == 1,
			g_Options.chams_weapon_wireframe,
			g_Options.chams_weapon_mat == 3,
			Color(g_Options.chams_weapon_color),
			true,
			g_Options.chams_weapon_mat == 4,
			g_Options.chams_weapon_mat == 2,
			g_Options.chams_weapon_mat == 5,
			g_Options.chams_weapon_mat == 6
		);
		if (g_Options.chams_weapon_reflect) {
			fnDME(g_MdlRender, 0, ctx, state, info, matrix);

			g_MdlRender->ForcedMaterialOverride(nullptr);

			g_MdlRender->ForcedMaterialOverride(materialGloss);
		}

	}
	else if (is_arm && !is_sleeve) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.other_no_hands) {
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			
			OverrideMaterial(
				false,
				g_Options.chams_arms_mat == 1,
				g_Options.chams_arms_wireframe,
				g_Options.chams_arms_mat == 3,
				Color(g_Options.chams_arms_color),
				true,
				g_Options.chams_arms_mat == 4,
				g_Options.chams_arms_mat == 2,
				g_Options.chams_arms_mat == 5,
				g_Options.chams_arms_mat == 6
			);
			if (g_Options.chams_arms_reflect) {
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);

				g_MdlRender->ForcedMaterialOverride(nullptr);

				g_MdlRender->ForcedMaterialOverride(materialGloss);
			}
		}
	}
}