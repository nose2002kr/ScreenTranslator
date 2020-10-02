#include "test.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"

HWND foundHWnd;
BOOL CALLBACK findNotepadWindow(HWND hwnd, LPARAM lParam) {
  static TCHAR buffer[50];

  GetWindowText(hwnd, buffer, 50);
  if (strstr(buffer, "Windows ¸Þ¸ðÀå")) {
    foundHWnd = hwnd;
    return FALSE;
  }

  return TRUE;
}

void test::imageProcess() {
  if (EnumWindows(findNotepadWindow, NULL)) {
    return;
  }

  TextOverlay::instnace()->setTargetWindow(foundHWnd);
  Image* img = TextOverlay::instnace()->windowScreenCapture();
  if (!img) {
    return;
  }

  cv::Mat image = imageUtil::toMat(img);
  cv::imwrite("./snap.png", image);

  OCR::instnace()->findOutTextInfos(image);
  delete img;

  TextOverlay::instnace()->updateCanvasWindow();
  TextOverlay::instnace()->showText();

  Sleep(100000000);
}