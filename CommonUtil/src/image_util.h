#pragma once
#include "common_util.h"

#include <opencv2/opencv.hpp>
#include <Windows.h>

namespace imageUtil {
  std::vector<cv::Rect> findContourBounds(const cv::Mat &binaryImage, size_t contourComplexity);
  std::vector<cv::Rect> detectLetters(const cv::Mat &img);

  cv::Rect mergeRect(cv::Rect lhs, cv::Rect rhs);
  std::vector<cv::Rect> reorganizeText(const std::vector<cv::Rect> &src);

  std::vector<cv::Vec3b> findDominantColors(const cv::Mat &img, int count);

  cv::Rect toCVRect(RECT rect);
  RECT toWinRect(cv::Rect rect);
  
  int Vec2Rgb(cv::Vec3b vec);
  
  cv::Mat toMat(Image *imgParam);

  cv::Rect normalize(const cv::Mat& img, cv::Rect rect);
  
  std::vector<cv::Rect> findDiffRange(const cv::Mat &first, const cv::Mat &second);
};