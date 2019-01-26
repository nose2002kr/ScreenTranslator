#pragma once

#include <string>
#include <windows.h>

#define RctW(rect) (int) (rect.right - rect.left)
#define RctH(rect) (int) (rect.bottom - rect.top)

struct TextInfo {
  RECT rect;
  int fontColor;
  int backgroundColor;
  std::string text;
};

struct Image {
  unsigned char* samples;
  int width;
  int height;
};