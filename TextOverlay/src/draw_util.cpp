#include "draw_util.h"
#include "common_util.h"

void drawDebugLine(ID2D1RenderTarget* pTarget, RECT rect) {
  ID2D1SolidColorBrush* brush = nullptr;
  pTarget->CreateSolidColorBrush(D2D1::ColorF(255, 0, 0, 255), &brush);

  pTarget->DrawLine(D2D1::Point2<float>(0, 0), D2D1::Point2<float>(30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>(0, 0), D2D1::Point2<float>(0, 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>((float) RctW(rect), 0), D2D1::Point2<float>((float) RctW(rect) - 30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>((float) RctW(rect), 0), D2D1::Point2<float>((float) RctW(rect), 0 + 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>(0, (float) RctH(rect)), D2D1::Point2<float>(0 + 30, (float) RctH(rect)), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>(0, (float) RctH(rect)), D2D1::Point2<float>(0, (float) RctH(rect) - 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>((float) RctW(rect), (float) RctH(rect)), D2D1::Point2<float>((float) RctW(rect) - 30, (float) RctH(rect)), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>((float) RctW(rect), (float) RctH(rect)), D2D1::Point2<float>((float) RctW(rect), (float) RctH(rect) - 30), brush, 4.0);
}