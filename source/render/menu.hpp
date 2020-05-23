#pragma once

#include <string>
#include "../helpers/singleton.hpp"
#include "../imgui/imgui.h"

struct IDirect3DDevice9;

class Menu
    : public Singleton<Menu>
{
public:
    void Initialize();
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();

    void Render();

    void RenderPreview();

    void Toggle();

    bool IsVisible() const { return _visible; }

private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;
};