#include "../pch.h"
#include "../include/D3D11Window.hpp"
#include "../include/Menu.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


typedef BOOL (WINAPI*hk_SetCursorPos)(int, int);
hk_SetCursorPos origSetCursorPos = NULL;

BOOL WINAPI HOOK_SetCursorPos(int X, int Y)
{
    if (DX11_Base::g_GameVariables->m_ShowMenu)
        return FALSE;

    return origSetCursorPos(X, Y);
}

bool HookCursor()
{
    if (MH_CreateHook(&SetCursorPos, &HOOK_SetCursorPos, reinterpret_cast<LPVOID*>(&origSetCursorPos)) != MH_OK)
        return FALSE;

    if (MH_EnableHook(&SetCursorPos) != MH_OK)
        return FALSE;

    return TRUE;
}

namespace DX11_Base
{
    static uint64_t* MethodsTable = NULL;

    LRESULT D3D11Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (g_GameVariables->m_ShowMenu)
        {
            ImGui_ImplWin32_WndProcHandler((HWND)g_D3D11Window->m_OldWndProc, msg, wParam, lParam);
            return TRUE;
        }
        return CallWindowProc((WNDPROC)g_D3D11Window->m_OldWndProc, hWnd, msg, wParam, lParam);
    }

    /// <summary>
    /// INITIALIZE
    /// </summary>
    bool D3D11Window::Hook()
    {
        if (InitHook())
        {
            CreateHook(8, (void**)&oIDXGISwapChainPresent, HookPresent);
            CreateHook(12, (void**)&oID3D11DrawIndexed, MJDrawIndexed);
            Sleep(1000);
#if DEBUG
            g_Console->printdbg("D3D11Window::Hook Initialized\n", g_Console->color.pink);
#endif
            return TRUE;
        }
#if DEBUG
        g_Console->printdbg("[+] D3D11Window::Hook Failed to Initialize\n", g_Console->color.red);
#endif
        return FALSE;
    }

    bool D3D11Window::CreateHook(uint16_t Index, void** Original, void* Function)
    {
        assert(Index >= 0 && Original != NULL && Function != NULL);
        void* target = (void*)MethodsTable[Index];
        if (MH_CreateHook(target, Function, Original) != MH_OK || MH_EnableHook(target) != MH_OK)
        {
            return FALSE;
        }
        return TRUE;
    }

    bool D3D11Window::InitHook()
    {
        if (!InitWindow())
            return FALSE;

        HMODULE D3D11Module = GetModuleHandleA("d3d11.dll");
        if (D3D11Module == NULL)
        {
            DeleteWindow();
            return FALSE;
        }
        HookCursor();
        D3D_FEATURE_LEVEL FeatureLevel;
        const D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0};

        DXGI_RATIONAL RefreshRate;
        RefreshRate.Numerator = 60;
        RefreshRate.Denominator = 1;

        DXGI_MODE_DESC BufferDesc;
        BufferDesc.Width = 100;
        BufferDesc.Height = 100;
        BufferDesc.RefreshRate = RefreshRate;
        BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        DXGI_SAMPLE_DESC SampleDesc;
        SampleDesc.Count = 1;
        SampleDesc.Quality = 0;

        DXGI_SWAP_CHAIN_DESC SwapChainDesc;
        SwapChainDesc.BufferDesc = BufferDesc;
        SwapChainDesc.SampleDesc = SampleDesc;
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.BufferCount = 1;
        SwapChainDesc.OutputWindow = WindowHwnd;
        SwapChainDesc.Windowed = 1;
        SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        IDXGISwapChain* SwapChain;
        ID3D11Device* Device;
        ID3D11DeviceContext* Context;
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, FeatureLevels, 1, D3D11_SDK_VERSION,
                                          &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context) < 0)
        {
            DeleteWindow();
            return FALSE;
        }

        MethodsTable = (uint64_t*)::calloc(205, sizeof(uint64_t));
        memcpy(MethodsTable, *(uint64_t**)SwapChain, 18 * sizeof(uint64_t));
        memcpy(MethodsTable + 18, *(uint64_t**)Device, 43 * sizeof(uint64_t));
        memcpy(MethodsTable + 18 + 43, *(uint64_t**)Context, 144 * sizeof(uint64_t));
        Sleep(1000);

        //	INIT NOTICE
        Beep(300, 300);

        MH_Initialize();
        SwapChain->Release();
        SwapChain = NULL;
        Device->Release();
        Device = NULL;
        Context->Release();
        Context = NULL;
        DeleteWindow();
        return TRUE;
    }

    bool D3D11Window::InitWindow()
    {
        WindowClass.cbSize = sizeof(WNDCLASSEX);
        WindowClass.style = CS_HREDRAW | CS_VREDRAW;
        WindowClass.lpfnWndProc = DefWindowProc;
        WindowClass.cbClsExtra = 0;
        WindowClass.cbWndExtra = 0;
        WindowClass.hInstance = GetModuleHandle(NULL);
        WindowClass.hIcon = NULL;
        WindowClass.hCursor = NULL;
        WindowClass.hbrBackground = NULL;
        WindowClass.lpszMenuName = NULL;
        WindowClass.lpszClassName = L"MJ";
        WindowClass.hIconSm = NULL;
        RegisterClassEx(&WindowClass);
        WindowHwnd = CreateWindow(WindowClass.lpszClassName, L"DX11 Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL,
                                  NULL, WindowClass.hInstance, NULL);
        if (WindowHwnd == NULL)
        {
            return FALSE;
        }
#if DEBUG
        g_Console->printdbg("D3D11Window::Window Created\n", g_Console->color.pink);
#endif
        return TRUE;
    }

    bool D3D11Window::DeleteWindow()
    {
        DestroyWindow(WindowHwnd);
        UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
        if (WindowHwnd != NULL)
        {
            return FALSE;
        }
#if DEBUG
        g_Console->printdbg("D3D11Window::Window Destroyed\n", g_Console->color.pink);
#endif
        return TRUE;
    }

    void embraceTheDarkness()
    {
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
        colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(8.00f, 8.00f);
        style.FramePadding = ImVec2(5.00f, 2.00f);
        style.CellPadding = ImVec2(6.00f, 6.00f);
        style.ItemSpacing = ImVec2(6.00f, 6.00f);
        style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
        style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
        style.IndentSpacing = 25;
        style.ScrollbarSize = 15;
        style.GrabMinSize = 10;
        style.WindowBorderSize = 1;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = 1;
        style.TabBorderSize = 1;
        style.WindowRounding = 7;
        style.ChildRounding = 4;
        style.FrameRounding = 3;
        style.PopupRounding = 4;
        style.ScrollbarRounding = 9;
        style.GrabRounding = 3;
        style.LogSliderDeadzone = 4;
        style.TabRounding = 4;
    }

    bool D3D11Window::Init(IDXGISwapChain* swapChain)
    {
        if (SUCCEEDED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_Device)))
        {
            ImGui::CreateContext();
            embraceTheDarkness();

            global_Fonts = std::make_unique<Fonts>();
            global_Fonts->LoadFonts();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.IniFilename = NULL;
            m_Device->GetImmediateContext(&m_DeviceContext);

            DXGI_SWAP_CHAIN_DESC Desc;
            swapChain->GetDesc(&Desc);
            g_GameVariables->g_GameWindow = Desc.OutputWindow;

            ID3D11Texture2D* BackBuffer;
            swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
            m_Device->CreateRenderTargetView(BackBuffer, NULL, &m_RenderTargetView);
            BackBuffer->Release();

            ImGui_ImplWin32_Init(g_GameVariables->g_GameWindow);
            ImGui_ImplDX11_Init(m_Device, m_DeviceContext);
            ImGui_ImplDX11_CreateDeviceObjects();
            ImGui::GetIO().ImeWindowHandle = g_GameVariables->g_GameWindow;
            m_OldWndProc = (WNDPROC)SetWindowLongPtr(g_GameVariables->g_GameWindow, GWLP_WNDPROC,
                                                     (__int3264)(LONG_PTR)WndProc);
            // hide the windows cursor
            b_ImGui_Initialized = TRUE;


#if DEBUG
            g_Console->printdbg("D3D11Window::Swapchain Initialized\n", g_Console->color.pink);
#endif
            return 1;
        }
        b_ImGui_Initialized = FALSE;
        return 0;
    }

    /// <summary>
    /// RENDER LOOP
    /// </summary>
    HRESULT APIENTRY D3D11Window::HookPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
    {
        if (g_KillSwitch)
        {
            g_Hooking->Unhook();
            g_D3D11Window->oIDXGISwapChainPresent(pSwapChain, SyncInterval, Flags);
            g_Running = FALSE;
            return 0;
        }
        g_D3D11Window->Overlay(pSwapChain);
        return g_D3D11Window->oIDXGISwapChainPresent(pSwapChain, SyncInterval, Flags);
    }

    void APIENTRY D3D11Window::MJDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation,
                                             INT BaseVertexLocation)
    {
        return;
    }

    static IDXGISwapChain* g_swap_chain = nullptr;

    void D3D11Window::Overlay(IDXGISwapChain* pSwapChain)
    {
        if (!b_ImGui_Initialized)
            Init(pSwapChain);

        if (g_swap_chain == nullptr)
            g_swap_chain = pSwapChain;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::GetIO().MouseDrawCursor = g_GameVariables->m_ShowMenu;
        //	Render Menu Loop
        global_Menu->Draw();
        ImGui::EndFrame();
        ImGui::Render();
        m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, NULL);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void D3D11Window::ResizeWindow()
    {
        //    Resize Window
        RECT rect;
        GetClientRect(g_GameVariables->g_GameWindow, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        //    Resize Render Target
        m_RenderTargetView->Release();
        ID3D11Texture2D* BackBuffer;
        g_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
        m_Device->CreateRenderTargetView(BackBuffer, NULL, &m_RenderTargetView);
        BackBuffer->Release();
    }

    /// <summary>
    /// UNHOOK
    /// </summary>
    void D3D11Window::Unhook()
    {
        SetWindowLongPtr(g_GameVariables->g_GameWindow, GWLP_WNDPROC, (LONG_PTR)m_OldWndProc);
        DisableAll();
        return;
    }

    void D3D11Window::DisableHook(uint16_t Index)
    {
        assert(Index >= 0);
        MH_DisableHook((void*)MethodsTable[Index]);
        return;
    }

    void D3D11Window::DisableAll()
    {
        DisableHook(8);
        DisableHook(12);
        free(MethodsTable);
        MethodsTable = NULL;
        return;
    }

    D3D11Window::~D3D11Window()
    {
        Unhook();
    }
}
