#define NOMINMAX
#include <Windows.h>
#include "config/config.hpp"
#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include "helpers/singleton.hpp"
#include "hooks/hooks.hpp"
#include "render/menu.hpp"
#include "config/options.hpp"
#include "render/render.hpp"
#include "helpers/xorstr.h"
#include <chrono>
#include <thread>
#include <utility>

void* saved_hwnd = nullptr;

auto CheckRemoteDebuggerPresentAPI()
{
	BOOL bIsDbgPresent = FALSE;
	CheckRemoteDebuggerPresent(GetCurrentProcess(), &bIsDbgPresent);
	return bIsDbgPresent;
}

void lockcursor()
{
	if (Menu::Get().IsVisible()) {
		g_VGuiSurface->UnlockCursor();
	}
	else {
		g_VGuiSurface->LockCursor();
	}
	g_Input->m_mouse_initiated = !Menu::Get().IsVisible();

	g_InputSystem->EnableInput(!Menu::Get().IsVisible());

	g_Input->m_mouse_active = Menu::Get().IsVisible();

	g_InputSystem->ResetInputState();
}

void swap_hwnd() 
{
	if (Menu::Get().IsVisible())
	{
		if (!saved_hwnd)
			std::swap(saved_hwnd, g_InputSystem->GetWindow());
	}
	else if (saved_hwnd)
		std::swap(saved_hwnd, g_InputSystem->GetWindow());
}

void LoadStatrack();

#include "features/lazy_importer.hpp"
#include "VMP/VMProtectSDK.h"

DWORD WINAPI OnDllAttach(LPVOID base)
{
	VMP_ULTRA("OnDllAttach");
	while (!GetModuleHandleA("serverbrowser.dll"))
		Sleep(1000);
#ifdef _DEBUG
	Utils::AttachConsole();
#endif
	try 
	{
		Utils::ConsolePrint(_xor_("Initializing...\n").c_str());
		Interfaces::Initialize();
		Interfaces::Dump();
		 
		LoadStatrack();

		InputSys::Get().Initialize();
		NetvarSys::Get().Initialize();
		Render::Get().Initialize();
	
		Menu::Get().Initialize();
		Hooks::Initialize();
		
		if (k_skins.size() == 0)
			initialize_kits();

		InputSys::Get().RegisterHotkey(VK_INSERT, [base]() 
			{
			Menu::Get().Toggle();
			swap_hwnd();
			});
		//InputSys::Get().RegisterHotkey(VK_END, [base]()		{	g_Unload = true;		});

		Utils::ConsolePrint("Finished.\n");		

		while (!g_Unload) { 

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		FreeLibraryAndExitThread((HMODULE)base, 0);
		return false;
	}
	catch (const std::exception& ex) {
		Utils::ConsolePrint("An error occured during initialization:\n");
		Utils::ConsolePrint("%s\n", ex.what());
		Utils::ConsolePrint("Press any key to exit.\n");
		Utils::ConsoleReadKey();
		Utils::DetachConsole();

		FreeLibraryAndExitThread((HMODULE)base, 0);
	}
return false;
    VMP_END;
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
	Utils::DetachConsole();
#endif

	return TRUE;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDll, _In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved)
{
	VMP_ULTRA("DllMain");
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		CreateThread(nullptr, NULL, OnDllAttach, hinstDll, NULL, nullptr);
		
		return TRUE;
	case DLL_PROCESS_DETACH:
		if (lpvReserved == nullptr)
			return OnDllDetach();
		return TRUE;
	default:
		return TRUE;
	}
	VMP_END;
}
