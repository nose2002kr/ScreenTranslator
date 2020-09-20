#include "common_util.h"

#include <Windows.h>
#include <time.h>
#include <fstream>

std::queue<int> g_msg;

std::string ANSIToUTF8(std::string ansi) {
  int	nLength, nLength2;
  BSTR	bstrCode;
  char*	pszUTFCode = NULL;
  nLength = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.length(), NULL, NULL);
  bstrCode = SysAllocStringLen(NULL, nLength);
  MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), ansi.length(), bstrCode, nLength);
  nLength2 = WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, 0, NULL, NULL);
  pszUTFCode = (char*)malloc(nLength2 + 1);
  WideCharToMultiByte(CP_UTF8, 0, bstrCode, -1, pszUTFCode, nLength2, NULL, NULL);
  std::string utf8(pszUTFCode);
  free(pszUTFCode);
  return utf8;
}

std::wstring strToWStr(std::string s) {
  wchar_t strUnicode[256] = { 0, };
  int nLen = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), NULL, NULL);
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.size(), strUnicode, nLen);
  return std::wstring(strUnicode);
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos)
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return str;
}

std::string getCurrentDateTime(std::string s) {
  time_t now = time(0);
  struct tm  tstruct;
  char  buf[80];
  localtime_s(&tstruct, &now);
  if (s == "now")
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
  else if (s == "date")
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
  return std::string(buf);
};

std::string padTo(std::string str, const size_t num, const char paddingChar = ' ') {
  if (num > str.size())
    str.insert(0, num - str.size(), paddingChar);
  return str;
}

void writeLog(logLevel lv, std::string logMsg) {
  std::string filePath = "./translater_" + getCurrentDateTime("date") + ".log";
  std::string now = getCurrentDateTime("now");
  std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
  ofs << now.c_str() << '\t';
  switch (lv) {
    case DEBUG: ofs << padTo("[DEBUG]", 10).c_str(); break;
    case INFO:  ofs << padTo("[INFO]",  10).c_str(); break;
    case WARN:  ofs << padTo("[WARN]",  10).c_str(); break;
    case FATAL: ofs << padTo("[FATAL]", 10).c_str(); break;
  }
  ofs << logMsg.c_str() << '\n';
  ofs.close();
}