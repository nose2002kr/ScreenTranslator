#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"
#include "KeyHook/src/keyHook.h"

bool termFlag = false;

void findingText() {
  OCR* ocr = OCR::instnace();
  TextOverlay* ov = TextOverlay::instnace();
  while (!termFlag) {
    ov->requestWindowScreenCapture();
    ocr->findOutTextInfos(ov->getCapturedImage(), &g_textInfo);
    ::Sleep(100);
  }
}

void showingText() {
  TextOverlay* ov = TextOverlay::instnace();
  while (!termFlag) {
    ov->showText(g_textInfo);
    ::Sleep(100);
  }
}

void translatingText() {
  Translate trans;
  while (!termFlag) {
    for (int i = 0 ; i < g_textInfo.size(); i++) {
      TextInfo info = g_textInfo.at(i);
      if (info.translated) continue;
      info.translatedText = trans.translate(info.ocrText);
      info.translated = true;
      trans.pushHistory(info.ocrText, info.translatedText);
      g_textInfo[i] = info;
    }

    ::Sleep(100);
  }
}

void keyHooking() {
  KeyHook* hook = KeyHook::instnace();
  
  // Key: Terminate app. [CTRL + ALT + Q]
  hook->registryFunction(KeySeq(true, false, true, 'q'), [] { termFlag = true; });
  // Key: Lock & Unlock window (do not switch windows any more). [CTRL + ALT + P]
  hook->registryFunction(KeySeq(true, false, true, 'p'), [] { TextOverlay::instnace()->toggleWindowLock(); });
  
  hook->startHook();
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
  
  std::thread showTh(showingText);
  std::thread findTh(findingText);
  std::thread translateTh(translatingText);
  std::thread hookTh(keyHooking);
  
  while (!termFlag) {
    if (g_msg.empty()) {
      ::Sleep(100);
      continue;
    }

    switch (g_msg.front()) {
      case MESSAGE_TO_CAPTURE_SCREEN: TextOverlay::instnace()->windowScreenCapture(); break;
      case MESSAGE_TO_UPDATE_CANVAS_WINDOW: TextOverlay::instnace()->updateCanvasWindow(); break;
    
      case MESSAGE_OCR_CANCEL: g_textInfo.clear(); OCR::instnace()->cancel(); break;
    }
    g_msg.pop();
  }

  translateTh.join();
  showTh.join();
  OCR::instnace()->cancel();
  findTh.join();
  KeyHook::instnace()->terminateHook();
  //hookTh.join();
  return 0;
}