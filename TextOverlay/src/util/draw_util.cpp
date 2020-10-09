#include "pch.h"

#include "draw_util.h"
#include "common_util.h"
#include "rect_util.h"

void drawDebugLine(ID2D1RenderTarget* pTarget, RECT rect) {
  RectWrapper rectWrapper(rect);
  ID2D1SolidColorBrush* brush = nullptr;
  pTarget->CreateSolidColorBrush(D2D1::ColorF(255, 0, 0, 255), &brush);

  pTarget->DrawLine(D2D1::Point2<float>(0, 0), D2D1::Point2<float>(30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>(0, 0), D2D1::Point2<float>(0, 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>((float)rectWrapper.w(), 0), D2D1::Point2<float>((float)rectWrapper.w() - 30, 0), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>((float)rectWrapper.w(), 0), D2D1::Point2<float>((float)rectWrapper.w(), 0 + 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>(0, (float)rectWrapper.h()), D2D1::Point2<float>(0 + 30, (float)rectWrapper.h()), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>(0, (float)rectWrapper.h()), D2D1::Point2<float>(0, (float)rectWrapper.h() - 30), brush, 4.0);

  pTarget->DrawLine(D2D1::Point2<float>((float)rectWrapper.w(), (float)rectWrapper.h()), D2D1::Point2<float>((float)rectWrapper.w() - 30, (float)rectWrapper.h()), brush, 4.0);
  pTarget->DrawLine(D2D1::Point2<float>((float)rectWrapper.w(), (float)rectWrapper.h()), D2D1::Point2<float>((float)rectWrapper.w(), (float)rectWrapper.h() - 30), brush, 4.0);
  brush->Release();
}