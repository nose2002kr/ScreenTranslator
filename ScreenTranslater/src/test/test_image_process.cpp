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
  
  Image* img = TextOverlay::instnace()->windowScreenCapture();
  if (!img) {
    return;
  }

  cv::Mat image = imageUtil::toMat(img);
  //cv::imwrite("./snap.png", image);

  cv::Mat dbg1 = cv::imread("./debug1.png");
  cv::Mat dbg2 = cv::imread("./debug2.png");

  OCR::instnace()->findOutTextInfos(dbg2);
  OCR::instnace()->findOutTextInfos(dbg1);
  delete img;
}