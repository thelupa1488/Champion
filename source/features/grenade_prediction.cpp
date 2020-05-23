#include "grenade_prediction.hpp"

static bool hitGlassAlready = false;

bool IsGrenade(C_BaseCombatWeapon* weapon)
{
	auto WeaponIDz = weapon->m_Item().m_iItemDefinitionIndex();
	if (WeaponIDz == WEAPON_HEGRENADE || WeaponIDz == WEAPON_INCGRENADE ||
		WeaponIDz == WEAPON_MOLOTOV || WeaponIDz == WEAPON_FLASHBANG ||
		WeaponIDz == WEAPON_DECOY || WeaponIDz == WEAPON_SMOKEGRENADE)
		return true;
	else
		return false;
}

void GrenadePrediction::View(CViewSetup* setup)
{
	if (g_LocalPlayer && g_LocalPlayer->IsAlive())
	{
		auto weapon = g_LocalPlayer->m_hActiveWeapon();
		if (!weapon) return;
		if (IsGrenade(weapon))
		{
			hitGlassAlready = false;
			type = weapon->m_Item().m_iItemDefinitionIndex();
			Simulate(setup);
		}
		else
			type = 0;
	}
}
void GrenadePrediction::Paint(CUserCmd* cmd)
{
	/* if the grenade prediction isn't enabled, return */
	if (!g_Options.esp_grenade_prediction)
		return;
	if (!g_LocalPlayer) return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;

	if (!weapon->IsGrenade()) return;

	if (g_LocalPlayer->m_iHealth() > 0)
	{
		if ((type) && path.size() > 1)
		{
			Vector nadeStart, nadeEnd;
			Vector nadeStart2, nadeEnd2;
			Vector prev = path[0];
			Vector prevoth = others[0].first;
			for (auto it = path.begin(), end = path.end(); it != end; ++it)
			{
				if (Math::WorldToScreen(prev, nadeStart) && Math::WorldToScreen(*it, nadeEnd))
				{
					Render::Get().RenderLine(nadeStart.x, nadeStart.y, nadeEnd.x, nadeEnd.y, Color(255, 255, 255, 255));
				}
				prev = *it;
			}
			for (auto it = others.begin(), end = others.end(); it != end; ++it) {

				
				if (Math::WorldToScreen(prevoth, nadeStart2)) {
					Render::Get().RenderCircleFilled(nadeStart2.x, nadeStart2.y, 4.f, 24, Color::White);
					Render::Get().RenderCircle(nadeStart2.x, nadeStart2.y, 3.6f, 16, Color::Black);
				}

				prevoth = it->first;
			}
		}
	}
}

void GrenadePrediction::Setup(Vector& vecSrc, Vector& vecThrow, QAngle viewangles)
{

	QAngle angThrow = viewangles;
	float pitch = angThrow.pitch;

	if (pitch <= 90.0f)
	{
		if (pitch < -90.0f)
		{
			pitch += 360.0f;
		}
	}
	else
	{
		pitch -= 360.0f;
	}
	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.pitch = a;

	// Gets ThrowVelocity from weapon files
	// Clamped to [15,750]
	float flVel = 750.0f * 0.9f;

	// Do magic on member of grenade object [esi+9E4h]
	// m1=1  m1+m2=0.5  m2=0
	float power = g_LocalPlayer->m_hActiveWeapon()->ThrowStrength();
	float b = power;
	// Clamped to [0,1]
	b = b * 0.7f;
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	Math::AngleVectors(angThrow, vForward, vRight, vUp);

	vecSrc = g_LocalPlayer->GetEyePos();
	float off = (power * 12.0f) - 12.0f;
	vecSrc.z += off;

	// Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest.MulAdd(vecDest, vForward, 22.0f);
	TraceHull(vecSrc, vecDest, tr);

	// After the hull trace it moves 6 units back along vForward
	// vecSrc = tr.endpos - vForward * 6
	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	// Finally calculate velocity
	vecThrow = g_LocalPlayer->m_vecVelocity(); vecThrow *= 1.25f;
	vecThrow.MulAdd(vecThrow, vForward, flVel);
}

void GrenadePrediction::Simulate(CViewSetup* setup)
{
	Vector vecSrc, vecThrow;
	QAngle angles; g_EngineClient->GetViewAngles(&angles);
	Setup(vecSrc, vecThrow, angles);

	float interval = g_GlobalVars->interval_per_tick;
	int logstep = (int)(0.05f / interval);
	int logtimer = 0;

	path.clear(); others.clear();
	for (unsigned int i = 0; i < path.max_size() - 1; ++i)
	{
		if (!logtimer) path.push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval);
		if ((s & 1) || vecThrow == Vector(0, 0, 0))
			break;

		// Reset the log timer every logstep OR we bounced
		if ((s & 2) || logtimer >= logstep) logtimer = 0;
		else ++logtimer;
	}
	path.push_back(vecSrc);
}

#define PI 3.14159265358979323846f

void VectorAngles(const Vector & forward, QAngle & angles)
{
	if (forward[1] == 0.0f && forward[0] == 0.0f)
	{
		angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f;
		angles[1] = 0.0f;
	}
	else
	{
		angles[0] = atan2(-forward[2], forward.Length2D()) * -180 / PI;
		angles[1] = atan2(forward[1], forward[0]) * 180 / PI;

		if (angles[1] > 90) angles[1] -= 180;
		else if (angles[1] < 90) angles[1] += 180;
		else if (angles[1] == 90) angles[1] = 0;
	}

	angles[2] = 0.0f;
}
int GrenadePrediction::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{
	Vector move;
	AddGravityMove(move, vecThrow, interval, false);
	bool hitGlass = false;
	// Push entity
	trace_t tr;
	//PushEntity(vecSrc, move, tr);

	Vector vecAbsEnd = vecSrc;
	vecAbsEnd += move;
	TraceHull(vecSrc, vecAbsEnd, tr, hitGlass);


	if (hitGlass && !hitGlassAlready) {
		vecThrow *= 0.4f;
		hitGlassAlready = true;
	}

	int result = 0;
	// Check ending conditions
	if (CheckDetonate(vecThrow, tr, tick, interval))
	{
		result |= 1;
	}

	if (tr.fraction != 1.0f)
	{
		result |= 2; // Collision!
		ResolveFlyCollisionCustom(tr, vecThrow, interval);
	}
	if ((result & 1) || vecThrow == Vector(0, 0, 0) || tr.fraction != 1.0f)
	{
		QAngle angles;
		VectorAngles((tr.endpos - tr.startpos).Normalized(), angles);
		others.push_back(std::make_pair(tr.endpos, angles));
	}

	vecSrc = tr.endpos;

	return result;
}


bool GrenadePrediction::CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	if (!type)
		return false;
	switch (type)
	{
	case WEAPON_SMOKEGRENADE:
	case WEAPON_DECOY:
		if (vecThrow.Length2D() < 0.1f)
		{
			int det_tick_mod = (int)(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;
		break;

	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
			return true;
		return (float)tick * interval > g_CVar->FindVar("molotov_throw_detonate_time")->GetFloat();
		break;
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return (float)tick * interval > 1.5f && !(tick % (int)(0.2f / interval));
		break;
	default:
		return false;
		break;
	}
}

void GrenadePrediction::TraceHull(Vector& src, Vector& end, trace_t& tr, bool hitGlass)
{
	Ray_t ray;
	ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));
	CTraceFilter filter;
	//filter.ccIgnore = "BaseCSGrenadeProjectile";
	filter.pSkip = g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());
	

	g_EngineTrace->TraceRay(ray, CONTENTS_WINDOW, &filter, &tr);
	if (tr.fraction != 1.0f) {
		hitGlass = true;
	}
	

	int flag = MASK_SOLID;

	g_EngineTrace->TraceRay(ray, flag, &filter, &tr);
}

void GrenadePrediction::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	Vector basevel(0.0f, 0.0f, 0.0f);
	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
		move.z = (vel.z + basevel.z) * frametime;
	else
	{
		float gravity = g_CVar->FindVar("sv_gravity")->GetFloat() * 0.4f;
		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;
		vel.z = newZ;
	}
}

void GrenadePrediction::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	TraceHull(src, vecAbsEnd, tr);
}

void GrenadePrediction::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval)
{
	float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
	float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	// Stop completely once we move too slow
	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
	if (flSpeedSqr < flMinSpeedSqr)
	{
		vecAbsVelocity.Zero();
	}

	// Stop if on ground
	if (tr.plane.normal.z > 0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}
}

int GrenadePrediction::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float    backoff;
	float    change;
	float    angle;
	int        i, blocked;

	blocked = 0;

	angle = normal[2];

	if (angle > 0)
	{
		blocked |= 1;        // floor
	}
	if (!angle)
	{
		blocked |= 2;        // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
		{
			out[i] = 0;
		}
	}

	return blocked;
}