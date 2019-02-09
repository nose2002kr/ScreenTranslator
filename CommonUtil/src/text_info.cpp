#include "text_info.h"
#include <mutex>

std::vector<TextInfo> g_textInfo;

std::mutex textInfoMutex;

void pushTextInfo(TextInfo info) {
  g_textInfo.push_back(info);
}
TextInfo getTextInfo(size_t i) {
  if (i < g_textInfo.size())
    return g_textInfo[i];
  else
    return TextInfo();
}
void clearTextInfo() {
  textInfoMutex.lock();
  g_textInfo.clear();
  textInfoMutex.unlock();
}
void updateTextInfo(size_t i, TextInfo info) {
  textInfoMutex.lock();
  if (i < g_textInfo.size())
    g_textInfo[i] = info;
  textInfoMutex.unlock();
}

int getTextInfoSize() {
  return g_textInfo.size();
}