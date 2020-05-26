#include "hooks.hpp"
#include "../features/skins.h"
#include "../config/options.hpp"
#include "../features/animfixed.h"
//void resolve(C_BasePlayer* player);
extern bool sendpacket;
bool* disablePostProcessing = *(bool**)(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "83 EC 4C 80 3D") + 5);

extern QAngle vangle;
extern matrix3x4_t m_real_matrix[128];
#include "ClientMode.h"
//#include "../features/c_resolver.h"
int entityId = -1;

const char* PadStringRight(std::string text, size_t value)
{
	text.insert(text.length(), value - text.length(), ' ');

	return text.c_str();
}



void update_local_ghetto(C_BasePlayer* local)
{
	static float proper_abs = local->GetPlayerAnimState()->m_flGoalFeetYaw;
	static std::array<float, 24> sent_pose_params = local->m_flPoseParameter();

	static AnimationLayer backup_layers[15];
	static int tick = 0;
	if (tick != g_GlobalVars->tickcount) // Only update animations each tick, though we are doing this each frame.
	{
		tick = g_GlobalVars->tickcount;
		std::memcpy(backup_layers, local->GetAnimOverlays(), (sizeof(AnimationLayer) * local->GetNumAnimOverlays()));
		//local->m_bClientSideAnimation() = true; //just does stuff like set m_bClientSideAnimation and m_iLastAnimUpdateFrameCount
		local->UpdateAnimationState(local->GetPlayerAnimState(), local->m_angEyeAngles());
		if (sendpacket)
		{
			proper_abs = local->GetPlayerAnimState()->m_flGoalFeetYaw;
			sent_pose_params = local->m_flPoseParameter();
		}
	}
	//local->m_bClientSideAnimation() = false;
	local->set_abs_angles(QAngle(0, proper_abs, 0));
	local->GetPlayerAnimState()->m_flUnknownFraction = 0.f; // Lol.
	std::memcpy(local->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * local->GetNumAnimOverlays()));
	local->m_flPoseParameter() = sent_pose_params;
}

bool CanSee[65];
#include "../features/entities.h"
void fixanims(C_BasePlayer* player) {
	if (player->m_flOldSimulationTime() != player->m_flSimulationTime() && player->GetPlayerAnimState())
	{
		const float curtime = g_GlobalVars->curtime;
		const float frametime = g_GlobalVars->frametime;

		static auto host_timescale = g_CVar->FindVar(("host_timescale"));

		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
		g_GlobalVars->curtime = player->m_flOldSimulationTime() + g_GlobalVars->interval_per_tick;

		Vector backup_origin = player->m_vecOrigin();
		Vector backup_absorigin = player->GetAbsOrigin();
		Vector backup_velocity = player->m_vecVelocity();
		int backup_flags = player->m_fFlags();
		int backup_eflags = player->get_eflags();

		AnimationLayer backup_layers[15];
		std::memcpy(backup_layers, player->GetAnimOverlays(), (sizeof(AnimationLayer) * player->GetNumAnimOverlays()));

		if (player->GetPlayerAnimState()->m_bOnGround)
		{
			player->m_fFlags() |= FL_ONGROUND;
		}
		else
		{
			player->m_fFlags() &= ~FL_ONGROUND;
		}
		player->get_eflags() &= ~0x1000;

		player->get_abs_velocity() = player->m_vecVelocity();

		// invalidates prior animations so the entity gets animated on our client 100% via UpdateClientSideAnimation
		if (player->GetPlayerAnimState()->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
			player->GetPlayerAnimState()->m_iLastClientSideAnimationUpdateFramecount = g_GlobalVars->framecount - 1;

		player->m_bClientSideAnimation() = true;

		// updates local animations + poses + calculates new abs angle based on eyeangles and other stuff
		player->UpdateClientSideAnimation();

		player->m_bClientSideAnimation() = false;

		// restore
		std::memcpy(player->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * player->GetNumAnimOverlays()));
		player->m_vecOrigin() = backup_origin;
		player->SetAbsOrigin(backup_absorigin);
		player->m_vecVelocity() = backup_velocity;
		player->m_fFlags() = backup_flags;
		player->get_eflags() = backup_eflags;

		g_GlobalVars->curtime = curtime;
		g_GlobalVars->frametime = frametime;
	}
}
void ResolverLegit(C_BasePlayer* pl) 
{
	if (!pl->GetPlayerAnimState() || pl->m_vecVelocity().Length() > 0 || !g_Options.resolver)
		return;

	if (fabs(std::remainderf(pl->m_flLowerBodyYawTarget(), 360.f) - std::remainderf(pl->m_angEyeAngles().yaw, 360.f)) < 60)
		return;

	auto lbyfix = Math::ClampYaw(std::remainderf(pl->m_flLowerBodyYawTarget() + 180, 360.f));
	pl->GetPlayerAnimState()->m_flGoalFeetYaw = lbyfix;
}
bool changeName(bool reconnect, const char* newName, float delay) noexcept;

void __stdcall Hooks::hkSuppressLists(int a2, bool a3) {
	static auto ofunc = partition_hook.get_original< SuppressLists >(16);
	static float SpawnTime = 0.0f;
	static auto OnRenderStart_Return = Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "FF 50 40 8B 1D ? ? ? ?") + 0x3;
	static auto FrameNetUpdateEnd_Return = Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "5F 5E 5D C2 04 00 83 3D ? ? ? ? ?");

	if (g_LocalPlayer && g_LocalPlayer->IsAlive()) 
	{
		if (_ReturnAddress() == OnRenderStart_Return) 
		{
			if (g_LocalPlayer->GetPlayerAnimState())
				g_LocalPlayer->SetAbsAngles(QAngle(0.0f, g_LocalPlayer->GetPlayerAnimState()->m_flGoalFeetYaw, 0.0f));

			//update_local_ghetto(g_LocalPlayer);
			//	Animation::Get().real_animation(sendpacket, nullptr);

			//if(Globals::cmd)
			//	entities::fetch_targets(Globals::cmd);

			auto state = g_LocalPlayer->GetPlayerAnimState();
			if(state)
				if (g_LocalPlayer->m_vecVelocity().Length2D() < 3.f)
					*reinterpret_cast<float*> (reinterpret_cast<uintptr_t> (state) + 292) = 0.f;

			if (vangle.yaw && g_Input->m_fCameraInThirdPerson)
				g_LocalPlayer->GetVAngles() = vangle;
			
			g_LocalPlayer->m_flFlashMaxAlpha() = g_Options.flash;
			auto csm = g_CVar->FindVar("cl_csm_enabled");
			if (g_Options.esp_optimise) 
			{		
				csm->m_fnChangeCallbacks.m_Size = 0;
				csm->SetValue(false);
				if(disablePostProcessing)
					*disablePostProcessing = true;				
			}
			else 
			{
				csm->m_fnChangeCallbacks.m_Size = 0;
				g_CVar->FindVar("cl_csm_enabled")->SetValue(true);
				if (disablePostProcessing)
					*disablePostProcessing = false;

			}
			for (int i = 1; i < 65; i++)
			{
				auto ent = (C_BasePlayer*)C_BasePlayer::GetEntityByIndex(i);
				if (!ent || !ent->IsAlive())
					continue;
				CanSee[ent->EntIndex()] = g_LocalPlayer->CanSeePlayer(ent, HITBOX_CHEST, true);
				/*if(!ent == g_LocalPlayer)
					fixanims(ent);
				c_resolver::resolve(ent);*/
			}


			//c_resolver::on_render_start();

		}
		else if (_ReturnAddress() == FrameNetUpdateEnd_Return) 
		{
			Skins::OnFrameStageNotify(true);
			//c_animation_system::Get().post_player_update();
			
			//g_LocalPlayer->GetAnimOverlay(7)->m_flWeight = 0.f;
			for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
			{
				auto ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
				if (ent && ent->IsAlive() && !ent->IsDormant() && ent != g_LocalPlayer) 
				{
					ResolverLegit(ent);
					auto VarMap = reinterpret_cast<uintptr_t>(ent) + 36;
					auto VarMapSize = *reinterpret_cast<int*>(VarMap + 20);

					for (auto index = 0; index < VarMapSize; index++)
						*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(VarMap) + index * 12) = 0;
				}
			}
		}
	}
	if(!g_EngineClient->IsInGame()) 
	{
		changeName(true, "", 0.f);
		g_Aimbot.Cleanup();
	}
	ofunc(g_SpatialPartition, a2, a3);
}