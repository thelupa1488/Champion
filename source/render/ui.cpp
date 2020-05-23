#include "ui.hpp"
#include "menu.hpp"
#include <deque>
#include <algorithm>
#include <vector>
#include "../valve_sdk/sdk.hpp"
//template <class T>
//bool ComboForSkins(const char* label, std::vector current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1) {
//	
//}
bool ImGui::ToggleButton(const char* label, bool* v, const ImVec2& size_arg)
{

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	auto& styles = ImGui::GetStyle();
	styles.FrameRounding = 0.f;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32((hovered && held || *v) ? ImGuiCol_ButtonActive : (hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	if (pressed)
		*v = !*v;

	styles.FrameRounding = 5.f;
	return pressed;
}

bool ImGui::checkbox(const char* label, bool* v)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const float square_sz = ImGui::GetFrameHeight();
	const ImVec2 pos = window->DC.CursorPos;
	const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
	ImGui::ItemSize(total_bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	/// TODO: fix piece of shit below

	static float timeBegin = -1.f;
	static ImGuiID timeID = 0;
	static const float timeActionLength = 0.40f;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
	{
		*v = !(*v);
		if (timeID == id)
		{
			// Fine tuning for the case when user clicks on the same checkbox twice quickly
			float elapsedTime = g_GlobalVars->curtime - timeBegin;
			if (elapsedTime > timeActionLength) timeBegin = g_GlobalVars->curtime;   // restart
			else
			{
				// We must invert the time, tweaking timeBegin
				const float newElapsedTime = timeActionLength - elapsedTime;
				timeBegin = g_GlobalVars->curtime - newElapsedTime;
			}
		}
		else
		{
			timeID = id;
			timeBegin = g_GlobalVars->curtime;
		}

		ImGui::MarkItemEdited(id);
	}

	float t = 0.f;    // In [0,1] 0 = OFF 1 = ON
	bool animationActive = false;
	if (timeID == id)
	{
		float elapsedTime = g_GlobalVars->curtime - timeBegin;
		if (elapsedTime > timeActionLength) { timeBegin = -1; timeID = 0; }
		else
		{
			t = 1 - elapsedTime / timeActionLength;
			animationActive = t > 0;
		}
	}
	//if (*v) t = 1.f-t;
	if (t < 0) t = 0;
	else if (t > 1) t = 1;

	float radius = 5.0f / t;
	if (radius > 10.0f)
		radius = 10.0f;

	const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
	ImGui::RenderNavHighlight(total_bb, id);

	if (*v)
		window->DrawList->AddCircleFilled({ check_bb.GetCenter().x - 3, check_bb.GetCenter().y - 1 }, radius, GetColorU32(ImGuiCol_FrameBgActive, t), 40);

	if (hovered)
	{
		if (!*v)
			window->DrawList->AddCircleFilled({ check_bb.GetCenter().x - 3, check_bb.GetCenter().y - 1 }, 10.0f, ImColor(1.0f, 1.0f, 1.0f, 0.10f), 40);
		else
			window->DrawList->AddCircleFilled({ check_bb.GetCenter().x - 3, check_bb.GetCenter().y - 1 }, 10.0f, GetColorU32(ImGuiCol_FrameBgActive, 0.10f), 40);
	}
	if (held && !pressed)
	{
		if (!*v)
			window->DrawList->AddCircleFilled({ check_bb.GetCenter().x - 3, check_bb.GetCenter().y - 1 }, 10.0f, ImColor(1.0f, 1.0f, 1.0f, 0.2f), 40);
		else
			window->DrawList->AddCircleFilled({ check_bb.GetCenter().x - 3, check_bb.GetCenter().y - 1 }, 10.0f, GetColorU32(ImGuiCol_FrameBgActive, 0.2f), 40);
	}



	//if ( animationActive )


	if (*v)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, GetColorU32(ImGuiCol_FrameBgActive, 1.f));
		ImGui::RenderTextClipped(ImVec2(check_bb.Min.x, check_bb.Min.y + 1), check_bb.Max - style.FramePadding, u8"\uf14a", nullptr, &label_size, ImVec2(0, 0), &check_bb);
		ImGui::PopStyleColor(1);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(170 / 255.f, 170 / 255.f, 170 / 255.f, 255));
		ImGui::RenderTextClipped(ImVec2(check_bb.Min.x, check_bb.Min.y + 1), check_bb.Max - style.FramePadding, u8"\uf0c8", nullptr, &label_size, ImVec2(0, 0), &check_bb);
		ImGui::PopStyleColor(1);
	}

	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y), label);

	/*ImGui::RenderFrame( check_bb.Min, check_bb.Max, ImGui::GetColorU32( ( held && hovered ) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg ), true, style.FrameRounding );
	if ( *v )
	{
		const float pad = ImMax( 1.0f, ( float )( int )( square_sz / 6.0f ) );
		ImGui::RenderCheckMark( check_bb.Min + ImVec2( pad, pad ), ImGui::GetColorU32( ImGuiCol_CheckMark ), square_sz - pad * 2.0f );
	}*/

	if (g.LogEnabled)
		ImGui::LogRenderedText(&total_bb.Min, *v ? "[x]" : "[ ]");

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}


static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

static auto vector_getter = [](void* vec, int idx, const char** out_text) {
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
	*out_text = vector.at(idx).c_str();
	return true;
};

bool ImGui::Combo(const char* label, int* currIndex, std::vector<std::string>& values) {
	if (values.empty()) { return false; }
	return ImGui::Combo(label, currIndex, vector_getter,
		static_cast<void*>(&values), values.size());
}

bool ImGui::BeginGroupBox(const char* name, const ImVec2& size_arg)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;

	window->DC.CursorPos.y += GImGui->FontSize / 2;
	const ImVec2 content_avail = ImGui::GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	if (size.x <= 0.0f) {
		size.x = ImMax(content_avail.x, 4.0f) - fabsf(size.x); // Arbitrary minimum zero-ish child size of 4.0f (0.0f causing too much issues)
	}
	if (size.y <= 0.0f) {
		size.y = ImMax(content_avail.y, 4.0f) - fabsf(size.y);
	}

	ImGui::SetNextWindowSize(size);
	bool ret;
	ImGui::Begin(name, &ret, flags);
	//bool ret = ImGui::Begin(name, NULL, size, -1.0f, flags);

	window = ImGui::GetCurrentWindow();

	auto padding = ImGui::GetStyle().WindowPadding;

	auto text_size = ImGui::CalcTextSize(name, NULL, true);

	if (text_size.x > 1.0f) {
		window->DrawList->PushClipRectFullScreen();
		//window->DrawList->AddRectFilled(window->DC.CursorPos - ImVec2{ 4, 0 }, window->DC.CursorPos + (text_size + ImVec2{ 4, 0 }), GetColorU32(ImGuiCol_ChildWindowBg));
		//RenderTextClipped(pos, pos + text_size, name, NULL, NULL, GetColorU32(ImGuiCol_Text));
		window->DrawList->PopClipRect();
	}
	//if (!(window->Flags & ImGuiWindowFlags_ShowBorders))
	//	ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;

	return ret;
}

void ImGui::EndGroupBox()
{
	ImGui::EndChild();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DC.CursorPosPrevLine.y -= GImGui->FontSize / 2;
}

bool ImGui::ListBox(const char* label, int* current_item, std::string items[], int items_count, int height_items) {
	char **tmp;
	tmp = new char*[items_count];//(char**)malloc(sizeof(char*) * items_count);
	for (int i = 0; i < items_count; i++) {
		//tmp[i] = new char[items[i].size()];//(char*)malloc(sizeof(char*));
		tmp[i] = const_cast<char*>(items[i].c_str());
	}

	const bool value_changed = ImGui::ListBox(label, current_item, Items_ArrayGetter, static_cast<void*>(tmp), items_count, height_items);
	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}

bool ImGui::Combo(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::Combo(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}
