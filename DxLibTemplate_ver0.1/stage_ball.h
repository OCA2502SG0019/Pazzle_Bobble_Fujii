#pragma once

void Ball_Init();
void Ball_DrawInDebug();
void Ball_Finalize();
void Ball_SetHandle(int handle);

// セルに割り当てるボール色の列挙
enum BallColor { BALL_NONE = 0, BALL_RED = 1, BALL_BLUE = 2, BALL_GREEN = 3, BALL_YELLOW = 4 };

// セルごとの色マッピング用 API
void Ball_SetStageCell(int cellIndex, BallColor color);
BallColor Ball_GetStageCell(int cellIndex);

// 色ごとの画像ハンドルを設定する（色に対応したグラフィックを登録）
void Ball_SetHandleForColor(BallColor color, int handle);

// ハンドル取得（色に対応する画像ハンドルを返す）
int Ball_GetHandleForColor(BallColor color);

// ステージの最大セル数を取得
int Ball_GetMaxCells();

