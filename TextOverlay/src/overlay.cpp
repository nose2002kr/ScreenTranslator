#include "pch.h"

#include "overlay.h"

#include <stdio.h>
#include <iostream>

#include <vector>
#include <memory>

#include "dwmapi.h"
#include "CaptureSnapshot.h"

#include "debug_util.h"
#include "util/draw_util.h"
#include "rect_util.h"

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment( lib, "d3d11.lib" )
#pragma comment(lib, "windowsapp")

#include <queue>

#define throwIfFail(func, failMsg) {HRESULT hr = func;\
if(!SUCCEEDED(hr)) { throw std::exception(failMsg); }}

TextOverlay* TextOverlay::g_inst = nullptr;

TextOverlay::TextOverlay(HINSTANCE hInstance) {
  buildCanvasWindow(hInstance);
  InitD2D();
  createRenderTarget();
}

Image*
TextOverlay::windowScreenCapture() {
  HWND hWnd = getTargetWindow();
  Image* img = CaptureSnapshot::inst().takeImage(hWnd);
  if (!img) return nullptr;
  return img;

}

ID2D1HwndRenderTarget*
TextOverlay::createRenderTarget() {

  RECT rect;
  GetClientRect(m_canvasWnd, &rect);
  RectWrapper rectWrapper(rect);
  D2D1_SIZE_U winSize = D2D1::SizeU(rectWrapper.w(), rectWrapper.h());
  throwIfFail(
    m_fac->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_canvasWnd, winSize, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
      &m_rt),
    "CreateHwndRenderTarget Failure.");
  return m_rt;
}

HRESULT
TextOverlay::InitD2D() {
  throwIfFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_fac), "CreateFactory Failure.");

  IDWriteFactory* writeFac = nullptr;
  throwIfFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFac), "DWriteCreateFactory Failure.");

  return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
  case WM_DESTROY:                                // If a close message is sent to the message loop
    PostQuitMessage(WM_QUIT);                   // Exit
    break;

  default:                                        // Handle all other messages in a normal way
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;

}

void
TextOverlay::buildCanvasWindow(HINSTANCE hInstance) {
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = (WNDPROC)WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hInstance = hInstance;
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.lpszClassName = "TEXT_OVERLAYER";
  wcex.lpszMenuName = NULL;
  wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  RegisterClassEx(&wcex);

  m_canvasWnd = CreateWindowEx(
    WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,// Optional window styles.
    "TEXT_OVERLAYER",                // Window class
    "TextOverlayer",                // Window text
    WS_POPUP | WS_VISIBLE,
    0, 0, 0, 0,
    NULL,       // Parent window    
    NULL,       // Menu
    hInstance,  // Instance handle
    NULL        // Additional application data
  );

  SetWindowPos(m_canvasWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  ShowWindow(m_canvasWnd, false);
  UpdateWindow(m_canvasWnd);
  ::SetLayeredWindowAttributes(m_canvasWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
}

bool
TextOverlay::isInvalidHwnd(HWND hWnd) {
  return hWnd == NULL || hWnd == m_canvasWnd;
}

void
TextOverlay::updateCanvasWindow() {
  RECT rect = getTargetWindowRect();
  RECT canvasRect;
  DwmGetWindowAttribute(m_canvasWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &canvasRect, sizeof(RECT));
  if (canvasRect.left == rect.left
    && canvasRect.top == rect.top
    && canvasRect.right == rect.right
    && canvasRect.bottom == rect.bottom)
    return;

  RectWrapper rectWrapper(rect);
  RectWrapper canvasRectWrapper(canvasRect);
  int w = rectWrapper.w();
  int h = rectWrapper.h();
  if (canvasRectWrapper.w() != w || canvasRectWrapper.h() != h) {
    g_msg.push(MESSAGE_OCR_CANCEL);
  }

  m_rt->Resize(D2D1::SizeU(w, h));
  ShowWindow(m_canvasWnd, true);
  SetWindowPos(m_canvasWnd, HWND_TOPMOST, rect.left, rect.top, w, h, NULL);
}

IDWriteFactory*
TextOverlay::getWriteFactory() {
  if (m_writeFac == nullptr) {
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_writeFac);
  }
  return m_writeFac;
}

IDWriteTextFormat*
TextOverlay::getTextFormat(float fontSize) {
  IDWriteTextFormat* textFormat = m_textFormats[fontSize];
  if (textFormat == NULL) {
    throwIfFail(
      getWriteFactory()->CreateTextFormat(
        L"Arial",
        NULL,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"en-us",
        &textFormat),
      "CreateTextFormat Failure.");
    m_textFormats[fontSize] = textFormat;
  }
  return textFormat;
}

HWND
TextOverlay::getTargetWindow() {
  if (m_lockWindow && m_lastWnd) {
    return m_lastWnd;
  }


  return m_lastWnd = ::GetForegroundWindow();
}

RECT
TextOverlay::getTargetWindowRect() {
  HWND hWnd = getTargetWindow();
  if (isInvalidHwnd(hWnd)) {
    return RECT{ 0, };
  }

  RECT rect;
  throwIfFail(DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT)), "getTargetWindowRect");
  return rect;
}

#define IGNORE_WHITE_COLOR(color) (color & 0xFFFFFF) == 0xFFFFFF ? 0xFEFEFE : color

void
TextOverlay::showText() {
  requestUpdateCanvasWindow();

  ID2D1HwndRenderTarget* pTarget = getRenderTarget();
  pTarget->BeginDraw();
  pTarget->Clear(D2D1::ColorF(1.0, 1.0, 1.0));
  pTarget->SetTransform(D2D1::Matrix3x2F::Identity());

  RectWrapper windowsRect = getTargetWindowRect();
#ifdef DEBUG_LEVEL1
  drawDebugLine(pTarget, windowsRect.toWinRect());
#endif

  for (int i = 0; i < getTextInfoSize(); i++) {
    TextInfo info = getTextInfo(i);
    if (rectUtil::isEmpty(info.rect)) continue;
    RectWrapper rect(info.rect);
    IDWriteTextFormat* textFormat = getTextFormat((float) rect.h() * 0.8f);
    ID2D1SolidColorBrush* backBrs = nullptr;
    ID2D1SolidColorBrush* fontBrs = nullptr;

    pTarget->CreateSolidColorBrush(D2D1::ColorF(IGNORE_WHITE_COLOR(info.backgroundColor)), (&backBrs));
    pTarget->CreateSolidColorBrush(D2D1::ColorF(IGNORE_WHITE_COLOR(info.fontColor)), (&fontBrs));
    D2D1_RECT_F d2Rect = D2D1::RectF((float)info.rect.left, (float)info.rect.top, (float)info.rect.right, (float)info.rect.bottom);
    pTarget->FillRectangle(d2Rect, backBrs);

    std::wstring drawingText = info.translated ? strToWStr(info.translatedText) : strToWStr(info.ocrText);
    IDWriteTextLayout* layout = nullptr;
    m_writeFac->CreateTextLayout(drawingText.c_str(), drawingText.length(), textFormat, windowsRect.w(), windowsRect.h(), &layout);
    pTarget->DrawTextLayout(D2D1::Point2<float>((float)info.rect.left, (float)info.rect.top), layout, fontBrs);
    layout->Release();
    backBrs->Release();
    fontBrs->Release();
  }

  pTarget->EndDraw();
}

Image
TextOverlay::getCapturedImage() {
  while (!m_lastImage.samples) {
    ::Sleep(10);
  }
  return m_lastImage;
}

void
TextOverlay::setTargetWindow(HWND hWnd) {
  m_lockWindow = true;
  m_lastWnd = hWnd;
}
