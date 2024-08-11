#include <windows.h>

#include "common_util.h"

#include "test/test.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"

#include "screenTranslator.h"

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  //TextOverlay::init(hInstance);
  //OCR::init("C:/Users/KS/cpp/tesseract-/tessdata");
  writeLog(DEBUG, "Running ScreenTranslater.");

  //test::thread();
  //test::imageProcess();
  //test::thread2();
  //test::translate();
  ScreenTranslator *translator = new ScreenTranslator(hInstance);


  TextOverlay::instnace()->updateCanvasWindow();
  TextOverlay::instnace()->showText();

  Sleep(100000000);

  writeLog(DEBUG, "Terminated ScreenTranslater.");

  return 0;
}