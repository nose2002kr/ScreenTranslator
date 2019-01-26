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
  Image img = ov->windowScreenCapture();
  
  OCR ocr("C:/Users/1004/C++/tesseract/tessdata");
  cv::Mat matImg(cv::Size(img.width, img.height), CV_8UC4, img.samples);
  cv::imwrite("./loadedImg.png", matImg);
  std::vector<TextInfo> infos = ocr.findOutTextInfos(&matImg);

  //Translate tr;
  //std::string translated = tr.translate(infos[0].text);
  
  while (true) {
    ov->showText(infos);
    ::Sleep(100);
  }
}