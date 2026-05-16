#pragma once
#include <vector>

// 12フレームの簡易ランチパッドアニメーション制御クラス
class LaunchPadAnimation {
public:
    LaunchPadAnimation();
    // フレームの画像ハンドルと描画位置（左上基準）で初期化する
    void init(const std::vector<int>& frames, int posX, int posY);
    // フレーム切替の間隔（更新フレーム数）を設定する。1で毎フレーム切替。
    void setStepInterval(int frames);
    // キー入力に応じてフレームを進める／戻す（毎フレーム呼ぶ）
    void update();
    // 現在のフレームを左上位置で描画する
    void draw() const;
    int getIndex() const { return m_index; }
    // 外部から停止フラグを設定（true でアニメを停止）
    void setPaused(bool paused);

private:
    std::vector<int> m_frames;
    int m_index;
    int m_posX;
    int m_posY;
    int m_width;
    int m_height;
    // フレーム切替の間隔（更新フレーム数）。1で毎フレーム進む。
    int m_stepInterval;
    // 次の切替までのカウンタ（0で即時切替）
    int m_stepTimer;
    // 外部からの一時停止フラグ
    bool m_paused;
};
