#include "hooks.hpp"
#include"../render/menu.hpp"

void __fastcall Hooks::hkLockCursor(void* _this)
{
	static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

	if (Menu::Get().IsVisible()) {
		g_VGuiSurface->UnlockCursor();
		g_InputSystem->ResetInputState();
		return;
	}
	ofunc(g_VGuiSurface);

}