#include "stage_ball.h"
#include "const.h"
#include "dxlib.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <vector>

// 色ごとのグラフィックハンドル
static int g_handleByColor[5] = { -1, -1, -1, -1, -1 };

// 簡易ステージのセルマッピング
// 合計セル数 = 8 (トップ) + 7 + 4*(8+7) + 8 (最終) = 83
static const int MAX_CELLS = 83;
static int g_cellColor[MAX_CELLS + 1];

// 指定セルに色を割り当てる
void Ball_SetStageCell(int cellIndex, BallColor color) {
    if (cellIndex < 1 || cellIndex > MAX_CELLS) return;
    g_cellColor[cellIndex] = (int)color;
}

// ハンドル取得
int Ball_GetHandleForColor(BallColor color) {
    if (color < 0 || color > 4) return -1;
    return g_handleByColor[(int)color];
}

int Ball_GetMaxCells() { return MAX_CELLS; }

BallColor Ball_GetStageCell(int cellIndex) {
    if (cellIndex < 1 || cellIndex > MAX_CELLS) return BALL_NONE;
    return (BallColor)g_cellColor[cellIndex];
}

// 色に対応する画像ハンドルを設定する
void Ball_SetHandleForColor(BallColor color, int handle) {
    if (color < 0 || color > 4) return;
    if (g_handleByColor[color] != -1) {
        DeleteGraph(g_handleByColor[color]);
    }
    g_handleByColor[color] = handle;
}

static int g_ballHandle = -1;

void Ball_Init()
{
    // マップを初期化（すべて None）
    for (int i = 0; i <= MAX_CELLS; ++i) g_cellColor[i] = BALL_NONE;
    // 赤ボールを指定セルに割り当て
    Ball_SetStageCell(1, BALL_RED);
    Ball_SetStageCell(2, BALL_RED);
    Ball_SetStageCell(9, BALL_RED);
    Ball_SetStageCell(10, BALL_RED);
    Ball_SetStageCell(20, BALL_RED);
    Ball_SetStageCell(21, BALL_RED);
    Ball_SetStageCell(27, BALL_RED);
    Ball_SetStageCell(28, BALL_RED);
    // 黄色ボールを指定セルに割り当て
    Ball_SetStageCell(3, BALL_YELLOW);
    Ball_SetStageCell(4, BALL_YELLOW);
    Ball_SetStageCell(11, BALL_YELLOW);
    Ball_SetStageCell(12, BALL_YELLOW);
    Ball_SetStageCell(22, BALL_YELLOW);
    Ball_SetStageCell(23, BALL_YELLOW);
    Ball_SetStageCell(29, BALL_YELLOW);
    Ball_SetStageCell(30, BALL_YELLOW);
    // 青ボールを指定セルに割り当て
    Ball_SetStageCell(5, BALL_BLUE);
    Ball_SetStageCell(6, BALL_BLUE);
    Ball_SetStageCell(13, BALL_BLUE);
    Ball_SetStageCell(14, BALL_BLUE);
    Ball_SetStageCell(16, BALL_BLUE);
    Ball_SetStageCell(17, BALL_BLUE);
    Ball_SetStageCell(24, BALL_BLUE);
    // 緑ボールを指定セルに割り当て
    Ball_SetStageCell(7, BALL_GREEN);
    Ball_SetStageCell(8, BALL_GREEN);
    Ball_SetStageCell(15, BALL_GREEN);
    Ball_SetStageCell(18, BALL_GREEN);
    Ball_SetStageCell(19, BALL_GREEN);
    Ball_SetStageCell(25, BALL_GREEN);
    Ball_SetStageCell(26, BALL_GREEN);
}

void Ball_SetHandle(int handle)
{
    // 互換のため red に対応するハンドルを設定
    Ball_SetHandleForColor(BALL_RED, handle);
}

void Ball_Finalize()
{
    // すべての色ハンドルを解放して初期化
    for (int i = 0; i <= 4; ++i) {
        if (g_handleByColor[i] != -1) { DeleteGraph(g_handleByColor[i]); g_handleByColor[i] = -1; }
    }
}

// デバッググリッド上のボールを描画
void Ball_DrawInDebug()
{
    // ハンドルが一つも設定されていなければ描画しない
    bool anyHandle = false;
    for (int i=1;i<=4;++i) if (g_handleByColor[i] != -1) anyHandle = true;
    if (!anyHandle) return;
    // 左上基準の固定 64x64 グリッド (redLeft, blueY を起点)
    const int redLeft = 384;
    const int redRight = 896;
    const int blueY = 96;
    const int size = 64;
    const int count = 8;
    const int count2 = 7;
    const int totalRows = 10; // 直接は使わないが明示のため残す
    int startX = redLeft;
    int startY = blueY;

    // debug.cpp と同じループ・位置計算で描画して位置が一致するようにする
    int cellIndex = 1;
    // 最上段（上段8個）
    for (int col = 0; col < count; ++col) {
        int rowIndex = 0;
        int x1 = startX + col * size;
        int y1 = startY + rowIndex * size;
        if (rowIndex > 0) y1 += -8 * rowIndex;
        BallColor c = Ball_GetStageCell(cellIndex);
        if (c != BALL_NONE) {
            int handle = g_handleByColor[(int)c];
            if (handle != -1) {
                int w=0,h=0; GetGraphSize(handle,&w,&h); if (w>0 && h>0) {
                    int dw = w*4; int dh = h*4;
                    int cx = x1 + size/2 - dw/2;
                    int cy = y1 + size/2 - dh/2;
                    DrawExtendGraph(cx, cy, cx+dw, cy+dh, handle, TRUE);
                }
            }
        }
        ++cellIndex;
    }

    // 最初の下段（7個）: 半セルオフセット＋クランプ
    {
        int idealStartX2 = startX + size / 2;
        int maxStartX2 = redRight - count2 * size;
        int startX2 = (idealStartX2 <= maxStartX2) ? idealStartX2 : maxStartX2;
        int rowIndexLower0 = 1;
        int y1_b = startY + size + (rowIndexLower0 > 0 ? -8 * rowIndexLower0 : 0);
        for (int col = 0; col < count2; ++col) {
            int x1 = startX2 + col * size;
            BallColor c = Ball_GetStageCell(cellIndex);
            if (c != BALL_NONE) {
                int handle = g_handleByColor[(int)c];
                if (handle != -1) {
                    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w>0 && h>0) {
                        int dw = w*4; int dh = h*4;
                        int cx = x1 + size/2 - dw/2;
                        int cy = y1_b + size/2 - dh/2;
                        DrawExtendGraph(cx, cy, cx+dw, cy+dh, handle, TRUE);
                    }
                }
            }
            ++cellIndex;
        }
    }

    // 追加セット（4セット）
    for (int set = 1; set <= 4; ++set) {
        int upperRowIndex = set * 2;
        int lowerRowIndex = upperRowIndex + 1;
        int y_top = startY + (2 * set) * size + (upperRowIndex > 0 ? -8 * upperRowIndex : 0);
        int y_bot = y_top + size + (lowerRowIndex > 0 ? -8 * lowerRowIndex : 0) - (upperRowIndex > 0 ? -8 * upperRowIndex : 0);
        for (int col = 0; col < count; ++col) {
            int x1 = startX + col * size;
            BallColor c = Ball_GetStageCell(cellIndex);
            if (c != BALL_NONE) {
                int handle = g_handleByColor[(int)c];
                if (handle != -1) {
                    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w>0 && h>0) {
                        int dw = w*4; int dh = h*4;
                        int cx = x1 + size/2 - dw/2;
                        int cy = y_top + size/2 - dh/2;
                        DrawExtendGraph(cx, cy, cx+dw, cy+dh, handle, TRUE);
                    }
                }
            }
            ++cellIndex;
        }
        int idealStartX_bot = startX + size / 2;
        int maxStartX_bot = redRight - count2 * size;
        int startX_bot = (idealStartX_bot <= maxStartX_bot) ? idealStartX_bot : maxStartX_bot;
        for (int col = 0; col < count2; ++col) {
            int x1 = startX_bot + col * size;
            BallColor c = Ball_GetStageCell(cellIndex);
            if (c != BALL_NONE) {
                int handle = g_handleByColor[(int)c];
                if (handle != -1) {
                    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w>0 && h>0) {
                        int dw = w*4; int dh = h*4;
                        int cx = x1 + size/2 - dw/2;
                        int cy = y_bot + size/2 - dh/2;
                        DrawExtendGraph(cx, cy, cx+dw, cy+dh, handle, TRUE);
                    }
                }
            }
            ++cellIndex;
        }
    }

    // 最下段（8個）
    {
        int finalRowIndex = 10;
        int y_top_final = startY + finalRowIndex * size + (finalRowIndex > 0 ? -8 * finalRowIndex : 0);
        for (int col = 0; col < count; ++col) {
            int x1 = startX + col * size;
            BallColor c = Ball_GetStageCell(cellIndex);
            if (c != BALL_NONE) {
                int handle = g_handleByColor[(int)c];
                if (handle != -1) {
                    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w>0 && h>0) {
                        int dw = w*4; int dh = h*4;
                        int cx = x1 + size/2 - dw/2;
                        int cy = y_top_final + size/2 - dh/2;
                        DrawExtendGraph(cx, cy, cx+dw, cy+dh, handle, TRUE);
                    }
                }
            }
            ++cellIndex;
        }
    }
}
