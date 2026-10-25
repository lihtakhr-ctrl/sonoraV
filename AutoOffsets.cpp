#include "Offsets.hpp"
#include "Memory.hpp"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>

GameOffsets goff;

/* ============================================================
 *  ViewMatrix を実測で探す
 *  ViewRender の周辺をスキャンし、VMatrix の形をした場所を検出する
 * ============================================================ */
static bool IsViewMatrix(const float* m)
{
    if (fabsf(m[3]) > 0.002f) return false;
    if (fabsf(m[7]) > 0.002f) return false;
    if (fabsf(m[11]) > 0.002f) return false;
    if (fabsf(m[15] - 1.0f) > 0.002f) return false;

    for (int r = 0; r < 3; ++r) {                 /* 各行の長さがほぼ1 */
        const float* v = m + r * 4;
        float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (fabsf(len - 1.0f) > 0.02f) return false;
    }

    float tlen = sqrtf(m[12] * m[12] + m[13] * m[13] + m[14] * m[14]);
    if (tlen < 1.0f) return false;                /* 平行移動が0付近は別物 */

    return true;
}

static bool FindViewMatrix(Memory& mem, uint64_t viewRender, uint64_t hint, uint64_t& out)
{
    const uint64_t WIN = 0x40000;               /* ヒントの前後 ±256KB */
    const uint64_t start = (hint > WIN) ? (hint - WIN) : 0;
    const size_t   CHUNK = 0x40000;
    std::vector<uint8_t> buf(CHUNK + 0x40);

    int shown = 0;
    for (uint64_t off = start; off < hint + WIN; off += CHUNK) {
        if (!mem.Read(viewRender + off, buf.data(), buf.size())) continue;

        const float* f = (const float*)buf.data();
        size_t n = (buf.size() - 64) / 4;

        for (size_t i = 0; i < n; ++i) {
            if (!IsViewMatrix(f + i)) continue;

            uint64_t hit = off + (uint64_t)i * 4;
            printf("[vmatrix] 候補 ViewRender+0x%llX (ヒント差 %+lld)\n",
                (unsigned long long)hit, (long long)hit - (long long)hint);
            if (!out) out = hit;
            if (++shown >= 12) return true;
        }
    }
    return out != 0;
}

/* ============================================================
 *  ビュー角の候補を探す（唯一ダンプに無い値のため）
 *  マウスを動かして「変化した float3 で roll==0」を列挙する
 * ============================================================ */
void ScanViewAngleCandidates(Memory& mem, uint64_t localPlayer)
{
    const size_t SIZE = 0x3000;
    std::vector<uint8_t> a(SIZE), b(SIZE);

    if (!mem.Read(localPlayer, a.data(), SIZE)) {
        printf("[view] ローカルプレイヤーを読めません\n");
        return;
    }

    printf("\n>>> 3秒以内にマウスを大きく左右に動かしてください <<<\n");
    Sleep(1500);

    if (!mem.Read(localPlayer, b.data(), SIZE)) return;

    printf("---- viewAngles 候補（変化した float3）----\n");
    int n = 0;
    for (size_t off = 0; off + 12 <= SIZE; off += 4) {
        float p1 = *(float*)(a.data() + off);
        float y1 = *(float*)(a.data() + off + 4);
        float r1 = *(float*)(a.data() + off + 8);
        float p2 = *(float*)(b.data() + off);
        float y2 = *(float*)(b.data() + off + 4);
        float r2 = *(float*)(b.data() + off + 8);

        if (p1 == p2 && y1 == y2 && r1 == r2) continue;   /* 変化なし */
        if (r2 != 0.0f) continue;                          /* roll は 0 固定 */
        if (fabsf(p2) > 90.5f || fabsf(y2) > 181.f) continue;

        printf("  0x%04llX : (%.2f, %.2f) -> (%.2f, %.2f)\n",
            (unsigned long long)off, p1, y1, p2, y2);
        ++n;
        if (n > 40) break;
    }
    if (!n) printf("  （変化を検出できませんでした。もう一度、大きく動かしてください）\n");
    printf("----------------------------------------\n\n");
}

/* ============================================================
 *  検証
 * ============================================================ */
bool ValidateOffsets(Memory& mem, uint64_t base, GameOffsets& o)
{
    float m[16]{};
    uint64_t vr, local;
    uint32_t h;
    int idx, hp;

    vr = mem.Read<uint64_t>(base + o.viewRender);
    if (vr < 0x10000) { printf("[!] ViewRender が不正 (0x%llX)\n", vr); return false; }

    mem.Read(vr + o.viewMatrix, m, sizeof(m));
    printf("[vmatrix] ダンプ値 +0x%llX の中身: ", o.viewMatrix);
    for (int i = 0; i < 16; ++i) printf("%.3f ", m[i]);
    printf("\n");

    /* ---- 形がおかしければ実測で探して修正する ---- */
    if (fabsf(m[15] - 1.0f) > 0.02f ||
        fabsf(m[3]) > 0.02f || fabsf(m[7]) > 0.02f || fabsf(m[11]) > 0.02f)
    {
        printf("[!] ViewMatrix の形がおかしい → 実測スキャンします\n");

        uint64_t found = 0;
        if (!FindViewMatrix(mem, vr, o.viewMatrix, found)) {
            printf("[!] 候補が見つかりませんでした\n");
            return false;
        }
        printf("[vmatrix] 0x%llX を採用します\n", found);
        o.viewMatrix = found;                    /* goff に反映される */
        mem.Read(vr + o.viewMatrix, m, sizeof(m));
    }

    if (fabsf(m[15] - 1.0f) > 0.02f ||
        fabsf(m[3]) > 0.02f || fabsf(m[7]) > 0.02f || fabsf(m[11]) > 0.02f)
    {
        printf("[!] ViewMatrix の形がおかしい (m[15]=%.3f m[3]=%.3f m[7]=%.3f m[11]=%.3f)\n",
            m[15], m[3], m[7], m[11]);
        return false;
    }

    h = mem.Read<uint32_t>(base + o.localHandle);
    idx = (int)(h & 0xFFFF);
    if (idx <= 0 || idx >= o.MAX_ENTITIES) { printf("[!] LocalEntityHandle が不正 (idx=%d)\n", idx); return false; }

    local = mem.Read<uint64_t>(base + o.entityList + (uint64_t)idx * o.ENTITY_STRIDE);
    if (local < 0x10000) { printf("[!] ローカルプレイヤーが取れず\n"); return false; }

    hp = mem.Read<int>(local + o.m_iHealth);
    if (hp < 0 || hp > 200) { printf("[!] m_iHealth が不正 (%d)\n", hp); return false; }

    printf("[ok] 検証成功 (localHP=%d, idx=%d)\n", hp, idx);
    return true;
}

void DumpOffsets()
{
    printf("---- 使用オフセット ----\n");
    printf(" entityList=0x%llX localHandle=0x%llX viewRender=0x%llX viewMatrix=0x%llX\n",
        goff.entityList, goff.localHandle, goff.viewRender, goff.viewMatrix);
    printf(" m_iHealth=0x%llX m_iTeamNum=0x%llX m_vecAbsOrigin=0x%llX m_bones=0x%llX\n",
        goff.m_iHealth, goff.m_iTeamNum, goff.m_vecAbsOrigin, goff.m_bones);
    printf(" m_cameraOrigin=0x%llX m_viewAngles=0x%llX\n",
        goff.m_cameraOrigin, goff.m_viewAngles);
    printf("------------------------\n");
}

/* ============================================================
 *  エントリ
 * ============================================================ */
bool AutoResolve(Memory& mem, uint64_t base, uint64_t size)
{
    printf("=== R5Reloaded オフセット適用 ===\n");

    /* PE の TimeDateStamp でビルド一致を確認 */
    uint32_t lfanew = mem.Read<uint32_t>(base + 0x3C);
    uint32_t stamp = mem.Read<uint32_t>(base + lfanew + 0x08);
    printf("[info] TimeDateStamp = 0x%08X (期待値 0x%08X)\n", stamp, R5R_TIMEDATESTAMP);
    if (stamp != R5R_TIMEDATESTAMP)
        printf("[warn] ビルドが違います。オフセットがズレている可能性あり\n");

    DumpOffsets();

    if (!ValidateOffsets(mem, base, goff)) {
        printf("[!] 検証失敗。R5Reloaded のダンプを再取得してください\n");
        return false;
    }

    /* ビュー角の確認用スキャン（初回だけ見たい場合はコメントアウト） */
    uint32_t h = mem.Read<uint32_t>(base + goff.localHandle);
    uint64_t local = mem.Read<uint64_t>(base + goff.entityList +
        (uint64_t)(h & 0xFFFF) * goff.ENTITY_STRIDE);
    if (local > 0x10000) ScanViewAngleCandidates(mem, local);

    printf("=== 完了 ===\n");
    return true;
}