#include "pch.h"
#include "../Public/GUI.h"
#include "../../FortniteGame/Public/BattleRoyaleGamePhaseLogic.h"
#include "../../FortniteGame/Public/BuildingSMActor.h"
#include "../../ImGui/imgui.h"
#include "../../ImGui/imgui_impl_dx11.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../Public/Configuration.h"
#include "../Public/Events.h"
#include <d3d11.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

UINT g_ResizeWidth = 0, g_ResizeHeight = 0;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

auto WindowWidth = 533;
auto WindowHeight = 400;

void GUI::Init()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    WNDCLASS wc{};
    wc.lpszClassName = L"Akyi";
    wc.lpfnWndProc = WndProc;
    RegisterClass(&wc);

    wchar_t buffer[67];
    swprintf_s(buffer, VersionInfo.EngineVersion >= 5.0 ? L"gsproductions" : L"gsproductions");

    auto hWnd = CreateWindow(wc.lpszClassName, buffer, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, 100, 100,
        (int)(WindowWidth * main_scale), (int)(WindowHeight * main_scale), nullptr, nullptr, nullptr, nullptr);

    IDXGISwapChain* g_pSwapChain = nullptr;
    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK) return;

    ID3D11RenderTargetView* g_mainRenderTargetView;
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    DWORD dwMyID = ::GetCurrentThreadId();
    DWORD dwCurID = ::GetWindowThreadProcessId(hWnd, NULL);
    AttachThreadInput(dwCurID, dwMyID, TRUE);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    SetForegroundWindow(hWnd);
    SetFocus(hWnd);
    SetActiveWindow(hWnd);
    AttachThreadInput(dwCurID, dwMyID, FALSE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImFontConfig FontConfig;
    FontConfig.FontDataOwnedByAtlas = false;
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF((void*)font, sizeof(font), 17.f, &FontConfig);

    auto& mStyle = ImGui::GetStyle();
    ImGuiStyle& style = mStyle;

    // Galaxy Theme
    mStyle.WindowRounding = 6.f; mStyle.ChildRounding = 6.f; mStyle.FrameRounding = 4.5f;
    mStyle.PopupRounding = 5.f; mStyle.TabRounding = 4.f; mStyle.ScrollbarRounding = 16.f;
    mStyle.GrabRounding = 16.f; mStyle.WindowBorderSize = 1.f; mStyle.FrameBorderSize = 0.5f;
    mStyle.ItemSpacing = ImVec2(20, 6); mStyle.ItemInnerSpacing = ImVec2(8, 4);
    mStyle.GrabMinSize = 14.f; mStyle.ScrollbarSize = 10.f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.06f, 0.17f, 0.90f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.05f, 0.14f, 0.96f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.03f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05f, 0.03f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.82f, 0.78f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.06f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.12f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.18f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.35f, 0.15f, 0.70f, 0.70f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.05f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.10f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_TabSelected] = ImVec4(0.12f, 0.07f, 0.26f, 1.00f);
    // Add more colors from your original file if needed

    style.ScaleAllSizes(main_scale);

    ImVec4 clear_color = ImVec4(0.03f, 0.02f, 0.08f, 1.00f);

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool done = false;
    bool g_SwapChainOccluded = false;

    static char commandBuffer[1024] = { 0 };   // ← MOVED OUTSIDE SWITCH (FIXED)

    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            Sleep(10); continue;
        }

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_mainRenderTargetView->Release();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(WindowWidth * main_scale, WindowHeight * main_scale), ImGuiCond_Always);

        ImGui::Begin("Akyi/gsproductions", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        static int SelectedUI = 0;
        static int hasEvent = 0;

        if (hasEvent == 0)
        {
            hasEvent = 1;
            for (auto& Event : Events::EventsArray)
                if (Event.EventVersion == VersionInfo.FortniteVersion)
                    hasEvent = 2;
        }

        if (ImGui::BeginTabBar(""))
        {
            if (ImGui::BeginTabItem("Main")) { SelectedUI = 0; ImGui::EndTabItem(); }
            if (gsStatus == StartedMatch)
            {
                if (ImGui::BeginTabItem("Zones")) { SelectedUI = 1; ImGui::EndTabItem(); }
                if (hasEvent == 2 && ImGui::BeginTabItem("Events")) { SelectedUI = 2; ImGui::EndTabItem(); }
            }
            if (ImGui::BeginTabItem("Playlist")) { SelectedUI = 5; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Dump")) { SelectedUI = 4; ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Misc")) { SelectedUI = 3; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        auto GameMode = UWorld::GetWorld() ? (AFortGameMode*)UWorld::GetWorld()->AuthorityGameMode : nullptr;

        switch (SelectedUI)
        {
        case 0: // Main
            if (gsStatus >= Joinable)
                ImGui::BeginChild("ServerInfo", ImVec2(245 * main_scale, 130 * main_scale), ImGuiChildFlags_Borders);
            ImGui::Text((std::string("Status: ") + (gsStatus == NotReady ? "Setting up the server..." : (gsStatus == Joinable ? "Joinable!" : "Match Started"))).c_str());
            if (gsStatus >= Joinable)
            {
                ImGui::Text((std::string("Player Count: ") + std::to_string(GameMode->HasAlivePlayers() ? GameMode->AlivePlayers.Num() : 0)).c_str());
                ImGui::Text((std::string("Port: ") + std::to_string(FConfiguration::Port)).c_str());
            }
            if (gsStatus >= Joinable) ImGui::EndChild();

            if (gsStatus <= Joinable)
                ImGui::Checkbox("Lategame", &FConfiguration::bLateGame);

            if (gsStatus == Joinable && ImGui::Button("Start Bus Early"))
            {
                if (UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetDefaultObj())
                    UFortGameStateComponent_BattleRoyaleGamePhaseLogic::bStartAircraft = true;
                else
                    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(L"startaircraft"), nullptr);
            }

            ImGui::InputText("Console Command", commandBuffer, 1024);
            if (ImGui::Button("Execute"))
            {
                std::string str = commandBuffer;
                auto wstr = std::wstring(str.begin(), str.end());
                UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(wstr.c_str()), nullptr);
            }
            break;

        case 1: // Zones
            if (ImGui::Button("Pause Safe Zone"))
            {
                UFortGameStateComponent_BattleRoyaleGamePhaseLogic::bPausedZone = true;
                UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(L"pausesafezone"), nullptr);
            }
            if (ImGui::Button("Resume Safe Zone"))
            {
                UFortGameStateComponent_BattleRoyaleGamePhaseLogic::bPausedZone = false;
                UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(L"startsafezone"), nullptr);
            }
            // Add your other zone buttons here
            break;

        case 2: // Events
            if (ImGui::Button("Start Event"))
                Events::StartEvent();
            break;

        case 3: // Misc
            ImGui::Checkbox("Infinite Materials", &FConfiguration::bInfiniteMats);
            ImGui::Checkbox("Infinite Ammo", &FConfiguration::bInfiniteAmmo);
            ImGui::Checkbox("Keep Inventory", &FConfiguration::bKeepInventory);
            ImGui::SliderInt("Siphon Amount:", &FConfiguration::SiphonAmount, 0, 200);
            ImGui::SliderInt("Tick Rate:", &FConfiguration::MaxTickRate, 30, 120);
            if (ImGui::Button("Reset Builds"))
            {
                TArray<ABuildingSMActor*> Builds;
                Utils::GetAll<ABuildingSMActor>(Builds);
                for (auto& Build : Builds)
                    if (Build->bPlayerPlaced) Build->K2_DestroyActor();
                Builds.Free();
            }
            if (ImGui::Button("Destroy Floor Loot"))
            {
                TArray<AFortPickupAthena*> Pickups;
                Utils::GetAll<AFortPickupAthena>(Pickups);
                for (auto& Pickup : Pickups)
                    Pickup->K2_DestroyActor();
                Pickups.Free();
            }
            break;

        case 4: // Dump
            static auto PlaylistClass = UFortPlaylistAthena::StaticClass();
            if (ImGui::Button("Dump Items"))
            {
                // Your original Dump Items code here
            }
            else if (PlaylistClass && ImGui::Button("Dump Playlists"))
            {
                // Your original Dump Playlists code here
            }
            break;

        case 5: // Playlist
            ImGui::Text("Change Game Mode / Playlist");
            ImGui::Separator();
            ImGui::Spacing();

            static int currentPlaylistIndex = 0;

            const char* playlistNames[] = {
                "Normal Solo", "Normal Duos", "Arena Solos", "Arena Duos",
                "GG Reverse", "Playground", "Playground V2", "Fill Solo",
                "Low Gravity Solo", "Bling Solo", "Carmine", "Ashton SM",
                "Battle Lab", "Mole Game"
            };

            const wchar_t* playlistPaths[] = {
                L"",
                L"/Game/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo",
                L"/Game/Athena/Playlists/Showdown/Playlist_ShowdownAlt_Solo.Playlist_ShowdownAlt_Solo",
                L"/Game/Athena/Playlists/Playlist_DefaultDuo.Playlist_DefaultDuo",
                L"/Game/Athena/Playlists/gg/Playlist_Gg_Reverse.Playlist_Gg_Reverse",
                L"/Game/Athena/Playlists/Playground/Playlist_Playground.Playlist_Playground",
                L"/Game/Athena/Playlists/Creative/Playlist_PlaygroundV2.Playlist_PlaygroundV2",
                L"/Game/Athena/Playlists/Fill/Playlist_Fill_Solo.Playlist_Fill_Solo",
                L"/Game/Athena/Playlists/Low/Playlist_Low_Solo.Playlist_Low_Solo",
                L"/Game/Athena/Playlists/Bling/Playlist_Bling_Solo.Playlist_Bling_Solo",
                L"/Game/Athena/Playlists/Carmine/Playlist_Carmine.Playlist_Carmine",
                L"/Game/Athena/Playlists/Ashton/Playlist_Ashton_Sm.Playlist_Ashton_Sm",
                L"/Game/Athena/Playlists/BattleLab/Playlist_BattleLab.Playlist_BattleLab",
                L"/MoleGame/Playlists/Playlist_MoleGame.Playlist_MoleGame"
            };

            ImGui::Combo("Select Playlist", &currentPlaylistIndex, playlistNames, IM_ARRAYSIZE(playlistNames));

            if (ImGui::Button("Apply Playlist"))
            {
                FConfiguration::Playlist = playlistPaths[currentPlaylistIndex];
            }

            ImGui::Spacing();
            ImGui::Text("Current Playlist:");
            std::wstring ws(FConfiguration::Playlist);
            std::string str(ws.begin(), ws.end());
            ImGui::TextWrapped(str.c_str());

            ImGui::Spacing();
            if (ImGui::Button("Restart Match (Recommended)"))
            {
                UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), FString(L"servertravel"), nullptr);
            }
            break;
        }

        ImGui::End();
        ImGui::Render();

        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_pSwapChain->Release();
    g_pd3dDeviceContext->Release();
    g_pd3dDevice->Release();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    TerminateProcess(GetCurrentProcess(), 0);
}