#include "test.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"
#include "KeyHook/src/keyHook.h"

#include <thread>

bool termFlag = false;

void findingText() {
  OCR* ocr = OCR::instnace();
  TextOverlay* ov = TextOverlay::instnace();
  cv::Mat prevImage;
  while (!termFlag) {
    Image* img = ov->windowScreenCapture();
    if (!img) {
      Sleep(100);
      continue;
    }
    cv::Mat image = imageUtil::toMat(*img);

    //delete[] img->samples;
    //delete img;

    std::vector<cv::Rect> diffRange;
    if (!prevImage.empty()) {
      cv::Mat prevImageG, imageG;
      cvtColor(prevImage, prevImageG, cv::COLOR_BGR2GRAY);
      cvtColor(image, imageG, cv::COLOR_BGR2GRAY);
      cv::Mat diff = imageG ^ prevImageG;
      if (!diff.empty()) {
        cv::threshold(diff, diff, 0, 255, cv::THRESH_OTSU + cv::THRESH_BINARY);
        diffRange = imageUtil::findContourBounds(diff, 4);
      }
    }

    if (diffRange.empty()) {
      ocr->findOutTextInfos(image);
    }
    else {
      for (auto rect : diffRange) {
        removeIntersectRect(imageUtil::toWinRect(rect));
        ocr->findOutTextInfos(image(rect));
      }
    }

    prevImage = image;
    cv::imwrite("./image-0.png", prevImage);
    ::Sleep(100);
  }
}

void showingText() {
  TextOverlay* ov = TextOverlay::instnace();
  while (!termFlag) {
    ov->showText();
    ::Sleep(100);
  }
}

void translatingText() {
  Translate trans;
  while (!termFlag) {
    for (int i = 0; i < getTextInfoSize(); i++) {
      TextInfo info = getTextInfo(i);
      if (info.translated) continue;
      info.translatedText = trans.translate(info.ocrText);
      info.translated = true;
      trans.pushHistory(info.ocrText, info.translatedText);
      updateTextInfo(i, info);
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

void test::thread() {
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

    case MESSAGE_OCR_CANCEL: clearTextInfo(); OCR::instnace()->cancel(); break;
    }
    g_msg.pop();
  }

  //translateTh.join();
  showTh.join();
  OCR::instnace()->cancel();
  findTh.join();
  KeyHook::instnace()->terminateHook();
  //hookTh.join();
}