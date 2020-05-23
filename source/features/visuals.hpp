#pragma once

#include "../helpers/singleton.hpp"
#include <map>
#include "../render/render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../hooks/c_cs_player_.h"


class bomb_planted_event : public IGameEventListener2
{
public:
	~bomb_planted_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};

class round_end_event : public IGameEventListener2
{
public:
	~round_end_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};



struct SoundInfo_t {
	int guid;
	float soundTime;
	float alpha;
	Vector soundPos;
};

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
private:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			int          dormantfade;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;
		void DrawFlags();
		void RenderCustomSkeleton(matrix3x4_t* mat);
		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderSkeleton();
		void RenderHealth();
		void RenderArmour();
		void debug_visuals();
	};

	std::map< int, std::vector< SoundInfo_t > > m_Sounds;
	CUtlVector<SndInfo_t> sounds;
	void StartExtESP();

	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderDefusingC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void DrawFOV();
	void PingIndicator();
	void RenderSounds();
	void UpdateSounds();
	float CurrentIndicatorHeight = 0.f;
	void DrawString(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...);
public:
	void AddToDrawList();
	void Listener();
	void ThirdPerson();
	void RenderSpectatorList();
	void Render();
};

class VGSHelper : public Singleton<VGSHelper>
{
public:
	void Init();
	void DrawLine(float x1, float y1, float x2, float y2, Color color, float size = 1.f);
};