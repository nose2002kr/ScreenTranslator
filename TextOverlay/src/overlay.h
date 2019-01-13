#pragma once

#include <exception>

#include "D2D1.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "dwrite.h"
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwrite.lib")

class TextOverlay {
private:
  static TextOverlay* g_inst;
   
  TextOverlay(HINSTANCE hInstance);
  ~TextOverlay();

public:
  static void init(HINSTANCE hInstance) {
    if (g_inst == nullptr) {
      g_inst = new TextOverlay(hInstance);
    }
  }
  static TextOverlay* instnace() {
    if (g_inst == nullptr) {
      throw std::exception("TextOverlay class is not initialized. first call init.");
    }
    return g_inst;
  }

  ID2D1HwndRenderTarget* getRenderTarget() { return m_rt; }

  bool isInvalidHwnd(HWND hWnd);
  void updateCanvasWindow(RECT rect);

private:
  void buildCanvasWindow(HINSTANCE hInstance);
  HRESULT	InitD3D();
  HRESULT InitD2D();
  ID2D1HwndRenderTarget* createRenderTarget();

  IDirect3D9* m_pD3D = nullptr;
  IDirect3DDevice9* m_pd3dDevice = nullptr;
  IDirect3DSurface9* m_pSurface = nullptr;
  
  HWND m_canvasWnd = nullptr;
  ID2D1Factory* m_fac = nullptr;
  IDWriteTextFormat* m_textFormat = nullptr;
  ID2D1HwndRenderTarget* m_rt = nullptr;
};
