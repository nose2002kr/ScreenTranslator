#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"

#include "dwmapi.h"

#include <comdef.h>
#include <CRTDBG.H>
#include <atlconv.h>

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  Sleep(2000);
  TextOverlay::init(hInstance);
  TextOverlay* ov = TextOverlay::instnace();
  Image img = ov->screenCapture();
  
  OCR ocr("C:/Users/1004/C++/tesseract/tessdata");
  cv::Mat matImg(cv::Size(img.width, img.height), CV_8UC4, img.samples);
  cv::imwrite("C:\\Users\\1004\\C++\\loadedImg.png", matImg);
  std::vector<TextInfo> infos = ocr.findOutTextInfos(&matImg);

  //Translate tr;
  //std::string translated = tr.translate(infos[0].text);
  
  ID2D1HwndRenderTarget* pTarget = ov->getRenderTarget();
  IDWriteFactory* writeFac = nullptr;
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFac);
  IDWriteTextFormat* textFormat = nullptr;
  writeFac->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24, L"en-us", &textFormat);
  while (true) {
    HWND hWnd = ::GetForegroundWindow();
    if (ov->isInvalidHwnd(hWnd)) {
      ::Sleep(100); continue;
    }
    USES_CONVERSION;
    RECT rect;
    HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
    ov->updateCanvasWindow(rect);

    pTarget->BeginDraw();
    pTarget->Clear(D2D1::ColorF(1.0, 1.0, 0.));
    pTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    //drawDebugLine(pTarget, rect, redBrush);
    for (auto info : infos) {
      ID2D1SolidColorBrush* backBrs = nullptr;
      ID2D1SolidColorBrush* fontBrs = nullptr;
      pTarget->CreateSolidColorBrush(D2D1::ColorF(info.backgroundColor), &backBrs);
      pTarget->CreateSolidColorBrush(D2D1::ColorF(info.fontColor), &fontBrs);
      D2D1_RECT_F d2Rect = D2D1::RectF(info.rect.left, info.rect.top, info.rect.right, info.rect.bottom);
      pTarget->FillRectangle(d2Rect, backBrs);
      pTarget->DrawTextA(A2W(info.text.c_str()), info.text.length(), textFormat, d2Rect, fontBrs);
    }

    pTarget->EndDraw();

    ::Sleep(100);
  }
}