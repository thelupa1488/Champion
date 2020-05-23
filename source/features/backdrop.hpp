#pragma once

#include "../render/render.hpp"
#include <algorithm>
#include <vector>
#include "../valve_sdk/math/Vector2D.hpp"

class Dot;
extern std::vector<Dot*> Dots;

class Dot
{
public:
    Dot(Vector2D p, Vector2D v)
    {
        RelativePosition = p;
        Velocity = v;
    }
    void Draw()
    {
        Render::Get().RenderBoxFilled(RelativePosition.x - 2, RelativePosition.y - 2, RelativePosition.x + 2, RelativePosition.y + 2, Color(255, 255, 255, 255)); // замените на вашу функцию рендера прямоугольника
        auto t = std::find(Dots.begin(), Dots.end(), this);
        if (t == Dots.end()) return;
        for (auto i = t; i != Dots.end(); i++)
        {
            if ((*i) == this) continue;
            Vector2D Pos = RelativePosition;
            float Dist = Pos.DistTo((*i)->RelativePosition);
            if (Dist < 128)
            {
                Vector2D Dir = ((*i)->RelativePosition - RelativePosition).Normalized();
                Render::Get().RenderLine(Pos.x, Pos.y, (Pos + Dir * Dist).x, (Pos + Dir * Dist).y, Color(255, 255, 255));
            }
        }
    }
    Vector2D RelativePosition = Vector2D(0, 0);
    float Rotation = 0;
    float Size = 0;
    Vector2D Velocity;
};

namespace BackDrop
{
    void DrawBackDrop();
}