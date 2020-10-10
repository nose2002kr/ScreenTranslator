#include "test.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"
#include "Translate/src/translate.h"
#include "KeyHook/src/keyHook.h"

#include <thread>


HWND foundHWnd2;
BOOL CALLBACK findNotepadWindow2(HWND hwnd, LPARAM lParam) {
  static TCHAR buffer[50];

  GetWindowText(hwnd, buffer, 50);
  if (strstr(buffer, "Windows ¸Þ¸ðÀå")) {
    foundHWnd2 = hwnd;
    return FALSE;
  }

  return TRUE;
}

BOOL CALLBACK findNotepadWindow(HWND hwnd, LPARAM lParam);

void findingText2() {
  while (true) {
    Image* img = TextOverlay::instnace()->windowScreenCapture();
    if (!img) {
      continue;
    }

    cv::Mat image = imageUtil::toMat(img);
    OCR::instnace()->findOutTextInfos(image);
    delete img;

    Sleep(100);
  }
}

void showingText2() {
  while (true) {
    //TextOverlay::instnace()->updateCanvasWindow();
    TextOverlay::instnace()->showText();
    Sleep(100);
  }
}

void
test::thread2() {
  if (EnumWindows(findNotepadWindow2, NULL)) {
    return;
  }

  TextOverlay::instnace()->setTargetWindow(foundHWnd2);

  std::thread findTh(findingText2);
  std::thread showTh(showingText2);

 while (true) {
    if (g_msg.empty()) {
      ::Sleep(100);
      continue;
    }

    switch (g_msg.front()) {
    case MESSAGE_TO_UPDATE_CANVAS_WINDOW: TextOverlay::instnace()->updateCanvasWindow(); break;
    case MESSAGE_OCR_CANCEL: clearTextInfo(); OCR::instnace()->cancel(); break;
    }
    g_msg.pop();
  }

  showTh.join();
  findTh.join();
}
