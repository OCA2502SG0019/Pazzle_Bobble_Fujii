#include "dxlib.h"
#include "const.h"
#include "keyManager.h"
#include "launch_pad_animation.h"
#include "arrow_animation.h"
#include "debug.h"
#include "border_line.h"
#include "stage_ball.h"
#include "shot_ball.h"
#include "falling_ball.h"
#include "bag.h"
#include <vector>
#include <string>
#include <cmath>

static inline int iRound(double v) { return v >= 0.0 ? (int)(v + 0.5) : (int)(v - 0.5); }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    SetGraphMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32);
    ChangeWindowMode(true);
    if (DxLib_Init() == -1)
    {
        return -1;
    }
    // 描画先画面を裏画面にする
    SetDrawScreen(DX_SCREEN_BACK);

    //---------------------------------------
    // 変数の作成や初期化、その他初期設定
    // ↓ システム初期化 ↓
    initKeyManager();

    // --- 複数画像の読み込み ---
    const char* filenames[] = {
        "stage_1.png",    // 0
		"scaffold_1.png", // 1
		// ランチパッドフレーム（インデックス 2..13 を使用）
		"launch_pad_1.png",  // 2
		"launch_pad_2.png",  // 3
		"launch_pad_3.png",  // 4
		"launch_pad_4.png",  // 5
		"launch_pad_5.png",  // 6
		"launch_pad_6.png",  // 7
		"launch_pad_7.png",  // 8
		"launch_pad_8.png",  // 9
		"launch_pad_9.png",  // 10
		"launch_pad_10.png", // 11
		"launch_pad_11.png", // 12
		"launch_pad_12.png", // 13

        // 必要ならここにファイル名を追加
    };
    const int fileCount = sizeof(filenames) / sizeof(filenames[0]);
    std::vector<int> images(fileCount, -1);

    int failedIndex = -1;
    for (int i = 0; i < fileCount; ++i)
    {
        images[i] = LoadGraph(filenames[i]);
        if (images[i] == -1)
        {
            failedIndex = i;
            break;
        }
    }

    if (failedIndex != -1)
    {
        // どのファイルの読み込みに失敗したかを通知
        std::string msg = std::string("画像の読み込みに失敗しました: ") + filenames[failedIndex];
        MessageBoxA(NULL, msg.c_str(), "LoadGraph Error", MB_OK | MB_ICONERROR);

        // 読み込めている画像は解放
        for (int j = 0; j < fileCount; ++j)
        {
            if (images[j] != -1) DeleteGraph(images[j]);
        }

        DxLib_End();
        return -1;
    }

    // インデックス 2 から読み込んだフレームでランチパッドアニメーションを作成
    std::vector<int> launchFrames;
    for (int i = 2; i <= 13 && i < fileCount; ++i) {
        launchFrames.push_back(images[i]);
    }

    // ランチパッドの左上位置 (左上基準)
    // 画像サイズが 56x40 のため、表示を2倍にするので幅は112になる
    // 親幅の中央に配置するには (親幅 - 56*2) / 2
    int posX = (WINDOW_WIDTH - 255) / 2;
    int posY = 705;

    LaunchPadAnimation launchAnim;
    launchAnim.init(launchFrames, posX, posY);
    launchAnim.setStepInterval(3); // ここで間隔を指定（3フレームごとに進む）

    // arrow画像を1..64で読み込む
    std::vector<int> arrowFrames;
    arrowFrames.reserve(64);
    for (int i = 1; i <= 64; ++i) {
        char buf[64];
        sprintf_s(buf, "arrow_%d.png", i);
        int h = LoadGraph(buf);
        arrowFrames.push_back(h);
    }

    // ここで座標を直接指定する（ウィンドウ左上を原点としてピクセル単位）
    const int arrowPosX = 510;
    const int arrowPosY = 640;

    ArrowAnimation arrowAnim;
    arrowAnim.init(arrowFrames, arrowPosX, arrowPosY);
    // 早く滑らかに動くよう間隔を小さくする（1で毎フレーム移動）
    arrowAnim.setStepInterval(1);
    // 押しっぱなしで秒単位の速度を設定（frames/sec）。例: 60で高速・滑らか
    arrowAnim.setHoldFps(80.0);

    DebugManager debug;

    BorderLine_Init();
    // mainで red_ball_1.png を読み込み、ball モジュールにハンドルを渡す
    int ballHandle = LoadGraph("red_ball_1.png");
    Ball_SetHandle(ballHandle);
    // yellow_ball_1.png を読み込み、黄色ボール用のハンドルを設定
    int yellowHandle = LoadGraph("yellow_ball_1.png");
    Ball_SetHandleForColor(BALL_YELLOW, yellowHandle);
    // blue_ball_1.png を読み込み、青ボール用のハンドルを設定
    int blueHandle = LoadGraph("blue_ball_1.png");
    Ball_SetHandleForColor(BALL_BLUE, blueHandle);
    // green_ball_1.png を読み込み、緑ボール用のハンドルを設定
    int greenHandle = LoadGraph("green_ball_1.png");
    Ball_SetHandleForColor(BALL_GREEN, greenHandle);
    // ball モジュールを初期化
    Ball_Init();
    // falling_ball モジュールを初期化
    Falling_Init();
    // bag 画像を読み込んでハンドルを設定
    int bagFrontHandle = LoadGraph("bag_1.png");
    int bagBackHandle = LoadGraph("bag_2.png");
    Bag_SetHandles(bagFrontHandle, bagBackHandle);
    // 起動時にショットボールをステージからランダム選択しておく（初期表示のため）
    Shot_PickRandomFromStage();

    // ↑ システム初期化 ↑
    //---------------------------------------



    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        //---------------------------------------
        // 更新
        // ↓ システム更新 ↓
        updateKeyState();
        // マウス左クリックの押下瞬間検出
        static int prevMouse = 0;
        int curMouse = GetMouseInput();
        bool leftPressedNow = (curMouse & MOUSE_INPUT_LEFT) != 0;
        bool leftPressedPrev = (prevMouse & MOUSE_INPUT_LEFT) != 0;
        if (leftPressedNow && !leftPressedPrev && !Shot_IsMoving()) {
            // 左クリックされた瞬間: 矢印軸からデバッグの白線と同じ軌道を計算してショットを移動
            int ax = arrowAnim.getPosX();
            int ay = arrowAnim.getPosY();
            int aw = arrowAnim.getDrawWidth();
            int ah = arrowAnim.getDrawHeight();
            if (aw > 0 && ah > 0) {
                double yellowX = ax + aw / 2.0;
                double yellowY = ay + ah / 2.0;
                double centerX = ax + aw / 2.0;
                double topY = ay;
                double radius = aw / 2.0;
                double centerY = topY + radius - 2.0;

                int fc = arrowAnim.getFrameCount();
                int maxValue = (fc > 0) ? (fc - 1) : 64;
                if (maxValue <= 0) maxValue = 64;
                int v = arrowAnim.getValue();
                if (v > maxValue) v = maxValue;
                if (v < -maxValue) v = -maxValue;
                double tval = (double)(v + maxValue) / (2.0 * maxValue);
                const double PI = acos(-1.0);
                double angleVal = -PI + tval * PI;
                double greenX = centerX + radius * cos(angleVal);
                double greenY = centerY + radius * sin(angleVal);

                double dirX = greenX - yellowX;
                double dirY = greenY - yellowY;
                double len = sqrt(dirX*dirX + dirY*dirY);
                if (len > 1e-6) {
                    dirX /= len; dirY /= len;

                    const double redLeft = 384.0;
                    const double redRight = 896.0;
                    const double blueY = 96.0;
                    const double effLeft = redLeft + 32.0;
                    const double effRight = redRight - 32.0;
                    const double effBlueY = blueY + 32.0;
                    const double EPS = 1e-4;
                    const int maxReflections = 100;

                    double curX = yellowX;
                    double curY = yellowY;
                    std::vector<std::pair<int,int>> pts;
                    pts.emplace_back(iRound(curX), iRound(curY));

                    for (int r = 0; r < maxReflections; ++r) {
                        double tWall = 1e308;
                        int hitWall = 0;
                        if (fabs(dirX) > 1e-9) {
                            double tL = (effLeft - curX) / dirX;
                            if (tL > EPS && tL < tWall) { tWall = tL; hitWall = -1; }
                            double tR = (effRight - curX) / dirX;
                            if (tR > EPS && tR < tWall) { tWall = tR; hitWall = +1; }
                        }
                        double tBlue = 1e308;
                        if (fabs(dirY) > 1e-9) {
                            double tB = (effBlueY - curY) / dirY;
                            if (tB > EPS) tBlue = tB;
                        }
                        if (tBlue < tWall) {
                            double hitX = curX + dirX * tBlue;
                            double hitY = curY + dirY * tBlue;
                            pts.emplace_back(iRound(hitX), iRound(hitY));
                            break;
                        }
                        if (tWall < 1e307) {
                            double hitX = curX + dirX * tWall;
                            double hitY = curY + dirY * tWall;
                            pts.emplace_back(iRound(hitX), iRound(hitY));
                            // reflect
                            curX = hitX; curY = hitY;
                            dirX = -dirX;
                            curX += dirX * EPS * 10.0;
                            curY += dirY * EPS * 10.0;
                            continue;
                        } else {
                            double farX = curX + dirX * 2000.0;
                            double farY = curY + dirY * 2000.0;
                            pts.emplace_back(iRound(farX), iRound(farY));
                            break;
                        }
                    }

                    // 開始位置を先頭にしてショット移動を開始
                    Shot_StartMovement(pts);
                }
            }
        }
        prevMouse = curMouse;
        // Ctrl+D でデバッグ表示トグル（押した瞬間だけ）
        if ((checkHitKey(KEY_INPUT_LCONTROL) || checkHitKey(KEY_INPUT_RCONTROL)) && pushHitKey(KEY_INPUT_D)) {
            // デバッグ表示をトグルする（入力は常に有効のまま）
            debug.toggle();
            // ※ 発射ごとにステージからランダム選択する仕様へ変更したため、
            //    ここではショット色の再選択を行わない。
        }
        // ↑ システム更新 ↑
        //---------------------------------------



        //---------------------------------------
        // 描画
        // ↓ 画面消去 ↓
        clsDx();
        ClearDrawScreen();
        // ↑ 画面消去 ↑
        //---------------------------------------

        // ステージ画像を描画 (左上 0,0)
        // images 配列を使って描画。ここでは 0 をウィンドウいっぱいに、1 を下部に描画する例を示す
        if (images.size() > 0 && images[0] != -1)
        {
            DrawExtendGraph(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, images[0], TRUE);
        }
        if (images.size() > 1 && images[1] != -1)
        {
            DrawExtendGraph(0, 864, WINDOW_WIDTH, WINDOW_HEIGHT, images[1], TRUE);
        }
        // ステージとスプライトの間に境界線を描画
        BorderLine_Draw();
        // ランチパッドアニメーションの更新と描画
        arrowAnim.update();
        // arrowが末端のときはランチパッド停止
        if (arrowAnim.isAtEnd()) {
            launchAnim.setPaused(true);
        } else {
            launchAnim.setPaused(false);
        }
        launchAnim.update();
        launchAnim.draw();
        // arrowアニメーションの描画は更新のあと
        arrowAnim.draw();

        // デバッグ表示
        debug.draw(launchAnim, arrowAnim);
        // デバッグ用のボールを最前面に描画
        Ball_DrawInDebug();
        // bag を中央に描画（後面→前面）
        Bag_DrawBothCentered(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
        // 孤立して落下中のボールを更新・描画
        Falling_Update();
        Falling_Draw();
        // shot の更新と描画
        Shot_Update();
        if (Shot_IsMoving()) {
            Shot_DrawCurrent();
        } else {
            // 移動中でなければ従来どおり arrow の中心に合わせて描画
            int ax = arrowAnim.getPosX();
            int ay = arrowAnim.getPosY();
            int aw = arrowAnim.getDrawWidth();
            int ah = arrowAnim.getDrawHeight();
            if (aw > 0 && ah > 0) {
                int centerX = ax + aw/2;
                int centerY = ay + ah/2;
                Shot_DrawSpriteAt(centerX, centerY);
            } else {
                Shot_DrawSprite();
            }
        }
        ScreenFlip();
    }

    // 読み込んだ画像を解放
    for (size_t i = 0; i < images.size(); ++i)
    {
        if (images[i] != -1) DeleteGraph(images[i]);
    }
    for (size_t i = 0; i < arrowFrames.size(); ++i)
    {
        if (arrowFrames[i] != -1) DeleteGraph(arrowFrames[i]);
    }

    // bag ハンドルの解放
    if (bagFrontHandle != -1) DeleteGraph(bagFrontHandle);
    if (bagBackHandle != -1) DeleteGraph(bagBackHandle);

    BorderLine_Finalize();
    Ball_Finalize();
    Falling_Finalize();

    DxLib_End();

    return 0;
}