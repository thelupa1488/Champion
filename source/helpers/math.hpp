#pragma once

#include "../valve_sdk/sdk.hpp"

#include <DirectXMath.h>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
//#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( g_GlobalVars->interval_per_tick *( t ) )

namespace Math
{
	void FixAngles(QAngle& angle);
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)& x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)& i;
	}
	inline float ClampYaw(float yaw)
	{
		while (yaw > 180.f)
			yaw -= 360.f;
		while (yaw < -180.f)
			yaw += 360.f;
		return yaw;
	}
	float VectorDistance(const Vector& v1, const Vector& v2);
	void VectorScale(float* src, float b, float* dst);
	void VectorScale(const Vector& in, Vector scale, Vector& result);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{

		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] > 180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
	bool IntersectionBoundingBox(const Vector& start, const Vector& dir, const Vector& min, const Vector& max, Vector* hit_point = nullptr);
	void AngleMatrix(const QAngle& angles, matrix3x4_t& matrix);
	void MatrixSetColumn(const Vector& in, int column, matrix3x4_t& out);
	void AngleMatrix(const QAngle& angles, const Vector& position, matrix3x4_t& matrix);
	Vector interpolate(const Vector from, const Vector to, const float percent);
	float interpolate(const float from, const float to, const float percent);
	float RandomFloat(float min, float max);
	void ClampAngles(QAngle& angles);
	void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void vector_i_transform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	void vector_i_rotate(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	bool intersect_line_with_bb(Vector& start, Vector& end, Vector& min, Vector& max);
	float segment_to_segment(const Vector s1, const Vector s2, const Vector k1, const Vector k2);
	Vector vector_rotate(const Vector& in1, const matrix3x4_t& in2);
	Vector vector_rotate(const Vector& in1, const Vector& in2);
	matrix3x4_t angle_matrix(const QAngle angles);
	void AngleVectors(const QAngle& angles, Vector& forward);
	void AngleVectors(const QAngle& angles, Vector& forward, Vector& right, Vector& up);
	void VectorAngles(const Vector& forward, QAngle& angles);
	void VectorAngles(const Vector& forward, Vector& up, QAngle& angles);
	bool WorldToScreen(const Vector& in, Vector& out);
	void MovementFix(CUserCmd* cmd, QAngle wish_angle, QAngle old_angles);
}