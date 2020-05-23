#pragma once
#include "../helpers/singleton.hpp"

class Glow
    : public Singleton<Glow>
{
    friend class Singleton<Glow>;

    Glow();
    ~Glow();

public:
    void Run();
    void Shutdown();
};