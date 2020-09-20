#pragma once
#include "common_util.h"

#include <opencv2/opencv.hpp>
#include <Windows.h>

//#define DEBUG_LEVEL2

namespace imageUtil {
  std::vector<cv::Rect> findContourBounds(cv::Mat binaryImage, size_t contourComplexity);
  std::vector<cv::Rect> detectLetters(cv::Mat img);

  cv::Rect mergeRect(cv::Rect lhs, cv::Rect rhs);
  std::vector<cv::Rect> reorganizeText(std::vector<cv::Rect> src);

  std::vector<cv::Vec3b> findDominantColors(cv::Mat img, int count);

  cv::Rect toCVRect(RECT rect);
  RECT toWinRect(cv::Rect rect);
  
  int Vec2Rgb(cv::Vec3b vec);
  
  cv::Mat toMat(Image imgParam);
};