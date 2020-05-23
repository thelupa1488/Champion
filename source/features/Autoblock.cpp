#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../helpers/math.hpp"
#include "../config/options.hpp"

void AutoBlock(CUserCmd* cmd)
{
	if (!g_Options.autoblock)
		return;

	if (!GetAsyncKeyState(g_Options.autoblock_bind))
		return;

	C_BasePlayer* localplayer = g_LocalPlayer;
	float bestdist = 200.f;
	int index = -1;

	for (int i = 1; i < 64; i++)
	{
		C_BasePlayer* entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!entity)
			continue;

		if (!entity->IsAlive() || entity->IsDormant() || entity == localplayer)
			continue;

		float dist = localplayer->GetAbsOrigin().DistTo(entity->GetAbsOrigin());

		if (dist < bestdist)
		{
			bestdist = dist;
			index = i;
		}
	}

	if (index == -1)
		return;

	C_BasePlayer* target = (C_BasePlayer*)g_EntityList->GetClientEntity(index);

	if (!target)
		return;

	{

		QAngle angles = Math::CalcAngle(localplayer->GetAbsOrigin(), target->GetAbsOrigin());

		QAngle shit;

		g_EngineClient->GetViewAngles(&shit);

		angles.yaw -= shit.yaw;
		Math::FixAngles(angles);

		if (angles.yaw < 0.20f)
			cmd->sidemove = 450.f;
		else if (angles.yaw > 0.20f)
			cmd->sidemove = -450.f;
	}
}