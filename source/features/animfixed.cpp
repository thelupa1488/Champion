#include "animfixed.h"


void Animation::fake_animation(CUserCmd* cmd)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (m_fake_spawntime != g_LocalPlayer->m_flSpawnTime() || m_should_update_fake)
	{
		init_fake_anim = false;
		m_fake_spawntime = g_LocalPlayer->m_flSpawnTime();
		m_should_update_fake = false;
	}

	if (!init_fake_anim)
	{
		m_fake_state = static_cast<CCSGOPlayerAnimState*> (g_pMemAlloc->Alloc(sizeof(CCSGOPlayerAnimState)));

		if (m_fake_state != nullptr)
			g_LocalPlayer->CreateAnimationState(m_fake_state);

		init_fake_anim = true;
	}

	AnimationLayer layer_backup[13];
	float pose_backup[24];

	std::memcpy(layer_backup, g_LocalPlayer->GetAnimOverlays(), sizeof(layer_backup));
	std::memcpy(pose_backup, g_LocalPlayer->m_flPoseParameter().data(), sizeof(pose_backup));

	const auto ba_curtime = g_GlobalVars->curtime;

	g_GlobalVars->curtime = g_LocalPlayer->m_flSimulationTime();

	g_LocalPlayer->UpdateAnimationState(m_fake_state, cmd->viewangles);

	m_fake_rotation = m_fake_state->m_flGoalFeetYaw;
	g_LocalPlayer->set_abs_angles(QAngle(0, m_fake_state->m_flGoalFeetYaw, 0));
	//ctx::client.should_setup_local_bones = true;
	m_got_fake_matrix = g_LocalPlayer->SetupBones(m_fake_matrix, 128, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_ATTACHMENT,
		g_GlobalVars->curtime);

	memcpy(m_fake_position_matrix, m_fake_matrix, sizeof(m_fake_position_matrix));

	const auto org_tmp = g_LocalPlayer->GetRenderOrigin();

	if (m_got_fake_matrix)
	{
		for (auto& i : m_fake_matrix)
		{
			i[0][3] -= org_tmp.x;
			i[1][3] -= org_tmp.y;
			i[2][3] -= org_tmp.z;
		}
	}

	g_LocalPlayer->set_abs_angles(QAngle(0, m_server_abs_rotation, 0)); // restore real abs rotation

	std::memcpy(m_fake_layers.data(), g_LocalPlayer->GetAnimOverlays(), sizeof(m_fake_layers));
	std::memcpy(m_fake_poses.data(), g_LocalPlayer->m_flPoseParameter().data(), sizeof(m_fake_poses));

	g_GlobalVars->curtime = ba_curtime;

	std::memcpy(g_LocalPlayer->GetAnimOverlays(), layer_backup, sizeof(layer_backup));
	std::memcpy(g_LocalPlayer->m_flPoseParameter().data(), pose_backup, sizeof(pose_backup));
}

void Animation::real_animation(const bool send_packet, CUserCmd * cmd)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (m_real_spawntime != g_LocalPlayer->m_flSpawnTime() || m_should_update_real)
	{
		init_real_anim = false;
		m_real_spawntime = g_LocalPlayer->m_flSpawnTime();
		m_should_update_real = false;
	}

	if (!init_real_anim)
	{
		m_real_state = static_cast<CCSGOPlayerAnimState*> (g_pMemAlloc->Alloc(sizeof(CCSGOPlayerAnimState)));

		if (m_real_state != nullptr)
			g_LocalPlayer->CreateAnimationState(m_real_state);

		init_real_anim = true;
	}

	g_LocalPlayer->InvalidateBoneCache();

	g_LocalPlayer->UpdateAnimationState(m_real_state, g_LocalPlayer->m_angEyeAngles());

	if (send_packet)
	{
		m_server_abs_rotation = m_real_state->m_flGoalFeetYaw;
		g_LocalPlayer->set_abs_angles(QAngle(0, m_server_abs_rotation, 0));

		m_got_real_matrix = g_LocalPlayer->SetupBones(m_real_matrix, 128, BONE_USED_BY_ANYTHING, 0.f);
		m_current_real_angle = cmd->viewangles;
		const auto org_tmp = g_LocalPlayer->GetRenderOrigin();

		if (m_got_real_matrix) // todo maybe not use this and set abs yaw in setup_bones,..
		{
			for (auto& i : m_real_matrix)
			{
				i[0][3] -= org_tmp.x;
				i[1][3] -= org_tmp.y;
				i[2][3] -= org_tmp.z;
			}
		}
	}

	if (m_fake_state && m_real_state)
		m_fake_delta = m_server_abs_rotation - m_fake_state->m_flGoalFeetYaw;

	//if ( m_real_state )
	//    console::write_line ( *reinterpret_cast< float* > ( uintptr_t ( m_real_state ) + 0x110 ) );

	std::memcpy(m_real_layers.data(), g_LocalPlayer->GetAnimOverlays(), sizeof(m_real_layers));
	std::memcpy(m_real_poses.data(), g_LocalPlayer->m_flPoseParameter().data(), sizeof(m_real_poses));
}

