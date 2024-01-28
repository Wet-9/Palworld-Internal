#pragma once
#include "Menu.h"


class Player : public Module
{
public:
    void Draw() override;
    void Tick() override;
};
