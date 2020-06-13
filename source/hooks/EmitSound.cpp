#include "hooks.hpp"
#include "../config/options.hpp"
#include "..\helpers\input.hpp"
void AutoAccept()
{
	if (g_Options.misc_autoaccept && !g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
	{
		static auto SetLocalPlayerReadyFn = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA(("client.dll")), ("55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12")));

		if (SetLocalPlayerReadyFn) 
		{
			SetLocalPlayerReadyFn("");

			FLASHWINFO fi;
			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = InputSys::Get().GetMainWindow();
			fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.uCount = 0;
			fi.dwTimeout = 0;
			FlashWindowEx(&fi);
		}
	}
}
void __fastcall Hooks::hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
	static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);

	if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep"))
		AutoAccept();

	ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
}
