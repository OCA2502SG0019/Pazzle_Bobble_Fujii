#include "arrow_animation.h"
#include "keyManager.h"
#include "dxlib.h"
#include <cstdio>

ArrowAnimation::ArrowAnimation()
    : m_index(0), m_value(0), m_posX(0), m_posY(0), m_width(0), m_height(0),
      m_stepInterval(3), m_stepTimer(0),
      m_prevPressedD(false), m_prevPressedA(false), m_flipped(false),
      m_holdFps(30.0), m_accumulator(0.0), m_lastTimeMs(0)
{
}

int ArrowAnimation::getDrawWidth() const
{
    if (m_width == 0) return 0;
    return (int)(m_width * 4.0f);
}

int ArrowAnimation::getDrawHeight() const
{
    if (m_height == 0) return 0;
    return (int)(m_height * 4.0f);
}

bool ArrowAnimation::isAtEnd() const
{
    if (m_frames.empty()) return true;
    int idx = abs(m_value);
    return idx >= (int)m_frames.size() - 1;
}

void ArrowAnimation::init(const std::vector<int>& frames, int posX, int posY)
{
    m_frames = frames;
    m_posX = posX;
    m_posY = posY;
    m_index = 0;
    m_value = 0;
    m_prevValue = 0;
    m_stepTimer = 0;
    m_prevPressedD = false;
    m_prevPressedA = false;
    m_flipped = false;

    m_width = 0;
    m_height = 0;
    if (!m_frames.empty() && m_frames[0] != -1) {
        GetGraphSize(m_frames[0], &m_width, &m_height);
    }
}

void ArrowAnimation::setStepInterval(int frames)
{
    if (frames < 1) frames = 1;
    m_stepInterval = frames;
}

void ArrowAnimation::update()
{
    if (m_frames.empty()) return;

    // 時間を取得して経過時間を計算
    unsigned int now = GetNowCount(); // ms
    double deltaSec = 0.0;
    if (m_lastTimeMs != 0) {
        deltaSec = (now - m_lastTimeMs) / 1000.0;
    }
    m_lastTimeMs = now;

    bool pressedD = checkHitKey(KEY_INPUT_D) != 0;
    bool pressedA = checkHitKey(KEY_INPUT_A) != 0;

    // ここでは A を負、D を正の増減として扱う。
    if (!pressedD && !pressedA) {
        m_stepTimer = 0;
        m_prevPressedD = false;
        m_prevPressedA = false;
        // 押されていないときは反転状態は論理値に従う（負値のみ反転）
        m_flipped = (m_value < 0);
        return;
    }

    // 両方同時押しは無視
    if (pressedD && pressedA) {
        m_prevPressedD = pressedD;
        m_prevPressedA = pressedA;
        return;
    }

    auto clampValue = [&](int v) {
        int maxIdx = (int)m_frames.size() - 1;
        if (maxIdx < 0) maxIdx = 0;
        if (v > maxIdx) v = maxIdx;
        if (v < -maxIdx) v = -maxIdx;
        return v;
    };

    auto applyDelta = [&](int delta) {
        m_prevValue = m_value;
        m_value = clampValue(m_value + delta);
        m_index = abs(m_value);
        m_flipped = (m_value < 0);
    };

    // 初回押下で即時に1つ動かす
    if (pressedD && !m_prevPressedD) {
        applyDelta(+1);
        m_stepTimer = 0;
        m_prevPressedD = true;
        m_prevPressedA = pressedA;
        return;
    }
    if (pressedA && !m_prevPressedA) {
        if (m_value == 0) {
            // arrow_1 のときは値を変えずに反転表示する
            m_index = 0;
            m_prevValue = m_value;
            // arrow_1 は反転しないようにする
            m_flipped = false;
            m_stepTimer = 0;
            m_prevPressedA = true;
            m_prevPressedD = pressedD;
            return;
        } else {
            applyDelta(-1);
            m_stepTimer = 0;
            m_prevPressedA = true;
            m_prevPressedD = pressedD;
            return;
        }
    }

    // 押しっぱなしの処理
    if (m_holdFps > 0.0) {
        // 時間ベースで滑らかに進める
        if (pressedD || pressedA) {
            m_accumulator += deltaSec * m_holdFps;
            while (m_accumulator >= 1.0) {
                if (pressedD) applyDelta(+1);
                else if (pressedA) applyDelta(-1);
                m_accumulator -= 1.0;
            }
        } else {
            m_accumulator = 0.0;
        }
    } else {
        // 既存のフレームカウントベース実装
        m_stepTimer++;
        if (m_stepTimer >= m_stepInterval) {
            if (pressedD) applyDelta(+1);
            else if (pressedA) applyDelta(-1);
            m_stepTimer = 0;
        }
    }

    m_prevPressedD = pressedD;
    m_prevPressedA = pressedA;
}

void ArrowAnimation::draw() const
{
    if (m_frames.empty()) return;
    if (m_index < 0 || m_index >= (int)m_frames.size()) return;
    int handle = m_frames[m_index];
    if (handle == -1) return;

    int w = m_width;
    int h = m_height;
    if (w == 0 || h == 0) {
        GetGraphSize(handle, &w, &h);
    }
    //サイズは4倍
    const float scale_w = 4.0f;
    const float scale_h = 4.0f;
    int dw = (int)(w * scale_w);
    int dh = (int)(h * scale_h);

    int x1 = m_posX;
    int y1 = m_posY;
    int x2 = m_posX + dw;
    int y2 = m_posY + dh;

    // 描画は m_flipped に応じて左右反転する
    if (!m_flipped) {
        DrawExtendGraph(x1, y1, x2, y2, handle, TRUE);
    } else {
        DrawExtendGraph(x2, y1, x1, y2, handle, TRUE);
    }
}
