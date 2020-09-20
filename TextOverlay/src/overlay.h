#pragma once

#include "common_util.h"
#include "text_info.h"
//#include "SimpleCapture.h"

class SimpleCapture;

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
  IDWriteTextFormat* getTextFormat(float fontSize);
  
  winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface> windowScreenCapture();
  bool isInvalidHwnd(HWND hWnd);
  void updateCanvasWindow();

  void showText();

  void requestWindowScreenCapture() { m_lastImage = Image{ 0, }; g_msg.push(MESSAGE_TO_CAPTURE_SCREEN); }
  void requestUpdateCanvasWindow() { g_msg.push(MESSAGE_TO_UPDATE_CANVAS_WINDOW); }
  Image getCapturedImage();
  void toggleWindowLock() { m_lockWindow = !m_lockWindow; }

private:
  void buildCanvasWindow(HINSTANCE hInstance);
  HRESULT	InitD3D();
  HRESULT InitD2D();
  ID2D1HwndRenderTarget* createRenderTarget();
  //Image D3SurfaceToImage(IDirect3DSurface9* surface, RECT rect);

  //IDirect3D9* m_pD3D = nullptr;
  //IDirect3DDevice9* m_pd3dDevice = nullptr;
  //IDirect3DSurface9* m_pSurface = nullptr;
  
  void startCaptureFromItem(winrt::Windows::Graphics::Capture::GraphicsCaptureItem item);
  void stopCapture();

  HWND m_canvasWnd = nullptr;
  ID2D1HwndRenderTarget* m_rt = nullptr;
  ID2D1Factory* m_fac = nullptr;
  IDWriteFactory* m_writeFac = nullptr;
  std::map<float, IDWriteTextFormat*> m_textFormats;

  HWND getTargetWindow();
  RECT getTargetWindowRect();
  int m_screenW = 0;
  int m_screenH = 0;

  Image m_lastImage;
  HWND m_lastWnd = nullptr;
  bool m_lockWindow = false;

  winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
  winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
  winrt::Windows::UI::Composition::CompositionSurfaceBrush m_brush{ nullptr };
  std::unique_ptr<SimpleCapture> m_capture{ nullptr };

};
