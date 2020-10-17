#pragma once

#include <opencv2/opencv.hpp>
#include "Windows.h"

class RectWrapper {
public:
  RectWrapper() = delete;
  RectWrapper(RECT rect);
  RectWrapper(cv::Rect rect);

  int x() const;
  int y() const;
  int w() const;
  int h() const;
  
  int l() const;
  int t() const;
  int r() const;
  int b() const;

  RECT toWinRect() const;
  cv::Rect toCVRect() const;
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

  bool contains(RectWrapper lhs, RectWrapper rhs);
  RectWrapper mergeRect(RectWrapper lhs, RectWrapper rhs);
  bool intersected(RectWrapper lhs, RectWrapper rhs);
  RectWrapper intersect(RectWrapper lhs, RectWrapper rhs);
  bool isEmpty(RectWrapper rect);

  std::string toString(RectWrapper rect);
};

