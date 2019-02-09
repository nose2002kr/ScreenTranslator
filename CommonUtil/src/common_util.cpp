#include "common_util.h"

#include <Windows.h>

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