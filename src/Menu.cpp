#include "../pch.h"
#include "../include/Menu.hpp"
#include "SDK.hpp"
#include "config.h"
#include <algorithm>


int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    char inputChar = data->EventChar;
    Config.Update(Config.inputTextBuffer);

    return 0;
}

SDK::FPalDebugOtomoPalInfo palinfo = SDK::FPalDebugOtomoPalInfo();
SDK::TArray<SDK::EPalWazaID> EA = {0U};

CatchRate CRate;
CatchRate OldRate;

void DetourCatchRate(SDK::APalCaptureJudgeObject* p_this)
{
    if (p_this)
    {
        //p_this->ChallengeCapture_ToServer(Config.localPlayer, Config.CatchRate);
        p_this->ChallengeCapture(Config.localPlayer, Config.CatchRate);
    }
}

void ToggleCatchRate(bool catchrate)
{
    if (catchrate)
    {
        if (CRate == NULL)
        {
            CRate = (CatchRate)(Config.ClientBase + Config.offset_CatchRate);
            MH_CreateHook(CRate, DetourCatchRate, reinterpret_cast<void**>(OldRate));
            MH_EnableHook(CRate);
            return;
        }
        MH_EnableHook(CRate);
        return;
    }
    MH_DisableHook(CRate);
}

void AddItem(SDK::UPalPlayerInventoryData* data, char* itemName, int count)
{
    SDK::UKismetStringLibrary* lib = SDK::UKismetStringLibrary::GetDefaultObj();
    
    //Convert FNAME
    wchar_t ws[255];
    swprintf(ws, 255, L"%hs", itemName);
    SDK::FName Name = lib->Conv_StringToName(SDK::FString(ws));
    //Call
    data->RequestAddItem(Name, count, true);
}

void SpawnPal(char* PalName, int rank, int lvl = 1)
{
    SDK::UKismetStringLibrary* lib = SDK::UKismetStringLibrary::GetDefaultObj();

    //Convert FNAME
    wchar_t ws[255];
    swprintf(ws, 255, L"%hs", PalName);
    SDK::FName Name = lib->Conv_StringToName(SDK::FString(ws));
    //Call
    if (Config.GetPalPlayerCharacter() != NULL)
    {
        if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController())
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState())
                {
                    EA[0] = SDK::EPalWazaID::AirCanon;
                    palinfo.Level = lvl;
                    palinfo.Rank = rank;
                    palinfo.PalName.Key = Name;
                    palinfo.WazaList = EA;
                    palinfo.PassiveSkill = NULL;
                    Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->
                           Debug_CaptureNewMonsterByDebugOtomoInfo_ToServer(palinfo);
                }
            }
        }
    }
}

void AnyWhereTP(SDK::FVector& vector, bool IsSafe)
{
    if (!IsSafe)
    {
        if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
        {
            SDK::FGuid guid = Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPlayerUId();
            vector = {vector.X, vector.Y + 100, vector.Z};
            Config.GetPalPlayerCharacter()->GetPalPlayerController()->Transmitter->Player->
                   RegisterRespawnLocation_ToServer(guid, vector);
            Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->RequestRespawn();
        }
    }
    else
    {
        if (Config.GetPalPlayerCharacter()->GetPalPlayerController())
        {
            vector = {vector.X, vector.Y + 100, vector.Z};
            Config.GetPalPlayerCharacter()->GetPalPlayerController()->Debug_Teleport2D(vector);
        }
    }
    return;
}

void ExploitFly(bool IsFly)
{
    SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
    if (p_appc != NULL)
    {
        if (IsFly)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                Config.GetPalPlayerCharacter()->GetPalPlayerController()->StartFlyToServer();
            }
        }
        else
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                Config.GetPalPlayerCharacter()->GetPalPlayerController()->EndFlyToServer();
            }
        }
    }
}

namespace DX11_Base
{
    namespace Styles
    {
        void InitStyle()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = ImGui::GetStyle().Colors;

            //	STYLE PROPERTIES
            style.WindowPadding = ImVec2(15, 15);
            style.WindowRounding = 5.0f;
            style.FramePadding = ImVec2(5, 5);
            style.FrameRounding = 4.0f;
            style.ItemSpacing = ImVec2(12, 8);
            style.ItemInnerSpacing = ImVec2(8, 6);
            style.IndentSpacing = 25.0f;
            style.ScrollbarSize = 15.0f;
            style.ScrollbarRounding = 9.0f;
            style.GrabMinSize = 5.0f;
            style.GrabRounding = 3.0f;

            //  Base ImGui Styling , Aplying a custyom style is left up to you.
            ImGui::StyleColorsDark();

            /// EXAMPLE COLOR 
            //colors[ImGuiCol_FrameBg] = ImVec4(0, 0, 0, 0);

            //	COLORS
            if (g_Menu->dbg_RAINBOW_THEME)
            {
                //  RGB MODE STLYE PROPERTIES
                colors[ImGuiCol_Separator] = ImVec4(g_Menu->dbg_RAINBOW);
                colors[ImGuiCol_TitleBg] = ImVec4(0, 0, 0, 1.0f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0, 0, 0, 1.0f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0, 0, 0, 1.0f);
            }
            else
            {
                /// YOUR DEFAULT STYLE PROPERTIES HERE
                colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
                colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
                colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
                colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
                colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
                colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
                colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
                colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
                colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
                colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
                colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
                colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
                colors[ImGuiCol_TabActive] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
                colors[ImGuiCol_TabHovered] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
            }
        }
    }

    namespace Tabs
    {
        void TABPlayer()
        {
            //�л�����һ��
            ImGui::Checkbox("SpeedHack", &Config.IsSpeedHack);

            ImGui::Checkbox("AttackHack", &Config.IsAttackModiler);

            ImGui::Checkbox("DefenseHack", &Config.IsDefuseModiler);

            ImGui::Checkbox("InfStamina", &Config.IsInfStamina);

            ImGui::Checkbox("Godmode", &Config.IsMuteki);

            ImGui::Checkbox("Revive", &Config.IsRevive);

            if (ImGui::Button("ToggleInfAmmo", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                if (Config.GetPalPlayerCharacter()->ShooterComponent != NULL && Config.GetPalPlayerCharacter()->
                    ShooterComponent->CanShoot())
                {
                    if (Config.GetPalPlayerCharacter()->ShooterComponent->GetHasWeapon() != NULL)
                    {
                        Config.GetPalPlayerCharacter()->ShooterComponent->GetHasWeapon()->IsRequiredBullet = !Config.
                            GetPalPlayerCharacter()->ShooterComponent->GetHasWeapon()->IsRequiredBullet;
                    }
                }
            }

            //��������һ��
            ImGui::SliderFloat("SpeedModifilers", &Config.SpeedModiflers, 1, 10);
            ImGui::SliderInt("AttackModifilers", &Config.DamageUp, 0, 200000);
            ImGui::SliderInt("defenseModifilers", &Config.DefuseUp, 0, 200000);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Checkbox("HUD overlay", &g_GameVariables->m_ShowHud);
        }

        
        void TABItemSpawner()
        {
            static int num_to_add = 1;
            static int category = 0;

            ImGui::InputInt("Num To Add", &num_to_add);

            ImGui::Combo("Item Category", &category,
                         "Accessories\0Ammo\0Armor\0Crafting Materials\0Eggs\0Food\0Hats\0\Medicine\0Money\0Other\0Pal Spheres\0Seeds\0Tools\0Weapons\0\All\0");

            std::initializer_list list = database::all;

            switch (category)
            {
            case 1:
                list = database::ammo;
                break;
            case 2:
                list = database::armor;
                break;
            case 3:
                list = database::craftingmaterials;
                break;
            case 4:
                list = database::eggs;
                break;
            case 5:
                list = database::food;
                break;
            case 6:
                list = database::hats;
                break;
            case 7:
                list = database::medicine;
                break;
            case 8:
                list = database::money;
                break;
            case 9:
                list = database::other;
                break;
            case 10:
                list = database::palspheres;
                break;
            case 11:
                list = database::seeds;
                break;
            case 12:
                list = database::toolss;
                break;
            case 13:
                list = database::weapons;
                break;
            case 14:
                list = database::all;
                break;
            default:
                list = database::all;
            }

            int cur_size = 0;

            static char item_search[100];;

            ImGui::InputText("Search", item_search, IM_ARRAYSIZE(item_search));
            for (const auto& item : list)
            {
                std::istringstream ss(item);
                std::string left_text, right_text;

                std::getline(ss, left_text, '|');
                std::getline(ss, right_text);

                auto right_to_lower = right_text;
                std::string item_search_to_lower = item_search;

                std::transform(right_to_lower.begin(), right_to_lower.end(), right_to_lower.begin(), ::tolower);
                std::transform(item_search_to_lower.begin(), item_search_to_lower.end(), item_search_to_lower.begin(),
                               ::tolower);

                if (item_search[0] != '\0' && (right_to_lower.find(item_search_to_lower) == std::string::npos))
                    continue;

                if (cur_size != 0 && cur_size < 20)
                {
                    ImGui::SameLine();
                }
                else if (cur_size != 0)
                {
                    cur_size = 0;
                }

                cur_size += right_text.length();

                ImGui::PushID(item);
                if (ImGui::Button(right_text.c_str()))
                {
                    SDK::UPalPlayerInventoryData* InventoryData = Config.GetPalPlayerCharacter()->
                                                                         GetPalPlayerController()->GetPalPlayerState()->
                                                                         GetInventoryData(); //rebas
                    AddItem(InventoryData, (char*)left_text.c_str(), num_to_add);
                }
                ImGui::PopID();
            }
        }

        void TABQuickTP()
        {
            if (ImGui::Button("Anubis", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                if (Config.GetPalPlayerCharacter() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                    {
                        SDK::FVector vector = {
                            Config.AnubisLocation[0], Config.AnubisLocation[1], Config.AnubisLocation[2]
                        };
                        AnyWhereTP(vector, Config.IsSafe);
                    }
                }
            }

            if (ImGui::Button("Frostallion", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                if (Config.GetPalPlayerCharacter() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                    {
                        SDK::FVector vector = {
                            Config.FrostallionLocation[0], Config.FrostallionLocation[1], Config.FrostallionLocation[2]
                        };
                        AnyWhereTP(vector, Config.IsSafe);
                    }
                }
            }

            if (ImGui::Button("Jetdragon", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                if (Config.GetPalPlayerCharacter() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                    {
                        SDK::FVector vector = {
                            Config.JetragonLocation[0], Config.JetragonLocation[1], Config.JetragonLocation[2]
                        };
                        AnyWhereTP(vector, Config.IsSafe);
                    }
                }
            }

            if (ImGui::Button("Paladius", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                if (Config.GetPalPlayerCharacter() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                    {
                        SDK::FVector vector = {
                            Config.PaladiusLocation[0], Config.PaladiusLocation[1], Config.PaladiusLocation[2]
                        };
                        AnyWhereTP(vector, Config.IsSafe);
                    }
                }
            }
        }

        void TABGameBreaking()
        {
            if (ImGui::Button("Max Level<50>", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
                SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
                if (p_appc != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                    {
                        if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                        {
                            Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->
                                   GrantExpForParty(99999999);
                        }
                    }
                }
            }
            if (ImGui::Button("All Effigies", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20))) //credit to bennett
            {
                SDK::APalPlayerCharacter* pPalCharacter = Config.GetPalPlayerCharacter();
                if (!pPalCharacter)
                    return;

                SDK::UWorld* world = Config.GetUWorld();
                if (!world)
                    return;

                SDK::TUObjectArray* objects = world->GObjects;

                for (int i = 0; i < objects->NumElements; ++i)
                {
                    SDK::UObject* object = objects->GetByIndex(i);

                    if (!object)
                    {
                        continue;
                    }

                    if (!object->IsA(SDK::APalLevelObjectRelic::StaticClass()))
                    {
                        continue;
                    }

                    SDK::APalLevelObjectObtainable* relic = (SDK::APalLevelObjectObtainable*)object;
                    if (!relic)
                    {
                        continue;
                    }

                    ((SDK::APalPlayerState*)pPalCharacter->PlayerState)->RequestObtainLevelObject_ToServer(relic);
                }
            }
        }

        void TABConfig()
        {
            ImGui::Text("Palworlds Internal");
            ImGui::Text("Version: v1.23");
            ImGui::Text("Credits to: deenart, xCENTx, and swordbluesword");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            if (ImGui::Button("Unload DLL", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
            {
#if DEBUG
                g_Console->printdbg("\n\n[+] Unloading Initialized [+]\n\n", g_Console->color.red);

#endif
                g_KillSwitch = TRUE;
            }
        }
    }

    void Menu::Draw()
    {
        if (Config.IsESP)
        {
            ESP();
        }
        if (GetAsyncKeyState(VK_DELETE) & 1)
            g_GameVariables->m_ShowMenu = !g_GameVariables->m_ShowMenu;
        if (g_GameVariables->m_ShowMenu)
            MainMenu();

        if (g_GameVariables->m_ShowHud)
            HUD(&g_GameVariables->m_ShowHud);

        if (g_GameVariables->m_ShowDemo)
            ImGui::ShowDemoWindow();
    }

    void Menu::MainMenu()
    {
        if (!g_GameVariables->m_ShowDemo)
            Styles::InitStyle();

        if (g_Menu->dbg_RAINBOW_THEME)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(g_Menu->dbg_RAINBOW));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(g_Menu->dbg_RAINBOW));
            ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(g_Menu->dbg_RAINBOW));
            //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(g_Menu->dbg_RAINBOW));
        }
        if (!ImGui::Begin("Pals Internal", &g_GameVariables->m_ShowMenu, 96))
        {
            ImGui::End();
            return;
        }
        if (g_Menu->dbg_RAINBOW_THEME)
        {
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }

        //  Display Menu Content
        //Tabs::TABMain();

        ImGui::Text("Please report any issues to daenart on discord ");

        if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Player"))
            {
                Tabs::TABPlayer();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Exploits"))
            {
                Tabs::TABExploit();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Item Spawner"))
            {
                Tabs::TABItemSpawner();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Quick TP"))
            {
                Tabs::TABQuickTP();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("GameBreaking"))
            {
                Tabs::TABGameBreaking();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Info"))
            {
                Tabs::TABConfig();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    void Menu::HUD(bool* p_open)
    {
        // Draws the player location in the top right of the screen
        SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
        if (p_appc != NULL && p_appc->GetPalPlayerController() != NULL)
        {
            ImGuiWindowFlags window_flags = 0;
            window_flags |= ImGuiWindowFlags_NoBackground;
            window_flags |= ImGuiWindowFlags_NoTitleBar;
            window_flags |= ImGuiWindowFlags_NoResize; // Disable window resizing
            window_flags |= ImGuiWindowFlags_NoMove; // Disable window moving

            SDK::FVector playerCoords = p_appc->K2_GetActorLocation();

            // Get the viewport size
            auto viewportSize = ImGui::GetMainViewport()->WorkSize;

            // Calculate the position for the HUD (top right corner of the viewport)
            ImVec2 window_pos = ImVec2(viewportSize.x - 10, 10); // Adjust the position as needed
            ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f); // Pivot at the top right of the window

            // Set the next window position
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

            // Begin the window
            if (ImGui::Begin("HUD", p_open, window_flags))
            {
                // Display the player coordinates
                ImGui::Text("Player Coords: (%.2f, %.2f, %.2f)", playerCoords.X, playerCoords.Y, playerCoords.Z);
            }
            // End the window
            ImGui::End();
        }
    }

    void Menu::Loops()
    {
        if ((GetAsyncKeyState(VK_F5) & 1))
        {
            SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
            if (p_appc != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                    {
                        if (Config.IsSafe)
                        {
                            Config.GetPalPlayerCharacter()->GetPalPlayerController()->TeleportToSafePoint_ToServer();
                        }
                        else
                        {
                            Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->
                                   RequestRespawn();
                        }
                    }
                }
            }
        }
        if ((GetAsyncKeyState(VK_F6) & 1))
        {
            SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
            if (p_appc != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
                {
                    if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                    {
                        SDK::FFixedPoint fixpoint = SDK::FFixedPoint();
                        fixpoint.Value = 99999999;
                        Config.GetPalPlayerCharacter()->ReviveCharacter_ToServer(fixpoint);
                    }
                }
            }
        }
        if (Config.IsSpeedHack)
        {
            if (Config.GetUWorld()
                || Config.GetUWorld()->PersistentLevel
                || Config.GetUWorld()->PersistentLevel->WorldSettings)
            {
                Config.GetUWorld()->OwningGameInstance->LocalPlayers[0]->PlayerController->AcknowledgedPawn->
                                                                         CustomTimeDilation = Config.SpeedModiflers;
            }
        }
        if (Config.GetPalPlayerCharacter() != NULL)
        {
            if (Config.GetPalPlayerCharacter()->CharacterParameterComponent != NULL)
            {
                Config.GetPalPlayerCharacter()->CharacterParameterComponent->bIsEnableMuteki = Config.IsMuteki;
            }
        }
        if (Config.IsAttackModiler)
        {
            if (Config.GetPalPlayerCharacter() != NULL && Config.GetPalPlayerCharacter()->CharacterParameterComponent->
                                                                 AttackUp != Config.DamageUp)
            {
                if (Config.GetPalPlayerCharacter()->CharacterParameterComponent != NULL)
                {
                    Config.GetPalPlayerCharacter()->CharacterParameterComponent->AttackUp = Config.DamageUp;
                }
            }
        }
        if (Config.IsDefuseModiler)
        {
            if (Config.GetPalPlayerCharacter() != NULL && Config.GetPalPlayerCharacter()->CharacterParameterComponent->
                                                                 DefenseUp != Config.DefuseUp)
            {
                if (Config.GetPalPlayerCharacter()->CharacterParameterComponent != NULL)
                {
                    Config.GetPalPlayerCharacter()->CharacterParameterComponent->DefenseUp = Config.DefuseUp;
                }
            }
        }
        if (Config.IsInfStamina)
        {
            if (Config.GetPalPlayerCharacter() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->CharacterParameterComponent != NULL)
                {
                    Config.GetPalPlayerCharacter()->CharacterParameterComponent->ResetSP();
                }
            }
        }
        if (Config.IsRevive)
        {
            if (Config.GetPalPlayerCharacter() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->CharacterParameterComponent != NULL)
                {
                    Config.GetPalPlayerCharacter()->CharacterParameterComponent->ResetDyingHP();
                }
            }
        }
    }
}
