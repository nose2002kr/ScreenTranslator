#pragma once

#include "windows.h"
#include <thread>

class ScreenTranslator {
public:
  ScreenTranslator() = delete;
  ScreenTranslator(HINSTANCE hInstance);
  ~ScreenTranslator();
  
private:
  void runShowTextThread();
  void runFindTextThread();
  void runTranslateTextThread();
  void installKeyHook();
  void runMessagwHandler();

  bool m_terminate = false;
  bool m_disableTranslate = true;
  bool m_disableOCR = false;

  std::thread m_findTxhread;
  std::thread m_showTxThread;
  std::thread m_translateTxThread;
};