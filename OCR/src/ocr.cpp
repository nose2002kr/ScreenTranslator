#include <string>
#include "baseapi.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
  string imPath = argv[1];

  
  tesseract::TessBaseAPI api;
  api.SetPageSegMode(tesseract::PSM_AUTO);  // Segmentation on auto 
  api.Init("C:/Users/1004/C++/tesseract/tessdata", "eng");   // path = parent directory of tessdata 
  // Open input image using OpenCV
  Mat im = cv::imread(imPath, IMREAD_COLOR);
  cv::cvtColor(im, im, COLOR_BGR2RGB);
  // Set image data
  api.SetImage(im.data, im.cols, im.rows, 4, 4* im.cols);
  //api.SetImage(image);       // Run the OCR 
  char* textOutput = new char[512];
  textOutput = api.GetUTF8Text();     // Get the text 
                                      
  // print recognized text
  cout << textOutput << endl; // Destroy used object and release memory ocr->End();

  return EXIT_SUCCESS;
}