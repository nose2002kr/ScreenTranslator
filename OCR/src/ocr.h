#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <baseapi.h>

#include "../../CommonUtil/src/common_util.h"

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

  void findOutTextInfos(Image imgParam, std::vector<TextInfo>* infos);
private:
  bool m_cancelFlag;
};
