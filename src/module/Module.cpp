#include "pch.h"
#include "include/Module.h"

#include "config.h"
#include "include/Fonts.h"
#include "libs/ImGui/imgui.h"
#define M_PI 3.14159265358979323846

void ModuleScope::DisableChildModules()
{
    for (auto& module : childModules)
    {
        module->SetEnabled(false);
    }
}

void ModuleScope::EnableChildModules()
{
    for (auto& module : childModules)
    {
        module->SetEnabled(true);
    }
}

void SideBarModule::Draw()
{
    const float deltaTime = ImGui::GetIO().DeltaTime; // Get the time since last frame
    static float gradientPhase = 0.0f; // Static variable to hold the phase over time
    const float gradientSpeed = 0.5f; // Speed of the gradient movement

    // Update the gradient phase with the delta time
    gradientPhase += deltaTime * gradientSpeed;
    // Ensure the phase wraps around [0, 1]
    if (gradientPhase > 1.0f) gradientPhase -= 1.0f;


    const float width = 133.f;
    const float sidebarHeight = ImGui::GetWindowHeight() * 0.866f;
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 nextPos = GetSide() == SideBarModule::Side::Left
                         ? ImVec2(windowPos.x, windowPos.y + ImGui::GetWindowHeight() * 0.133f)
                         : ImVec2(windowPos.x + ImGui::GetWindowWidth() - width,
                                  windowPos.y + ImGui::GetWindowHeight() * 0.133f);

    auto drawList = ImGui::GetWindowDrawList();

    // Background gradient colors
    ImVec4 color_start = ImVec4(0.25f, 0.2f, 0.25f, 1.0f); // Dark purple
    ImVec4 color_end = ImVec4(0.15f, 0.15f, 0.2f, 1.0f); // Dark gray
    // Draw sidebar background with gradient, padding, and rounding
    ImVec2 minRect = nextPos;
    ImVec2 maxRect = ImVec2(nextPos.x + width, nextPos.y + sidebarHeight);
    drawList->AddRectFilledMultiColor(minRect, maxRect,
                                      ImColor(color_start), ImColor(color_start),
                                      ImColor(color_end), ImColor(color_end)); // Rounded corners


    // Border color
    ImVec4 borderColor = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // Slightly lighter than the darkest color of the background

    // Draw the border
    drawList->AddRect(minRect, maxRect, ImColor(borderColor), 10.0f, ImDrawFlags_RoundCornersAll, 2.0f);
    // Calculate the time-dependent offset for the "chasing" effect
    float time = ImGui::GetTime();


    // Define your gradient colors here
    ImVec4 col1 = ImVec4(0.3f, 0.15f, 0.3f, 1.0f); // Dark purple
    ImVec4 col2 = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray

    // Calculate the number of gradient steps based on the sidebar dimensions
    int numSteps = static_cast<int>((width + sidebarHeight) / gradientSpeed);

    // We will draw the border in segments to create the gradient effect
    float borderThickness = 4.0f; // Thickness of the border
    for (int step = 0; step < numSteps; ++step)
    {
        // Interpolate between col1 and col2 based on the sine of the phase
        float t = (sin(2.0f * M_PI * (gradientPhase + step / static_cast<float>(numSteps))) + 1.0f) * 0.5f;
        ImVec4 col = ImLerp(col1, col2, t); // Linear interpolation between two colors
        ImU32 color = ImGui::ColorConvertFloat4ToU32(col);

        float offset = step * borderThickness;

        // Draw each segment of the border
        // Left border
        drawList->AddRectFilled(ImVec2(nextPos.x, nextPos.y + offset),
                                ImVec2(nextPos.x + borderThickness, nextPos.y + offset + borderThickness),
                                color);

        // Right border
        drawList->AddRectFilled(ImVec2(nextPos.x + width - borderThickness, nextPos.y + offset),
                                ImVec2(nextPos.x + width, nextPos.y + offset + borderThickness),
                                color);

        // Top border
        drawList->AddRectFilled(ImVec2(nextPos.x + offset, nextPos.y),
                                ImVec2(nextPos.x + offset + borderThickness, nextPos.y + borderThickness),
                                color);

        // Bottom border
        drawList->AddRectFilled(ImVec2(nextPos.x + offset, nextPos.y + sidebarHeight - borderThickness),
                                ImVec2(nextPos.x + offset + borderThickness, nextPos.y + sidebarHeight),
                                color);
    }

    // Sidebar content
    ImGui::SetNextWindowPos(nextPos);
    if (ImGui::BeginChild("##sidebar", ImVec2(width, sidebarHeight), true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground))
    {
        // Draw each module in the sidebar
        for (auto& module : childModules)
        {
            if (module->IsEnabled())
                module->Draw();
        }
    }
    // End child window
    ImGui::EndChild();
    if (activeTab != nullptr)
    {
        //Creates a new child window that is at 210, and 
        // Draw the active tab's content
        ImGui::SetNextWindowPos(ImVec2(windowPos.x + width + 10.0f, windowPos.y + ImGui::GetWindowHeight() * 0.133f));
        if (ImGui::BeginChild("##content", ImVec2(ImGui::GetWindowWidth() - width - 20.0f, sidebarHeight), true,
                              ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::Text(activeTab->GetName().c_str());
            ImGui::Separator();
            for (auto& module : activeTab->childModules)
            {
                if (module->IsEnabled())
                    module->Draw();
            }
        }
    }
    ImGui::EndChild();
}


void TabModule::Draw()
{
    ImVec2 cursorPos = ImGui::GetCursorPos();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    float tabWidth = windowSize.x - 20.0f; // Slightly smaller than the sidebar
    float tabHeight = 40.0f; // Reduced tab height
    float iconSize = 24.0f; // Icon size

    auto drawList = ImGui::GetWindowDrawList();

    ImVec2 tabTopLeft = ImVec2(windowPos.x + 10.0f, windowPos.y + cursorPos.y);
    ImVec2 tabBottomRight = ImVec2(tabTopLeft.x + tabWidth, tabTopLeft.y + tabHeight);

    // Check if the tab is hovered or clicked
    bool isHovered = ImGui::IsMouseHoveringRect(tabTopLeft, tabBottomRight);
    bool isClicked = isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    // Change cursor to a pointer when hovered

    // Toggle the checked state on click
    if (isClicked)
    {
        onClick();
    }

    // Colors
    ImVec4 bgColor = checked
                         ? ImVec4(0.35f, 0.3f, 0.35f, 1.0f)
                         : (isHovered ? ImVec4(0.3f, 0.25f, 0.3f, 0.9f) : ImVec4(0.25f, 0.2f, 0.25f, 0.8f));
    ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    // Shadow effect
    ImVec2 shadowOffset(3.0f, 3.0f);
    auto min = ImVec2(tabTopLeft.x + shadowOffset.x, tabTopLeft.y + shadowOffset.y);
    auto max = ImVec2(tabBottomRight.x + shadowOffset.x, tabBottomRight.y + shadowOffset.y);
    drawList->AddRectFilled(min, max, ImColor(0, 0, 0, 50), 10.0f, ImDrawFlags_RoundCornersAll);

    // Tab background
    drawList->AddRectFilled(tabTopLeft, tabBottomRight, ImColor(bgColor), 10.0f, ImDrawFlags_RoundCornersAll);

    // Icon
    if (!icon.empty() && global_Fonts->Icons)
    {
        drawList->AddText(global_Fonts->Icons, iconSize,
                          ImVec2(tabTopLeft.x + 15.0f, tabTopLeft.y + (tabHeight - iconSize) / 2),
                          ImColor(textColor), icon.c_str());
    }

    // Text
    drawList->AddText(ImVec2(tabTopLeft.x + 40.0f, tabTopLeft.y + (tabHeight - iconSize) / 2),
                      ImColor(textColor), moduleName.c_str());

    // Adjust cursor position for the next tab
    ImGui::SetCursorPosY(cursorPos.y + tabHeight + 10.0f); // Include some space between tabs
}

//    auto cursorPos2 = ImGui::GetCursorPos();
//     if (!checked)
//         return;
//     //Begin the tab window
//     //Move the cursor to  the top left of the window
//     ImGui::SetCursorPos(ImVec2(windowPos.x + 200,
//                                windowPos.y + ImGui::GetWindowHeight() * 0.133f));
//     //Draw text at the top of the tab
//     ImGui::Text(moduleName.c_str());
//     ImGui::Separator();
//     // Draw each module in the tab
//     for (auto& module : childModules)
//     {
//         if (module->IsEnabled())
//             module->Draw();
//     }
//     //restore the cursor position
//     ImGui::SetCursorPos(cursorPos2);
// }


void TabModule::Tick()
{
}
