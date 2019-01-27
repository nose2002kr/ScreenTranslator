#include "draw_util.h"
#include "../../CommonUtil/src/common_util.h"

void drawDebugLine(ID2D1RenderTarget* pTarget, RECT rect) {
  ID2D1SolidColorBrush* brush = nullptr;
  pTarget->CreateSolidColorBrush(D2D1::ColorF(255, 0, 0, 255), &brush);

  pTarget->DrawLine(D2D1::Point2(0, 0), D2D1::Point2(30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2(0, 0), D2D1::Point2(0, 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2(RctW(rect), 0), D2D1::Point2(RctW(rect) - 30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2(RctW(rect), 0), D2D1::Point2(RctW(rect), 0 + 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2(0, RctH(rect)), D2D1::Point2(0 + 30, RctH(rect)), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2(0, RctH(rect)), D2D1::Point2(0, RctH(rect) - 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2(RctW(rect), RctH(rect)), D2D1::Point2(RctW(rect) - 30, RctH(rect)), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2(RctW(rect), RctH(rect)), D2D1::Point2(RctW(rect), RctH(rect) - 30), brush, 4.0);
}