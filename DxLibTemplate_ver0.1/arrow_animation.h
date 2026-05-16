
#pragma once
#include <vector>

// 矢印アニメーション制御クラス
class ArrowAnimation {
public:
    ArrowAnimation();
    // フレーム画像ハンドル群と描画左上位置で初期化
    void init(const std::vector<int>& frames, int posX, int posY);
    // 押しっぱなし時の更新間隔（フレーム数）
    void setStepInterval(int frames);
    // 押しっぱなし時のフレーム進行速度を秒単位で設定（frames/sec）。0で無効。
    void setHoldFps(double fps) { m_holdFps = fps; }
    // 毎フレーム呼ぶ更新処理
    void update();
    // 描画（左上基準）
    void draw() const;
    // 矢印が末端フレームに到達しているかを返す
    bool isAtEnd() const;
    // デバッグ用情報取得
    int getValue() const { return m_value; }
    int getIndex() const { return m_index; }
    bool getFlipped() const { return m_flipped; }
    // 描画位置とサイズ取得（デバッグ用）
    int getPosX() const { return m_posX; }
    int getPosY() const { return m_posY; }
    int getDrawWidth() const;
    int getDrawHeight() const;
    int getFrameCount() const { return (int)m_frames.size(); }

private:
    std::vector<int> m_frames;
    int m_index;
    // 論理的な位置。arrow_1 を 0 として、正は右（非反転）、負は左（反転）
    int m_value;
    // 1フレーム前の論理位置（符号変化検出用）
    int m_prevValue;
    int m_posX;
    int m_posY;
    int m_width;
    int m_height;
    int m_stepInterval;
    int m_stepTimer;
    // 押しっぱなし時の時間ベースでのフレーム進行用の設定
    double m_holdFps; // 押しっぱなし時のフレーム進行速度 (frames/sec)
    double m_accumulator; // 秒単位の加算器
    unsigned int m_lastTimeMs; // 前回更新時の時刻(ms)
    bool m_prevPressedD;
    bool m_prevPressedA;
    bool m_flipped;
};
