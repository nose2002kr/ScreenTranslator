#include <windows.h>

#include "common_util.h"

#include "test/test.h"

int CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nCmdShow
) {
  writeLog(DEBUG, "Running ScreenTranslater.");

  //test::thread();
  test::imageProcess();
  writeLog(DEBUG, "Terminated ScreenTranslater.");

  return 0;
}