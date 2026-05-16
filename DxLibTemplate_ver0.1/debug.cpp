#include "debug.h"
#include "launch_pad_animation.h"
#include "arrow_animation.h"
#include "keyManager.h"
#include "const.h"
#include "stage_ball.h"
#include "shot_ball.h"
#include "dxlib.h"
#include <cmath>

DebugManager::DebugManager()
    : m_enabled(false)
{
}

void DebugManager::toggle()
{
    m_enabled = !m_enabled;
}

bool DebugManager::isEnabled() const
{
    return m_enabled;
}

void DebugManager::draw(const LaunchPadAnimation& launch, const ArrowAnimation& arrow) const
{
    if (!m_enabled) return;
    const double PI = acos(-1.0);

    // デバッグモード中に画面中央縦ラインを赤で表示
    DrawLine(896, 0, 896, WINDOW_HEIGHT, GetColor(255, 0, 0));
    DrawLine(384, 0, 384, WINDOW_HEIGHT, GetColor(255, 0, 0));
    DrawLine(0, 96, WINDOW_WIDTH, 96, GetColor(0, 0, 255));
    // デバッグ: y=736 に緑の水平線を表示（64x64 グリッドに合わせる）
    DrawLine(0, 736, WINDOW_WIDTH, 736, GetColor(0, 255, 0));



    // 固定サイズ 64x64 のマスを左の赤の線と青の線の交点を左上基準として描画
    {
        const int redLeft = 384;
        const int redRight = 896;
        const int blueY = 96;
        const int size = 64; // 固定セルサイズ
        const int count = 8;
        const int count2 = 7;

        int startX = redLeft;
        int startY = blueY;

        int cellIndex = 1;
        // row shift: rows >=1 は累積で上方向に 8 * rowIndex ピクセルシフト
        auto rowShift = [&](int rowIndex) -> int { if (rowIndex <= 0) return 0; return -8 * rowIndex; };

        // 上段: rowIndex = 0
        for (int col = 0; col < count; ++col) {
            int rowIndex = 0;
            int x1 = startX + col * size;
            int y1 = startY + rowShift(rowIndex);
            int cx = x1 + size/2;
            int cy = y1 + size/2;
            int r = size/2 - 1;
            DrawCircle(cx, cy, r, GetColor(255,255,255), FALSE);
            char numBuf[32]; sprintf_s(numBuf, "%d", cellIndex); DrawString(cx - 6, cy - 6, numBuf, GetColor(255,255,255)); ++cellIndex;
        }

        // 下段: 7個。半セルオフセットを保持しつつ必要なら右端にクランプ
        int idealStartX2 = startX + size / 2; // 上段からの半セルシフト
        int maxStartX2 = redRight - count2 * size; // 右端が redRight 内に収まるようにする
        int startX2 = (idealStartX2 <= maxStartX2) ? idealStartX2 : maxStartX2;
        int rowIndexLower0 = 1;
        int y1_b = startY + size + rowShift(rowIndexLower0);
        for (int col = 0; col < count2; ++col) {
            int x1 = startX2 + col * size;
            int y1 = y1_b;
            int cx = x1 + size/2;
            int cy = y1 + size/2;
            int r = size/2 - 1;
            DrawCircle(cx, cy, r, GetColor(255,255,255), FALSE);
            char numBuf2[32]; sprintf_s(numBuf2, "%d", cellIndex); DrawString(cx - 6, cy - 6, numBuf2, GetColor(255,255,255)); ++cellIndex;
        }

        // 追加のセット（4セット）
        for (int set = 1; set <= 4; ++set) {
            int upperRowIndex = set * 2; // 2,4,6,8
            int lowerRowIndex = upperRowIndex + 1;
            int y_top = startY + (2 * set) * size + rowShift(upperRowIndex);
            int y_bot = y_top + size + rowShift(lowerRowIndex) - rowShift(upperRowIndex);
            for (int col = 0; col < count; ++col) {
                int x1 = startX + col * size;
                int cx = x1 + size/2;
                int cy = y_top + size/2;
                int r = size/2 - 1;
                DrawCircle(cx, cy, r, GetColor(255,255,255), FALSE);
                char bufNum[32]; sprintf_s(bufNum, "%d", cellIndex); DrawString(cx - 6, cy - 6, bufNum, GetColor(255,255,255)); ++cellIndex;
            }
            int idealStartX_bot = startX + size / 2;
            int maxStartX_bot = redRight - count2 * size;
            int startX_bot = (idealStartX_bot <= maxStartX_bot) ? idealStartX_bot : maxStartX_bot;
            for (int col = 0; col < count2; ++col) {
                int x1 = startX_bot + col * size;
                int cx = x1 + size/2;
                int cy = y_bot + size/2;
                int r = size/2 - 1;
                DrawCircle(cx, cy, r, GetColor(255,255,255), FALSE);
                char bufNum2[32]; sprintf_s(bufNum2, "%d", cellIndex); DrawString(cx - 6, cy - 6, bufNum2, GetColor(255,255,255)); ++cellIndex;
            }
        }
        // 追加: 最下部にさらに1行（上段 8個）を追加
        {
            int finalRowIndex = 10; // 既存の 0..9 行の後
            int y_top_final = startY + finalRowIndex * size + rowShift(finalRowIndex);
            for (int col = 0; col < count; ++col) {
                int x1 = startX + col * size;
                int cx = x1 + size/2;
                int cy = y_top_final + size/2;
                int r = size/2 - 1;
                DrawCircle(cx, cy, r, GetColor(255,255,255), FALSE);
                char bufNum[32]; sprintf_s(bufNum, "%d", cellIndex); DrawString(cx - 6, cy - 6, bufNum, GetColor(255,255,255)); ++cellIndex;
            }
        }
    }

    // シンプルに画面左上にデバッグ情報を表示する
    int x = 10;
    int y = 10;
    char buf[256];

    sprintf_s(buf, "LaunchPad frame: %d", launch.getIndex());
    DrawString(x, y, buf, GetColor(255,255,255));
    y += 20;

    sprintf_s(buf, "Arrow value: %d", arrow.getValue());
    DrawString(x, y, buf, GetColor(255,255,255));
    y += 20;

    sprintf_s(buf, "Arrow index: %d", arrow.getIndex());
    DrawString(x, y, buf, GetColor(255,255,255));
    y += 20;

    sprintf_s(buf, "Arrow flipped: %d", arrow.getFlipped() ? 1 : 0);
    DrawString(x, y, buf, GetColor(255,255,255));
    y += 20;

    // shot_ball 情報を表示
    BallColor shotColor = Shot_GetColor();
    const char* colorName = "None";
    int colorCol = GetColor(255,255,255);
    switch (shotColor) {
    case BALL_RED: colorName = "red"; colorCol = GetColor(255,0,0); break;
    case BALL_BLUE: colorName = "blue"; colorCol = GetColor(0,0,255); break;
    case BALL_GREEN: colorName = "green"; colorCol = GetColor(0,255,0); break;
    case BALL_YELLOW: colorName = "yellow"; colorCol = GetColor(255,255,0); break;
    default: break;
    }
    char shotBuf[128]; sprintf_s(shotBuf, "shot_ball color: %s", colorName);
    DrawString(x, y, shotBuf, colorCol);
    y += 20;

    // (削除) arrow_1 の黄色点。半円は下で描画され、デバッグモードでは常時可視。

    // デバッグ: 矢の軸（スプライトの中心）に常時黄色の点を表示する
    {
        int ax = arrow.getPosX();
        int ay = arrow.getPosY();
        int aw = arrow.getDrawWidth();
        int ah = arrow.getDrawHeight();
        if (aw > 0 && ah > 0) {
            int size = 6;
            int cx = ax + aw / 2;
            int cy = ay + ah / 2;
            DrawBox(cx - size/2, cy - size/2, cx + size/2, cy + size/2, GetColor(255,255,0), TRUE);
        }
    }

    // 常に半円（128分割）を arrow_1 の上頂点を基準に描画する
    {
        int ax = arrow.getPosX();
        int ay = arrow.getPosY();
        int aw = arrow.getDrawWidth();
        if (aw > 0) {
            double centerX = ax + aw / 2.0;
            double topY = ay;
            double radius = aw / 2.0;
            const int divisions = 128;
            // 全体を上に2ピクセルシフト
            double centerY = topY + radius - 2.0;
            double prevX = centerX + radius * cos(-PI);
            double prevY = centerY + radius * sin(-PI);
            for (int i = 1; i <= divisions; ++i) {
                double t = (double)i / (double)divisions;
                double angle = -PI + t * PI; // 角度範囲: [-PI,0]
                double x = centerX + radius * cos(angle);
                double y = centerY + radius * sin(angle);
                DrawLine((int)prevX, (int)prevY, (int)x, (int)y, GetColor(255,255,0));
                prevX = x;
                prevY = y;
            }
            // 半円上を移動する緑の点を arrow の値（A/Dキー）に合わせて描画する
            int frameCount = arrow.getFrameCount();
            int maxValue = (frameCount > 0) ? (frameCount - 1) : 64; // フォールバック: 64
            if (maxValue <= 0) maxValue = 64;
            int v = arrow.getValue();
            if (v > maxValue) v = maxValue;
            if (v < -maxValue) v = -maxValue;
            double tval = (double)(v + maxValue) / (2.0 * maxValue); // 0..1（0から1の範囲）
            double angleVal = -PI + tval * PI; // [-PI,0] にマップ
            double gx = centerX + radius * cos(angleVal);
            double gy = centerY + radius * sin(angleVal);
            int gvx = (int)std::round(gx);
            int gvy = (int)std::round(gy);
            DrawBox(gvx - 3, gvy - 3, gvx + 3, gvy + 3, GetColor(0,255,0), TRUE);
        }
    }

    // 黄色中心から緑点へ白い光線を引き、垂直な赤ラインで反射させる
    {
        int ax = arrow.getPosX();
        int ay = arrow.getPosY();
        int aw = arrow.getDrawWidth();
        int ah = arrow.getDrawHeight();
        if (aw > 0 && ah > 0) {
            double yellowX = ax + aw / 2.0;
            double yellowY = ay + ah / 2.0;

            // compute green position same as semicircle logic
            double centerX = ax + aw / 2.0;
            double topY = ay;
            double radius = aw / 2.0;
            double centerY = topY + radius - 2.0; // same upward shift

            int fc = arrow.getFrameCount();
            int maxValue = (fc > 0) ? (fc - 1) : 64;
            if (maxValue <= 0) maxValue = 64;
            int v = arrow.getValue();
            if (v > maxValue) v = maxValue;
            if (v < -maxValue) v = -maxValue;
            double tval = (double)(v + maxValue) / (2.0 * maxValue);
            double angleVal = -PI + tval * PI;
            double greenX = centerX + radius * cos(angleVal);
            double greenY = centerY + radius * sin(angleVal);

            double dirX = greenX - yellowX;
            double dirY = greenY - yellowY;
            double len = sqrt(dirX*dirX + dirY*dirY);
            if (len <= 1e-6) return; // 描画するものがない
            dirX /= len; dirY /= len;

            const double redLeft = 384.0;
            const double redRight = 896.0;
            const double EPS = 1e-4;
            const double effLeft = redLeft + 32.0;
            const double effRight = redRight - 32.0;
            const double blueY = 96.0;
            const double effBlueY = blueY + 32.0;
            const int maxReflections = 100;

            double curX = yellowX;
            double curY = yellowY;

            for (int r = 0; r < maxReflections; ++r) {
                double tWall = 1e308;
                int hitWall = 0;
                if (fabs(dirX) > 1e-9) {
                    double tL = (effLeft - curX) / dirX;
                    if (tL > EPS && tL < tWall) { tWall = tL; hitWall = -1; }
                    double tR = (effRight - curX) / dirX;
                    if (tR > EPS && tR < tWall) { tWall = tR; hitWall = +1; }
                }

                // 青の水平線 y=effBlueY との交差をチェック
                double tBlue = 1e308;
                if (fabs(dirY) > 1e-9) {
                    double tB = (effBlueY - curY) / dirY;
                    if (tB > EPS) tBlue = tB;
                }

                // 青線との交差が壁との交差より先に起こる場合は青線まで描画して処理を終える
                if (tBlue < tWall) {
                    double hitX = curX + dirX * tBlue;
                    double hitY = curY + dirY * tBlue;
                    DrawLine((int)std::round(curX), (int)std::round(curY), (int)std::round(hitX), (int)std::round(hitY), GetColor(255,255,255));
                    break; // stop tracing after hitting blue line
                }

                if (tWall < 1e307) {
                    double hitX = curX + dirX * tWall;
                    double hitY = curY + dirY * tWall;
                    DrawLine((int)std::round(curX), (int)std::round(curY), (int)std::round(hitX), (int)std::round(hitY), GetColor(255,255,255));
                    // reflect on vertical wall
                    curX = hitX;
                    curY = hitY;
                    dirX = -dirX;
                    // nudge forward to avoid immediate re-hit
                    curX += dirX * EPS * 10.0;
                    curY += dirY * EPS * 10.0;
                    continue;
                } else {
                    // 壁との交差が無ければ長い先まで線を引いて終了
                    double farX = curX + dirX * 2000.0;
                    double farY = curY + dirY * 2000.0;
                    DrawLine((int)std::round(curX), (int)std::round(curY), (int)std::round(farX), (int)std::round(farY), GetColor(255,255,255));
                    break;
                }
            }
        }

    // shot_ball の黄色マーカーを arrow_1 の中心（上頂点基準の中心）に描画
    {
        int ax = arrow.getPosX();
        int ay = arrow.getPosY();
        int aw = arrow.getDrawWidth();
        int ah = arrow.getDrawHeight();
        if (aw > 0 && ah > 0) {
            int centerX = ax + aw/2;
            int centerY = ay + ah/2;
            Shot_DrawMarkerAt(centerX, centerY);
        }
    }
    }
}
