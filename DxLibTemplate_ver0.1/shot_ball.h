#pragma once


#include "stage_ball.h"
#include <vector>


// ShotBall はステージ上のボールからランダムに1つを選んで
// デバッグ用の黄色点位置に表示し、実際のボール画像をメインループで描画するモジュールです。

// 選択を行う（stage_ball に登録されたボールセルの中から1つ選ぶ）
void Shot_PickRandomFromStage();

// main などの描画ループで呼んで実際にボール画像を描画する（stage 上の選択セルに対応）
void Shot_DrawSprite();

// デバッグ表示用に黄色の点（マーカー）を描画する
void Shot_DrawMarker();

// 指定座標（中心）にスプライト／マーカーを表示するバージョン
void Shot_DrawSpriteAt(int centerX, int centerY);
void Shot_DrawMarkerAt(int centerX, int centerY);

// デバッグ用に現在選択されているショットボールの色を取得
BallColor Shot_GetColor();

// 選択されているセルインデックスを取得（存在しない場合は -1）
int Shot_GetCellIndex();

// 動的にショットを軌道に沿って移動させるための API
// メインループで毎フレーム Shot_Update() を呼び、描画は
// Shot_DrawCurrent() を用いて行う。
void Shot_StartMovement(const std::vector<std::pair<int,int>>& pathPoints);
void Shot_Update();
void Shot_DrawCurrent();
bool Shot_IsMoving();


