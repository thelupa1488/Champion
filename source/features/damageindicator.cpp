#include "DamageIndicator.h"
#include "../helpers/math.hpp"
#include "../render/render.hpp"
player_hurt_event player_hurt_listener;
bullet_impact_event bullet_impact_listener;
DamageIndicators damage_indicators;

void DamageIndicators::paint() {

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer || !g_Options.esp_damageindicator) {
		floatingTexts.empty();	
		floatingTexts.clear();
			return;
	}
	if (!floatingTexts.size())
		return;
	for (int i = 0; i < floatingTexts.size(); i++)
	{
		DamageIndicator_t* txt = &floatingTexts.at(i);

		if (!txt->valid)
			continue;

		
		

		float endTime = txt->startTime + 2.5f;

		if (endTime < g_GlobalVars->curtime)
		{
			txt->valid = false;
			continue;
		}

		Vector origin_screen;

		if (!Math::WorldToScreen(txt->hitPosition, origin_screen))
			return;

		float t = 1.0f - (endTime - g_GlobalVars->curtime) / (endTime - txt->startTime);

		origin_screen.y -= t * (35.0f);
		origin_screen.x -= (float)txt->randomIdx * t * 3.0f;

	
		char msg[12];
		if (g_Options.esp_damageindicator_color)
			sprintf_s(msg, 12, "%dHP", txt->damage);
		else
			sprintf_s(msg, 12, "-%dHP", txt->damage);


		Color damage_color;
		if (txt->damage >= 66 && txt->damage <= 800)
			damage_color = Color(255, 0, 0, 255);
		else if (txt->damage >= 33 && txt->damage <= 65)
			damage_color = Color(255, 160, 0, 255);
		else if (txt->damage >= 0 && txt->damage <= 32)
			damage_color = Color(0, 255, 0, 255);

		Render::Get().RenderText(msg, ImVec2(origin_screen.x, origin_screen.y), 15.f, damage_color, true);
	}
}

void DamageIndicators::Listener()
{
	g_GameEvents->AddListener(&player_hurt_listener, "player_hurt", false);
	g_GameEvents->AddListener(&bullet_impact_listener, "bullet_impact", false);
}
void player_hurt_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (!strstr(p_event->GetName(), "player_hurt")) 
		return;
	C_BasePlayer* hurt = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(p_event->GetInt("userid"))));
	C_BasePlayer* attacker = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(p_event->GetInt("attacker")));

	if (hurt != g_LocalPlayer && attacker == g_LocalPlayer)
	{
		int dmg_health = p_event->GetInt("dmg_health");
		int hitgroup = p_event->GetInt("hitgroup");

		if (g_Options.esp_damageindicator_color)
			dmg_health = hurt->m_iHealth() - dmg_health;

		if (dmg_health < 0)
			dmg_health = 0;

		DamageIndicator_t DmgIndicator;
		DmgIndicator.startTime = g_GlobalVars->curtime;
		DmgIndicator.hitgroup = hitgroup;
		DmgIndicator.hitPosition = damage_indicators.c_impactpos;
		DmgIndicator.damage = dmg_health;
		DmgIndicator.randomIdx = Math::RandomFloat(-20.f, 20.f);
		DmgIndicator.valid = true;

		if(damage_indicators.c_impactpos.x)
			damage_indicators.floatingTexts.push_back(DmgIndicator);

		if (g_Options.hitsound)
			g_VGuiSurface->PlaySound_("buttons\\arena_switch_press_02.wav");
	}
	
}

void bullet_impact_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (!strstr(p_event->GetName(), "bullet_impact"))
		return;


	float x = p_event->GetFloat("x");
	float y = p_event->GetFloat("y");
	float z = p_event->GetFloat("z");

	C_BasePlayer* target = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(p_event->GetInt("userid"))));

	if (!target || target != g_LocalPlayer)
		return;

	damage_indicators.c_impactpos = Vector(x, y, z);

}
