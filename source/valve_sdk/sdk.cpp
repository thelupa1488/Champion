#include "sdk.hpp"

#include "../Helpers/Utils.hpp"

void(__cdecl* RandomSeed)(uint32_t seed);

namespace Interfaces
{
    CreateInterfaceFn get_module_factory(HMODULE module)
    {
        return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
    }

	class InterfaceReg
	{
	private:

		using InstantiateInterfaceFn = void* (*)();

	public:

		InstantiateInterfaceFn m_CreateFn;
		const char* m_pName;

		InterfaceReg* m_pNext;
	};


	template<typename T>
	T* get_interface(const char* modName, const char* ifaceName, bool exact = false)
	{
		T* iface = nullptr;
		InterfaceReg* ifaceRegList;
		int partMatchLen = strlen(ifaceName);

		DWORD ifaceFn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(modName), "CreateInterface"));

		if (!ifaceFn)
			return nullptr;

		unsigned int jmpStart = (unsigned int)(ifaceFn)+4;
		unsigned int jmpTarget = jmpStart + *(unsigned int*)(jmpStart + 1) + 5;

		ifaceRegList = **reinterpret_cast<InterfaceReg***>(jmpTarget + 6);

		for (InterfaceReg* cur = ifaceRegList; cur; cur = cur->m_pNext)
		{
			if (exact == true)
			{
				if (strcmp(cur->m_pName, ifaceName) == 0)
					iface = reinterpret_cast<T*>(cur->m_CreateFn());
			}
			else
			{
				if (!strncmp(cur->m_pName, ifaceName, partMatchLen) && std::atoi(cur->m_pName + partMatchLen) > 0)
					iface = reinterpret_cast<T*>(cur->m_CreateFn());
			}
		}
		return iface;
	}
	/*template <typename isnterface>
	isnterface* GetInterface(const char* filename, const char* version, bool exact = false)
	{
		void* library = dlopen(filename, RTLD_NOLOAD | RTLD_NOW | RTLD_LOCAL);

		if (!library)
			return nullptr;

		void* interfaces_sym = dlsym(library, "s_pInterfaceRegs");

		if (!interfaces_sym)
		{
			dlclose(library);
			return nullptr;
		}

		dlclose(library);

		InterfaceReg* interfaces = *reinterpret_cast<InterfaceReg**>(interfaces_sym);

		InterfaceReg* cur_interface;

		for (cur_interface = interfaces; cur_interface; cur_interface = cur_interface->m_pNext)
		{
			if (exact)
			{
				if (strcmp(cur_interface->m_pName, version) == 0)
					return reinterpret_cast<isnterface*>(cur_interface->m_CreateFn());
			}
			else
			{
				if (strstr(cur_interface->m_pName, version) && strlen(cur_interface->m_pName) - 3 == strlen(version))
					return reinterpret_cast<isnterface*>(cur_interface->m_CreateFn());
			}
		}

		return nullptr;
	}

    template<typename T>
    T* get_interface(CreateInterfaceFn f, const char* szInterfaceVersion)
    {
        auto result = reinterpret_cast<T*>(f(szInterfaceVersion, nullptr));

        if(!result) {
            throw std::runtime_error(std::string("[get_interface] Failed to GetOffset interface: ") + szInterfaceVersion);
        }

        return result;
    }*/

    void Initialize()
    {
        auto engineFactory    = "engine.dll";
		auto clientFactory 	  = "client_panorama.dll";

        auto valveStdFactory  = "vstdlib.dll";
        auto vguiFactory      = "vguimatsurface.dll";
        auto vgui2Factory     = "vgui2.dll";
        auto matSysFactory    = "materialsystem.dll";
        auto dataCacheFactory = "datacache.dll";
        auto vphysicsFactory  = "vphysics.dll";
        auto inputSysFactory  = "inputsystem.dll";
		auto localizeFactory  = "localize.dll";

        g_CHLClient           = get_interface<IBaseClientDLL>      (clientFactory   , "VClient0");
		 g_EntityList          = get_interface<IClientEntityList>   (clientFactory   , "VClientEntityList");
        g_Prediction          = get_interface<IPrediction>         (clientFactory   , "VClientPrediction");
        g_GameMovement        = get_interface<CGameMovement>       (clientFactory   , "GameMovement");
        g_MdlCache            = get_interface<IMDLCache>           (dataCacheFactory, "MDLCache");
        g_EngineClient        = get_interface<IVEngineClient>      (engineFactory   , "VEngineClient");
        g_MdlInfo             = get_interface<IVModelInfoClient>   (engineFactory   , "VModelInfoClient");
        g_MdlRender           = get_interface<IVModelRender>       (engineFactory   , "VEngineModel");
        g_RenderView          = get_interface<IVRenderView>        (engineFactory   , "VEngineRenderView");
        g_EngineTrace         = get_interface<IEngineTrace>        (engineFactory   , "EngineTraceClient");
        g_DebugOverlay        = get_interface<IVDebugOverlay>      (engineFactory   , "VDebugOverlay");
		g_SpatialPartition	  = get_interface<uintptr_t>		   (engineFactory	, "SpatialPartition");
        g_GameEvents          = get_interface<IGameEventManager2>  (engineFactory   , "GAMEEVENTSMANAGER002", true);
        g_EngineSound         = get_interface<IEngineSound>        (engineFactory   , "IEngineSoundClient");
        g_MatSystem           = get_interface<IMaterialSystem>     (matSysFactory   , "VMaterialSystem");
        g_CVar                = get_interface<ICvar>               (valveStdFactory , "VEngineCvar");
        g_VGuiPanel           = get_interface<IPanel>              (vgui2Factory    , "VGUI_Panel");
        g_VGuiSurface         = get_interface<ISurface>            (vguiFactory     , "VGUI_Surface");
        g_PhysSurface         = get_interface<IPhysicsSurfaceProps>(vphysicsFactory , "VPhysicsSurfaceProps");
        g_InputSystem         = get_interface<IInputSystem>        (inputSysFactory , "InputSystemVersion");
		g_Localize			  = get_interface<ILocalize>(localizeFactory, "Localize_");

		

        auto client = GetModuleHandleW(L"client_panorama.dll");
        auto engine = GetModuleHandleW(L"engine.dll");
		auto dx9api = GetModuleHandleW(L"shaderapidx9.dll");

        g_GlobalVars = **(CGlobalVarsBase * **)((*(DWORD * *)g_CHLClient)[0] + 0x1B);
        g_ClientMode = **(IClientMode * **)((*(DWORD * *)g_CHLClient)[10] + 0x5);
		g_Input = *(CInput * *)(Utils::PatternScan(client, "B9 ? ? ? ? F3 0F 11 04 24 FF 50 10") + 1);
		g_MoveHelper      =      **(IMoveHelper***)(Utils::PatternScan(client, "8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01") + 2);
		g_RenderBeams = *(IViewRenderBeams * *)(Utils::PatternScan(client, "A1 ? ? ? ? FF 10 A1 ? ? ? ? B9") + 0x1);
		g_GlowObjManager = *(CGlowObjectManager * *)(Utils::PatternScan(client, "0F 11 05 ? ? ? ? 83 C8 01") + 3);
		g_ViewRender      =        *(IViewRender**)(Utils::PatternScan(client, "A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10") + 1);
		g_PlayerResource = *(C_CSPlayerResource***)(Utils::PatternScan(client, "A1 ? ? ? ? 57 85 C0 74 08") + 0x1);
		g_D3DDevice9 = **(IDirect3DDevice9 * **)(Utils::PatternScan(dx9api, "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
		g_match_framework = **(match_framework***)(Utils::PatternScan(client, "8B 0D ? ? ? ? 8B 01 FF 50 2C 8D 4B 18") + 2);
		g_ClientState     =     **(CClientState***)(Utils::PatternScan(engine, "A1 ? ? ? ? 8B 80 ? ? ? ? C3") + 1);
		g_LocalPlayer = *(C_LocalPlayer*)(Utils::PatternScan(client, "8B 0D ? ? ? ? 83 FF FF 74 07") + 2);
		g_WeaponSystem = *(IWeaponSystem**)(Utils::PatternScan(client, "8B 35 ? ? ? ? FF 10 0F B7 C0") + 2);
		g_pMemAlloc = *(IMemAlloc * *)(GetProcAddress(GetModuleHandleA("tier0.dll"), "g_pMemAlloc"));
		g_GameRules = *(IGameRules***)(Utils::PatternScan(client, "E8 ? ? ? ? A1 ? ? ? ? 85 C0 0F 84 ? ? ? ?") + 0x6);

		RandomSeed = reinterpret_cast<decltype(RandomSeed)>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed"));
    }

    void Dump()
    {
        // Ugly macros ugh
        #define STRINGIFY_IMPL(s) #s
        #define STRINGIFY(s)      STRINGIFY_IMPL(s)
        #define PRINT_INTERFACE(name) Utils::ConsolePrint("%-20s: %p\n", STRINGIFY(name), name)

        PRINT_INTERFACE(g_CHLClient   );
        PRINT_INTERFACE(g_EntityList  );
        PRINT_INTERFACE(g_Prediction  );
        PRINT_INTERFACE(g_GameMovement);
        PRINT_INTERFACE(g_MdlCache    );
        PRINT_INTERFACE(g_EngineClient);
        PRINT_INTERFACE(g_MdlInfo     );
        PRINT_INTERFACE(g_MdlRender   );
        PRINT_INTERFACE(g_RenderView  );
        PRINT_INTERFACE(g_EngineTrace );
        PRINT_INTERFACE(g_DebugOverlay);
        PRINT_INTERFACE(g_GameEvents  );
        PRINT_INTERFACE(g_EngineSound );
        PRINT_INTERFACE(g_MatSystem   );
        PRINT_INTERFACE(g_CVar        );
        PRINT_INTERFACE(g_VGuiPanel   );
        PRINT_INTERFACE(g_VGuiSurface );
        PRINT_INTERFACE(g_PhysSurface );
        PRINT_INTERFACE(g_InputSystem );
    }
}
