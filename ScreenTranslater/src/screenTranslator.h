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
  void installKeyHook();
  void runMessagwHandler();

  bool m_terminate = false;

  std::thread m_findTxhread;
  std::thread m_showTxThread;
};