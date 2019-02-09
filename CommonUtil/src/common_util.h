#pragma once

#include <string>
#include <windows.h>
#include <queue>

#define RctW(rect) (int) (rect.right - rect.left)
#define RctH(rect) (int) (rect.bottom - rect.top)

struct TextInfo {
  RECT rect;
  int fontColor;
  int backgroundColor;
  std::string ocrText;
  
  bool translated;
  std::string translatedText;
};

struct Image {
  unsigned char* samples;
  int width;
  int height;
};

#define MESSAGE_TO_CAPTURE_SCREEN 100
#define MESSAGE_TO_UPDATE_CANVAS_WINDOW 101

#define MESSAGE_OCR_CANCEL 200

extern std::queue<int> g_msg;

// string util
std::string ANSIToUTF8(std::string ansi);
std::wstring strToWStr(std::string s);