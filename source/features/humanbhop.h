#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../render/render.hpp"
#include "../helpers/math.hpp"
#include "../config/options.hpp"

class HumanBhop : public Singleton<HumanBhop>
{
public:
	void Human_Bhop(CUserCmd* cmd);
};