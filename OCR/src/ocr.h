#pragma once
#include <vector>
#include "tesseract/baseapi.h"

#include "common_util.h"
#include "text_info.h"

#include "image_util.h"

class OCR {
  private:
    static OCR* g_inst;

    OCR(std::string);
    ~OCR();

    tesseract::TessBaseAPI* api;
public:
  static void init(std::string tessdataPath) {
    if (g_inst == nullptr) {
      g_inst = new OCR(tessdataPath);
    }
  }
  static OCR* instnace() {
    if (g_inst == nullptr) {
      throw std::exception("OCR class is not initialized. first call init.");
    }
    return g_inst;
  }

  void cancel() { m_cancelFlag = true; }

  void findOutTextInfos(Image *imgParam);
  void findOutTextInfos(cv::Mat img);
private:
  bool m_cancelFlag;
};
