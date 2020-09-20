#pragma once
#include <Windows.h>
#include <vector>

struct TextInfo {
  RECT rect;
  int fontColor;
  int backgroundColor;
  std::string ocrText;

  bool translated;
  std::string translatedText;
};

extern std::vector<TextInfo> g_textInfo;

void pushTextInfo(TextInfo info);
TextInfo getTextInfo(size_t i);
void clearTextInfo();
void updateTextInfo(size_t i, TextInfo info);
int getTextInfoSize();
void removeIntersectRect(RECT rect);