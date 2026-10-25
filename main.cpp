#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <cstdio>
#include <cstdarg>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "Config.hpp"
#include "Game.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

Settings cfg;
int g_screenW = 1920, g_screenH = 1080;

/* ============================================================
 *  ホットキー（変えたい場合はここ）
 * ============================================================ */
namespace hk
{
    constexpr int Menu1 = VK_INSERT;
    constexpr int Menu2 = VK_F9;
    constexpr int Esp = VK_F1;
    constexpr int Silent = VK_F2;
    constexpr int Aimbot = VK_F3;
    constexpr int Hud = VK_F4;   // デバッグHUD表示切替
}

/* ============================================================
 *  キー入力（クールダウン付き・フォーカス不要）
 * ============================================================ */
static bool      g_prevKey[256] = {};
static ULONGLONG g_lastPress[256] = {};
static int       g_pressCount = 0;
static char      g_lastKeyName[32] = "-";

static bool KeyPressed(int vk, int cooldownMs = 300)
{
    if (vk < 0 || vk > 255) return false;

    bool now = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool edge = now && !g_prevKey[vk];
    g_prevKey[vk] = now;
    if (!edge) return false;

    ULONGLONG t = GetTickCount64();
    if (t - g_lastPress[vk] < (ULONGLONG)cooldownMs) {
        printf("[key] %d はクールダウン中に再検出\n", vk);
        return false;
    }
    g_lastPress[vk] = t;
    ++g_pressCount;
    sprintf_s(g_lastKeyName, "VK 0x%02X", vk);
    printf("[key] 押下 vk=0x%02X (通算 %d 回)\n", vk, g_pressCount);
    return true;
}

/* ===================== 通知 ===================== */
static char      g_notify[128] = {};
static ULONGLONG g_notifyUntil = 0;

static void Notify(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsprintf_s(g_notify, fmt, ap);
    va_end(ap);
    g_notifyUntil = GetTickCount64() + 3000;   /* 3秒 */
}

static void DrawNotify()
{
    if (!g_notify[0]) return;
    ULONGLONG now = GetTickCount64();
    if (now >= g_notifyUntil) { g_notify[0] = 0; return; }

    float remain = (float)(g_notifyUntil - now) / 1000.f;
    float alpha = std::clamp(remain / 0.5f, 0.f, 1.f);   /* 最後の0.5秒でフェード */

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImVec2 sz = ImGui::CalcTextSize(g_notify);
    ImVec2 pos{ (g_screenW - sz.x) * 0.5f, 70.f };

    dl->AddRectFilled({ pos.x - 12, pos.y - 7 }, { pos.x + sz.x + 12, pos.y + sz.y + 7 },
        IM_COL32(0, 0, 0, (int)(180 * alpha)), 6.f);
    dl->AddText(pos, IM_COL32(255, 220, 80, (int)(255 * alpha)), g_notify);
}

/* ===================== デバッグHUD ===================== */
static bool g_showHud = true;

static void DrawHud()
{
    if (!g_showHud) return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    char buf[384];

    bool alive   = g_game.Alive();
    int  enemies = (int)g_game.players.size();

    sprintf_s(buf,
        "FPS %.0f | PID %lu | alive=%d\n"
        "Enemies: %d | Self: %s\n"
        "menu=%d  esp=%d  silent=%d  aimbot=%d\n"
        "key=%s  cnt=%d\n"
        "lp=0x%llX  team=%d",
        ImGui::GetIO().Framerate, g_game.mem.Pid(), alive ? 1 : 0,
        enemies,
        (g_game.localPlayer > 0x10000) ? "ALIVE" : "DEAD/NONE",
        cfg.menuOpen ? 1 : 0, cfg.esp ? 1 : 0, cfg.silentAim ? 1 : 0, cfg.aimbot ? 1 : 0,
        g_lastKeyName, g_pressCount,
        g_game.localPlayer, g_game.localTeam);

    ImU32 bgCol  = (alive && enemies == 0) ? IM_COL32(120, 0, 0, 180) : IM_COL32(0, 0, 0, 160);
    ImU32 txtCol = (alive && enemies == 0) ? IM_COL32(255, 80, 80, 255) : IM_COL32(0, 255, 180, 255);

    dl->AddRectFilled({ 8, 8 }, { 360, 110 }, bgCol, 4.f);
    dl->AddText({ 14, 12 }, txtCol, buf);
}

/* ===================== D3D11 ===================== */
static ID3D11Device* g_dev = nullptr;
static ID3D11DeviceContext* g_ctx = nullptr;
static IDXGISwapChain* g_swap = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static HWND                    g_hwnd = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static void ApplyClickThrough(bool clickThrough)
{
    LONG_PTR ex = GetWindowLongPtrW(g_hwnd, GWL_EXSTYLE);
    if (clickThrough) {
        ex |=  WS_EX_TRANSPARENT;
        ex |=  WS_EX_NOACTIVATE;   // メニューOFF時はフォーカスを奪わない
    } else {
        ex &= ~WS_EX_TRANSPARENT;
        ex &= ~WS_EX_NOACTIVATE;   // メニューON時はフォーカスを受け取る
    }
    SetWindowLongPtrW(g_hwnd, GWL_EXSTYLE, ex);

    /* exstyle 変更を確実に反映させる */
    SetWindowPos(g_hwnd, NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED |
        (clickThrough ? SWP_NOACTIVATE : 0));

    /* メニューON時はフォーカスを明示的に取得する */
    if (!clickThrough)
        SetForegroundWindow(g_hwnd);
}

static void ToggleMenu()
{
    cfg.menuOpen = !cfg.menuOpen;
    ApplyClickThrough(!cfg.menuOpen);
    Notify("Menu %s", cfg.menuOpen ? "ON" : "OFF");
    printf("[menu] %s\n", cfg.menuOpen ? "OPEN" : "CLOSE");
}

static LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM w, LPARAM l)
{
    if (ImGui_ImplWin32_WndProcHandler(h, msg, w, l)) return true;
    if (msg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProcW(h, msg, w, l);
}

static void InitConsole()
{
    if (!AllocConsole()) return;
    SetConsoleOutputCP(CP_UTF8);          /* ★ 日本語の文字化け対策 */
    FILE* f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
    SetConsoleTitleW(L"R5R Cheat - log");
}

static bool CreateOverlay()
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"R5ROverlayWnd";
    RegisterClassExW(&wc);

    g_screenW = GetSystemMetrics(SM_CXSCREEN);
    g_screenH = GetSystemMetrics(SM_CYSCREEN);

    /* 初期状態: cfg.menuOpen = true なのでクリックスルーOFF・NOACTIVATEなし */
    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"R5R Overlay", WS_POPUP,
        0, 0, g_screenW, g_screenH, nullptr, nullptr, wc.hInstance, nullptr);
    if (!g_hwnd) return false;

    SetLayeredWindowAttributes(g_hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = g_screenW;
    sd.BufferDesc.Height = g_screenH;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL lvl;
    const D3D_FEATURE_LEVEL lvls[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        lvls, 2, D3D11_SDK_VERSION, &sd, &g_swap,
        &g_dev, &lvl, &g_ctx)))
        return false;

    ID3D11Texture2D* back = nullptr;
    g_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back);
    g_dev->CreateRenderTargetView(back, nullptr, &g_rtv);
    if (back) back->Release();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_dev, g_ctx);
    return true;
}

/* ===================== ESP ===================== */
static void DrawESP()
{
    if (!cfg.esp) return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const float cx = g_screenW * 0.5f, cy = g_screenH * 0.5f;

    for (const Player& p : g_game.players) {
        Vec2 sh{}, sf{};
        if (!WorldToScreen(p.head, sh, g_game.viewMatrix, g_screenW, g_screenH)) continue;
        if (!WorldToScreen(p.origin, sf, g_game.viewMatrix, g_screenW, g_screenH)) continue;

        float h = sf.y - sh.y;
        if (h < 2.f || h > 2000.f) continue;
        float w = h * 0.5f;
        float x = sh.x - w * 0.5f, y = sh.y;

        ImU32 col = (p.team == g_game.localTeam) ? IM_COL32(0, 255, 120, 220)
            : IM_COL32(255, 80, 80, 230);
        if (cfg.box) dl->AddRect({ x, y }, { x + w, y + h }, col, 0.f, 0, 1.5f);

        if (cfg.health) {
            float hp = std::clamp(p.health / 100.f, 0.f, 1.f);
            dl->AddRectFilled({ x - 6, y + h * (1.f - hp) }, { x - 2, y + h },
                IM_COL32(60, 255, 60, 220));
        }
        if (cfg.shield && p.shield > 0.f) {
            float sv = std::clamp(p.shield / 100.f, 0.f, 1.f);
            dl->AddRectFilled({ x - 11, y + h * (1.f - sv) }, { x - 7, y + h },
                IM_COL32(90, 170, 255, 220));
        }
        if (cfg.name && !p.name.empty()) {
            ImVec2 sz = ImGui::CalcTextSize(p.name.c_str());
            dl->AddText({ sh.x - sz.x * 0.5f, y - 16 }, IM_COL32(255, 255, 255, 235), p.name.c_str());
        }
        if (cfg.distance) {
            char b[32];
            sprintf_s(b, "[%.0fm]", p.distance);
            ImVec2 sz = ImGui::CalcTextSize(b);
            dl->AddText({ sh.x - sz.x * 0.5f, y + h + 2 }, IM_COL32(230, 230, 230, 220), b);
        }
    }

    if (cfg.aimbot)
        dl->AddCircle({ cx, cy }, cfg.aimFov, IM_COL32(255, 255, 255, 80), 64, 1.2f);
    if (cfg.silentAim)
        dl->AddCircle({ cx, cy }, cfg.silentFov, IM_COL32(255, 200, 0, 90), 64, 1.2f);
}

/* ===================== メニュー ===================== */
static void DrawMenu()
{
    if (!cfg.menuOpen) return;

    ImGui::SetNextWindowSize({ 420, 580 }, ImGuiCond_FirstUseEver);
    ImGui::Begin("R5Reloaded Cheat", &cfg.menuOpen);

    ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("PID %lu / 敵 %d", g_game.mem.Pid(), (int)g_game.players.size());
    ImGui::TextDisabled("INSERT / F9 = メニュー   F4 = HUD");
    ImGui::TextDisabled("F1=ESP  F2=Silent  F3=Aimbot");
    ImGui::Separator();

    if (ImGui::CollapsingHeader("ESP", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable##esp", &cfg.esp);
        ImGui::Checkbox("Box##esp", &cfg.box);
        ImGui::Checkbox("Health##esp", &cfg.health);
        ImGui::Checkbox("Shield##esp", &cfg.shield);
        ImGui::Checkbox("Name##esp", &cfg.name);
        ImGui::Checkbox("Distance##esp", &cfg.distance);
        ImGui::Checkbox("Team check##esp", &cfg.teamCheck);
    }

    if (ImGui::CollapsingHeader("Silent Aim", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enable##silent", &cfg.silentAim);
        ImGui::Checkbox("射撃キーで発動##silent", &cfg.silentOnKey);
        ImGui::Checkbox("離したら視点を戻す##silent", &cfg.silentRestore);
        ImGui::Checkbox("先読み(弾速)##silent", &cfg.silentPredict);
        if (cfg.silentPredict)
            ImGui::SliderFloat("弾速##silent", &cfg.silentSpeed, 5000.f, 60000.f, "%.0f u/s");

        const char* bones[] = { "頭", "首", "胸" };
        int boneIdx = (cfg.silentBone == BONE_HEAD) ? 0
            : (cfg.silentBone == BONE_NECK) ? 1 : 2;
        if (ImGui::Combo("ボーン##silent", &boneIdx, bones, 3))
            cfg.silentBone = (boneIdx == 0) ? BONE_HEAD
            : (boneIdx == 1) ? BONE_NECK : BONE_CHEST;

        ImGui::SliderFloat("FOV##silent", &cfg.silentFov, 5.f, 600.f, "%.0f px");
        ImGui::SliderFloat("Smooth##silent", &cfg.silentSmooth, 1.f, 20.f, "%.1f");
    }

    if (ImGui::CollapsingHeader("Aimbot (通常)")) {
        ImGui::Checkbox("Enable##aimbot", &cfg.aimbot);
        ImGui::Checkbox("Hold key##aimbot", &cfg.aimOnKey);
        if (cfg.aimOnKey) {
            /* キー名を表示 */
            const char* keyName = "Right Click";
            if      (cfg.aimKey == VK_RBUTTON)  keyName = "Right Click";
            else if (cfg.aimKey == VK_LBUTTON)  keyName = "Left Click";
            else if (cfg.aimKey == VK_MBUTTON)  keyName = "Middle Click";
            else if (cfg.aimKey == VK_XBUTTON1) keyName = "Mouse4";
            else if (cfg.aimKey == VK_XBUTTON2) keyName = "Mouse5";
            ImGui::TextDisabled("  発動キー: %s", keyName);
        }
        ImGui::SliderFloat("FOV##aimbot", &cfg.aimFov, 5.f, 600.f, "%.0f px");
        ImGui::SliderFloat("Smooth##aimbot", &cfg.aimSmooth, 1.f, 30.f, "%.1f");

        const char* abones[] = { "頭", "首", "胸" };
        int aboneIdx = (cfg.aimBone == BONE_HEAD) ? 0
            : (cfg.aimBone == BONE_NECK) ? 1 : 2;
        if (ImGui::Combo("ボーン##aimbot", &aboneIdx, abones, 3))
            cfg.aimBone = (aboneIdx == 0) ? BONE_HEAD
            : (aboneIdx == 1) ? BONE_NECK : BONE_CHEST;
    }

    if (ImGui::CollapsingHeader("Misc")) {
        ImGui::SliderInt("Max FPS", &cfg.maxFps, 30, 300);
        ImGui::Checkbox("デバッグHUD (F4)", &g_showHud);
    }

    ImGui::End();
}

/* ===================== main ===================== */
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    InitConsole();
    printf("==== R5Reloaded Cheat 起動 ====\n");

    if (!g_game.Attach()) {
        printf("[!] アタッチ失敗\n");
        char msg[512];
        sprintf_s(msg,
            "r5apex.exe をアタッチできませんでした。\n\n"
            "・R5Reloaded を起動してから実行していますか？\n"
            "・retail Apex が同時に起動していませんか？\n\n"
            "詳しくはコンソールウィンドウを見てください。");
        MessageBoxA(nullptr, msg, "R5Reloaded Cheat", MB_ICONERROR);
        return 1;
    }
    printf("[ok] アタッチ完了 pid=%lu\n", g_game.mem.Pid());

    if (!CreateOverlay()) {
        printf("[!] オーバーレイ作成失敗\n");
        MessageBoxA(nullptr, "オーバーレイ作成に失敗", "R5Reloaded Cheat", MB_ICONERROR);
        return 1;
    }
    printf("[ok] オーバーレイ作成完了\n");

    while (true)
    {
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) goto done;
        }

        /* ---- ホットキー：両方のキーを必ず評価する ---- */
        {
            bool m1 = KeyPressed(hk::Menu1);
            bool m2 = KeyPressed(hk::Menu2);
            if (m1 || m2) ToggleMenu();

            if (KeyPressed(hk::Esp)) { cfg.esp = !cfg.esp;       Notify("ESP %s", cfg.esp ? "ON" : "OFF"); }
            if (KeyPressed(hk::Silent)) { cfg.silentAim = !cfg.silentAim; Notify("SilentAim %s", cfg.silentAim ? "ON" : "OFF"); }
            if (KeyPressed(hk::Aimbot)) { cfg.aimbot = !cfg.aimbot;    Notify("Aimbot %s", cfg.aimbot ? "ON" : "OFF"); }
            if (KeyPressed(hk::Hud)) { g_showHud = !g_showHud;     Notify("HUD %s", g_showHud ? "ON" : "OFF"); }
        }

        if (g_game.Alive()) {
            g_game.Update();
            g_game.RunSilentAim();
            g_game.RunAimbot();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawESP();
        DrawMenu();
        DrawNotify();
        DrawHud();

        ImGui::Render();

        const float clear[4] = { 0, 0, 0, 1 };
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_ctx->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swap->Present(0, 0);

        Sleep(1000 / std::max(30, cfg.maxFps));
    }

done:
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    return 0;
}