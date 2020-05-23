#pragma once
#include "../config/options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "aimbot.hpp"
#include <map>
#include <deque>

struct backtrack_data {
	float simTime;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[128];
	matrix3x4_t boneMatrixCur[128];
};

class Backtrack {
public:
	void OnMove(CUserCmd* pCmd);
	std::map<int, std::deque<backtrack_data>> data;
private:

	float correct_time = 0.0f;
	float latency = 0.0f;
	float lerp_time = 0.0f;
	
};
extern Backtrack g_Backtrack;
