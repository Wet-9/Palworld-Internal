#include "pch.h"
#include "include/Menu.h"

#include "config.h"
#include "include/Console.hpp"
#include "include/Game.hpp"
#define M_PI 3.14159265358979323846
#include <vector>
#include <algorithm> // For std::remove_if
#include <array>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string GetCompileDateString()
{
    std::string compileDate = __DATE__; // "Mmm dd yyyy"
    std::istringstream dateStream(compileDate);
    std::string month, day, year;
    dateStream >> month >> day >> year;

    // Convert month abbreviation to number (example: Jan -> 01, Feb -> 02, etc.)
    std::map<std::string, std::string> monthConversion = {
        {"Jan", "01"}, {"Feb", "02"}, {"Mar", "03"}, {"Apr", "04"},
        {"May", "05"}, {"Jun", "06"}, {"Jul", "07"}, {"Aug", "08"},
        {"Sep", "09"}, {"Oct", "10"}, {"Nov", "11"}, {"Dec", "12"}
    };
    std::string monthNumber = monthConversion[month];

    return "V" + monthNumber + "." + day + "." + year; // Format: "Vmm.dd.yyyy"
}

// Raindrop struct to represent a single raindrop
struct Raindrop
{
    ImVec2 position; // Position of the raindrop
    float velocity; // Speed of the raindrop's fall
    float size; // Size of the raindrop
    float opacity; // Opacity of the raindrop, for fading effect

    Raindrop(const ImVec2& pos, float vel, float sz) : position(pos), velocity(vel), size(sz), opacity(1.0f)
    {
        // Initialize the raindrop with a given position, velocity, and size
    }

    // Update raindrop properties per frame
    void Update(float deltaTime)
    {
        position.y += velocity * deltaTime; // Move the raindrop down
        // Reduce the size and opacity to simulate fading out
        size *= (1.0f - deltaTime);
        opacity *= (2.0f - deltaTime);
        if (opacity < 0.0f) opacity = 0.0f;
    }

    // Check if the raindrop is fully faded or off-screen and should be removed
    bool IsExpired(float windowHeight) const
    {
        return opacity <= 0.0f || position.y > windowHeight;
    }
};

std::vector<Raindrop> raindrops;


void CreateRaindrop(const ImVec2& screenSize)
{
    // Random position at the top of the screen
    float x_pos = static_cast<float>(std::rand() % static_cast<int>(screenSize.x));
    // Random velocity and size for variation
    float velocity = 100.0f + static_cast<float>(std::rand() % 100);
    float size = 2.0f + static_cast<float>(std::rand() % 3);
    raindrops.emplace_back(ImVec2(x_pos, -size), velocity, size);
}

void UpdateAndDrawRaindrops(float deltaTime, ImDrawList* draw_list, const ImVec2& screenSize)
{
    // Update raindrops
    for (auto& drop : raindrops)
    {
        drop.Update(deltaTime);
    }

    // Remove expired raindrops
    raindrops.erase(std::remove_if(raindrops.begin(), raindrops.end(),
                                   [screenHeight = screenSize.y](const Raindrop& drop)
                                   {
                                       return drop.IsExpired(screenHeight);
                                   }), raindrops.end());
    auto window_pos = ImGui::GetWindowPos();
    //A dark purple color with variable opacity
    ImVec4 col1 = ImVec4(0.3f, 0.15f, 0.3f, 1.0f); // Dark purple
    ImVec4 col2 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
    for (const auto& drop : raindrops)
    {
        //Lerp the color based on the opacity
        auto baseColor = ImLerp(col1, col2, drop.opacity);

        ImVec4 raindropColor = ImVec4(baseColor.x, baseColor.y, baseColor.z, drop.opacity * 0.8f);
        // Light blue color with variable opacity
        auto rel_pos = ImVec2(window_pos.x + drop.position.x, window_pos.y + drop.position.y);
        // Offset position to center the raindrop
        draw_list->AddCircleFilled(rel_pos, drop.size, ImGui::ColorConvertFloat4ToU32(raindropColor));
    }
}

void TABExploit()
{
    // ImGui::Checkbox("SafeTeleport", &Config.IsSafe); // Safe teleporting seems to be broken, so I disabled it for now.
    ImGui::InputFloat3("Pos:", Config.Pos);
    ImGui::InputInt("EXP:", &Config.EXP);
    ImGui::Checkbox("Give Exp To All", &Config.GiveToAll);
    if (ImGui::Button("Give exp", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
        if (p_appc != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                {
                    if (Config.EXP >= 0)
                    {
                        Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->
                               GrantExpForParty(Config.EXP);
                    }
                }
            }
        }
    }


    /*if (ImGui::BeginCombo("Pal Name", combo_preview_Pal, flags))
    {
        for (int n = 0; n < IM_ARRAYSIZE(PalNames); n++)
        {
            const bool is_selected = (palSelecteditem == n);
            if (ImGui::Selectable(PalNames[n], is_selected))
                palSelecteditem = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    //ImGui::InputText("PalName", Config.PalName, sizeof(Config.PalName));
    ImGui::InputInt("PalRank", &Config.PalRank);
    ImGui::InputInt("Pallvl", &Config.PalLvL);
    if (ImGui::Button("Spawn Pal", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        if (Config.GetPalPlayerCharacter() != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                {
                    if (Config.PalName != NULL)
                    {
                        g_Console->printdbg("\n\n[+] PalName: %s [+]\n\n", g_Console->color.green, Config.ItemName);
                        SpawnPal((char*)combo_preview_Pal, Config.PalRank, Config.PalLvL);
                    }
                }
            }
        }
    }*/
    if (ImGui::Button("Teleport Home", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
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
                        Config.GetPalPlayerCharacter()->GetPalPlayerController()->
                               TeleportToSafePoint_ToServer();
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
    /*if (ImGui::Button("AnywhereTP", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        if (Config.GetPalPlayerCharacter() != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                if (Config.Pos != NULL)
                {
                    SDK::FVector vector = { Config.Pos[0],Config.Pos[1],Config.Pos[2] };
                    AnyWhereTP(vector, Config.IsSafe);
                }
            }
        }
    }*/

    if (ImGui::Button("ToggleFly", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        Config.IsToggledFly = !Config.IsToggledFly;
        // ExploitFly(Config.IsToggledFly);
    }

    if (ImGui::Button("NormalHealth", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
        if (p_appc != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                {
                    SDK::FFixedPoint fixpoint = SDK::FFixedPoint();
                    fixpoint.Value = Config.GetPalPlayerCharacter()->CharacterParameterComponent->GetMaxHP().
                                            Value;
                    Config.GetPalPlayerCharacter()->ReviveCharacter_ToServer(fixpoint);
                }
            }
        }
    }

    if (ImGui::Button("GodHealth", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
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

    if (ImGui::Button("MaxWeight", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        SDK::APalPlayerCharacter* p_appc = Config.GetPalPlayerCharacter();
        if (p_appc != NULL)
        {
            if (Config.GetPalPlayerCharacter()->GetPalPlayerController() != NULL)
            {
                if (Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState() != NULL)
                {
                    Config.GetPalPlayerCharacter()->GetPalPlayerController()->GetPalPlayerState()->InventoryData
                          ->MaxInventoryWeight = Config.MaxWeight;
                }
            }
        }
    }

    if (ImGui::Button("Catch Rate", ImVec2(ImGui::GetContentRegionAvail().x - 3, 20)))
    {
        Config.isCatchRate = !Config.isCatchRate;
        // ToggleCatchRate(Config.isCatchRate);
    }
    ImGui::InputFloat("Catch Rate Modifier", &Config.CatchRate);
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
    ImGui::Checkbox("HUD overlay", &DX11_Base::g_GameVariables->m_ShowHud);
}

Menu::Menu() noexcept
{
    // Initialize random seed
    std::srand(static_cast<unsigned>(std::time(0)));

    this->rootModuleGroup = std::make_unique<SideBarModule>("Menu");
    auto player = std::make_unique<TabModule>("Player", ICON_FA_USER);
    player->AddModule(std::make_unique<InlinedModule>("Players", &TABPlayer));
    this->rootModuleGroup->AddModule(std::move(player));
    auto exploit = std::make_unique<TabModule>("Exploit", ICON_FA_USER_SECRET);
    exploit->AddModule(std::make_unique<InlinedModule>("Exploits", &TABExploit));
    this->rootModuleGroup->AddModule(std::move(exploit));
    auto items = std::make_unique<TabModule>("Items", ICON_FA_BRIEFCASE);
    items->AddModule(std::make_unique<InlinedModule>("items", &TABItemSpawner));
    this->rootModuleGroup->AddModule(std::move(items));
}

// Renders a blurred background for the menu bar, allows for moving of the window by clicking and dragging the menu bar
void RenderMenuBar()
{
    const std::string title = "Equinox";
    const std::string buildInfo = GetCompileDateString();


    const float menuBarHeightPercent = 0.133f;
    const ImVec2 windowSize = ImGui::GetWindowSize();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Dynamic gradient background for the menu bar
    const ImVec2 menuBarTopLeft(windowPos.x, windowPos.y);
    const ImVec2 menuBarBottomRight(windowPos.x + windowSize.x, windowPos.y + windowSize.y * menuBarHeightPercent);
    ImVec4 color_start = ImVec4(0.25f, 0.2f, 0.25f, 1.0f); // Dark purple
    ImVec4 color_end = ImVec4(0.15f, 0.15f, 0.2f, 1.0f); // Dark gray
    draw_list->AddRectFilledMultiColor(menuBarTopLeft, menuBarBottomRight,
                                       ImGui::ColorConvertFloat4ToU32(color_start),
                                       ImGui::ColorConvertFloat4ToU32(color_end),
                                       ImGui::ColorConvertFloat4ToU32(color_end),
                                       ImGui::ColorConvertFloat4ToU32(color_start));


    if (global_Fonts != nullptr && global_Fonts->Bold != nullptr) // Check if the Header font is loaded
    {
        // Drawing the title "Equinox"
        ImGui::PushFont(global_Fonts->Bold);
        ImVec2 titleSize = ImGui::CalcTextSize(title.c_str());
        ImVec2 titlePos(
            windowPos.x + (windowSize.x - titleSize.x) / 2,
            windowPos.y + (windowSize.y * menuBarHeightPercent - titleSize.y) / 2
        );
        ImVec4 shadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        ImVec4 titleColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        draw_list->AddText(global_Fonts->Bold, global_Fonts->Bold->FontSize, ImVec2(titlePos.x + 3, titlePos.y + 3),
                           ImGui::ColorConvertFloat4ToU32(shadowColor), title.c_str());
        draw_list->AddText(global_Fonts->Bold, global_Fonts->Bold->FontSize, titlePos,
                           ImGui::ColorConvertFloat4ToU32(titleColor),
                           title.c_str());
        ImGui::PopFont();
    }

    // Drawing the build info with shadow and box
    if (global_Fonts != nullptr && global_Fonts->Regular != nullptr)
    {
        ImVec2 buildInfoSize = ImGui::CalcTextSize(buildInfo.c_str());
        ImVec2 buildInfoPos(
            windowPos.x + windowSize.x - buildInfoSize.x - 20, // 20 pixels padding from right
            windowPos.y + windowSize.y * menuBarHeightPercent - buildInfoSize.y - 10 // 10 pixels padding from bottom
        );

        // Draw a rounded box with a shadow for the build info
        ImVec4 shadowColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f); // Semi-transparent black for shadow
        ImVec4 boxColor = ImVec4(0.32f, 0.29f, 0.313f, 0.93f); // Gray color for the box
        ImVec2 boxPadding(8, 4); // Padding inside the box

        ImVec2 boxMin = ImVec2(buildInfoPos.x - boxPadding.x, buildInfoPos.y - boxPadding.y);
        ImVec2 boxMax = ImVec2(buildInfoPos.x + buildInfoSize.x + boxPadding.x,
                               buildInfoPos.y + buildInfoSize.y + boxPadding.y);

        ImVec2 shadowOffset(2, 2); // Shadow offset
        ImVec2 shadowMin = ImVec2(boxMin.x, boxMin.y);
        ImVec2 shadowMax = ImVec2(boxMax.x + shadowOffset.x, boxMax.y + shadowOffset.y);

        draw_list->AddRectFilled(shadowMin, shadowMax, ImGui::ColorConvertFloat4ToU32(shadowColor), 5.0f);
        draw_list->AddRectFilled(boxMin, boxMax, ImGui::ColorConvertFloat4ToU32(boxColor), 5.0f);

        // Draw the build info text
        ImGui::PushFont(global_Fonts->Regular);
        draw_list->AddText(global_Fonts->Regular, global_Fonts->Regular->FontSize, buildInfoPos,
                           ImGui::ColorConvertFloat4ToU32(ImVec4(0.66f, 0.66f, 0.66f, 1.0f)),
                           buildInfo.c_str());
        ImGui::PopFont();
    }


    // Update the cursor position to be at the bottom of the menu bar
    ImGui::SetCursorPos(ImVec2(0, windowSize.y * menuBarHeightPercent));

    //Draw seperator line
    ImGui::GetWindowDrawList()->AddLine(ImVec2(windowPos.x, windowPos.y + windowSize.y * 0.133f),
                                        ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y * 0.133f),
                                        ImGui::GetColorU32(ImGuiCol_Separator), 1.0f);
}

// Linear interpolation function
ImVec4 Lerp(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

void RenderGradientBackground(float delta_time)
{
    static float transition_factor = 0.0f;
    static ImVec4 color_top_left = ImVec4(0.3f, 0.15f, 0.3f, 1.0f); // Dark purple
    static ImVec4 color_top_right = ImVec4(0.15f, 0.15f, 0.15f, 1.0f); // Dark gray
    static ImVec4 color_bottom_left = ImVec4(0.15f, 0.15f, 0.15f, 1.0f); // Dark gray
    static ImVec4 color_bottom_right = ImVec4(0.3f, 0.15f, 0.3f, 1.0f); // Dark purple

    // Update transition factor over time
    transition_factor += delta_time * 0.1f; // Adjust speed as needed
    if (transition_factor > 1.0f)
    {
        transition_factor -= 1.0f;

        // Cycle the colors
        ImVec4 temp = color_top_left;
        color_top_left = color_top_right;
        color_top_right = color_bottom_right;
        color_bottom_right = color_bottom_left;
        color_bottom_left = temp;
    }

    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Interpolated colors for the current transition
    ImVec4 interp_top_left = Lerp(color_bottom_left, color_top_left, transition_factor);
    ImVec4 interp_top_right = Lerp(color_top_left, color_top_right, transition_factor);
    ImVec4 interp_bottom_right = Lerp(color_top_right, color_bottom_right, transition_factor);
    ImVec4 interp_bottom_left = Lerp(color_bottom_right, color_bottom_left, transition_factor);

    // Draw the gradient
    draw_list->AddRectFilledMultiColor(
        window_pos,
        ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y),
        ImGui::ColorConvertFloat4ToU32(interp_top_left),
        ImGui::ColorConvertFloat4ToU32(interp_top_right),
        ImGui::ColorConvertFloat4ToU32(interp_bottom_right),
        ImGui::ColorConvertFloat4ToU32(interp_bottom_left)
    );
}

// Use std::chrono to get the current time since epoch
long long GetCurrentTimeMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void RenderWindow(std::function<void()> renderFunction)
{
    auto window_size = ImGui::GetWindowSize();
    auto window_pos = ImGui::GetWindowPos();
    auto window_center = ImVec2(window_pos.x + window_size.x / 2, window_pos.y + window_size.y / 2);
    //Allow for moving of the window by clicking and dragging the menu bar
    auto mouse_pos = ImGui::GetMousePos();
    auto mouse_delta = ImGui::GetMouseDragDelta();
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && mouse_pos.y < window_center.y - window_size.y / 2 + window_size
        .y * 0.2f)
    {
        ImGui::SetWindowPos(ImVec2(window_pos.x + mouse_delta.x, window_pos.y + mouse_delta.y));
    }
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("##MainMenu", &DX11_Base::g_GameVariables->m_ShowMenu,
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar))
    {
        RenderGradientBackground(ImGui::GetIO().DeltaTime);
        auto deltaTime = ImGui::GetIO().DeltaTime;
        // Example: Add a new raindrop every frame
        CreateRaindrop(ImGui::GetWindowSize());
        UpdateAndDrawRaindrops(deltaTime, ImGui::GetWindowDrawList(), ImGui::GetWindowSize());

        RenderMenuBar();
        renderFunction();
    }
    ImGui::End();
}

void Menu::Draw() const
{
    if (!DX11_Base::g_GameVariables->m_ShowMenu) return;
    RenderWindow([this]()
    {
        this->rootModuleGroup->Draw();
    });
}


void Menu::Tick() const
{
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
