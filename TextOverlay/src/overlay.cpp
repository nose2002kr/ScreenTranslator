#include <stdio.h>
#include <iostream>

#include <vector>
#include <memory>

#include "dwmapi.h"

#include "D2D1.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "dwrite.h"
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwrite.lib")

#include "draw_util.h"

#define throwIfFail(func, failMsg) {HRESULT hr = func;\
if(!SUCCEEDED(hr)) { throw std::exception(failMsg); }}

ID2D1HwndRenderTarget* createRenderTarget(HWND hwnd, ID2D1Factory* fac) {

  RECT rect;
  GetClientRect(hwnd, &rect);
  D2D1_SIZE_U winSize = D2D1::SizeU(RctW(rect), RctH(rect));
  ID2D1HwndRenderTarget* target = nullptr;
  throwIfFail(
    fac->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, winSize, D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS), &target),
    "CreateHwndRenderTarget Failure.");
  return target;
}

IDirect3D9*			g_pD3D = NULL;
IDirect3DDevice9*	g_pd3dDevice = NULL;
IDirect3DSurface9*	g_pSurface = NULL;

HRESULT	InitD3D()
{
  D3DDISPLAYMODE ddm;
  D3DPRESENT_PARAMETERS d3dpp = { 0, };

  if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
  {
    return E_FAIL;
  }

  if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))
  {
    return E_FAIL;
  }

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

  if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
  {
    return E_FAIL;
  }

  //if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_R8G8BA8B8G8R8, D3DPOOL_SYSTEMMEM, &g_pSurface, NULL)))
  if (FAILED(g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &g_pSurface, NULL)))
  {
    return E_FAIL;
  }

  return S_OK;
}

HWND canvasWnd;
ID2D1Factory* fac = nullptr;
IDWriteTextFormat* textFormat = nullptr;
ID2D1SolidColorBrush* redBlush = nullptr;
HRESULT InitD2D() {
  throwIfFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &fac), "CreateFactory Failure.");
  Sleep(1000);

  IDWriteFactory* writeFac = nullptr;
  throwIfFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFac), "DWriteCreateFactory Failure.");
  
  throwIfFail(
    writeFac->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, L"en-us", &textFormat),
    "CreateTextFormat Failure.");

  
  return S_OK;
}

std::vector<byte> ToPixels(HBITMAP BitmapHandle)
{
  int width;
  int height;
  BITMAP Bmp = { 0 };
  BITMAPINFO Info = { 0 };
  std::vector<byte> Pixels = std::vector<byte>();

  HDC DC = CreateCompatibleDC(NULL);
  std::memset(&Info, 0, sizeof(BITMAPINFO)); //not necessary really..
  HBITMAP OldBitmap = (HBITMAP)SelectObject(DC, BitmapHandle);
  GetObject(BitmapHandle, sizeof(Bmp), &Bmp);

  Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  Info.bmiHeader.biWidth = width = Bmp.bmWidth;
  Info.bmiHeader.biHeight = height = Bmp.bmHeight;
  Info.bmiHeader.biPlanes = 1;
  Info.bmiHeader.biBitCount = Bmp.bmBitsPixel;
  Info.bmiHeader.biCompression = BI_RGB;
  Info.bmiHeader.biSizeImage = ((width * Bmp.bmBitsPixel + 31) / 32) * 4 * height;

  Pixels.resize(Info.bmiHeader.biSizeImage);
  GetDIBits(DC, BitmapHandle, 0, height, &Pixels[0], &Info, DIB_RGB_COLORS);
  SelectObject(DC, OldBitmap);

  height = std::abs(height);
  DeleteDC(DC);
  return Pixels;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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

void buildCanvasWindow(HINSTANCE hInstance) {
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

  canvasWnd = CreateWindowEx(
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

  SetWindowPos(canvasWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  ShowWindow(canvasWnd, false);
  UpdateWindow(canvasWnd);
  ::SetLayeredWindowAttributes(canvasWnd, RGB(255, 255, 0), 0, LWA_COLORKEY);
  //::SetLayeredWindowAttributes(canvasWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
  //::SetLayeredWindowAttributes(canvasWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
  
  buildCanvasWindow(hInstance);
  InitD3D();
  InitD2D();

  ID2D1HwndRenderTarget* pTarget = nullptr;
  while (true) {
    HWND hWnd = ::GetForegroundWindow();
    if (hWnd == canvasWnd) {
      ::Sleep(100); continue;
    }

    RECT rect;
    HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    HDC hdc = ::GetDC(hWnd);
    ShowWindow(canvasWnd, true);
    SetWindowPos(canvasWnd, HWND_TOPMOST, rect.left, rect.top, RctW(rect), RctH(rect), NULL);

    if (pTarget == NULL || pTarget->GetHwnd() != hWnd) {
      pTarget = createRenderTarget(canvasWnd, fac);
      throwIfFail(pTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBlush), "CreateSolidColorBrush Failure.");
    }
    if (pTarget->GetSize().height != RctH(rect)
     && pTarget->GetSize().width != RctW(rect)) {
      pTarget->Resize(D2D1::SizeU(RctW(rect), RctH(rect)));
    }
    
    pTarget->BeginDraw();
    pTarget->Clear(D2D1::ColorF(1.0, 1.0, 0.));
    pTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    drawDebugLine(pTarget, rect, redBlush);
    
    pTarget->EndDraw();
    
    ::Sleep(100);
  }
  return 0;
}













void testFunc3() {
  /*HDC hDest = CreateCompatibleDC(hdc);
  HBITMAP bitmap = CreateCompatibleBitmap(hdc, RctW(rect), RctH(rect));
  SelectObject(hDest, bitmap);
  BitBlt(hDest, 0, 0, RctW(rect), RctH(rect), hdc, 0, 0, SRCCOPY);
  ReleaseDC(NULL, hdc);
  DeleteObject(bitmap);
  std::vector<byte> buf = ToPixels(bitmap);
  ID2D1Bitmap *pBackBufBitmap;
  hr = pTarget->CreateBitmap(
    D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top),
    &buf[0],
    RctW(rect) * 4,
    D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
    &pBackBufBitmap);*/
}



void testFunc2() {

  ID2D1Factory* fac = nullptr;
  throwIfFail(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &fac), "CreateFactory Failure.");
  Sleep(1000);

  HWND hwnd = ::GetForegroundWindow();
  InitD3D();

  IDWriteFactory* writeFac = nullptr;
  throwIfFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFac), "DWriteCreateFactory Failure.");
  IDWriteTextFormat* textFormat = nullptr;
  throwIfFail(
    writeFac->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, L"en-us", &textFormat),
    "CreateTextFormat Failure.");

  ID2D1RenderTarget* pTarget = createRenderTarget(canvasWnd, fac);
  ID2D1SolidColorBrush* redBlush = nullptr;
  throwIfFail(pTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBlush), "CreateSolidColorBrush Failure.");

  while (true) {
    //ShowWindow(canvasWnd, false);
    g_pd3dDevice->GetFrontBufferData(0, g_pSurface);
    //ShowWindow(canvasWnd, true);
    HWND hwnd2 = ::GetForegroundWindow();

    RECT rect;
    HRESULT hr = DwmGetWindowAttribute(hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    rect.left += 8;
    rect.top += 8;
    rect.right -= 8;
    rect.bottom -= 8;
    SetWindowPos(canvasWnd, HWND_TOPMOST, rect.left, rect.top, rect.right, rect.bottom, NULL);

    BYTE* pBits = new BYTE[(rect.bottom - rect.top) * (rect.right - rect.left) * 4];
    int scanline = (rect.right - rect.left) * 4;

    //g_pSurface->LockRect
    D3DXSaveSurfaceToFile("C:/temp/test.bmp", D3DXIFF_BMP, g_pSurface, NULL, NULL);		//Save to File
    D3DLOCKED_RECT	lockedRect;
    if (FAILED(g_pSurface->LockRect(&lockedRect, &rect, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY)))
    {
      //ErrorMessage("Unable to Lock Front Buffer Surface");	break;
    }

    for (int i = 0; i < rect.bottom - rect.top; i++)
    {
      memcpy(
        (BYTE*)pBits + (i * scanline),
        (BYTE*)lockedRect.pBits + ((rect.top + i) * scanline + rect.left),
        scanline);
    }

    g_pSurface->UnlockRect();

    ID2D1Bitmap *pBackBufBitmap;
    hr = pTarget->CreateBitmap(
      D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top),
      pBits,
      scanline,
      D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
      &pBackBufBitmap);
    /*throwIfFail(
      pTarget->CreateBitmap(D2D1::SizeU(rect.right-rect.left, rect.bottom - rect.top), pBits, 0, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &pBackBufBitmap),
      "CreateBitmap");*/

    pTarget->BeginDraw();
    pTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    pTarget->DrawBitmap(pBackBufBitmap, D2D1::RectF(0, 0, rect.right - rect.left, rect.bottom - rect.top));
    for (int i = 0; i < 1000; i++) {
      pTarget->DrawTextA(L"Hellllo", 7, textFormat, D2D1::Rect(0, 0 + i * 10, 100, 10 + i * 10), redBlush);
    }

    pTarget->EndDraw();

    delete[] pBits;
    Sleep(10);
  }
}


void testFunc(ID2D1Factory* fac) {
  HWND hwnd = ::GetForegroundWindow();
  ID2D1RenderTarget* pTarget = createRenderTarget(hwnd, fac);

  ID2D1SolidColorBrush* redBlush = nullptr;
  throwIfFail(pTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &redBlush), "CreateSolidColorBrush Failure.");
  
  IDWriteFactory* writeFac = nullptr;
  throwIfFail(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFac), "DWriteCreateFactory Failure.");

  IDWriteTextFormat* textFormat = nullptr;
  throwIfFail(
    writeFac->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 10, L"en-us", &textFormat),
    "CreateTextFormat Failure.");

  IDWriteTextLayout* textLayout;
  throwIfFail(writeFac->CreateTextLayout(L"HELLO", 6, textFormat, 100, 100, &textLayout),
    "CreateTextLayout Failure.");

  ID2D1Layer *pLayer = NULL;
  RECT rect;
  GetClientRect(hwnd, &rect);
  D2D1_SIZE_U winSize = D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top);
  D2D1_SIZE_F winSizeF = D2D1::SizeF(rect.right - rect.left, rect.bottom - rect.top);
  
  /*
  ID2D1Bitmap *pBackBufBitmap;
  throwIfFail(
    pTarget->CreateBitmap(winSize, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), &pBackBufBitmap),
    "CreateBitmap");

  throwIfFail(pBackBufBitmap->CopyFromRenderTarget(&D2D1::Point2U(0, 0), pTarget, &D2D1::RectU(0, 0, pBackBufBitmap->GetSize().width, pBackBufBitmap->GetSize().height))
    , "Copt Failure");*/
  
  //hr = pTarget->CreateLayer(NULL, &pLayer);
  while (true) {

    //pTarget->SetTransform(D2D1::Matrix3x2F::Translation(300, 250));
    pTarget->BeginDraw();
    //pTarget->PushLayer(D2D1::LayerParameters(),pLayer);
    pTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    //pTarget->DrawBitmap(pBackBufBitmap, D2D1::RectF(0, 0, rect.right - rect.left, rect.bottom - rect.top));
    
    // Push the layer with the content bounds.
    
    for (int i = 0; i < 1000; i++) {
      pTarget->DrawTextA(L"Hellllo", 7, textFormat, D2D1::Rect(0, 0 + i * 10, 100, 10 + i * 10), redBlush);
    }
  
    ///pBackBufBitmap->CopyFromRenderTarget(&D2D1::Point2U(0, 0), pTarget, &D2D1::RectU(0, 0, pBackBufBitmap->GetSize().width, pBackBufBitmap->GetSize().height));
    //pTarget->PopLayer();
    pTarget->EndDraw();
    ::Sleep(100);
  }

  //SafeRelease(redBlush);
  //SafeRelease(textFormat);
}