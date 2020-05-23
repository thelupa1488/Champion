#pragma once
#include <string>
#include <vector>
#include "../valve_sdk/misc/Color.hpp"
#include "../helpers/utils.hpp"
#include "../render/render.hpp"
#include "../valve_sdk/csgostructs.hpp"

class CNotification
{
	public:
		void Push(const Color& clr, const char* msg);
		void Draw();

	private:
		struct Notify
		{
			Notify(Color clr = Color(255, 255, 255), float life = 0.f, std::string text = "") 
			{
				this->clr = clr;
				this->life = life;
				this->text = text;
			}

			Color clr;
			float life;
			std::string text;
		};

		std::vector<Notify> m_Notifications;
};

extern CNotification g_Notification;