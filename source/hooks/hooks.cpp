#include "hooks.hpp"
#include <intrin.h>  
#include "../config/options.hpp"
#include "../helpers/input.hpp"
#include "../helpers/utils.hpp"
#include "../features/glow.hpp"
#include "../features/skins.h"
#include "../features/visuals.hpp"
//#include "features/c_events.h"
#pragma comment(lib,"ws2_32")
#include "../features/AnimationSystem.h"
#include "../features/Notification.h"



#pragma intrinsic(_ReturnAddress)  

float side = 1.0f;
float next_lby = 0.0f;

//void ApplyStickers(C_BaseCombatWeapon* pItem);

#include "../helpers/xorstr.h"
#include <locale>
#include <chrono>

#include "../features/damageindicator.h"
#include "DirectHooks.h"
#include "c_cs_player_.h"
#include "ClientMode.h"

void FakePrime()
{
	static auto prime = Utils::PatternScan(GetModuleHandleA("client.dll"), "A1 ? ? ? ? 85 C0 75 07 83 F8 05 0F 94 C0 C3");
	DWORD old_protect;
	VirtualProtect(prime, 5, PAGE_EXECUTE_READWRITE, &old_protect);
	{
		char primetrue[] = { 0x31,  0xC0,  0xFE ,  0xC0,  0xC3 };
		char primefalse[] = { 0x31, 0xC0, 0xC3 };

		if (!g_Options.misc_fake_prime) memcpy(prime, primefalse, 5);
		else memcpy(prime, primetrue, 5);
	}
	VirtualProtect(prime, 5, old_protect, nullptr);
}
void NightMode() 
{
	static bool OldNightmode;
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		OldNightmode = false;
		return;
	}
	ConVar* staticdrop;
	ConVar* r_3dsky;
	if (OldNightmode != g_Options.other_nightmode)
	{

		staticdrop = g_CVar->FindVar("r_DrawSpecificStaticProp");
		r_3dsky = g_CVar->FindVar("r_3dsky");
		staticdrop->SetValue(0);
		for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
		{
			IMaterial* pMaterial = g_MatSystem->GetMaterial(i);
			if (!pMaterial || pMaterial->IsErrorMaterial())
				continue;

			if (pMaterial->GetReferenceCount() <= 0) {
				continue;
			}

			if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp") || (strstr(pMaterial->GetTextureGroupName(), "Model")))
			{


				if (g_Options.other_nightmode) {

					Utils::LoadNamedSkys("sky_descent");

					if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
					{
						bool bFound = false;
						IMaterialVar* pMatVar = pMaterial->FindVar("$envmaptint", &bFound);
						if (bFound)
							(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pMatVar + 0x2c))((uintptr_t)pMatVar, 0.2f, 0.2f, 0.2f);

						pMaterial->ColorModulate(0.2f, 0.2f, 0.2f);
					}
					else if (strstr(pMaterial->GetTextureGroupName(), "Model"))
						pMaterial->ColorModulate(0.6f, 0.6f, 0.6f);
					else
						pMaterial->ColorModulate(0.2f, 0.2f, 0.2f);

					r_3dsky->SetValue(0);
				}
				else {
					Utils::LoadNamedSkys("sky_cs15_daylight04_hdr");
					pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
					r_3dsky->SetValue(1);
				}
			}
		}
		OldNightmode = g_Options.other_nightmode;
	}
}

namespace Hooks 
{
	bool __fastcall hk_is_hltv()
	{
		const auto org_f = engine_hook.get_original< is_hltv_t >(93);

		if(!g_EngineClient->IsInGame())
			return org_f();

		static auto ptr_accumulate_layers = Utils::PatternScan(GetModuleHandleA("client.dll"), ("84 C0 75 0D F6 87"));
		static auto setupvelocity_call = reinterpret_cast<void*> (Utils::PatternScan(GetModuleHandleA("client.dll"),
			("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0")));

		if (reinterpret_cast<uintptr_t> (_ReturnAddress()) == reinterpret_cast<uintptr_t> (ptr_accumulate_layers) && c_animation_system::Get().enable_bones)
			return true;

		if (reinterpret_cast<uintptr_t> (_ReturnAddress()) == reinterpret_cast<uintptr_t> (setupvelocity_call))
			return true;

		return org_f();
	}

	MDLHandle_t __fastcall HK_FindMDL(void* ecx, void* edx, char* FilePath)
	{
		auto oFindMdl = findmdl_hook.get_original<iFindMdl>(10);		

		if (strstr(FilePath, "v_rif_ak47.mdl"))
		{
			sprintf(FilePath, "materials/weapons/");
		}

		return oFindMdl(ecx, FilePath);
	}

	const char* __fastcall HK_GetModelName(void* pEcx, void* pEdx, void* pModel)
	{
		uintptr_t* uipStackPointer;

		static auto pGetModelName = findmdl_hook.get_original<o_MdlName>(18);

		static auto uiGetModelNameReturnAddress = (uintptr_t)Utils::PatternScan(GetModuleHandleA("client.dll"), "EB 23 8B 7D FC 8B CF 53 FF 75 08");

		__asm
		{
			mov uipStackPointer, ebp;
		}

		if (uipStackPointer[1] == uiGetModelNameReturnAddress)
		{
			uipStackPointer[1] += 2;

			return 0;
		}

		return pGetModelName(pEcx, pModel);
	}

	

	void EventListeners() {

		damage_indicators.Listener();
		Visuals::Get().Listener();
	}
	void Initialize()
	{
		VMP_ULTRA("Hooks Initialize");
		//ConVar* blur = g_CVar->FindVar("@panorama_disable_blur"); blur->SetValue(1);
		EventListeners();
		direct_hook.setup(g_D3DDevice9);
		direct_hook.hook_index(index::Reset, hkReset);
		direct_hook.hook_index(index::EndScene, hkEndScene);
		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), hkRecvProxy);

		clientstate_hook.setup((void*)((uintptr_t)g_ClientState + 0x8));
		clientstate_hook.hook_index(64, hkReadPacketEntities);

		partition_hook.setup(g_SpatialPartition);
		partition_hook.hook_index(16, hkSuppressLists);


		clientmode_hook.setup(g_ClientMode);
		clientmode_hook.hook_index(index::CreateMove, hkCreateMove);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);

		mdlrender_hook.setup(g_MdlRender);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);

		vguipanel_hook.setup(g_VGuiPanel);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);

		VMP_END;
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		Glow::Get().Shutdown();
		sequence_hook->~recv_prop_hook();
#ifdef _DEBUG
		partition_hook.unhook_all();
		direct_hook.unhook_all();
		clientstate_hook.unhook_all();
		vguipanel_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sv_cheats.unhook_all();
#endif
	}
	//--------------------------------------------------------------------------------

	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);


		if (!strcmp("HudZoom", g_VGuiPanel->GetName(panel)) && g_Options.nozoom)
			return;

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "MatSystemTopPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) {
			//AsusProps();
			NightMode();
			Render::Get().BeginScene();
			FakePrime();
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkReadPacketEntities(void* pClientState, void* pEDX, void* pEntInfo) {
		static auto original = clientstate_hook.get_original<ReadPacketEntities>(64);

		original(pClientState, pEntInfo);
		Skins::OnFrameStageNotify(false);
		//if(g_EngineClient->IsInGame() && g_LocalPlayer)
		//	animation_system.server_layers = *g_LocalPlayer->GetAnimOverlays();
	}

	bool AddCCSPlayerListener(IGameEventListener2* listener, const char* name, const bool serverside)
	{
		static const auto return_to_c_csplayer_ctor = reinterpret_cast<void*>(
			reinterpret_cast<uint32_t>(Utils::PatternScan(GetModuleHandle(L"client.dll"), "FF 50 0C C7 87")) + 3);

		static auto _add_listener = gameevents_hook.get_original<add_listener_t>(3);

		if (_ReturnAddress() == return_to_c_csplayer_ctor)
		{
			const auto player = reinterpret_cast<C_BasePlayer*>(
				reinterpret_cast<uint32_t>(listener) - 0x3870);

			C_BasePlayer_::apply_to_player(player);
		}

		return _add_listener(g_GameEvents, listener, name, serverside);
	}
	

	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		
		return ofunc(pConVar);
	}

	bool __fastcall hkgrenadepreviewGetBool(PVOID pConVar, void* edx)
	{
		static auto ofunc = sv_cheats.get_original<bool(__thiscall*)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (g_Options.esp_grenade_prediction)
			return true;
		return ofunc(pConVar);
	}
	
}
