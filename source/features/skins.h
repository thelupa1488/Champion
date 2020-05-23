#include "../config/options.hpp"
#include "../valve_sdk/sdk.hpp"
#include <limits> 

class player_death_event : public IGameEventListener2
{
public:
	~player_death_event() { g_GameEvents->RemoveListener(this); }

	virtual void FireGameEvent(IGameEvent* p_event);
};



namespace Skins {
	void Listener();
	void OnFrameStageNotify(bool frame_end);
};