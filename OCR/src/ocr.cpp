#include "ocr.h"
//#define DEBUG_LEVEL2

OCR* OCR::g_inst = nullptr;

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

#ifdef DEBUG_LEVEL2
  cv::Mat quantized = get_quantized_image(classes, root);
  cv::Mat viewable = get_viewable_image(classes);
  cv::Mat dom = get_dominant_palette(colors);

  cv::imwrite("./classification.png", viewable);
  cv::imwrite("./quantized.png", quantized);
  cv::imwrite("./palette.png", dom);
#endif

  return colors;
}
std::vector<cv::Rect>
detectLetters(cv::Mat img) {
  std::vector<cv::Rect> boundRect;
  cv::Mat img_gray;
  cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  cv::Mat img_sobelX;
  cv::Mat img_sobelY;
  cv::Sobel(img_gray, img_sobelX, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Sobel(img_gray, img_sobelY, CV_8U, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);
  cv::Mat img_sobel = img_sobelX + img_sobelY;
  cv::Mat img_threshold;
  cv::threshold(img_sobel, img_threshold, 0, 255, cv::THRESH_OTSU + cv::THRESH_BINARY);
  cv::morphologyEx(img_threshold, img_threshold, cv::MORPH_CLOSE, getStructuringElement(cv::MORPH_RECT, cv::Size(3, 1))); //Does the trick
  std::vector< std::vector< cv::Point> > contours;
  cv::findContours(img_threshold, contours, 0, 1);
  std::vector<std::vector<cv::Point> > contours_poly(contours.size());
#ifdef DEBUG_LEVEL2
  cv::imwrite("./detect-img_gray.png", img_gray);
  cv::imwrite("./detect-img_sobel.png", img_sobel);
  cv::imwrite("./detect-img_threshold.png", img_threshold);
#endif
  for (int i = 0; i < contours.size(); i++) {
#ifdef DEBUG_LEVEL2
    cv::Mat dbgImg = img.clone();
    cv::polylines(dbgImg, contours[i], false, cv::Scalar(0., 255, 0., 255), 1, 8, 0);
    cv::imwrite("./detect-contours.png", dbgImg);
#endif
    if (contours[i].size() > 40) {
      cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
      cv::Rect appRect(boundingRect(cv::Mat(contours_poly[i])));
      if (appRect.width > appRect.height &&
          appRect.height < 100)
        boundRect.push_back(appRect);
    }
  }
  return boundRect;
}

cv::Rect rectMerge(cv::Rect A, cv::Rect B) {
  int left = std::min(A.x, B.x);
  int top = std::min(A.y, B.y);
  int right = std::max(A.x + A.width, B.x + B.width);
  int bottom = std::max(A.y + A.height, B.y + B.height);
  return cv::Rect(left, top, right - left, bottom - top);
}

std::vector<cv::Rect> reorganizeText(std::vector<cv::Rect> src) {
  std::vector<cv::Rect> dst;

  for (auto st = src.begin(); st != src.end(); st++) {
    int pv = dst.size();
    for (size_t i = 0; i < dst.size(); i++) {
      cv::Rect dt = dst[i];
      double spacing = (*st).height * 0.4;
      if ((*st).y - spacing < dt.y &&
          (*st).y + spacing > dt.y &&
          (*st).x < dt.x) {

        if ((*st).br().x + spacing > dt.x) {
          dst[i] = rectMerge(dt, *st);
          pv = -1;
        } else {
          pv = i;
        }
        break;
      }
    }

    if (pv < 0) continue;
    (*st).x -= 1;
    (*st).y -= 1;
    (*st).width += 2;
    (*st).height += 2;

    dst.insert(dst.begin() + pv, *st);
  }

  return dst;
}

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

#define W2CRect(rect) cv::Rect(rect.left, rect.top, RctW(rect), RctH(rect))
#define C2WRect(rect) RECT{(rect).x, (rect).y, (rect).br().x, (rect).br().y}

#define VEC2RGB(vec) vec[2] << 16 | vec[1] << 8 | vec[0]

std::vector<TextInfo>
OCR::findOutTextInfos(Image imgParam) {
  cv::Mat img(cv::Size(imgParam.width, imgParam.height), CV_8UC4, imgParam.samples);
  std::vector<TextInfo> textInfos;

  std::vector<cv::Rect> detectedLetterBoxes = detectLetters(img);
  std::vector<cv::Rect> letterBBoxes = reorganizeText(detectedLetterBoxes);
  for (auto it = letterBBoxes.rbegin(); it != letterBBoxes.rend(); ++it) {
    if (m_cancelFlag) {
      m_cancelFlag = false;
      return std::vector<TextInfo>();
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
    std::vector<cv::Vec3b> vec = find_dominant_colors(cropImg, 2);
    TextInfo tInfo;

    tInfo.backgroundColor = static_cast<int>(VEC2RGB(vec[1]));
    tInfo.fontColor = static_cast<int>(VEC2RGB(vec[0]));
    tInfo.text = std::string(textOutput);
    tInfo.rect = C2WRect(*it);
    textInfos.push_back(tInfo);
  }

#ifdef DEBUG_LEVEL2
  for (int i = 0; i < textInfos.size(); i++) {
    cv::rectangle(*img, W2CRect(textInfos[i].rect), cv::Scalar(0, 255, 0, 255), 3, 8, 0);
  }
  cv::imwrite("./searchedImage.png", *img);
#endif
  return textInfos;
}