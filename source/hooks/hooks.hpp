#pragma once

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/vfunc_hook.hpp"
#include <d3d9.h>



namespace index
{
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 24;
    constexpr auto PlaySound                = 82;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
	constexpr auto ClientModeCreateMove = 24;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
}

namespace Hooks
{
    void Initialize();

	
    void Shutdown();
	inline vgui::HFont espfont;
    inline vfunc_hook hlclient_hook;
	inline std::unique_ptr<c_hook<uint32_t>> player_hook;
	
	inline vfunc_hook direct_hook;
	inline vfunc_hook engine_hook;
	inline vfunc_hook findmdl_hook;
	inline vfunc_hook post_data_update;
	inline vfunc_hook vguipanel_hook;
	inline vfunc_hook gameevents_hook;
	inline recv_prop_hook* sequence_hook;
	inline vfunc_hook vguisurf_hook;
	inline vfunc_hook mdlrender_hook;
	inline vfunc_hook clientstate_hook;
	inline vfunc_hook partition_hook;
	inline vfunc_hook viewrender_hook;
	inline vfunc_hook sound_hook;
	inline vfunc_hook clientmode_hook;
	inline vfunc_hook sv_cheats;
	inline vfunc_hook cl_grenadepreview;
	inline vfunc_hook ccsplayer_vtablehook;
	using SuppressLists = bool(__thiscall*)(void*, int, bool);
	using iFindMdl = MDLHandle_t(__thiscall*)(void*, char*);
	using o_MdlName = const char*(__thiscall*)(void*, void*);
	using EyeAng = QAngle*(__thiscall*)(void*, void*);
	using ReadPacketEntities = void(__thiscall*)(void*, void*);
	using FireEvent = bool(__thiscall*)(IGameEventManager2*, IGameEvent* pEvent);
	using PostUpdate = void(__thiscall*)(IClientNetworkable* thisptr, int update_type);
	typedef bool(__thiscall* add_listener_t)(IGameEventManager2*, IGameEventListener2*, const char*, bool);
	using is_hltv_t = bool(__fastcall*) ();
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	void __fastcall hkEmitSound1(void* _this, int, IRecipientFilter & filter, int iEntIndex, int iChannel, const char * pSoundEntry, unsigned int nSoundEntryHash, const char * pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector * pOrigin, const Vector * pDirection, void * pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
    void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
	long __stdcall	hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
	long __stdcall  hkEndScene(IDirect3DDevice9* pDevice);
	void hkRecvProxy(const CRecvProxyData* pData, void* entity, void* output);
	bool __stdcall hkCreateMove(float input_sample_frametime, CUserCmd* cmd);
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1);
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView);
	using CreateMoveClientMode = bool(__thiscall*)(IClientMode*, float, CUserCmd*);
	void __fastcall hkReadPacketEntities(void* pClientState, void* pEDX, void* pEntInfo);
	bool AddCCSPlayerListener(IGameEventListener2* listener, const char* name, const bool serverside);
	void __stdcall hkSuppressLists(int a2, bool a3);
	void __fastcall hkLockCursor(void* _this);
   
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx);
	bool __fastcall hkgrenadepreviewGetBool(PVOID pConVar, void* edx);
}
