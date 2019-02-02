#include "keyHook.h"

#pragma comment( lib, "user32.lib" )

#include <iostream>
#include <windows.h>
#include <stdio.h>

KeyHook* KeyHook::g_inst = nullptr;

HHOOK hKeyboardHook;

__declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
  DWORD SHIFT_key = 0;
  DWORD CTRL_key = 0;
  DWORD ALT_key = 0;

  if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
  {
    KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);
    DWORD dwMsg = 1;
    dwMsg += hooked_key.scanCode << 16;
    dwMsg += hooked_key.flags << 24;
    TCHAR lpszKeyName[1024] = { 0 };

    int i = GetKeyNameText(dwMsg, (lpszKeyName + 1), 0xFF) + 1;

    int key = hooked_key.vkCode;

    SHIFT_key = GetAsyncKeyState(VK_SHIFT);
    CTRL_key = GetAsyncKeyState(VK_CONTROL);
    ALT_key = GetAsyncKeyState(VK_MENU);

    if (key >= 'A' && key <= 'Z')
    {

      if (GetAsyncKeyState(VK_SHIFT) >= 0) key += 32;

      auto func = KeyHook::instnace()->findFunction(KeySeq(CTRL_key, SHIFT_key, ALT_key, key));
      if (func) func();

      SHIFT_key = 0;
      CTRL_key = 0;
      ALT_key = 0;
    }
  }
  return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

void MessageLoop()
{
  MSG message;
  while (GetMessage(&message, NULL, 0, 0))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

void 
KeyHook::startHook() {
  hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC) KeyboardEvent, GetModuleHandle(NULL), NULL);
  MessageLoop();
  
}

void
KeyHook::terminateHook() {
  PostQuitMessage(0);
  UnhookWindowsHookEx(hKeyboardHook);
}