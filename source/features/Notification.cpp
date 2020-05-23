#include "Notification.h"

CNotification g_Notification;

void CNotification::Push(const Color& clr, const char* msg)
{
	this->m_Notifications.push_back(Notify(clr, 8.f, msg));
}

void CNotification::Draw() 
{
	int x = 8;
	int y = 5;

	int fontTall = ImGui::CalcTextSize("A").y;

	for (int i = 0; i < this->m_Notifications.size(); i++) 
	{
		auto& notify = this->m_Notifications[i];

		auto clr = notify.clr;

		float timeleft = notify.life;

		if (timeleft < .5f)
		{
			float f = std::clamp(timeleft, 0.0f, .5f) / .5f;

			clr[3] = (int)(f * 255.0f);

			if (i == 0 && f < 0.2f)
			{
				y -= fontTall * (1.0f - f / 0.2f);
			}
		}
		else
		{
			clr[3] = 255;
		}

		SIZE sz;

		Render::Get().RenderText(notify.text,x, y, 15, clr);

		y += fontTall;
	}

	for (int i = 0; i < this->m_Notifications.size(); i++)
	{
		auto& notify = this->m_Notifications[i];

		notify.life -= g_GlobalVars->frametime;

		if (notify.life <= 0.0f)
		{
			this->m_Notifications.erase(this->m_Notifications.begin() + i);
		}
	}
}