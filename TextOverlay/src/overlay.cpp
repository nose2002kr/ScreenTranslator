//#define DEBUG_LEVEL2
#include "overlay.h"

#include <stdio.h>
#include <iostream>

#include <vector>
#include <memory>

#include "dwmapi.h"
#include "draw_util.h"

#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwrite.lib")

#include <CRTDBG.H>
#include <atlconv.h>
#include <queue>

#define throwIfFail(func, failMsg) {HRESULT hr = func;\
if(!SUCCEEDED(hr)) { throw std::exception(failMsg); }}

TextOverlay *TextOverlay::g_inst = nullptr;

TextOverlay::TextOverlay(HINSTANCE hInstance) {
  buildCanvasWindow(hInstance);
  InitD3D();
  InitD2D();
  createRenderTarget();
}

Image
TextOverlay::D3SurfaceToImage(IDirect3DSurface9* surface, RECT rect) {
  if (rect.left < 0 || rect.top < 0) {
    int adj = rect.left < rect.top ? rect.left : rect.top;
    rect.left -= adj;
    rect.top -= adj;
    rect.right += adj;
    rect.bottom += adj;
  }
  int w = RctW(rect);
  int h = RctH(rect);

  BYTE* pBits = new BYTE[w * h * 4];
  int scanline = w * 4;
  RECT screenRect = { 0, 0, m_screenW, m_screenH };
  D3DLOCKED_RECT	lockedRect;
  if (FAILED(surface->LockRect(&lockedRect, &screenRect, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY)))
  {
    //ErrorMessage("Unable to Lock Front Buffer Surface");	break;
  }

  for (int i = 0; i < h; i++) {
    memcpy((BYTE*)pBits + (i * scanline),
      (BYTE*)lockedRect.pBits + (((rect.top + i) * (m_screenW) + rect.left) * 4),
      scanline);
  }

  surface->UnlockRect();

  m_lastImage = Image{ pBits, w, h };
  return m_lastImage;
}

Image
TextOverlay::screenCapture() {
  RECT canvasRect;
  DwmGetWindowAttribute(m_canvasWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &canvasRect, sizeof(RECT));

  SetWindowPos(m_canvasWnd, HWND_TOPMOST, INT_MAX, INT_MAX, 0, 0, SWP_NOSIZE);
  m_pd3dDevice->GetFrontBufferData(0, m_pSurface);
  SetWindowPos(m_canvasWnd, HWND_TOPMOST, canvasRect.left, canvasRect.top, 0, 0, SWP_NOSIZE);
#ifdef DEBUG_LEVEL2
  D3DXSaveSurfaceToFile("./capturedImage.bmp",
    D3DXIFF_BMP,
    m_pSurface,
    NULL,
    NULL);
#endif
  
  return D3SurfaceToImage(m_pSurface, RECT{ 0, 0, m_screenW, m_screenH });
}

Image
TextOverlay::windowScreenCapture() {
  HWND hWnd = ::GetForegroundWindow();
  if (isInvalidHwnd(hWnd)) {
    return Image{ 0, };
  }

  RECT canvasRect;
  DwmGetWindowAttribute(m_canvasWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &canvasRect, sizeof(RECT));

  SetWindowPos(m_canvasWnd, HWND_TOPMOST, INT_MAX, INT_MAX, 0, 0, SWP_NOSIZE);
  m_pd3dDevice->GetFrontBufferData(0, m_pSurface);
  SetWindowPos(m_canvasWnd, HWND_TOPMOST, canvasRect.left, canvasRect.top, 0, 0, SWP_NOSIZE);
#ifdef DEBUG_LEVEL2
  D3DXSaveSurfaceToFile("./capturedImage.bmp",
    D3DXIFF_BMP,
    m_pSurface,
    NULL,
    NULL);
#endif

  RECT windowRect;
  DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &windowRect, sizeof(RECT));
  return D3SurfaceToImage(m_pSurface, windowRect);
  
}

ID2D1HwndRenderTarget* 
TextOverlay::createRenderTarget() {

  RECT rect;
  GetClientRect(m_canvasWnd, &rect);
  D2D1_SIZE_U winSize = D2D1::SizeU(RctW(rect), RctH(rect));
  throwIfFail(
    m_fac->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(m_canvasWnd, winSize, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS),
      &m_rt),
    "CreateHwndRenderTarget Failure.");
  return m_rt;
}

HRESULT
TextOverlay::InitD3D() {
  D3DDISPLAYMODE ddm;
  D3DPRESENT_PARAMETERS d3dpp = { 0, };

  if ((m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
  {
    return E_FAIL;
  }

  throwIfFail(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm), "GetAdapterDisplayMode failure.");

  d3dpp.Windowed = true;
  d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
  d3dpp.BackBufferFormat = ddm.Format;
  d3dpp.BackBufferHeight = m_screenW = ddm.Width;
  d3dpp.BackBufferWidth = m_screenH = ddm.Height;
  d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.hDeviceWindow = NULL;
  d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
  d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

  throwIfFail(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice),
    "CreateDevice failure.");

  //if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_R8G8BA8B8G8R8, D3DPOOL_SYSTEMMEM, &g_pSurface, NULL)))
  throwIfFail(m_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &m_pSurface, NULL), 
    "CreateOffscreenPlainSurface failrue.");

  return S_OK;
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
  ::SetLayeredWindowAttributes(m_canvasWnd, RGB(255, 255, 0), 0, LWA_COLORKEY);
}

bool
TextOverlay::isInvalidHwnd(HWND hWnd) {
  return hWnd == m_canvasWnd;
}

void
TextOverlay::updateCanvasWindow() {
  RECT rect = getForegroundWindowRect();
  RECT canvasRect;
  DwmGetWindowAttribute(m_canvasWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &canvasRect, sizeof(RECT));
  if (canvasRect.left == rect.left
   && canvasRect.top == rect.top
   && canvasRect.right == rect.right
   && canvasRect.bottom == rect.bottom)
    return;

  m_rt->Resize(D2D1::SizeU(RctW(rect), RctH(rect)));
  ShowWindow(m_canvasWnd, true);
  SetWindowPos(m_canvasWnd, HWND_TOPMOST, rect.left, rect.top, RctW(rect), RctH(rect), NULL);
}

IDWriteFactory* 
TextOverlay::getWriteFactory() {
  if (m_writeFac == nullptr) {
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_writeFac);
  }
  return m_writeFac;
}

IDWriteTextFormat*
TextOverlay::getTextFormat(int fontSize) {
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

RECT 
TextOverlay::getForegroundWindowRect() {
  HWND hWnd = ::GetForegroundWindow();
  if (isInvalidHwnd(hWnd)) {
    return RECT{ 0, };
  }

  RECT rect;
  throwIfFail(DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT)), "getForegroundWindowRect");
  return rect;
}
void
TextOverlay::showText(std::vector<TextInfo> infos) {
  requestUpdateCanvasWindow();

  ID2D1HwndRenderTarget* pTarget = getRenderTarget(); 
  pTarget->BeginDraw();
  pTarget->Clear(D2D1::ColorF(1.0, 1.0, 0.));
  pTarget->SetTransform(D2D1::Matrix3x2F::Identity());
  
  drawDebugLine(pTarget, getForegroundWindowRect());
  
  USES_CONVERSION;
  for (auto info : infos) {
    IDWriteTextFormat* textFormat = getTextFormat(RctH(info.rect));
    ID2D1SolidColorBrush* backBrs = nullptr;
    ID2D1SolidColorBrush* fontBrs = nullptr;
    pTarget->CreateSolidColorBrush(D2D1::ColorF(info.backgroundColor), &backBrs);
    pTarget->CreateSolidColorBrush(D2D1::ColorF(info.fontColor), &fontBrs);
    D2D1_RECT_F d2Rect = D2D1::RectF(info.rect.left, info.rect.top, info.rect.right, info.rect.bottom);
    pTarget->FillRectangle(d2Rect, backBrs);
    pTarget->DrawTextA(A2W(info.text.c_str()), info.text.length(), textFormat, d2Rect, fontBrs);
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