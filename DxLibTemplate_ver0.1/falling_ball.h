#pragma once

#include <vector>
#include "stage_ball.h"

// falling_ball モジュール: 孤立して落下するボールを管理
void Falling_Init();
void Falling_Update();
void Falling_Draw();
void Falling_Finalize();

// 指定セルから落下ボールを生成する（セルの色は spawn 前に取得してください）
void Falling_SpawnFromCell(int cellIndex, BallColor color);
// バッチで複数セルから落下ボールを生成する。遅延は同時生成群内の y 座標に基づき割り振られる。
void Falling_SpawnBatch(const std::vector<std::pair<int,BallColor>>& items);

