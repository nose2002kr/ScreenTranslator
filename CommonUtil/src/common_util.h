#pragma once

#include <queue>
#include <string>

#define RctW(rect) (rect.right - rect.left)
#define RctH(rect) (rect.bottom - rect.top)

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
std::string replaceAll(std::string str, const std::string& from, const std::string& to);

enum logLevel {DEBUG, INFO, WARN, FATAL};
void writeLog(logLevel lv, std::string logMsg);