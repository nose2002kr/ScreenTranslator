#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <baseapi.h>

#include "../../CommonUtil/src/common_util.h"

class OCR {
public:
  OCR(std::string);
  ~OCR();
  std::vector<TextInfo> findOutTextInfos(cv::Mat* img);

  tesseract::TessBaseAPI* api;
};
