#pragma once
#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"




class Animation : public Singleton<Animation>
{

public:

	void fake_animation(CUserCmd* cmd);
	void real_animation(const bool send_packet, CUserCmd* cmd);
	matrix3x4_t m_real_matrix[128];
private:

	float m_server_abs_rotation = 0.f;
	float m_real_spawntime = 0.f;
	bool m_should_update_real = false;
	bool init_real_anim = false;
	std::array< AnimationLayer, 13 > m_real_layers;
	std::array< float, 24 > m_real_poses;
	
	bool m_got_real_matrix = false;
	CCSGOPlayerAnimState* m_real_state = nullptr;
	QAngle m_current_real_angle = QAngle(0.f, 0.f, 0.f);
	float m_fake_delta = 0.f;
	CCSGOPlayerAnimState* m_fake_state = nullptr;
	matrix3x4_t m_fake_matrix[128];
	std::array< AnimationLayer, 13 > m_fake_layers;
	std::array< float, 24 > m_fake_poses;
	bool init_fake_anim = false;
	bool m_should_update_fake = false;
	float m_fake_spawntime = 0.f;
	float m_fake_rotation = 0.f;
	bool m_got_fake_matrix = false;
	matrix3x4_t m_fake_position_matrix[128];
};