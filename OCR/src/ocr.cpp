#include "ocr.h"

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

void
OCR::findOutTextInfos(Image imgParam) {
  findOutTextInfos(imageUtil::toMat(imgParam));
}

void
OCR::findOutTextInfos(cv::Mat img) {

  std::vector<cv::Rect> detectedLetterBoxes = imageUtil::detectLetters(img);
  std::vector<cv::Rect> letterBBoxes = imageUtil::reorganizeText(detectedLetterBoxes);
  for (auto it = letterBBoxes.rbegin(); it != letterBBoxes.rend(); ++it) {
    if (m_cancelFlag) {
      m_cancelFlag = false;
      return;
    }

    cv::Mat cropImg = img(*it);
    resize(cropImg, cropImg, cv::Size(cropImg.cols, cropImg.rows));//resize image
#ifdef DEBUG_LEVEL2
    cv::imwrite("./crop.png", cropImg);
#endif
    api->SetImage(cropImg.data, cropImg.cols, cropImg.rows, cropImg.channels(), cropImg.step1());
    char* textOutput = api->GetUTF8Text();     // Get the text 
    if (strlen(textOutput) == 0) continue;
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
  }

#ifdef DEBUG_LEVEL2
  for (int i = 0; i < textInfos.size(); i++) {
    cv::rectangle(*img, W2CRect(textInfos[i].rect), cv::Scalar(0, 255, 0, 255), 3, 8, 0);
  }
  cv::imwrite("./searchedImage.png", *img);
#endif
  return;
}