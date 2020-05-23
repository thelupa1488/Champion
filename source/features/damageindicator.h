#pragma once

#include "../hooks/hooks.hpp"
#include "../config/options.hpp"
#define MAX_FLOATING_TEXTS 50
class player_hurt_event : public IGameEventListener2
{
public:
	~player_hurt_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};

class bullet_impact_event : public IGameEventListener2
{
public:
	~bullet_impact_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};

struct DamageIndicator_t {
	bool valid = false;
	float startTime = 1.f;
	int damage = 0;
	int hitgroup = 0;
	Vector hitPosition = Vector(0, 0, 0);
	int randomIdx = 0;
};

class DamageIndicators {
public:
	std::vector<DamageIndicator_t> floatingTexts;
	int floatingTextsIdx = 0;
	Vector c_impactpos = Vector(0, 0, 0);
	void paint();
	void Listener();

};

extern DamageIndicators damage_indicators;