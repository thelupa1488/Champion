#include "render.hpp"

#include <mutex>

#include "../features/visuals.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/input.hpp"
#include "menu.hpp"
#include "../hooks/hooks.hpp"
#include "../config/options.hpp"
#include "../fonts/fonts.hpp"
#include "../helpers/math.hpp"

ImFont* g_pDefaultFont;
ImFont* g_pSecondFont;
ImFont* g_pDerekFont;

ImFont* g_Font1;
ImFont* g_Font2;

IDirect3DTexture9* brand_img = nullptr;
IDirect3DTexture9* bgbrand_img = nullptr;

ImDrawListSharedData _data;

std::mutex render_mutex;

void Render::Initialize()
{
	ImGui::CreateContext();

	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();

	//ImGuiFreeType::BuildFontAtlas(ImGui::GetIO().Fonts);
}

void Render::GetFonts() {
	// menu font
	ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
		Fonts::bahnschrift,
		sizeof(Fonts::bahnschrift),
		13.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()
	);

	// esp font
	g_pDefaultFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
		Fonts::bahnschrift,
		sizeof(Fonts::bahnschrift),
		14.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()
	);
		
	// tab font
	g_pDerekFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Derek_compressed_data,
		Fonts::Derek_compressed_size,
		40.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// font for watermark; just example
	g_pSecondFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
		Fonts::bahnschrift,
		sizeof(Fonts::bahnschrift),
		24.f,
		nullptr,
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()
	);
}

void Render::ClearDrawList() {
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}
float frameRate = 0.f;
void watermark() {
	if (g_Options.misc_watermark)
	{
		int width, height;
		g_EngineClient->GetScreenSize(width, height);


		frameRate = 0.9f * frameRate + (1.0 - 0.9f) * g_GlobalVars->absoluteframetime;
		std::stringstream ss;

		auto net_channel = g_EngineClient->GetNetChannelInfo();
		auto latency = (net_channel && g_EngineClient->IsInGame() && !g_EngineClient->IsPlayingDemo()) ? net_channel->GetAvgLatency(FLOW_OUTGOING) : 0.0f;

		// Корректировка значения из source sdk 2013.
		static auto cl_updaterate = g_CVar->FindVar("cl_updaterate");
		latency -= 0.5f / cl_updaterate->GetFloat();

		// Финальная корректировка значения и приведение его в тип std::string.
		auto ping = std::to_string((int)(std::fmax(0.0f, latency) * 1000.0f));


		Render::Get().RenderBoxFilled(12, 10, 250, 50, Color(26, 27, 22, 80)); // background

		Render::Get().RenderCoalBox(12, 10, 250, 50, Color(207, 135, 15, 255)); // linhas

		ss << "Champion | THELUPA | " << "fps:" << static_cast<int>(1.f / frameRate) << " | ping: " << ping.c_str() << "ms";

		Render::Get().RenderText(ss.str(), ImVec2(35, 23), 13.f, Color(255, 255, 255, 255), false, false);
	}
}
extern float side;
extern float delta_time;
void Arrows(float screen_width, float screen_height) {
	auto client_viewangles = QAngle();
	g_EngineClient->GetViewAngles(&client_viewangles);
	const auto screen_center = Vector2D(screen_width / 2.f, screen_height / 2.f);

	constexpr auto radius = 225.f;
	auto draw_arrow = [&](float rot, Color color) -> void
	{
		std::vector<ImVec2> vertices;
		vertices.push_back((ImVec2(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
		vertices.push_back((ImVec2(screen_center.x + cosf(rot + DEG2RAD(2)) * (radius - 6), screen_center.y + sinf(rot + DEG2RAD(2)) * (radius - 6)))); //25
		vertices.push_back((ImVec2(screen_center.x + cosf(rot - DEG2RAD(2)) * (radius - 6), screen_center.y + sinf(rot - DEG2RAD(2)) * (radius - 6)))); //25

		/*static int texture_id = g_VGuiSurface->CreateNewTextureID(true); // 
		static unsigned char buf[4] = { 255, 255, 255, 255 };
		g_VGuiSurface->DrawSetTextureRGBA(texture_id, buf, 1, 1); //
		g_VGuiSurface->DrawSetColor(color); //
		g_VGuiSurface->DrawSetTexture(texture_id); //
		g_VGuiSurface->DrawTexturedPolygon(3, ); //*/
		Render::Get().RenderTriangle(vertices.at(0), vertices.at(1), vertices.at(2), Color::White, 2.f);
	};

	static auto alpha = 0.f; static auto plus_or_minus = false;
	if (alpha <= 0.f || alpha >= 255.f) plus_or_minus = !plus_or_minus;
	alpha += plus_or_minus ? (255.f / 7 * 0.015) : -(255.f / 7 * 0.015); alpha = std::clamp(alpha, 0.f, 255.f);

	auto fake_color = Color(200, 0, 0, 170);
	const auto fake_rot = DEG2RAD((side < 0.0f ? 90 : -90) - 90);
	draw_arrow(fake_rot, fake_color);


}

#include "../features/backdrop.hpp"
void Render::BeginScene() {
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();

	static float alpha;
	if (Menu::Get().IsVisible() && alpha < 0.5)			alpha += 0.01;
	else if (!Menu::Get().IsVisible())		alpha = 0.01;
	if (Menu::Get().IsVisible())
		BackDrop::DrawBackDrop();

	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(w, h), ImGui::GetColorU32(ImVec4(0, 0, 0, alpha)));

	float x = w * 0.5f;
	float y = h * 0.5f;

	//if (Menu::Get().IsVisible())
	//	draw_list->AddImage(bgbrand_img, ImVec2(x, y), ImVec2(x + 200, y + 140));

	watermark();

	if (g_EngineClient->IsInGame() && g_LocalPlayer)
	{
		if (g_Options.misc_desync)
			Arrows(w,h);
		
		if (g_Options.nozoom && g_LocalPlayer->m_bIsScoped() && g_LocalPlayer->m_hActiveWeapon() && 
			g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_SG556 &&
			g_LocalPlayer->m_hActiveWeapon()->m_iItemDefinitionIndex() != WEAPON_AUG)
		{
			Render::Get().RenderLine((float)0, y, (float)w, y, Color::Black);
			Render::Get().RenderLine(x, (float)0, x, (float)h, Color::Black);
		}
		//if (g_Options.misc_desync)
		//	Render::Get().RenderText(side < 0.0f ? ">" : " <", side > 0.0f ? x - 120 : x + 120, y - 7.5F, 24.f, Color::White, true, g_pSecondFont);

		Visuals::Get().AddToDrawList();
	}
	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();
}

ImDrawList* Render::RenderScene()
{
	if (render_mutex.try_lock())
	{
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}


float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) {
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}
float Render::RenderTextNoOutline(const std::string& text, const ImVec2& pos, float size, Color color, bool center, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

	if (!pFont->ContainerAtlas)
	{
		return 0.f;
	}

	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
	{
		draw_list->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y), GetU32(color), text.c_str());
	}
	else
	{
		draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y), GetU32(color), text.c_str());
	}

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}