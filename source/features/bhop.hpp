#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
class C_BasePlayer;

namespace BunnyHop
{
    void OnCreateMove(CUserCmd* cmd);
	void AutoStrafe(CUserCmd* cmd, QAngle va);
}