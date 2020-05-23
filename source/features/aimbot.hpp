#pragma once
#include "../config/options.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Aimbot {
public:
	void OnMove(CUserCmd* pCmd);
	bool IsEnabled(CUserCmd* pCmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);
	aimbot_settings settings;
	void Cleanup();
	bool IsRcs();
	float GetSmooth();
	float GetFov();
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
private:
	void RCS(QAngle curang, QAngle& angle, C_BasePlayer* target, bool should_run);	
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsNotSilent(float fov);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone);
	float shot_delay_time = 0;
	bool shot_delay = false;
	bool silent_enabled = false;
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	bool is_delayed = false;
	int kill_delay_time = 0;
	bool kill_delay = false;
	C_BasePlayer* target = NULL;
};
extern Aimbot g_Aimbot;
