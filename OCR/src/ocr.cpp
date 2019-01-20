#include <string>
#include <opencv2/opencv.hpp>
#include "tesseract/baseapi.h"

using namespace std;
using namespace cv;

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <queue>

using namespace std;

typedef struct t_color_node {
  cv::Mat       mean;       // The mean of this node
  cv::Mat       cov;
  uchar         classid;    // The class ID

  t_color_node  *left;
  t_color_node  *right;
} t_color_node;

cv::Mat get_dominant_palette(std::vector<cv::Vec3b> colors) {
  const int tile_size = 64;
  cv::Mat ret = cv::Mat(tile_size, tile_size*colors.size(), CV_8UC3, cv::Scalar(0));

  for (int i = 0; i < colors.size(); i++) {
    cv::Rect rect(i*tile_size, 0, tile_size, tile_size);
    cv::rectangle(ret, rect, cv::Scalar(colors[i][0], colors[i][1], colors[i][2]), cv::FILLED);
  }

  return ret;
}

std::vector<t_color_node*> get_leaves(t_color_node *root) {
  std::vector<t_color_node*> ret;
  std::queue<t_color_node*> queue;
  queue.push(root);

  while (queue.size() > 0) {
    t_color_node *current = queue.front();
    queue.pop();

    if (current->left && current->right) {
      queue.push(current->left);
      queue.push(current->right);
      continue;
    }

    ret.push_back(current);
  }

  return ret;
}

std::vector<cv::Vec3b> get_dominant_colors(t_color_node *root) {
  std::vector<t_color_node*> leaves = get_leaves(root);
  std::vector<cv::Vec3b> ret;

  for (int i = 0; i < leaves.size(); i++) {
    cv::Mat mean = leaves[i]->mean;
    ret.push_back(cv::Vec3b(mean.at<double>(0)*255.0f,
      mean.at<double>(1)*255.0f,
      mean.at<double>(2)*255.0f));
  }

  return ret;
}

int get_next_classid(t_color_node *root) {
  int maxid = 0;
  std::queue<t_color_node*> queue;
  queue.push(root);

  while (queue.size() > 0) {
    t_color_node* current = queue.front();
    queue.pop();

    if (current->classid > maxid)
      maxid = current->classid;

    if (current->left != NULL)
      queue.push(current->left);

    if (current->right)
      queue.push(current->right);
  }

  return maxid + 1;
}

void get_class_mean_cov(cv::Mat img, cv::Mat classes, t_color_node *node) {
  const int width = img.cols;
  const int height = img.rows;
  const uchar classid = node->classid;

  cv::Mat mean = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  cv::Mat cov = cv::Mat(3, 3, CV_64FC1, cv::Scalar(0));

  // We start out with the average color
  double pixcount = 0;
  for (int y = 0; y < height; y++) {
    cv::Vec3b* ptr = img.ptr<cv::Vec3b>(y);
    uchar* ptrClass = classes.ptr<uchar>(y);
    for (int x = 0; x < width; x++) {
      if (ptrClass[x] != classid)
        continue;

      cv::Vec3b color = ptr[x];
      cv::Mat scaled = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
      scaled.at<double>(0) = color[0] / 255.0f;
      scaled.at<double>(1) = color[1] / 255.0f;
      scaled.at<double>(2) = color[2] / 255.0f;

      mean += scaled;
      cov = cov + (scaled * scaled.t());

      pixcount++;
    }
  }

  cov = cov - (mean * mean.t()) / pixcount;
  mean = mean / pixcount;

  // The node mean and covariance
  node->mean = mean.clone();
  node->cov = cov.clone();

  return;
}

void partition_class(cv::Mat img, cv::Mat classes, uchar nextid, t_color_node *node) {
  const int width = img.cols;
  const int height = img.rows;
  const int classid = node->classid;

  const uchar newidleft = nextid;
  const uchar newidright = nextid + 1;

  cv::Mat mean = node->mean;
  cv::Mat cov = node->cov;
  cv::Mat eigenvalues, eigenvectors;
  cv::eigen(cov, eigenvalues, eigenvectors);

  cv::Mat eig = eigenvectors.row(0);
  cv::Mat comparison_value = eig * mean;

  node->left = new t_color_node();
  node->right = new t_color_node();

  node->left->classid = newidleft;
  node->right->classid = newidright;

  // We start out with the average color
  for (int y = 0; y < height; y++) {
    cv::Vec3b* ptr = img.ptr<cv::Vec3b>(y);
    uchar* ptrClass = classes.ptr<uchar>(y);
    for (int x = 0; x < width; x++) {
      if (ptrClass[x] != classid)
        continue;

      cv::Vec3b color = ptr[x];
      cv::Mat scaled = cv::Mat(3, 1,
        CV_64FC1,
        cv::Scalar(0));

      scaled.at<double>(0) = color[0] / 255.0f;
      scaled.at<double>(1) = color[1] / 255.0f;
      scaled.at<double>(2) = color[2] / 255.0f;

      cv::Mat this_value = eig * scaled;

      if (this_value.at<double>(0, 0) <= comparison_value.at<double>(0, 0)) {
        ptrClass[x] = newidleft;
      } else {
        ptrClass[x] = newidright;
      }
    }
  }
  return;
}

cv::Mat get_quantized_image(cv::Mat classes, t_color_node *root) {
  std::vector<t_color_node*> leaves = get_leaves(root);

  const int height = classes.rows;
  const int width = classes.cols;
  cv::Mat ret(height, width, CV_8UC3, cv::Scalar(0));

  for (int y = 0; y < height; y++) {
    uchar *ptrClass = classes.ptr<uchar>(y);
    cv::Vec3b *ptr = ret.ptr<cv::Vec3b>(y);
    for (int x = 0; x < width; x++) {
      uchar pixel_class = ptrClass[x];
      for (int i = 0; i < leaves.size(); i++) {
        if (leaves[i]->classid == pixel_class) {
          ptr[x] = cv::Vec3b(leaves[i]->mean.at<double>(0) * 255,
            leaves[i]->mean.at<double>(1) * 255,
            leaves[i]->mean.at<double>(2) * 255);
        }
      }
    }
  }

  return ret;
}

cv::Mat get_viewable_image(cv::Mat classes) {
  const int height = classes.rows;
  const int width = classes.cols;

  const int max_color_count = 12;
  cv::Vec3b *palette = new cv::Vec3b[max_color_count];
  palette[0] = cv::Vec3b(0, 0, 0);
  palette[1] = cv::Vec3b(255, 0, 0);
  palette[2] = cv::Vec3b(0, 255, 0);
  palette[3] = cv::Vec3b(0, 0, 255);
  palette[4] = cv::Vec3b(255, 255, 0);
  palette[5] = cv::Vec3b(0, 255, 255);
  palette[6] = cv::Vec3b(255, 0, 255);
  palette[7] = cv::Vec3b(128, 128, 128);
  palette[8] = cv::Vec3b(128, 255, 128);
  palette[9] = cv::Vec3b(32, 32, 32);
  palette[10] = cv::Vec3b(255, 128, 128);
  palette[11] = cv::Vec3b(128, 128, 255);

  cv::Mat ret = cv::Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
  for (int y = 0; y < height; y++) {
    cv::Vec3b *ptr = ret.ptr<cv::Vec3b>(y);
    uchar *ptrClass = classes.ptr<uchar>(y);
    for (int x = 0; x < width; x++) {
      int color = ptrClass[x];
      if (color >= max_color_count) {
        printf("You should increase the number of predefined colors!\n");
        continue;
      }
      ptr[x] = palette[color];
    }
  }

  return ret;
}

t_color_node* get_max_eigenvalue_node(t_color_node *current) {
  double max_eigen = -1;
  cv::Mat eigenvalues, eigenvectors;

  std::queue<t_color_node*> queue;
  queue.push(current);

  t_color_node *ret = current;
  if (!current->left && !current->right)
    return current;

  while (queue.size() > 0) {
    t_color_node *node = queue.front();
    queue.pop();

    if (node->left && node->right) {
      queue.push(node->left);
      queue.push(node->right);
      continue;
    }

    cv::eigen(node->cov, eigenvalues, eigenvectors);
    double val = eigenvalues.at<double>(0);
    if (val > max_eigen) {
      max_eigen = val;
      ret = node;
    }
  }

  return ret;
}

std::vector<cv::Vec3b> find_dominant_colors(cv::Mat img, int count) {
  const int width = img.cols;
  const int height = img.rows;

  cv::Mat classes = cv::Mat(height, width, CV_8UC1, cv::Scalar(1));
  t_color_node *root = new t_color_node();

  root->classid = 1;
  root->left = NULL;
  root->right = NULL;

  t_color_node *next = root;
  get_class_mean_cov(img, classes, root);
  for (int i = 0; i < count - 1; i++) {
    next = get_max_eigenvalue_node(root);
    partition_class(img, classes, get_next_classid(root), next);
    get_class_mean_cov(img, classes, next->left);
    get_class_mean_cov(img, classes, next->right);
  }

  std::vector<cv::Vec3b> colors = get_dominant_colors(root);

  cv::Mat quantized = get_quantized_image(classes, root);
  cv::Mat viewable = get_viewable_image(classes);
  cv::Mat dom = get_dominant_palette(colors);

  cv::imwrite("./classification.png", viewable);
  cv::imwrite("./quantized.png", quantized);
  cv::imwrite("./palette.png", dom);

  return colors;
}
std::vector<cv::Rect>
detectLetters(cv::Mat img)
{
  std::vector<cv::Rect> boundRect;
  cv::Mat img_gray, img_sobel, img_threshold, element;
  cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  cv::Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::threshold(img_sobel, img_threshold, 0, 255, cv::THRESH_OTSU + cv::THRESH_BINARY);
  element = getStructuringElement(cv::MORPH_RECT, cv::Size(17, 3));
  cv::morphologyEx(img_threshold, img_threshold, cv::MORPH_CLOSE, element); //Does the trick
  std::vector< std::vector< cv::Point> > contours;
  cv::findContours(img_threshold, contours, 0, 1);
  std::vector<std::vector<cv::Point> > contours_poly(contours.size());
  for (int i = 0; i < contours.size(); i++)
    if (contours[i].size() > 100)
    {
      cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
      cv::Rect appRect(boundingRect(cv::Mat(contours_poly[i])));
      if (appRect.width > appRect.height)
        boundRect.push_back(appRect);
    }
  return boundRect;
}


Mat hwnd2mat(HWND hwnd)
{
  RECT rc;
  GetClientRect(hwnd, &rc);
  int width = rc.right;
  int height = rc.bottom;

  Mat src;
  src.create(height, width, CV_8UC4);

  HDC hdc = GetDC(hwnd);
  HDC memdc = CreateCompatibleDC(hdc);
  HBITMAP hbitmap = CreateCompatibleBitmap(hdc, width, height);
  HBITMAP oldbmp = (HBITMAP)SelectObject(memdc, hbitmap);

  BitBlt(memdc, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
  SelectObject(memdc, oldbmp);

  BITMAPINFOHEADER  bi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB };
  GetDIBits(hdc, hbitmap, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  DeleteObject(hbitmap);
  DeleteDC(memdc);
  ReleaseDC(hwnd, hdc);

  return src;
}

struct TextInfo {
  cv::Rect rect;
  int fontColor;
  int backgroundColor;
  std::string text;
};

cv::Rect rectMerge(Rect A, Rect B) {
  int left = min(A.x, B.x);
  int top = min(A.y, B.y);
  int right = max(A.x + A.width, B.x + B.width);
  int bottom = max(A.y + A.height, B.y + B.height);
  return Rect(left, top, right - left, bottom - top);
}
std::vector<cv::Rect> reorganizeText(std::vector<cv::Rect> src) {
  std::vector<cv::Rect> dst;

  cv::Rect rect;
  for (auto it = src.end() - 1; it != src.begin(); it--) {
    if (rect.empty()) {
      rect = *it;
      continue;
    }

    double spacing = rect.height * 0.8;
    if (rect.y - spacing < it->y && rect.br().y + spacing > it->y
      && rect.br().x + spacing > it->x) {
        rect = rectMerge(rect, *it);
    } else {
      dst.push_back(rect);
      rect = *it;
      //rect = cv::Rect();
    }
  }

  dst.push_back(rect);
  return dst;
}
void
ocrTest() {
  
  HWND hwndDesktop = GetDesktopWindow();
  Mat src = hwnd2mat(hwndDesktop);
  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  // Initialize tesseract-ocr with English, without specifying tessdata path
  if (api->Init("C:/Users/1004/C++/tesseract/tessdata", "eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }

  // Open input image with leptonica library
  api->SetImage(src.data, src.cols, src.rows, 4, 4 * src.cols);
  //Pix *image = pixRead("C:/Users/1004/Pictures/sample.tif");
  //api->SetImage(image);
  // Get OCR result
  char* outText = api->GetUTF8Text();
  printf("OCR output:\n%s", outText);

  // Destroy used object and release memory
  api->End(); 
  //delete[] outText;
  //pixDestroy(&image);

  ::Sleep(2000);
  

  /*
  HWND hwndDesktop = GetDesktopWindow();
  Mat src = hwnd2mat(hwndDesktop);
  cv::imwrite("C:/Users/1004/C++/capturedImage.png", src);
  tesseract::TessBaseAPI api;
  api.SetPageSegMode(tesseract::PSM_AUTO);  // Segmentation on auto 
  api.Init("C:/Users/1004/C++/tesseract/tessdata", "eng");   // path = parent directory of tessdata 
  // Open input image using OpenCV
  std::vector<cv::Rect> letterBBoxes1 = detectLetters(src);

  for (int i = 0; i < letterBBoxes1.size(); i++) {
    cv::Mat cropImg = src(letterBBoxes1[i]);
    cv::rectangle(src, letterBBoxes1[i], cv::Scalar(0, 255, 0, 255), 3, 8, 0);
    //cv::threshold(cropImg, cropImg, 0, 255, cv::THRESH_BINARY);
    cv::imwrite("C:/Users/1004/C++/crop.png", cropImg);
    //api.SetImage(cropImg.data, cropImg.cols, cropImg.rows, 4, 4 * cropImg.cols);
    api.SetImage(cropImg.data, cropImg.cols, cropImg.rows, 4, 4 * cropImg.cols);
    // Set image data
    //api.SetImage(image);       // Run the OCR 
    char* textOutput = new char[512];
    textOutput = api.GetUTF8Text();     // Get the text 

    // print recognized text
    cout << textOutput << endl; // Destroy used object and release memory ocr->End();
  }

  cv::imwrite("C:/Users/1004/C++/searchedImage.png", src);
  */

}

std::vector<TextInfo> findOutTextInfos(cv::Mat img) {
  char *outText;
  HWND hwndDesktop = GetDesktopWindow();
  //Mat src = hwnd2mat(hwndDesktop);
  Mat src = imread("C:\\Users\\1004\\Pictures\\Dr_DeernD.png");
  int count = 2;
  if (count <= 0 || count > 255) {
    printf("The color count needs to be between 1-255. You picked: %d\n", count);
    return std::vector<TextInfo>();
  }

  std::vector<cv::Rect> letterBBoxes1 = detectLetters(src);
  tesseract::TessBaseAPI api;
  if (api.Init("C:/Users/1004/C++/tesseract/tessdata", "eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }
  api.SetPageSegMode(tesseract::PSM_AUTO);

  //  std::sort(letterBBoxes1.begin(), letterBBoxes1.end(), rectComp);

  std::vector<TextInfo> textInfos;
  letterBBoxes1 = reorganizeText(letterBBoxes1);

  for (auto it = letterBBoxes1.rbegin(); it != letterBBoxes1.rend(); ++it) {
    cv::Mat cropImg = src(*it);
    //cv::rectangle(src, letterBBoxes1[i], cv::Scalar(0, 255, 0, 255), 3, 8, 0);
    resize(cropImg, cropImg, cv::Size(cropImg.cols / 2.5, cropImg.rows / 2.5));//resize image
    cv::imwrite("C:/Users/1004/C++/crop.png", cropImg);
    api.SetImage(cropImg.data, cropImg.cols, cropImg.rows, 4, 4 * cropImg.cols);
    char* textOutput;
    textOutput = api.GetUTF8Text();     // Get the text 

    cout << textOutput << endl; // Destroy used object and release memory ocr->End();
    std::vector<cv::Vec3b> vec = find_dominant_colors(cropImg, 2);
    TextInfo tInfo;

    tInfo.backgroundColor = static_cast<int>(vec[0][0]);
    tInfo.fontColor = static_cast<int>(vec[1][0]);
    tInfo.text = std::string(textOutput);
    tInfo.rect = *it;
    textInfos.push_back(tInfo);
  }

  for (int i = 0; i < textInfos.size(); i++) {
    cv::rectangle(src, textInfos[i].rect, cv::Scalar(0, 255, 0, 255), 3, 8, 0);
  }
  cv::imwrite("C:/Users/1004/C++/searchedImage.png", src);
}

int main(int argc, char* argv[])
{
  ocrTest();
  //return 0;
  
  return EXIT_SUCCESS;
}