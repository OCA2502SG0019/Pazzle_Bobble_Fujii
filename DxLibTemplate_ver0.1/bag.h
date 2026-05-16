#pragma once

// bag モジュール: 前面/後面のバッグ画像を同じ位置・大きさで描画するための簡易 API
void Bag_SetHandles(int frontHandle, int backHandle);
// バッグ描画の位置オフセットを設定（中心座標に対するピクセル単位のずれ）
void Bag_SetOffset(int offsetX, int offsetY);
void Bag_DrawBothCentered(int centerX, int centerY);
void Bag_Finalize();

