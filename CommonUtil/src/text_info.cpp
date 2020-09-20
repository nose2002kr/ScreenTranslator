#include "text_info.h"
#include <mutex>
#include <algorithm>

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

bool intersect(RECT lhs, RECT rhs) {
  return (lhs.left < rhs.right && lhs.right > rhs.left &&
          lhs.top > rhs.bottom && lhs.bottom < rhs.top);
}

void removeIntersectRect(RECT rect) {
  textInfoMutex.lock();
  std::vector<TextInfo> filtered;
  std::copy_if(g_textInfo.begin(), g_textInfo.end(), std::back_inserter(filtered), [&rect](TextInfo info) {return !intersect(info.rect, rect); });
  g_textInfo.clear();
  g_textInfo.assign(filtered.begin(), filtered.end());
  textInfoMutex.unlock();
}