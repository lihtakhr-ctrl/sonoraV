#pragma once
#include <windows.h>

struct Settings
{
    /* ---- ESP ---- */
    bool esp       = true;
    bool box       = true;
    bool health    = true;
    bool shield    = true;
    bool name      = true;
    bool distance  = true;
    bool skeleton  = false;
    bool teamCheck = true;

    /* ---- Aimbot（視点が動く普通のエイム）---- */
    bool  aimbot     = false;
    bool  aimOnKey   = true;
    int   aimKey     = VK_RBUTTON;   // 右クリック（旧: VK_XBUTTON2 = マウス5）
    float aimFov     = 150.f;
    float aimSmooth  = 6.f;
    int   aimBone    = 8;

    /* ---- SilentAim（撃ってる間だけ弾を曲げる）---- */
    bool  silentAim    = false;
    bool  silentOnKey  = true;
    int   silentKey    = VK_LBUTTON;
    float silentFov    = 150.f;
    int   silentBone   = 8;
    float silentSmooth = 1.f;      /* 1 = 即座 */
    bool  silentRestore= true;     /* 離したら元の視点に戻す */
    bool  silentVisibleOnly = false;
    bool  silentPredict= false;
    float silentSpeed  = 30000.f;  /* 弾速 (units/s) 先読み用 */

    /* ---- その他 ---- */
    int   maxFps   = 144;
    bool  menuOpen = true;
};

extern Settings cfg;

/* ボーンID */
enum BoneId { BONE_HEAD = 8, BONE_NECK = 6, BONE_CHEST = 4 };