
#include "DirectHooks.h"
#include "../config/options.hpp"

long __stdcall Hooks::hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	static auto _reset = direct_hook.get_original<decltype(&hkReset)>(index::Reset);
	Menu::Get().OnDeviceLost();
	auto hr = _reset(device, pPresentationParameters);
	if (hr >= 0) Menu::Get().OnDeviceReset();
	return hr;
}
struct FreeType
{
	enum FontBuildMode
	{
		FontBuildMode_FreeType,
		FontBuildMode_Stb
	};

	FontBuildMode BuildMode;
	bool          WantRebuild;
	float         FontsMultiply;
	int           FontsPadding;
	unsigned int  FontsFlags;

	FreeType()
	{
		BuildMode = FontBuildMode_FreeType;
		WantRebuild = true;
		FontsMultiply = 1.0f;
		FontsPadding = 1;
		FontsFlags = ImGuiFreeType::ForceAutoHint | ImGuiFreeType::LightHinting;//0;// 
	}

	// Call _BEFORE_ NewFrame()
	bool UpdateRebuild()
	{
		if (!WantRebuild)
			return false;

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->TexGlyphPadding = FontsPadding;
		for (int n = 0; n < io.Fonts->ConfigData.Size; n++)
		{
			ImFontConfig* font_config = (ImFontConfig*)&io.Fonts->ConfigData[n];
			font_config->RasterizerMultiply = FontsMultiply;
			font_config->RasterizerFlags = (BuildMode == FontBuildMode_FreeType) ? FontsFlags : 0x00;
		}
		if (BuildMode == FontBuildMode_FreeType)
			ImGuiFreeType::BuildFontAtlas(io.Fonts, FontsFlags);
		else if (BuildMode == FontBuildMode_Stb)
			io.Fonts->Build();
		WantRebuild = false;
		return true;
	}

	// Call to draw interface
	void ShowFreetypeOptionsWindow()
	{
		ImGui::Begin("FreeType Options");
		ImGui::ShowFontSelector("Fonts");
		WantRebuild |= ImGui::DragFloat("Multiply", &FontsMultiply, 0.001f, 0.0f, 2.0f);
		WantRebuild |= ImGui::DragInt("Padding", &FontsPadding, 0.1f, 0, 16);
		if (BuildMode == FontBuildMode_FreeType)
		{
			//ForceAutoHint //LightHinting
			WantRebuild |= ImGui::CheckboxFlags("NoHinting", &FontsFlags, ImGuiFreeType::NoHinting);
			WantRebuild |= ImGui::CheckboxFlags("NoAutoHint", &FontsFlags, ImGuiFreeType::NoAutoHint);
			WantRebuild |= ImGui::CheckboxFlags("ForceAutoHint", &FontsFlags, ImGuiFreeType::ForceAutoHint);
			WantRebuild |= ImGui::CheckboxFlags("LightHinting", &FontsFlags, ImGuiFreeType::LightHinting);
			WantRebuild |= ImGui::CheckboxFlags("MonoHinting", &FontsFlags, ImGuiFreeType::MonoHinting);
			WantRebuild |= ImGui::CheckboxFlags("Bold", &FontsFlags, ImGuiFreeType::Bold);
			WantRebuild |= ImGui::CheckboxFlags("Oblique", &FontsFlags, ImGuiFreeType::Oblique);
			WantRebuild |= ImGui::CheckboxFlags("Monochrome", &FontsFlags, ImGuiFreeType::Monochrome);
			//ImGui::SameLine();
			//ImGui::Checkbox("Auth tab", &auth);
		}
		ImGui::End();
	}
};
FreeType free_type;
long __stdcall Hooks::hkEndScene(IDirect3DDevice9* pDevice)
{
	static auto _end_scene = direct_hook.get_original<decltype(&hkEndScene)>(index::EndScene);


	if (g_Options.misc_antiobs) {
		static uintptr_t gameoverlay_return_address = 0;
			if (!gameoverlay_return_address) {
				MEMORY_BASIC_INFORMATION info;
				VirtualQuery(_ReturnAddress(), &info, sizeof(MEMORY_BASIC_INFORMATION));

				char mod[MAX_PATH];
				GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

				if (strstr(mod, ("gameoverlay")))
					gameoverlay_return_address = (uintptr_t)(_ReturnAddress());
			}

			if (gameoverlay_return_address != (uintptr_t)(_ReturnAddress()))
				return _end_scene(pDevice);
	}
		
	DWORD colorwrite, srgbwrite;
	IDirect3DVertexDeclaration9* vert_dec = nullptr;
	IDirect3DVertexShader9* vert_shader = nullptr;
	DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
	pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
	pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	//removes the source engine color correction
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->GetVertexDeclaration(&vert_dec);
	pDevice->GetVertexShader(&vert_shader);
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);


	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	

	ImDrawList* esp_drawlist = nullptr;
	esp_drawlist = Render::Get().RenderScene();

	Menu::Get().Render(); 

	//Render::Get().BeginScene();


	ImGui::Render(esp_drawlist);
	
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
	pDevice->SetVertexDeclaration(vert_dec);
	pDevice->SetVertexShader(vert_shader);
	
	return _end_scene(pDevice);
}

/*void ImGui::Render(ImDrawList* our_list)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.Initialized);

    if (g.FrameCountEnded != g.FrameCount)
        EndFrame();
    g.FrameCountRendered = g.FrameCount;

    // Gather ImDrawList to render (for each active window)
    g.IO.MetricsRenderVertices = g.IO.MetricsRenderIndices = g.IO.MetricsRenderWindows = 0;
    g.DrawDataBuilder.Clear();

	// Adding our custom drawlist to imgui's drawdata
	if (our_list && !our_list->VtxBuffer.empty())
		AddDrawListToDrawData(&g.DrawDataBuilder.Layers[0], our_list);

    ImGuiWindow* windows_to_render_front_most[2];
    windows_to_render_front_most[0] = (g.NavWindowingTarget && !(g.NavWindowingTarget->Flags & ImGuiWindowFlags_NoBringToFrontOnFocus)) ? g.NavWindowingTarget->RootWindow : NULL;
    windows_to_render_front_most[1] = g.NavWindowingTarget ? g.NavWindowingList : NULL;
    for (int n = 0; n != g.Windows.Size; n++)
    {
        ImGuiWindow* window = g.Windows[n];
        if (IsWindowActiveAndVisible(window) && (window->Flags & ImGuiWindowFlags_ChildWindow) == 0 && window != windows_to_render_front_most[0] && window != windows_to_render_front_most[1])
            AddRootWindowToDrawData(window);
    }
    for (int n = 0; n < IM_ARRAYSIZE(windows_to_render_front_most); n++)
        if (windows_to_render_front_most[n] && IsWindowActiveAndVisible(windows_to_render_front_most[n])) // NavWindowingTarget is always temporarily displayed as the front-most window
            AddRootWindowToDrawData(windows_to_render_front_most[n]);
    g.DrawDataBuilder.FlattenIntoSingleLayer();

    // Draw software mouse cursor if requested
    if (g.IO.MouseDrawCursor)
        RenderMouseCursor(&g.OverlayDrawList, g.IO.MousePos, g.Style.MouseCursorScale, g.MouseCursor);

    if (!g.OverlayDrawList.VtxBuffer.empty())
        AddDrawListToDrawData(&g.DrawDataBuilder.Layers[0], &g.OverlayDrawList);

    // Setup ImDrawData structure for end-user
    SetupDrawData(&g.DrawDataBuilder.Layers[0], &g.DrawData);
    g.IO.MetricsRenderVertices = g.DrawData.TotalVtxCount;
    g.IO.MetricsRenderIndices = g.DrawData.TotalIdxCount;

    // (Legacy) Call the Render callback function. The current prefer way is to let the user retrieve GetDrawData() and call the render function themselves.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
    if (g.DrawData.CmdListsCount > 0 && g.IO.RenderDrawListsFn != NULL)
        g.IO.RenderDrawListsFn(&g.DrawData);
#endif
}
*/