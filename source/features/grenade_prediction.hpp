#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../render/render.hpp"
#include "../helpers/math.hpp"
#include "../config/options.hpp"
enum ACT
{
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP,
};

//shonax wtf
class GrenadePrediction : public Singleton<GrenadePrediction>
{
private:
	std::vector<Vector> path;
	std::vector<std::pair<Vector, QAngle>> others;
	//std::vector<Vector> others;
	int type = 0;
	int act = 0;

	void Setup(Vector& vecSrc, Vector& vecThrow, QAngle viewangles);
	void Simulate(CViewSetup* setup);

	int Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval);

	void TraceHull(Vector& src, Vector& end, trace_t& tr, bool hitGlass = 0);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(Vector& src, const Vector& move, trace_t& tr);
	void ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
public:
	void Tick(int buttons);
	void View(CViewSetup* setup);
	void Paint(CUserCmd* cmd);
};