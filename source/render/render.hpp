#pragma once
#include <string>
#include <sstream>
#include <stdint.h>
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

#include "../helpers/singleton.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_freetype.h"
//#pragma comment(lib, "freetype.lib")
#include "../imgui/imgui_internal.h"
#include "../imgui/impl/imgui_impl_dx9.h"
#include "../imgui/impl/imgui_impl_win32.h"

#include "../valve_sdk/misc/Color.hpp"

extern ImFont* g_pDefaultFont;
extern ImFont* g_pSecondFont;
extern ImFont* g_pDerekFont;

extern ImFont* g_Font1;
extern ImFont* g_Font2;

extern IDirect3DTexture9* brand_img;
extern IDirect3DTexture9* bgbrand_img;

class Vector;

class Render
	: public Singleton<Render>
{
	friend class Singleton<Render>;

private:
	ImDrawList * draw_list_act;
	ImDrawList * draw_list_rendering;
	ImDrawList* draw_list;
	ImDrawData draw_data;

	ImU32 GetU32(Color _color)
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
	}
public:
	
	void Initialize();
	void GetFonts();
	void ClearDrawList();
	void BeginScene();
	ImDrawList* RenderScene();

	float RenderText(const std::string& text, ImVec2 position, float size, Color color, bool center = false, bool outline = true, ImFont* pFont = g_pDefaultFont);
	float RenderTextNoOutline(const std::string& text, const ImVec2& pos, float size, Color color, bool center = false, ImFont* pFont = g_pDefaultFont);

	void RenderCircle3D(Vector position, float points, float radius, Color color);

	void RenderImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(0, 0), const ImVec2& uv_b = ImVec2(1, 1), ImU32 col = 0xFFFFFFFF) {
		ImGui::GetOverlayDrawList()->AddImage(user_texture_id, a, b, uv_a, uv_b, col);
	}

	template <class T>
	inline void RenderBoxByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0)
			RenderBox(x1, y1, x2, y2, color, thickness);
		else if (type == 1)
			RenderCoalBox(x1, y1, x2, y2, color);
		else if (type == 2)
			RenderBox(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderBoxFilledByType(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0 || type == 1)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness);
		else if (type == 2)
			RenderBoxFilled(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	inline void RenderCoalBox(T x1, T y1, T x2, T y2, Color color, float th = 1.f) {
		int w = x2 - x1;
		int h = y2 - y1;

		int iw = w / 4;
		int ih = h / 4;
		// top
		RenderLine(x1, y1, x1 + iw, y1, color, th);					// left
		RenderLine(x1 + w - iw, y1, x1 + w, y1, color, th);			// right
		RenderLine(x1, y1, x1, y1 + ih, color, th);					// top left
		RenderLine(x1 + w - 1, y1, x1 + w - 1, y1 + ih, color, th);	// top right
																	// bottom
		RenderLine(x1, y1 + h, x1 + iw, y1 + h, color, th);			// left
		RenderLine(x1 + w - iw, y1 + h, x1 + w, y1 + h, color, th);	// right
		RenderLine(x1, y1 + h - ih, x1, y1 + h, color, th);			// bottom left
		RenderLine(x1 + w - 1, y1 + h - ih, x1 + w - 1, y1 + h, color, th);	// bottom right
	}


	/*--------------------------------------------------\
	|           x1              x2              y1               y2             |
	|     ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom     |
	\--------------------------------------------------*/

	template <class T>
	inline void RenderEdgesBox(T x1, T y1, T x2, T y2, Color color, float edge_size/*, bool bordered = false*/) {
		if (fabs(x1 - x2) < (edge_size * 2))
			edge_size = fabs(x1 - x2) / 4.f;

		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 + edge_size + (0.5f * edge_size)), GetU32(color), 1.0f);
		draw_list->AddLine(ImVec2(x2, y1), ImVec2(x2, y1 + edge_size + (0.5f * edge_size)), GetU32(color), 1.0f);

		draw_list->AddLine(ImVec2(x1, y2), ImVec2(x1, y2 - edge_size - (0.5f * edge_size)), GetU32(color), 1.0f);
		draw_list->AddLine(ImVec2(x2, y2), ImVec2(x2, y2 - edge_size - (0.5f * edge_size)), GetU32(color), 1.0f);

		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x1 + edge_size, y1), GetU32(color), 1.0f);
		draw_list->AddLine(ImVec2(x2, y1), ImVec2(x2 - edge_size, y1), GetU32(color), 1.0f);

		draw_list->AddLine(ImVec2(x1, y2), ImVec2(x1 + edge_size, y2), GetU32(color), 1.0f);
		draw_list->AddLine(ImVec2(x2 + 1, y2), ImVec2(x2 - edge_size, y2), GetU32(color), 1.0f);
		//if (bordered) {}
	}
	template <class T>
	inline void RenderBox(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15, thickness);
	}
	inline void RenderBox(RECT r, Color color, float thickness = 1.f, float rounding = 0.f) {
		RenderBox(r.left, r.top, r.right, r.bottom, color, thickness, rounding);
	}
	template <class T>
	inline void RenderBoxFilled(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), rounding, 15);
	}
	template <class T>
	inline void RenderLine(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f) {
		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), GetU32(color), thickness);
	}
	inline void RenderTriangle(ImVec2 x, ImVec2 y, ImVec2 z, Color color, float thickness = 1.f) {
		draw_list->AddTriangle(x, y, z, GetU32(color), thickness);
		draw_list->AddTriangleFilled(x, y, z,GetU32(Color(color.r(), color.g(), color.b(), 180)));
	}
	template <class T>
	inline float RenderText(const std::string& text, T x, T y, float size, Color clr, bool center = false, bool outline = true, ImFont* pFont = g_pDefaultFont) {
		return RenderText(text, ImVec2(x, y), size, clr, center, outline, pFont);
	}
	template <class T>
	inline void RenderCircle(T x, T y, float radius, int points, Color color, float thickness = 1.f) {
		draw_list->AddCircle(ImVec2(x, y), radius, GetU32(color), points, thickness);
	}
	template <class T>
	inline void RenderCircleFilled(T x, T y, float radius, int points, Color color) {
		draw_list->AddCircleFilled(ImVec2(x, y), radius, GetU32(color), points);
	}
};