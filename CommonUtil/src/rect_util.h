#pragma once

#include <opencv2/opencv.hpp>
#include "Windows.h"

class RectWrapper {
public:
  RectWrapper() = delete;
  RectWrapper(RECT rect);
  RectWrapper(cv::Rect rect);

  int x();
  int y();
  int w();
  int h();
  
  int l();
  int t();
  int r();
  int b();

  RECT toWinRect();
  cv::Rect toCVRect();
private:
  enum Type {
    UNDEFINED = 0,
    WIN_RECT = 1,
    CV_RECT = 2
  };
  Type m_type = UNDEFINED;
  RECT m_winRect;
  cv::Rect m_cvRect;
};

namespace rectUtil {
  cv::Rect toCVRect(RECT rect);
  RECT toWinRect(cv::Rect rect);

  bool contain(RectWrapper lhs, RectWrapper rhs);
  RectWrapper mergeRect(RectWrapper lhs, RectWrapper rhs);
  bool intersect(RectWrapper lhs, RectWrapper rhs);
  bool isEmpty(RectWrapper rect);
};

