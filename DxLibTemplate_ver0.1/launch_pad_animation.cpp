#include "launch_pad_animation.h"
#include "keyManager.h"
#include "dxlib.h"

LaunchPadAnimation::LaunchPadAnimation()
    : m_index(0), m_posX(0), m_posY(0), m_width(0), m_height(0), m_stepInterval(0), m_stepTimer(0), m_paused(false)
{
}

void LaunchPadAnimation::init(const std::vector<int>& frames, int posX, int posY)
{
    m_frames = frames;
    m_posX = posX;
    m_posY = posY;
    m_index = 0;
    m_stepTimer = 0;

    if (!m_frames.empty()) {
        if (m_index >= (int)m_frames.size()) m_index = 0;
    }
    // 最初の有効な画像からサイズを取得しようとする
    m_width = 0;
    m_height = 0;
    if (!m_frames.empty() && m_frames[0] != -1)
    {
        GetGraphSize(m_frames[0], &m_width, &m_height);
    }

    m_paused = false;
}

void LaunchPadAnimation::setStepInterval(int frames)
{
    if (frames < 1) frames = 1;
    m_stepInterval = frames;
}

void LaunchPadAnimation::setPaused(bool paused)
{
    m_paused = paused;
}

void LaunchPadAnimation::update()
{
    if (m_frames.empty()) return;
    if (m_paused) return;
    // A/Dキーが押されている場合、間隔に応じて進める/戻す
    bool pressedD = checkHitKey(KEY_INPUT_D);
    bool pressedA = checkHitKey(KEY_INPUT_A);

    if (!pressedD && !pressedA) {
        // 押されていないときはタイマーをリセットして次の押下で即時反応させる
        m_stepTimer = 0;
        return;
    }

    // 押下中の初回は即時切替、それ以降は間隔を使って切替
    if (m_stepTimer == 0) {
        if (pressedD) {
            m_index++;
            if (m_index >= (int)m_frames.size()) m_index = 0;
        } else if (pressedA) {
            m_index--;
            if (m_index < 0) m_index = (int)m_frames.size() - 1;
        }
        m_stepTimer = 1;
        return;
    }

    m_stepTimer++;
    if (m_stepTimer >= m_stepInterval) {
        if (pressedD) {
            m_index++;
            if (m_index >= (int)m_frames.size()) m_index = 0;
        } else if (pressedA) {
            m_index--;
            if (m_index < 0) m_index = (int)m_frames.size() - 1;
        }
        // 次は間隔の初回として扱うため1にする（0だと上の分岐で即時再実行される）
        m_stepTimer = 1;
    }
}

void LaunchPadAnimation::draw() const
{
    if (m_frames.empty()) return;
    int handle = m_frames[m_index];
    if (handle == -1) return;

    int w = m_width;
    int h = m_height;
    if (w == 0 || h == 0)
    {
        GetGraphSize(handle, &w, &h);
    }

    // 表示を4倍にする
    const float scale_w = 4.0f;
	const float scale_h = 4.0f;
    w *= scale_w;
    h *= scale_h;

    // 左上基準で描画する
    int x1 = m_posX;
    int y1 = m_posY;
    int x2 = m_posX + w;
    int y2 = m_posY + h;

    DrawExtendGraph(x1, y1, x2, y2, handle, TRUE);
}
