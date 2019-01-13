#include "overlay.h"

#include <stdio.h>
#include <iostream>

#include <vector>
#include <memory>

#include "dwmapi.h"
#include "draw_util.h"

#define throwIfFail(func, failMsg) {HRESULT hr = func;\
if(!SUCCEEDED(hr)) { throw std::exception(failMsg); }}

TextOverlay *TextOverlay::g_inst = nullptr;

TextOverlay::TextOverlay(HINSTANCE hInstance) {
  buildCanvasWindow(hInstance);
  InitD3D();
  InitD2D();
  createRenderTarget();
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
  d3dpp.BackBufferHeight = ddm.Width;
  d3dpp.BackBufferWidth = ddm.Height;
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
  
  throwIfFail(
    writeFac->CreateTextFormat(
      L"Arial",
      NULL,
      DWRITE_FONT_WEIGHT_REGULAR,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      10,
      L"en-us",
      &m_textFormat),
    "CreateTextFormat Failure.");
  
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
  //::SetLayeredWindowAttributes(m_canvasWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
  //::SetLayeredWindowAttributes(m_canvasWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
}

bool
TextOverlay::isInvalidHwnd(HWND hWnd) {
  return hWnd == m_canvasWnd;
}

void
TextOverlay::updateCanvasWindow(RECT rect) {
  if (m_rt->GetSize().height == RctH(rect)
   && m_rt->GetSize().width == RctW(rect))
    return;

  m_rt->Resize(D2D1::SizeU(RctW(rect), RctH(rect)));
  ShowWindow(m_canvasWnd, true);
  SetWindowPos(m_canvasWnd, HWND_TOPMOST, rect.left, rect.top, RctW(rect), RctH(rect), NULL);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){

  TextOverlay::init(hInstance);
  TextOverlay* overlayer = TextOverlay::instnace();
  

  ID2D1HwndRenderTarget* pTarget = overlayer->getRenderTarget();
  ID2D1SolidColorBrush* redBrush = nullptr;
  throwIfFail(pTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBrush), "CreateSolidColorBrush Failure.");
  while (true) {
    HWND hWnd = ::GetForegroundWindow();
    if (overlayer->isInvalidHwnd(hWnd)) {
      ::Sleep(100); continue;
    }

    RECT rect;
    HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    overlayer->updateCanvasWindow(rect);
    
    pTarget->BeginDraw();
    pTarget->Clear(D2D1::ColorF(1.0, 1.0, 0.));
    pTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    drawDebugLine(pTarget, rect, redBrush);
    
    pTarget->EndDraw();
    
    ::Sleep(100);
  }
  return 0;
}