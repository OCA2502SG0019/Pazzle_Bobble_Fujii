#include "falling_ball.h"
#include "dxlib.h"
#include "const.h"
#include <vector>
#include <cmath>
#include <algorithm>

struct FallingBall {
    double x;
    double y;
    double vy;
    BallColor color;
    int handle;
    bool alive;
    int delayFrames; // 落下開始までの待機フレーム数（下側から順に小さくする）
    bool started;
};

static std::vector<FallingBall> g_falling;

static bool GetCellCenterLocal(int targetCell, int &outCx, int &outCy)
{
    if (targetCell <= 0) return false;
    const int redLeft = 384;
    const int redRight = 896;
    const int blueY = 96;
    const int size = 64;
    const int count = 8;
    const int count2 = 7;
    int startX = redLeft;
    int startY = blueY;

    int cellIndex = 1;
    // 上段
    for (int col = 0; col < count; ++col) {
        if (cellIndex == targetCell) {
            int x1 = startX + col * size;
            int y1 = startY;
            outCx = x1 + size/2;
            outCy = y1 + size/2;
            return true;
        }
        ++cellIndex;
    }
    // 下段
    int idealStartX2 = startX + size/2;
    int maxStartX2 = redRight - count2 * size;
    int startX2 = (idealStartX2 <= maxStartX2) ? idealStartX2 : maxStartX2;
    int rowIndexLower0 = 1;
    int y1_b = startY + size + (rowIndexLower0 > 0 ? -8 * rowIndexLower0 : 0);
    for (int col = 0; col < count2; ++col) {
        if (cellIndex == targetCell) {
            int x1 = startX2 + col * size;
            outCx = x1 + size/2;
            outCy = y1_b + size/2;
            return true;
        }
        ++cellIndex;
    }
    // 追加セット
    for (int set = 1; set <= 4; ++set) {
        int upperRowIndex = set * 2;
        int lowerRowIndex = upperRowIndex + 1;
        int y_top = startY + (2 * set) * size + (upperRowIndex > 0 ? -8 * upperRowIndex : 0);
        int y_bot = y_top + size + (lowerRowIndex > 0 ? -8 * lowerRowIndex : 0) - (upperRowIndex > 0 ? -8 * upperRowIndex : 0);
        for (int col = 0; col < count; ++col) {
            if (cellIndex == targetCell) {
                int x1 = startX + col * size;
                outCx = x1 + size/2;
                outCy = y_top + size/2;
                return true;
            }
            ++cellIndex;
        }
        int idealStartX_bot = startX + size / 2;
        int maxStartX_bot = redRight - count2 * size;
        int startX_bot = (idealStartX_bot <= maxStartX_bot) ? idealStartX_bot : maxStartX_bot;
        for (int col = 0; col < count2; ++col) {
            if (cellIndex == targetCell) {
                int x1 = startX_bot + col * size;
                outCx = x1 + size/2;
                outCy = y_bot + size/2;
                return true;
            }
            ++cellIndex;
        }
    }
    // 最下段
    {
        int finalRowIndex = 10;
        int y_top_final = startY + finalRowIndex * size + (finalRowIndex > 0 ? -8 * finalRowIndex : 0);
        for (int col = 0; col < count; ++col) {
            if (cellIndex == targetCell) {
                int x1 = startX + col * size;
                outCx = x1 + size/2;
                outCy = y_top_final + size/2;
                return true;
            }
            ++cellIndex;
        }
    }
    return false;
}

void Falling_Init()
{
    g_falling.clear();
}

void Falling_Finalize()
{
    g_falling.clear();
}

void Falling_SpawnFromCell(int cellIndex, BallColor color)
{
    int cx=0, cy=0;
    if (!GetCellCenterLocal(cellIndex, cx, cy)) return;
    FallingBall fb;
    fb.x = (double)cx;
    fb.y = (double)cy;
    fb.vy = 0.0;
    fb.color = color;
    fb.handle = Ball_GetHandleForColor(color);
    fb.alive = true;
    fb.started = false;
    // 単体生成では遅延なし（バッチ生成を使って落下順序を決定する）
    fb.delayFrames = 0;
    g_falling.push_back(fb);
}

void Falling_SpawnBatch(const std::vector<std::pair<int,BallColor>>& items)
{
    if (items.empty()) return;
    struct Tmp { int cell; BallColor color; int cx; int cy; };
    std::vector<Tmp> tmp;
    tmp.reserve(items.size());
    for (auto &it : items) {
        int cell = it.first; BallColor col = it.second;
        int cx=0, cy=0;
        if (!GetCellCenterLocal(cell, cx, cy)) continue;
        tmp.push_back({cell, col, cx, cy});
    }
    if (tmp.empty()) return;
    // y が大きい（画面下）順にソートして下から先に落ちるようにする
    std::sort(tmp.begin(), tmp.end(), [](const Tmp &a, const Tmp &b){ return a.cy > b.cy; });

    const int baseDelay = 2; // フレーム単位の基本遅延。小さくするとラグが減る。
    for (size_t i = 0; i < tmp.size(); ++i) {
        FallingBall fb;
        fb.x = (double)tmp[i].cx;
        fb.y = (double)tmp[i].cy;
        fb.vy = 0.0;
        fb.color = tmp[i].color;
        fb.handle = Ball_GetHandleForColor(fb.color);
        fb.alive = true;
        fb.started = false;
        fb.delayFrames = (int)(i * baseDelay);
        g_falling.push_back(fb);
    }
}

void Falling_Update()
{
    const double gravity = 0.8; // 加速度
    for (size_t i = 0; i < g_falling.size(); ++i) {
        if (!g_falling[i].alive) continue;
        // 開始遅延を経過させる
        if (!g_falling[i].started) {
            if (g_falling[i].delayFrames > 0) {
                g_falling[i].delayFrames -= 1;
                continue;
            } else {
                g_falling[i].started = true;
                // 落下開始時の初速度を少し与えて見た目を良くする
                g_falling[i].vy = 2.0;
            }
        }
        // 落下中は重力で速度を増加させる
        g_falling[i].vy += gravity;
        g_falling[i].y += g_falling[i].vy;
        // 画面外に出たら消滅
        if (g_falling[i].y - 64.0 > WINDOW_HEIGHT + 200) {
            g_falling[i].alive = false;
        }
    }
    // 末尾から消す
    for (int i = (int)g_falling.size() - 1; i >= 0; --i) {
        if (!g_falling[i].alive) g_falling.erase(g_falling.begin() + i);
    }
}

void Falling_Draw()
{
    for (size_t i = 0; i < g_falling.size(); ++i) {
        const FallingBall &fb = g_falling[i];
        if (!fb.alive) continue;
        if (fb.handle == -1) continue;
        int w=0,h=0; GetGraphSize(fb.handle, &w, &h);
        if (w<=0 || h<=0) continue;
        int dw = w * 4;
        int dh = h * 4;
        int left = (int)std::round(fb.x) - dw/2;
        int top = (int)std::round(fb.y) - dh/2;
        DrawExtendGraph(left, top, left + dw, top + dh, fb.handle, TRUE);
    }
}
