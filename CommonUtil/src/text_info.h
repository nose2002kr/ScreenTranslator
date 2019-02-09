#pragma once
#include "common_util.h"

extern std::vector<TextInfo> g_textInfo;

void pushTextInfo(TextInfo info);
TextInfo getTextInfo(size_t i);
void clearTextInfo();
void updateTextInfo(size_t i, TextInfo info);
int getTextInfoSize();