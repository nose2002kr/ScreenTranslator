#include "screenTranslator.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"
#include "KeyHook/src/keyHook.h"

#include "messageHandler.h"

ScreenTranslator::ScreenTranslator(HINSTANCE hInstance) {
  TextOverlay::init(hInstance);
  OCR::init("C:/Users/KS/cpp/tesseract-/tessdata");

  runFindTextThread();
  runShowTextThread();
  installKeyHook();
  runMessagwHandler();
}

ScreenTranslator::~ScreenTranslator() {
  m_terminate = true;
}

void
ScreenTranslator::runFindTextThread() {
  m_findTxhread = std::thread([&] {
    while (!m_terminate) {
      Image* img = TextOverlay::instnace()->windowScreenCapture();
      if (!img) {
        continue;
      }

      cv::Mat image = imageUtil::toMat(img);
      OCR::instnace()->findOutTextInfos(image);
      delete img;

      Sleep(100);
    }
  });
}

void 
ScreenTranslator::runShowTextThread() {
  m_showTxThread = std::thread([&] {
    while (!m_terminate) {
    TextOverlay::instnace()->showText();
    Sleep(100);
  }});
}

void
ScreenTranslator::installKeyHook() {

}

void
ScreenTranslator::runMessagwHandler() {
  MessageHandler handler;
  handler.handle();
}