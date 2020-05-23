#pragma once
#include "hooks.hpp"
#include "../render/menu.hpp"
#include "../render/render.hpp"

class Direct : public Singleton<Direct> {
	typedef long(__stdcall* reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	typedef long(__stdcall* present_t)(IDirect3DDevice9*);
public:
	static void Hook();
private:	
	inline static reset_t _reset;
	inline static present_t _end_scene;
	static long __stdcall hkEndScene(IDirect3DDevice9* device);
	static long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
};