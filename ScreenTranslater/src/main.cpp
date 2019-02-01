#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"

#include "dwmapi.h"

#include <comdef.h>
#include <CRTDBG.H>
#include <atlconv.h>

bool doYouWantBreakIt = false;

void findingText(std::vector<TextInfo>* infos) {
  OCR* ocr = OCR::instnace();
  TextOverlay* ov = TextOverlay::instnace();
  while (!doYouWantBreakIt) {
    ov->requestWindowScreenCapture();
    ocr->findOutTextInfos(ov->getCapturedImage(), infos);
    ::Sleep(100);
  }
}

void showingText(std::vector<TextInfo>* infos) {
  TextOverlay* ov = TextOverlay::instnace();
  while (!doYouWantBreakIt) {
    ov->showText(*infos);
    ::Sleep(100);
  }
}

void translatingText(std::vector<TextInfo>* infos) {
  Translate trans;
  while (!doYouWantBreakIt) {
    std::vector<TextInfo> copies(*infos);
    for (auto info : copies) {
      if (info.translated) continue;
      info.translatedText = trans.translate(info.ocrText);
      info.translated = true;
      trans.pushHistory(info.ocrText, info.translatedText);
    }
    if (copies.size() <= infos->size()) {
      for (int i = 0; i < copies.size(); i++) {
        if ((*infos)[i].ocrText == copies[i].ocrText) {
          (*infos)[i] = copies[i];
        }
      }
    }

    ::Sleep(100);
  }
}
#include <fstream>
int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  TextOverlay::init(hInstance);
  OCR::init("C:/Users/1004/C++/tesseract/tessdata");
  
  std::vector<TextInfo> infos;

  //Translate tr;
  //std::string translated = tr.translate(infos[0].text);
  
  std::thread st(showingText, &infos);
  std::thread ft(findingText, &infos);
  std::thread tt(translatingText, &infos);
  
  
  while (!doYouWantBreakIt) {
    if (g_msg.empty()) {
      ::Sleep(100);
      continue;
    }

    switch (g_msg.front()) {
      case MESSAGE_TO_CAPTURE_SCREEN: TextOverlay::instnace()->windowScreenCapture(); break;
      case MESSAGE_TO_UPDATE_CANVAS_WINDOW: TextOverlay::instnace()->updateCanvasWindow(); break;
    
      case MESSAGE_OCR_CANCEL: infos.clear(); OCR::instnace()->cancel(); break;
    }
    g_msg.pop();
  }

  tt.join();
  st.join();
  OCR::instnace()->cancel();
  ft.join();
}