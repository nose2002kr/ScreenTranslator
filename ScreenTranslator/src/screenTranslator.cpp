#include "screenTranslator.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"
#include "KeyHook/src/keyHook.h"

#include "messageHandler.h"

ScreenTranslator::ScreenTranslator(HINSTANCE hInstance) {
  TextOverlay::init(hInstance);
  OCR::init("./res/tessdata");

  runFindTextThread();
  runShowTextThread();
  runTranslateTextThread();
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
ScreenTranslator::runTranslateTextThread() {
  m_translateTxThread = std::thread([&] {
    while (!m_terminate) {
      for (int i = 0; i < getTextInfoSize(); i++) {
        TextInfo info = getTextInfo(i);
        if (info.translated) continue;
        info.translatedText = Translate::instance().translate(info.ocrText);
        info.translated = true;
        updateTextInfo(i, info);
      }
      ::Sleep(100);
    }
  });
}

void
ScreenTranslator::installKeyHook() {
  KeyHook* hook = KeyHook::instnace();

  // Key: Terminate app. [CTRL + ALT + Q]
  hook->registryFunction(KeySeq(true, false, true, 'q'), [&] { m_terminate = true; MessageHandler().terminate(); });
  // Key: Lock & Unlock window (do not switch windows any more). [CTRL + ALT + P]
  hook->registryFunction(KeySeq(true, false, true, 'p'), [] { TextOverlay::instnace()->toggleWindowLock(); });

  hook->startHook();
}

void
ScreenTranslator::runMessagwHandler() {
  MessageHandler handler;
  handler.start();
}