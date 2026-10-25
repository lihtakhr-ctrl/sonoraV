#pragma once
/* ============================================================
 *  R5Reloaded 専用オフセット
 *  GameVersion      = v3.0.9.1094
 *  TimeDateStamp    = 0x5dba2a2f
 *  モジュール        = r5apex.exe
 * ============================================================ */
#include <cstdint>

/* このダンプと一致するか確認するための値 */
#define R5R_TIMEDATESTAMP 0x5dba2a2f

struct GameOffsets
{
    /* ---- モジュール相対ポインタ ---- */
    uint64_t entityList        = 0x1f33f58;   /* cl_entitylist          */
    uint64_t localHandle       = 0x173daa4;   /* LocalEntityHandle      */
    uint64_t viewRender        = 0xd4138f0;   /* ViewRender             */
    uint64_t viewMatrix        = 0x1a93d0;    /* ViewRender + これ       */
    uint64_t netVarTable       = 0x285a01a0;  /* NetworkVarTablePtr     */
    uint64_t inputSystem       = 0x1878f00;   /* InputSystem            */
   uint64_t nameList          = 0;           /* ダンプに無い → 後述     */

    /* ---- Game.cpp が参照している名前（追加） ---- */
    uint64_t localPlayer       = 0;        /* R5RはLocalEntityHandle経由で取得するため未使用 */
    uint64_t m_shieldMax       = 0x0174;   /* = m_shieldHealthMax と同義 */
    /* ---- エンティティ相対 ---- */
    uint64_t m_vecAbsOrigin    = 0x014c;
    uint64_t m_vecAbsVelocity  = 0x0140;
    uint64_t m_iHealth         = 0x03e0;
    uint64_t m_iMaxHealth      = 0x0510;
    uint64_t m_iTeamNum        = 0x03f0;
    uint64_t m_lifeState       = 0x0730;
    uint64_t m_shieldHealth    = 0x0170;
    uint64_t m_shieldHealthMax = 0x0174;
    uint64_t m_nForceBone      = 0x0e90;
    uint64_t m_bones           = 0x0e90 + 0x48;   /* 0x0ED8 */
    uint64_t m_cameraOrigin    = 0x1b68;
    uint64_t m_cameraAngles    = 0x1b74;
    uint64_t m_ammoPoolCapacity= 0x219c;
    uint64_t m_bleedoutState   = 0x2348;
    uint64_t m_communityName   = 0x21a9;

    /* ビュー角（※唯一ダンプに無いので推定値。下の「自動検出」参照） */
    uint64_t m_viewAngles      = 0x219c - 0x14;   /* = 0x2188 */

    /* ---- レイアウト ---- */
    uint64_t ENTITY_STRIDE = 0x20;     /* CEntInfo */
    uint64_t BONE_STRIDE   = 0x30;
    uint64_t BONE_POS      = 0xcc;
    int      HEAD_BONE     = 8;
    int      MAX_ENTITIES  = 0x10000;  /* NUM_ENT_ENTRIES */
};

extern GameOffsets goff;

bool AutoResolve(class Memory& mem, uint64_t base, uint64_t size);
bool ValidateOffsets(class Memory& mem, uint64_t base, GameOffsets& o);
void DumpOffsets();
void ScanViewAngleCandidates(class Memory& mem, uint64_t localPlayer);