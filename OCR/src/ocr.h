#pragma once
#include <vector>

#define HUNSPELL_STATIC
#include "hunspell.h"

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
    Hunhandle* spellObj;
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

  bool findOutTextInfos(Image *imgParam);
  bool findOutTextInfos(const cv::Mat &img, int relx = 0, int rely = 0, bool useDiff = true);
private:
  bool m_cancelFlag = false;
  cv::Mat m_lastImage;
  std::mutex m_mtx;
};
