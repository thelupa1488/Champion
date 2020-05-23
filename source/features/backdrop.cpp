#include "backdrop.hpp"
#include "../render/menu.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../config/options.hpp"

std::vector<Dot*> Dots;

void BackDrop::DrawBackDrop()
{
    if (g_Options.misc_backdrop) {
        static int width, height;
        g_EngineClient->GetScreenSize(width, height);

        Render::Get().RenderBoxFilled(0, 0, width, height, Color(0.f, 0.f, 0.f, 0.70f)); // замените на вашу функцию рендера прямоугольника

        int s = rand() % 14;

        if (s == 0)
            Dots.push_back(new Dot(Vector2D(rand() % (int)width, -16), Vector2D((rand() % 7) - 3, rand() % 3 + 1)));
        else if (s == 1)
            Dots.push_back(new Dot(Vector2D(rand() % (int)width, (int)height + 16), Vector2D((rand() % 7) - 3, -1 * (rand() % 3 + 1))));
        else if (s == 2)
            Dots.push_back(new Dot(Vector2D(-16, rand() % (int)height), Vector2D(rand() % 3 + 1, (rand() % 7) - 3)));
        else if (s == 3)
            Dots.push_back(new Dot(Vector2D((int)width + 16, rand() % (int)height), Vector2D(-1 * (rand() % 3 + 1), (rand() % 7) - 3)));

        for (auto i = Dots.begin(); i < Dots.end();)
        {
            if ((*i)->RelativePosition.y < -20 || (*i)->RelativePosition.y > height + 20 || (*i)->RelativePosition.x < -20 || (*i)->RelativePosition.x > width + 20)
            {
                delete (*i);
                i = Dots.erase(i);
            }
            else
            {
                (*i)->RelativePosition = (*i)->RelativePosition + (*i)->Velocity * (1.f);
                i++;
            }
        }
        for (auto i = Dots.begin(); i < Dots.end(); i++)
            (*i)->Draw();
    }
}