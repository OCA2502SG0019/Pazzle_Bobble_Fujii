#include "dxlib.h"
#include "bag.h"

// バックグラウンドで保持するハンドルとサイズ
static int g_frontHandle = -1; // 前面 (bag_1)
static int g_backHandle = -1;  // 後面 (bag_2)
static int g_frontW = 0, g_frontH = 0;
static int g_backW = 0, g_backH = 0;
// 描画位置のオフセット（中心座標からのずれ、ピクセル）
// デフォルトで下にずらすオフセット（ピクセル）。必要なら Bag_SetOffset で上書き可能。
static int g_offsetX = -190, g_offsetY = 355;

// 指定されたハンドルを登録し、画像サイズを取得する
void Bag_SetHandles(int frontHandle, int backHandle)
{
    g_frontHandle = frontHandle;
    g_backHandle = backHandle;
    if (g_frontHandle != -1) {
        GetGraphSize(g_frontHandle, &g_frontW, &g_frontH);
    } else {
        g_frontW = g_frontH = 0;
    }
    if (g_backHandle != -1) {
        GetGraphSize(g_backHandle, &g_backW, &g_backH);
    } else {
        g_backW = g_backH = 0;
    }
}

// 指定した中心座標に対して、後面→前面の順で同じ表示サイズ・同じ座標で描画する。
// 前面画像は原寸の4倍で描画し、後面は前面と同じ描画サイズになるようスケーリングして描画する。
void Bag_DrawBothCentered(int centerX, int centerY)
{
    if (g_frontHandle == -1 && g_backHandle == -1) return;

    const float frontScale = 4.0f; // 前面は4倍で描画

    // 前面の描画サイズ (ピクセル)
    float frontDrawW = (g_frontW > 0) ? (g_frontW * frontScale) : 0.0f;
    float frontDrawH = (g_frontH > 0) ? (g_frontH * frontScale) : 0.0f;

    // 後面のスケールを計算（幅基準で合わせる）。高さ基準に合わせたい場合はそちらに変更可能。
    float backScale = frontScale;
    if (g_backW > 0 && g_frontW > 0) {
        backScale = frontDrawW / (float)g_backW;
    }

    // 実際に描画する座標にオフセットを加える
    float drawX = (float)(centerX + g_offsetX);
    float drawY = (float)(centerY + g_offsetY);

    // 後面を先に描画（背景）。DrawRotaGraphF は与えた座標を中心として描画する
    if (g_backHandle != -1) {
        DrawRotaGraphF(drawX, drawY, backScale, 0.0, g_backHandle, TRUE, FALSE);
    }

    // 前面を描画（手前）
    if (g_frontHandle != -1) {
        DrawRotaGraphF(drawX, drawY, frontScale, 0.0, g_frontHandle, TRUE, FALSE);
    }
}

// 後始末: ハンドルをクリアするだけ（実際の DeleteGraph は呼び出し側で行う）
void Bag_Finalize()
{
    g_frontHandle = -1;
    g_backHandle = -1;
    g_frontW = g_frontH = g_backW = g_backH = 0;
}

// 描画位置のオフセットを設定（中心座標に対するピクセル単位のずれ）
void Bag_SetOffset(int offsetX, int offsetY)
{
    g_offsetX = offsetX;
    g_offsetY = offsetY;
}
