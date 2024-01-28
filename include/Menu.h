#pragma once
#include "Module.h"
#include "libs/ImGui/imgui.h"
#include "include/Fonts.h"

class Menu
{
public:
    //	INITIALIZE CLASS
    Menu() noexcept;
    ~Menu() noexcept = default;
    Menu(Menu const&) = delete;
    Menu(Menu&&) = delete;
    Menu& operator=(Menu const&) = delete;
    Menu& operator=(Menu&&) = delete;

    //	FORWARD DECLARE FUNCTIONS
    void Draw() const;
    void Tick() const;

private:
    std::unique_ptr<ModuleScope> rootModuleGroup;
};

inline std::unique_ptr<Menu> global_Menu;
