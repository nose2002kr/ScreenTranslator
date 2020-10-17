#include "ocr.h"

#include "debug_util.h"
#include "rect_util.h"

OCR* OCR::g_inst = nullptr;

OCR::OCR(std::string tessdataPath) {
  api = new tesseract::TessBaseAPI();
  if (api->Init(tessdataPath.c_str(), "eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }

  api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
  api->SetVariable("tessedit_char_whitelist", "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+[],./<>?`~;'\"{}\\|- ");
}

OCR::~OCR() {
  if (api) delete api;
}

bool
OCR::findOutTextInfos(Image *imgParam) {
  return findOutTextInfos(imageUtil::toMat(imgParam));
}

bool
OCR::ocrText(const cv::Mat &image, cv::Rect cropRect, int relx, int rely) {
  cv::Mat cropImg = image(imageUtil::normalize(image, cropRect));
  //resize(cropImg, cropImg, cv::Size(cropImg.cols, cropImg.rows));//resize image

  cropRect.x += relx;
  cropRect.y += rely;

#ifdef DEBUG_LEVEL1
  cv::imwrite(DEBUG_LEVEL1"letterBox.png", cropImg);
#endif
  api->SetImage(cropImg.data, cropImg.cols, cropImg.rows, cropImg.channels(), cropImg.step1());
  char* textOutput = api->GetUTF8Text();     // Get the text 
  if (!textOutput) return false;
  if (strlen(textOutput) == 0) {
    delete[] textOutput;
    return false;
  }

  writeLog(DEBUG, std::string("  detected text: ") + textOutput);
  removeIntersectRect(rectUtil::toWinRect(cropRect));

  std::vector<cv::Vec3b> vec = imageUtil::findDominantColors(cropImg, 2);
  TextInfo tInfo{ 0, };

  tInfo.backgroundColor = static_cast<int>(imageUtil::Vec2Rgb(vec[1]));
  tInfo.fontColor = static_cast<int>(imageUtil::Vec2Rgb(vec[0]));
  tInfo.ocrText = replaceAll(std::string(textOutput), "\n", "");
  tInfo.rect = rectUtil::toWinRect(cropRect);
  pushTextInfo(tInfo);
  delete[] textOutput;
  return true;
}

bool
OCR::findOutTextInfos(const cv::Mat &img, int relx, int rely, bool useDiff) {
  writeLog(DEBUG, "try find out text infos");
  bool found = false;
  if (img.empty()) return found;

  std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
  if (!useDiff 
    || m_lastImage.empty() 
    || img.cols * img.rows != m_lastImage.cols * m_lastImage.rows) {

    std::vector<cv::Rect> detectedLetterBoxes = imageUtil::detectLetters(img);
    std::vector<cv::Rect> letterBBoxes = detectedLetterBoxes;// imageUtil::reorganizeText(detectedLetterBoxes);
    writeLog(DEBUG, "detected letter boxes(" + std::to_string(letterBBoxes.size()) + ")");
    for (auto it = letterBBoxes.begin(); it != letterBBoxes.end(); ++it) {
      if (m_cancelFlag) {
        writeLog(INFO, "canceled ocr");
        m_cancelFlag = false;
        m_lastImage = cv::Mat();
        return found;
      }

      ocrText(img, *it, relx, rely);
    }

#ifdef DEBUG_LEVEL1
    cv::Mat debugImg = img.clone();
    for (int i = 0; i < g_textInfo.size(); i++) {
      cv::rectangle(debugImg, rectUtil::toCVRect(g_textInfo[i].rect), cv::Scalar(0, 255, 0, 255), 3, 8, 0);
    }
    cv::imwrite(DEBUG_LEVEL1"searchedImage.png", debugImg);
#endif
  } else {
#ifdef DEBUG_LEVEL1
    cv::imwrite(DEBUG_LEVEL1"img.png", img);
    cv::imwrite(DEBUG_LEVEL1"lastImage.png", m_lastImage);
#endif
    std::vector<cv::Rect> diffRange = imageUtil::findDiffRange(m_lastImage, img);
    if (!diffRange.empty()) {
      for (auto rect : diffRange) {
        cv::Mat crop = img(rect);
#ifdef DEBUG_LEVEL1
        cv::imwrite(DEBUG_LEVEL1"crop.png", crop);
#endif
        
        writeLog(DEBUG, "partitial find out text infos [" + rectUtil::toString(rect) + "]");
        bool foundFromParticle = OCR::instnace()->findOutTextInfos(crop, relx + rect.x, rely + rect.y, false);
        if (!foundFromParticle) {
          removeIntersectRect(rectUtil::toWinRect(rect));
        }
        found = found | foundFromParticle;
      }
    }
  }
  if (found) {
    m_lastImage = img.clone();
  }
  writeLog(DEBUG, "complete find out text infos");
  return found;
}