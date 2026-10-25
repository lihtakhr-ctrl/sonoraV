#define NOMINMAX
#include "Game.hpp"
#include <windows.h>
#include <cmath>
#include <algorithm>
#include "Config.hpp"

#define RAD2DEG 57.29577951308232f

Game g_game;

static struct {
    bool active = false;
    Vec3 saved{};
} g_silent;

Vec3  operator-(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
float Length(const Vec3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

bool WorldToScreen(const Vec3& w, Vec2& out, const Mat4& vm, int sw, int sh)
{
    float cx = w.x * vm.m[0][0] + w.y * vm.m[1][0] + w.z * vm.m[2][0] + vm.m[3][0];
    float cy = w.x * vm.m[0][1] + w.y * vm.m[1][1] + w.z * vm.m[2][1] + vm.m[3][1];
    float cw = w.x * vm.m[0][3] + w.y * vm.m[1][3] + w.z * vm.m[2][3] + vm.m[3][3];
    if (cw < 0.01f) return false;
    out.x = (sw * 0.5f) * (1.f + cx / cw);
    out.y = (sh * 0.5f) * (1.f - cy / cw);
    return true;
}

static Vec3 CalcAngle(const Vec3& from, const Vec3& to)
{
    Vec3 d = to - from;
    float len = Length(d);
    if (len < 0.001f) return { 0, 0, 0 };
    return { -asinf(d.z / len) * RAD2DEG, atan2f(d.y, d.x) * RAD2DEG, 0.f };
}

static float NormYaw(float y)
{
    while (y > 180.f) y -= 360.f;
    while (y < -180.f) y += 360.f;
    return y;
}

bool Game::Attach()
{
    if (!mem.Attach(L"r5apex.exe")) return false;
    if (!mem.ModuleBase(L"r5apex.exe", moduleBase, moduleSize)) return false;
    return AutoResolve(mem, moduleBase, moduleSize);
}

bool Game::Update()
{
    if (!mem.Alive()) return false;

    /* --- ローカルプレイヤーを直接読む（R5Reloaded） --- */
    uint64_t lp = mem.Read<uint64_t>(moduleBase + goff.localPlayer);
    if (lp < 0x10000) return false;
    localPlayer = lp;

    /* --- 自分が死亡中なら敵リストをクリアして処理スキップ --- */
    int localLife = mem.Read<int>(localPlayer + goff.m_lifeState);
    if (localLife != 0) {
        players.clear();
        return true;   // 接続は維持したまま ESP だけ止める
    }

    localTeam   = mem.Read<int>(localPlayer + goff.m_iTeamNum);
    localOrigin = mem.Read<Vec3>(localPlayer + goff.m_vecAbsOrigin);
    localCam    = mem.Read<Vec3>(localPlayer + goff.m_cameraOrigin);

    /* --- ViewMatrix は3段ポインタ --- */
    uint64_t vr = mem.Read<uint64_t>(moduleBase + goff.viewRender);
    if (vr < 0x10000) return false;
    uint64_t pvm = mem.Read<uint64_t>(vr + goff.viewMatrix);
    if (pvm < 0x10000) return false;
    mem.Read(pvm, &viewMatrix, sizeof(Mat4));

    static std::vector<uint8_t> ebuf;
    ebuf.resize((size_t)goff.MAX_ENTITIES * goff.ENTITY_STRIDE);
    if (!mem.Read(moduleBase + goff.entityList, ebuf.data(), ebuf.size())) return false;

    players.clear();

    for (int i = 1; i < goff.MAX_ENTITIES; ++i) {
        uint64_t ent = *(uint64_t*)(ebuf.data() + (size_t)i * goff.ENTITY_STRIDE);
        if (!ent || ent == localPlayer) continue;

        /* --- 生死・健康チェック --- */
        int lifeState = mem.Read<int>(ent + goff.m_lifeState);
        if (lifeState != 0) continue;   // 0 = LIFE_ALIVE のみ（死体・幽霊を除外）

        int health    = mem.Read<int>(ent + goff.m_iHealth);
        int team      = mem.Read<int>(ent + goff.m_iTeamNum);
        int shieldMax = mem.Read<int>(ent + goff.m_shieldMax);

        if (health <= 0 || health > 100) continue;  // health は最大100
        if (team < 1    || team > 100)  continue;   // R5R は高いチーム番号を使う場合あり
        if (cfg.teamCheck && team == localTeam) continue;

        /* --- origin がゼロなら無効エンティティ --- */
        Vec3 origin = mem.Read<Vec3>(ent + goff.m_vecAbsOrigin);
        if (origin.x == 0.f && origin.y == 0.f && origin.z == 0.f) continue;

        Player p{};
        p.ptr    = ent;
        p.index  = i;
        p.health = (float)health;
        p.team   = team;
        p.shield = (float)mem.Read<int>(ent + goff.m_shieldHealth);
        p.origin = origin;
        p.bones  = mem.Read<uint64_t>(ent + goff.m_bones);
        p.head   = BonePos(p, BONE_HEAD);

        Vec3 d = p.origin - localOrigin;
        p.distance = Length(d) * 0.01905f;

        if (goff.nameList) {
            uint64_t np = mem.Read<uint64_t>(moduleBase + goff.nameList + (uint64_t)i * 8);
            if (np) p.name = mem.ReadStr(np, 32);
        }
        players.push_back(p);
    }
    return true;
}

/* m_pBoneMatrix は matrix3x4 の配列ポインタ。
 * 平行移動成分は各行列の +0x0C / +0x1C / +0x2C。 */
Vec3 Game::BonePos(const Player& p, int bone)
{
    if (!p.bones) return { p.origin.x, p.origin.y, p.origin.z + 64.f };

    struct Mat3x4 { float m[3][4]; };   // 0x30 bytes
    Mat3x4 b{};

    if (!mem.Read(p.bones + (uint64_t)bone * goff.BONE_STRIDE, &b, sizeof(b)))
        return { p.origin.x, p.origin.y, p.origin.z + 64.f };

    /* R5Reloaded の BoneMatrix はワールド絶対座標なので origin を足さない */
    return { b.m[0][3], b.m[1][3], b.m[2][3] };
}

const Player* Game::PickTarget(float fovPx, int bone, bool visibleOnly)
{
    if (players.empty()) return nullptr;

    extern int g_screenW, g_screenH;
    const float cx = g_screenW * 0.5f, cy = g_screenH * 0.5f;

    float best = fovPx;
    const Player* target = nullptr;

    for (const Player& p : players) {
        Vec3 aim = BonePos(p, bone);
        Vec2 s{};
        if (!WorldToScreen(aim, s, viewMatrix, g_screenW, g_screenH)) continue;

        float d = sqrtf((s.x - cx) * (s.x - cx) + (s.y - cy) * (s.y - cy));
        if (d < best) { best = d; target = &p; }
    }
    (void)visibleOnly;
    return target;
}

/* ================= 通常エイム（視点が動く） ================= */
void Game::RunAimbot()
{
    if (!cfg.aimbot || !localPlayer || players.empty()) return;
    if (cfg.aimOnKey && !(GetAsyncKeyState(cfg.aimKey) & 0x8000)) return;

    const Player* t = PickTarget(cfg.aimFov, cfg.aimBone, false);
    if (!t) return;

    Vec3 aim = BonePos(*t, cfg.aimBone);
    Vec3 want = CalcAngle(localCam, aim);
    Vec3 cur = mem.Read<Vec3>(localPlayer + goff.m_viewAngles);

    float s = std::max(1.f, cfg.aimSmooth);
    Vec3 next{ cur.x + (want.x - cur.x) / s,
               cur.y + NormYaw(want.y - cur.y) / s,
               cur.z };
    mem.Write(localPlayer + goff.m_viewAngles, next);
}

/* ================= SilentAim（弾だけ曲げる） ================= */
void Game::RunSilentAim()
{
    if (!cfg.silentAim || !localPlayer) return;

    bool fire = cfg.silentOnKey ? ((GetAsyncKeyState(cfg.silentKey) & 0x8000) != 0) : true;

    if (fire && !g_silent.active) {
        g_silent.saved = mem.Read<Vec3>(localPlayer + goff.m_viewAngles);
        g_silent.active = true;
    }

    if (fire) {
        const Player* t = PickTarget(cfg.silentFov, cfg.silentBone, cfg.silentVisibleOnly);
        if (!t) return;

        Vec3 aim = BonePos(*t, cfg.silentBone);

        if (cfg.silentPredict) {
            Vec3  vel = mem.Read<Vec3>(t->ptr + goff.m_vecAbsVelocity);
            float dist = Length(aim - localCam);
            float tt = dist / std::max(1.f, cfg.silentSpeed);
            aim.x += vel.x * tt;
            aim.y += vel.y * tt;
            aim.z += vel.z * tt;
        }

        Vec3 want = CalcAngle(localCam, aim);
        Vec3 cur = mem.Read<Vec3>(localPlayer + goff.m_viewAngles);

        Vec3 out;
        if (cfg.silentSmooth > 1.0f) {
            float s = cfg.silentSmooth;
            out = { cur.x + (want.x - cur.x) / s,
                    cur.y + NormYaw(want.y - cur.y) / s,
                    0.f };
        }
        else {
            out = { want.x, want.y, 0.f };
        }
        mem.Write(localPlayer + goff.m_viewAngles, out);
    }
    else if (g_silent.active) {
        if (cfg.silentRestore)
            mem.Write(localPlayer + goff.m_viewAngles, g_silent.saved);
        g_silent.active = false;
    }
}