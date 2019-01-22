#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <baseapi.h>

struct TextInfo {
  cv::Rect rect;
  int fontColor;
  int backgroundColor;
  std::string text;
};

class OCR {
public:
  OCR(std::string);
  ~OCR();
  std::vector<TextInfo> findOutTextInfos(cv::Mat* img);

  tesseract::TessBaseAPI* api;
};
