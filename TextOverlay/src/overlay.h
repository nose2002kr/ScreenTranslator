#pragma once

#include "../../CommonUtil/src/common_util.h"

#include <exception>

#include "D2D1.h"
#include "d3d9.h"
#include "d3dx9.h"
#include "dwrite.h"

#include <vector>
#include <map>

class TextOverlay {
private:
  static TextOverlay* g_inst;
   
  TextOverlay(HINSTANCE hInstance);
  ~TextOverlay() {}

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
  IDWriteFactory* getWriteFactory();
  IDWriteTextFormat* getTextFormat(int fontSize);
  
  Image screenCapture();
  Image windowScreenCapture();
  bool isInvalidHwnd(HWND hWnd);
  void updateCanvasWindow();

  void showText(std::vector<TextInfo> infos);

  void requestWindowScreenCapture() { g_msg.push(MESSAGE_TO_CAPTURE_SCREEN); }
  void requestUpdateCanvasWindow() { g_msg.push(MESSAGE_TO_UPDATE_CANVAS_WINDOW); }
  Image getCapturedImage();

private:
  void buildCanvasWindow(HINSTANCE hInstance);
  HRESULT	InitD3D();
  HRESULT InitD2D();
  ID2D1HwndRenderTarget* createRenderTarget();
  Image D3SurfaceToImage(IDirect3DSurface9* surface, RECT rect);

  IDirect3D9* m_pD3D = nullptr;
  IDirect3DDevice9* m_pd3dDevice = nullptr;
  IDirect3DSurface9* m_pSurface = nullptr;
  
  HWND m_canvasWnd = nullptr;
  ID2D1HwndRenderTarget* m_rt = nullptr;
  ID2D1Factory* m_fac = nullptr;
  IDWriteFactory* m_writeFac = nullptr;
  std::map<int, IDWriteTextFormat*> m_textFormats;

  RECT getForegroundWindowRect();
  int m_screenW;
  int m_screenH;

  Image m_lastImage;

};
