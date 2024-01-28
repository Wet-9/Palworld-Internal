#pragma once

#include "pch.h"

#include <functional>

#include "Console.hpp"

class Module
{
public:
    //take away the ability to copy this class
    Module(Module const&) = delete;
    Module(Module&&) = delete;
    Module& operator=(Module const&) = delete;
    Module& operator=(Module&&) = delete;


    explicit Module(std::string name) : moduleName(std::move(name))
    {
    }

    virtual ~Module() = default;

    virtual void Draw() = 0;
    virtual void Tick() = 0;

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool state) { enabled = state; }
    std::string GetName() const { return moduleName; }

protected:
    bool enabled = true;
    std::string moduleName;
};

class ModuleScope : public Module
{
public:
    ModuleScope(std::string name, std::vector<std::unique_ptr<Module>> child_modules)
        : Module(std::move(name)),
          childModules(std::move(child_modules))
    {
    }

    //Only a name, no child modules
    explicit ModuleScope(std::string name)
        : Module(std::move(name))
    {
        //Create an empty vector
        childModules = std::vector<std::unique_ptr<Module>>();
    }


    void Draw() override
    {
        for (auto& module : childModules)
            if (module->IsEnabled()) module->Draw();
    }

    void Tick() override
    {
        // for (auto& module : childModules)
        // {
        //     if (module->IsEnabled()) module->Tick();
        // }
    }

    void DisableChildModules();
    void EnableChildModules();

    virtual ModuleScope* AddModule(std::unique_ptr<Module> module)
    {
        childModules.push_back(std::move(module));
        return this;
    }

protected:
    std::vector<std::unique_ptr<Module>> childModules;
};


class TabModule final : public ModuleScope
{
public:
    TabModule(std::string name, std::string icon)
        : ModuleScope(std::move(name)), icon(std::move(icon))
    {
    }


    void Draw() override;
    void Tick() override;

private:
    std::string icon;
    std::function<void()> onClick;
    bool checked = false;
    friend class SideBarModule;
};

class InlinedModule final : public Module
{
public:
    InlinedModule(std::string name, std::function<void()> drawFunction)
        : Module(std::move(name)), drawFunction(std::move(drawFunction))
    {
    }

    void Draw() override
    {
        drawFunction();
    }

    void Tick() override
    {
    }

private:
    std::function<void()> drawFunction;
};

class SideBarModule final : public ModuleScope
{
public:
    SideBarModule(std::string name, std::vector<std::unique_ptr<Module>> child_modules)
        : ModuleScope(std::move(name), std::move(child_modules))
    {
    }

    //Only a name, no child modules
    explicit SideBarModule(std::string name)
        : ModuleScope(std::move(name), {})
    {
    }

    void Draw() override;

    enum class Side
    {
        Left,
        Right
    };

    ModuleScope* AddModule(std::unique_ptr<Module> module) override
    {
        if (auto tabModule = dynamic_cast<TabModule*>(module.get()))
        {
            tabModule->onClick = [this, tabModule]()
            {
                if (activeTab != tabModule)
                {
                    if (activeTab != nullptr)
                    {
                        activeTab->checked = false;
                        activeTab->DisableChildModules();
                    }
                    activeTab = tabModule;
                    activeTab->checked = true;
                    activeTab->EnableChildModules();
                }
            };
        }
        childModules.push_back(std::move(module));
        return this;
    }

    Side GetSide() const { return side; }

protected:
    Side side = Side::Left;
    TabModule* activeTab = nullptr;
};


class CheckboxModule final : public Module
{
public:
    CheckboxModule(std::string name, bool* value)
        : Module(std::move(name)), value(value)
    {
    }

    void Draw() override
    {
        ImGui::Checkbox(GetName().c_str(), value);
    }

    void Tick() override
    {
    }

private:
    bool* value;
};
