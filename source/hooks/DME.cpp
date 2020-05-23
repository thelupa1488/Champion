#include "hooks.hpp"
#include "../features/chams.hpp"
extern matrix3x4_t m_real_matrix[128];
extern bool m_got_real_matrix;
void __fastcall Hooks::hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
{
	static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

	if (g_MdlRender->IsForcedMaterialOverride() &&
		!strstr(pInfo.pModel->szName, "arms") &&
		!strstr(pInfo.pModel->szName, "weapons/v_")) {
		return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
	}
	/*auto entity = dynamic_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(pInfo.entity_index));

	if (entity && entity->IsPlayer() && entity == g_LocalPlayer)
	{
		if (!m_got_real_matrix)
			return;

		if (!pCustomBoneToWorld)
			return;

		memcpy(pCustomBoneToWorld, m_real_matrix, sizeof(m_real_matrix));

		for (auto i = 0; i < 128; i++)
		{
			pCustomBoneToWorld[i][0][3] += pInfo.origin.x;
			pCustomBoneToWorld[i][1][3] += pInfo.origin.y;
			pCustomBoneToWorld[i][2][3] += pInfo.origin.z;
		}
	}*/
	Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);
	ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
	g_MdlRender->ForcedMaterialOverride(nullptr);
}