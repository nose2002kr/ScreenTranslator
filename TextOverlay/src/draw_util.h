#pragma once
#include "D2D1.h"
#define RctW(rect) (int) (rect.right - rect.left)
#define RctH(rect) (int) (rect.bottom - rect.top)

void drawDebugLine(ID2D1RenderTarget* pTarget, RECT rect, ID2D1Brush* brush);