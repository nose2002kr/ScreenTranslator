#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"

#include "dwmapi.h"

#include <comdef.h>
#include <CRTDBG.H>
#include <atlconv.h>

void findingOutTextInfos() {

}

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
  
  OCR::init("C:/Users/1004/C++/tesseract/tessdata");
  //std::vector<TextInfo> infos = ocr.findOutTextInfos(&matImg);
  std::vector<TextInfo> infos;

  //Translate tr;
  //std::string translated = tr.translate(infos[0].text);
  std::thread()
  while (true) {
    ov->showText(infos);
    ::Sleep(100);
  }
}