#include "border_line.h"
#include "const.h"
#include "dxlib.h"

static int g_borderHandle = -1;

void BorderLine_Init()
{
    if (g_borderHandle != -1) return;
    g_borderHandle = LoadGraph("border_line.png");
}

void BorderLine_Draw()
{
    if (g_borderHandle == -1) return;
    int w = 0, h = 0;
    GetGraphSize(g_borderHandle, &w, &h);
    if (w == 0 || h == 0) return;
    const float scale = 4.0f;
    int dw = (int)(w * scale);
    int dh = (int)(h * scale);
    int x1 = (WINDOW_WIDTH - dw) / 2;
    int y1 = 700;
    int x2 = x1 + dw;
    int y2 = y1 + dh;
    DrawExtendGraph(x1, y1, x2, y2, g_borderHandle, TRUE);
}

void BorderLine_Finalize()
{
    if (g_borderHandle != -1) {
        DeleteGraph(g_borderHandle);
        g_borderHandle = -1;
    }
}
