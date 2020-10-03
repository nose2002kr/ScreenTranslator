#include "ocr.h"

#define DEBUG_LEVEL2
OCR* OCR::g_inst = nullptr;

OCR::OCR(std::string tessdataPath) {
  api = new tesseract::TessBaseAPI();
  if (api->Init(tessdataPath.c_str(), "eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }

  api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
}

OCR::~OCR() {
  delete api;
}

bool
OCR::findOutTextInfos(Image *imgParam) {
  return findOutTextInfos(imageUtil::toMat(imgParam));
}

bool
OCR::findOutTextInfos(cv::Mat img, int relx, int rely, bool useDiff) {
  bool found = false;

  std::unique_lock<std::mutex> lock(m_mtx, std::try_to_lock);
  if (!useDiff 
    || m_lastImage.empty() 
    || img.cols * img.rows != m_lastImage.cols * m_lastImage.rows) {

    std::vector<cv::Rect> detectedLetterBoxes = imageUtil::detectLetters(img);
    std::vector<cv::Rect> letterBBoxes = imageUtil::reorganizeText(detectedLetterBoxes);
    for (auto it = letterBBoxes.rbegin(); it != letterBBoxes.rend(); ++it) {
      if (m_cancelFlag) {
        m_cancelFlag = false;
        m_lastImage = cv::Mat();
        return found;
      }

      cv::Mat cropImg = img(imageUtil::normalize(img, *it));
      resize(cropImg, cropImg, cv::Size(cropImg.cols, cropImg.rows));//resize image
      
      it->x += relx;
      it->y += rely;

#ifdef DEBUG_LEVEL2
      cv::imwrite("./letterBox.png", cropImg);
#endif
      api->SetImage(cropImg.data, cropImg.cols, cropImg.rows, cropImg.channels(), cropImg.step1());
      char* textOutput = api->GetUTF8Text();     // Get the text 
      if (!textOutput) continue;
      if (strlen(textOutput) == 0) {
        delete[] textOutput;
        continue;
      }
#ifdef DEBUG_LEVEL2
      std::cout << textOutput << std::endl; // Destroy used object and release memory ocr->End();
#endif
      std::vector<cv::Vec3b> vec = imageUtil::findDominantColors(cropImg, 2);
      TextInfo tInfo{ 0, };
      
      tInfo.backgroundColor = static_cast<int>(imageUtil::Vec2Rgb(vec[1]));
      tInfo.fontColor = static_cast<int>(imageUtil::Vec2Rgb(vec[0]));
      tInfo.ocrText = replaceAll(std::string(textOutput), "\n", "");
      tInfo.rect = imageUtil::toWinRect(*it);
      pushTextInfo(tInfo);
      delete[] textOutput;
      found = true;
    }

#ifdef DEBUG_LEVEL2
    cv::Mat debugImg = img.clone();
    for (int i = 0; i < g_textInfo.size(); i++) {
      cv::rectangle(debugImg, imageUtil::toCVRect(g_textInfo[i].rect), cv::Scalar(0, 255, 0, 255), 3, 8, 0);
    }
    cv::imwrite("./searchedImage.png", debugImg);
#endif
  } else {
#ifdef DEBUG_LEVEL2
    cv::imwrite("./img.png", img);
    cv::imwrite("./lastImage.png", m_lastImage);
#endif
    std::vector<cv::Rect> diffRange = imageUtil::findDiffRange(m_lastImage, img);
    if (!diffRange.empty()) {
      for (auto rect : diffRange) {
        cv::Mat crop = img(rect);
#ifdef DEBUG_LEVEL2
        cv::imwrite("./crop.png", crop);
#endif
        found = found | OCR::instnace()->findOutTextInfos(crop, relx + rect.x, rely + rect.y, false);
      }
    }
  }
  if (found) {
    m_lastImage = img.clone();
  }
}