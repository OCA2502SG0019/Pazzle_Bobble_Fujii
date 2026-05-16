#include "shot_ball.h"
#include "falling_ball.h"
#include "const.h"
#include "dxlib.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// 軌道移動用の内部状態
static std::vector<std::pair<int,int>> g_pathPoints;
static double g_posX = 0.0;
static double g_posY = 0.0;
static size_t g_pathIndex = 0; // 次の目標点のインデックス
static bool g_moving = false;
static double g_moveSpeed = 16.0; // ピクセル/フレーム

// 選択されたセルのインデックス（1..MAX_CELLS）、未選択時は -1
static int g_shotCell = -1;

// 3つ以上つながっていたら消滅させる関数
static void Shot_CheckAndRemoveConnected(int startIndex);

// 指定セルの中心座標を計算する。成功すれば true を返す。
static bool GetCellCenter(int targetCell, int &outCx, int &outCy)
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

// 指定座標に最も近いセルのインデックスを返す（見つからなければ -1）
static int FindNearestCell(double px, double py)
{
    int nearest = -1;
    double bestD = 1e308;
    int maxCells = Ball_GetMaxCells();
    for (int i = 1; i <= maxCells; ++i) {
        int cx = 0, cy = 0;
        if (!GetCellCenter(i, cx, cy)) continue;
        double dx = cx - px;
        double dy = cy - py;
        double d2 = dx*dx + dy*dy;
        if (d2 < bestD) { bestD = d2; nearest = i; }
    }
    return nearest;
}

void Shot_PickRandomFromStage()
{
    // 初期化として乱数シードを一度だけ設定
    static bool seeded = false;
    if (!seeded) { std::srand((unsigned)std::time(nullptr)); seeded = true; }

    int maxCells = Ball_GetMaxCells();
    std::vector<int> occupied;
    for (int i = 1; i <= maxCells; ++i) {
        BallColor c = Ball_GetStageCell(i);
        if (c != BALL_NONE) occupied.push_back(i);
    }
    if (occupied.empty()) { g_shotCell = -1; return; }
    int idx = std::rand() % (int)occupied.size();
    g_shotCell = occupied[idx];
}

// メインループでボール画像を描画する（選択セルが無ければ何もしない）
void Shot_DrawSprite()
{
    if (g_shotCell <= 0) return;
    BallColor c = Ball_GetStageCell(g_shotCell);
    int handle = Ball_GetHandleForColor(c);
    if (handle == -1) return;

    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w<=0 || h<=0) return;
    // 画面の中心にスプライトを表示する（4倍表示）
    // デフォルトは画面中央に描画する（既存の挙動）
    Shot_DrawSpriteAt(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
}

// デバッグモードで表示する黄色のマーカーを描画
void Shot_DrawMarker()
{
    if (g_shotCell <= 0) return;
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
        if (cellIndex == g_shotCell) {
            int x1 = startX + col * size;
            int y1 = startY;
            int cx = x1 + size/2;
            int cy = y1 + size/2;
            DrawBox(cx-4, cy-4, cx+4, cy+4, GetColor(255,255,0), TRUE);
            return;
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
        if (cellIndex == g_shotCell) {
            int x1 = startX2 + col * size;
            int cx = x1 + size/2;
            int cy = y1_b + size/2;
            DrawBox(cx-4, cy-4, cx+4, cy+4, GetColor(255,255,0), TRUE);
            return;
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
            if (cellIndex == g_shotCell) {
                int x1 = startX + col * size;
                int cx = x1 + size/2;
                int cy = y_top + size/2;
                DrawBox(cx-4, cy-4, cx+4, cy+4, GetColor(255,255,0), TRUE);
                return;
            }
            ++cellIndex;
        }
        int idealStartX_bot = startX + size / 2;
        int maxStartX_bot = redRight - count2 * size;
        int startX_bot = (idealStartX_bot <= maxStartX_bot) ? idealStartX_bot : maxStartX_bot;
        for (int col = 0; col < count2; ++col) {
            if (cellIndex == g_shotCell) {
                int x1 = startX_bot + col * size;
                int cx = x1 + size/2;
                int cy = y_bot + size/2;
                DrawBox(cx-4, cy-4, cx+4, cy+4, GetColor(255,255,0), TRUE);
                return;
            }
            ++cellIndex;
        }
    }
    // 最下段
    {
        int finalRowIndex = 10;
        int y_top_final = startY + finalRowIndex * size + (finalRowIndex > 0 ? -8 * finalRowIndex : 0);
        for (int col = 0; col < count; ++col) {
            if (cellIndex == g_shotCell) {
                int x1 = startX + col * size;
                int cx = x1 + size/2;
                int cy = y_top_final + size/2;
                DrawBox(cx-4, cy-4, cx+4, cy+4, GetColor(255,255,0), TRUE);
                return;
            }
            ++cellIndex;
        }
    }
}

void Shot_DrawSpriteAt(int centerX, int centerY)
{
    if (g_shotCell <= 0) return;
    BallColor c = Ball_GetStageCell(g_shotCell);
    int handle = Ball_GetHandleForColor(c);
    if (handle == -1) return;
    int w=0,h=0; GetGraphSize(handle,&w,&h); if (w<=0 || h<=0) return;
    int dw = w * 4; int dh = h * 4;
    int left = centerX - dw/2;
    int top = centerY - dh/2;
    DrawExtendGraph(left, top, left + dw, top + dh, handle, TRUE);
}

void Shot_DrawMarkerAt(int centerX, int centerY)
{
    // 小さな四角で中心位置を示す
    DrawBox(centerX-6, centerY-6, centerX+6, centerY+6, GetColor(255,255,0), TRUE);
}

BallColor Shot_GetColor()
{
    if (g_shotCell <= 0) return BALL_NONE;
    return Ball_GetStageCell(g_shotCell);
}

int Shot_GetCellIndex() { return g_shotCell; }

void Shot_StartMovement(const std::vector<std::pair<int,int>>& pathPoints)
{
    g_pathPoints = pathPoints;
    if (g_pathPoints.empty()) {
        g_moving = false;
        return;
    }
    // 開始位置を現在の先頭に設定
    g_posX = (double)g_pathPoints[0].first;
    g_posY = (double)g_pathPoints[0].second;
    g_pathIndex = 1; // 次は 1
    g_moving = (g_pathPoints.size() > 1);
}

void Shot_Update()
{
    if (!g_moving) return;
    if (g_pathIndex >= g_pathPoints.size()) { g_moving = false; return; }
    double tx = (double)g_pathPoints[g_pathIndex].first;
    double ty = (double)g_pathPoints[g_pathIndex].second;
    double dx = tx - g_posX;
    double dy = ty - g_posY;
    double dist = std::sqrt(dx*dx + dy*dy);
    if (dist <= g_moveSpeed || dist <= 1e-6) {
        // 到達
        g_posX = tx; g_posY = ty;
        ++g_pathIndex;
        if (g_pathIndex < g_pathPoints.size()) { 
            return; 
        }
    } else {
        double nx = dx / dist;
        double ny = dy / dist;
        g_posX += nx * g_moveSpeed;
        g_posY += ny * g_moveSpeed;
    }
    // 移動後に衝突判定: 天井(blueY) かステージ上のボールとの接触
    const double blueY = 96.0;
    // 天井に触れるかを判定: ボールの上端(g_posY - 32)が天井(blueY)に達した場合、または軌道の終点（青線等）に達した場合
    if (g_posY - 32.0 <= blueY + 1.0 || g_pathIndex >= g_pathPoints.size()) {
        BallColor col = Shot_GetColor();
        if (col != BALL_NONE) {
            // 最寄りセルを検索
            int nearest = FindNearestCell(g_posX, g_posY);
            if (nearest != -1) {
                Ball_SetStageCell(nearest, col);
                // 配置された後につながりをチェックして消去する
                Shot_CheckAndRemoveConnected(nearest);
                // 発射したボールがステージに配置された後で、次のショット色を
                // ステージ内のボールからランダムに選ぶ仕様とする。
                Shot_PickRandomFromStage();
            }
        }
        g_moving = false;
        return;
    }

    // ステージのボールとの接触判定（半径閾値）
    // NOTE: 空のマス（BALL_NONE）では衝突とみなさないようにする
    {
        int maxCells = Ball_GetMaxCells();
        const double threshold = 64.0; // 衝突判定距離（ボールの中心同士の距離が直径に等しいかどうか）
        double thr2 = threshold * threshold;
        for (int i = 1; i <= maxCells; ++i) {
            // そのセルに既にボールが無ければ衝突判定の対象外
            if (Ball_GetStageCell(i) == BALL_NONE) continue;
            int cx=0, cy=0;
            if (!GetCellCenter(i, cx, cy)) continue;
            double ddx = cx - g_posX;
            double ddy = cy - g_posY;
            double d2 = ddx*ddx + ddy*ddy;
            if (d2 <= thr2) {
                // 実際のボールの端に触れたら消滅、最寄りマスに生成
                BallColor col = Shot_GetColor();
                if (col != BALL_NONE) {
                    int nearest = FindNearestCell(g_posX, g_posY);
                    if (nearest != -1) {
                        Ball_SetStageCell(nearest, col);
                        // 配置された後につながりをチェックして消去する
                        Shot_CheckAndRemoveConnected(nearest);
                        // 発射したボールがステージに配置された後で、次のショット色を
                        // ステージ内のボールからランダムに選ぶ仕様とする。
                        Shot_PickRandomFromStage();
                    }
                }
                g_moving = false;
                return;
            }
        }
    }
}

void Shot_DrawCurrent()
{
    if (!g_moving) return;
    // 描画は中心座標を想定する
    Shot_DrawSpriteAt((int)std::round(g_posX), (int)std::round(g_posY));
}

bool Shot_IsMoving() { return g_moving; }

static void Shot_CheckAndRemoveConnected(int startIndex)
{
    BallColor targetColor = Ball_GetStageCell(startIndex);
    if (targetColor == BALL_NONE) return;

    std::vector<int> connected;
    int maxCells = Ball_GetMaxCells();
    std::vector<bool> visited(maxCells + 1, false);
    std::vector<int> stack;

    stack.push_back(startIndex);
    visited[startIndex] = true;

    // BFS/DFSで繋がっている同色のボールを探す
    while (!stack.empty()) {
        int curr = stack.back();
        stack.pop_back();
        connected.push_back(curr);

        int cx1 = 0, cy1 = 0;
        if (!GetCellCenter(curr, cx1, cy1)) continue;

        for (int i = 1; i <= maxCells; ++i) {
            if (visited[i]) continue;
            if (Ball_GetStageCell(i) != targetColor) continue;

            int cx2 = 0, cy2 = 0;
            if (!GetCellCenter(i, cx2, cy2)) continue;

            double dx = cx1 - cx2;
            double dy = cy1 - cy2;
            double distSq = dx * dx + dy * dy;

            // 隣接判定(横方向は中心距離64, 斜め方向は約64.5なので4500未満なら隣接している)
            if (distSq <= 4500.0) {
                visited[i] = true;
                stack.push_back(i);
            }
        }
    }

    // 3個以上繋がっていたら消去する
    if (connected.size() >= 3) {
        for (size_t i = 0; i < connected.size(); ++i) {
            Ball_SetStageCell(connected[i], BALL_NONE);
        }
    }

    // 消去後、天井(青い線)に接続していない（浮いている）ボール群を全て削除する
    // アルゴリズム: 天井に接しているすべてのボールを起点に BFS で到達可能なボールをマークし、
    // マークされていない残りのボールは浮遊しているとみなして消す。
    {
        int maxCellsAny = Ball_GetMaxCells();
        std::vector<bool> reachable(maxCellsAny + 1, false);
        std::vector<int> queue;
        const int blueY = 96;
        const int size = 64;

        // 天井に接しているボールを起点に追加
        for (int i = 1; i <= maxCellsAny; ++i) {
            if (Ball_GetStageCell(i) == BALL_NONE) continue;
            int cx=0, cy=0;
            if (!GetCellCenter(i, cx, cy)) continue;
            if (cy - size/2 <= blueY + 1) {
                reachable[i] = true;
                queue.push_back(i);
            }
        }

        // BFS で到達可能なボールを全てマーク
        for (size_t qi = 0; qi < queue.size(); ++qi) {
            int cur = queue[qi];
            int cx1 = 0, cy1 = 0;
            if (!GetCellCenter(cur, cx1, cy1)) continue;
            for (int j = 1; j <= maxCellsAny; ++j) {
                if (reachable[j]) continue;
                if (Ball_GetStageCell(j) == BALL_NONE) continue;
                int cx2 = 0, cy2 = 0;
                if (!GetCellCenter(j, cx2, cy2)) continue;
                double dx = cx1 - cx2;
                double dy = cy1 - cy2;
                double distSq = dx * dx + dy * dy;
                if (distSq <= 4500.0) {
                    reachable[j] = true;
                    queue.push_back(j);
                }
            }
        }

        // 到達不能な（浮いている）ボールは落下オブジェクトとしてバッチ生成し、ステージ上のセルは空にする
        std::vector<std::pair<int,BallColor>> toSpawn;
        for (int i = 1; i <= maxCellsAny; ++i) {
            BallColor bc = Ball_GetStageCell(i);
            if (bc != BALL_NONE && !reachable[i]) {
                toSpawn.emplace_back(i, bc);
                Ball_SetStageCell(i, BALL_NONE);
            }
        }
        if (!toSpawn.empty()) {
            Falling_SpawnBatch(toSpawn);
        }
    }
}

