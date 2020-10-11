#include "image_util.h"

#include "rect_util.h"
#include "debug_util.h"

namespace imageUtil {
typedef struct t_color_node {
  cv::Mat       mean;       // The mean of this node
  cv::Mat       cov;
  uchar         classid;    // The class ID

  t_color_node  *left;
  t_color_node  *right;
  ~t_color_node() {
    if (left) delete left;
    if (right) delete right;
  }

} t_color_node;

static cv::Mat getDominantPalette(std::vector<cv::Vec3b> colors) {
  const int tile_size = 64;
  cv::Mat ret = cv::Mat(tile_size, (int) (tile_size*colors.size()), CV_8UC3, cv::Scalar(0));

  for (int i = 0; i < (int) colors.size(); i++) {
    cv::Rect rect(i * tile_size, 0, tile_size, tile_size);
    cv::rectangle(ret, rect, cv::Scalar(colors[i][0], colors[i][1], colors[i][2]), cv::FILLED);
  }

  return ret;
}

static std::vector<t_color_node*> getLeaves(t_color_node *root) {
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

static std::vector<cv::Vec3b> getDominantColors(t_color_node *root) {
  std::vector<t_color_node*> leaves = getLeaves(root);
  std::vector<cv::Vec3b> ret;

  for (size_t i = 0; i < leaves.size(); i++) {
    cv::Mat mean = leaves[i]->mean;
    ret.push_back(cv::Vec3b((uchar)(mean.at<double>(0) * 255.0f),
      (uchar)(mean.at<double>(1) * 255.0f),
      (uchar)(mean.at<double>(2) * 255.0f)));
  }

  return ret;
}

static int getNextClassid(t_color_node *root) {
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

static void getClassMeanCov(const cv::Mat &img, const cv::Mat &classes, t_color_node *node) {
  const int width = img.cols;
  const int height = img.rows;
  const uchar classid = node->classid;

  cv::Mat mean = cv::Mat(3, 1, CV_64FC1, cv::Scalar(0));
  cv::Mat cov = cv::Mat(3, 3, CV_64FC1, cv::Scalar(0));

  // We start out with the average color
  double pixcount = 0;
  for (int y = 0; y < height; y++) {
    const cv::Vec3b* ptr = img.ptr<cv::Vec3b>(y);
    const uchar* ptrClass = classes.ptr<uchar>(y);
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

static void partitionClass(const cv::Mat &img, cv::Mat classes, uchar nextid, t_color_node *node) {
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
    const cv::Vec3b* ptr = img.ptr<cv::Vec3b>(y);
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

static cv::Mat getQuantizedImage(const cv::Mat &classes, t_color_node *root) {
  std::vector<t_color_node*> leaves = getLeaves(root);

  const int height = classes.rows;
  const int width = classes.cols;
  cv::Mat ret(height, width, CV_8UC3, cv::Scalar(0));

  for (int y = 0; y < height; y++) {
    const uchar *ptrClass = classes.ptr<uchar>(y);
    cv::Vec3b *ptr = ret.ptr<cv::Vec3b>(y);
    for (int x = 0; x < width; x++) {
      uchar pixel_class = ptrClass[x];
      for (size_t i = 0; i < leaves.size(); i++) {
        if (leaves[i]->classid == pixel_class) {
          ptr[x] = cv::Vec3b((uchar)(leaves[i]->mean.at<double>(0) * 255),
            (uchar)(leaves[i]->mean.at<double>(1) * 255),
            (uchar)(leaves[i]->mean.at<double>(2) * 255));
        }
      }
    }
  }

  return ret;
}

static cv::Mat getViewableImage(const cv::Mat &classes) {
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
    const  uchar *ptrClass = classes.ptr<uchar>(y);
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

static t_color_node* getMaxEigenvalueNode(t_color_node *current) {
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

std::vector<cv::Vec3b> findDominantColors(const cv::Mat &img, int count) {
  const int width = img.cols;
  const int height = img.rows;

  cv::Mat img3C = img;
  if (img.channels() == 4) {
    cv::cvtColor(img, img3C, cv::COLOR_RGBA2RGB);
  } 
  cv::Mat classes = cv::Mat(height, width, CV_8UC1, cv::Scalar(1));
  t_color_node *root = new t_color_node();

  root->classid = 1;
  root->left = NULL;
  root->right = NULL;

  t_color_node *next = root;
  getClassMeanCov(img3C, classes, root);
  for (int i = 0; i < count - 1; i++) {
    next = getMaxEigenvalueNode(root);
    partitionClass(img3C, classes, getNextClassid(root), next);
    getClassMeanCov(img3C, classes, next->left);
    getClassMeanCov(img3C, classes, next->right);
  }

  std::vector<cv::Vec3b> colors = getDominantColors(root);
  
#ifdef DEBUG_LEVEL3
  cv::Mat quantized = getQuantizedImage(classes, root);
  cv::Mat viewable = getViewableImage(classes);
  cv::Mat dom = getDominantPalette(colors);

  cv::imwrite(DEBUG_LEVEL3"classification.png", viewable);
  cv::imwrite(DEBUG_LEVEL3"quantized.png", quantized);
  cv::imwrite(DEBUG_LEVEL3"palette.png", dom);
#endif
  delete root;

  return colors;
}

static inline
bool isSameLine(cv::Rect prev, cv::Rect cur) {
  int h = min(prev.height, cur.height);
  return prev.y - h * 0.8f < cur.y
      && prev.y + h * 0.8f > cur.y;
}

static inline
bool isForward(cv::Rect prev, cv::Rect cur) {
  return cur.x < prev.x;
}

static inline
int findNearstRect(const std::vector<cv::Rect>& boundRect, cv::Rect cur, bool onlyForward = true, int* distance = nullptr) {
  int foundIndex = -1;
  int xDistance = INT_MAX;
  for (int j = (int)boundRect.size() - 1; j >= 0; j--) {
    if (!isSameLine(boundRect[j], cur)) {
      break;
    }
    if ((!onlyForward) || (onlyForward && isForward(boundRect[j], cur))) {
      if (abs(xDistance) > abs(cur.x - (boundRect[j].x + boundRect[j].width))) {
        xDistance = cur.x - (boundRect[j].x + +boundRect[j].width);
        foundIndex = j;
        if (distance) *distance = xDistance;
      }
    }
  }
  return foundIndex;
}

static inline
int findForwardRect(const std::vector<cv::Rect>& boundRect, cv::Rect cur, int* distance = nullptr) {
  return findNearstRect(boundRect, cur, true, distance);
}

static inline
int findJustAroundRect(const std::vector<cv::Rect> &boundRect, cv::Rect cur) {
  int distance = INT_MAX;
  int foundIndex = findNearstRect(boundRect, cur, false, &distance);
  return abs(distance) < cur.height * 0.8f ? (distance >= 0 ? foundIndex + 1 : foundIndex)
    : -1;
}

std::vector<cv::Rect> findContourBounds(const cv::Mat &binaryImage, size_t contourComplexity) {
  std::vector<cv::Rect> boundRect;

  std::vector< std::vector< cv::Point> > contours;
  cv::findContours(binaryImage, contours, 0, 1);
  std::vector<std::vector<cv::Point> > contours_poly(contours.size());
#ifdef DEBUG_LEVEL2
  cv::imwrite(DEBUG_LEVEL2"detect-img_gray.png", binaryImage);
#endif
  // it is should be sort by position. (in y order, then in x order.)
  for (int i = (int) contours.size() - 1; i >= 0; i--) {
#ifdef DEBUG_LEVEL2
    cv::Mat dbgImg = binaryImage.clone();
    cvtColor(dbgImg, dbgImg, cv::COLOR_GRAY2BGR);
    cv::polylines(dbgImg, contours[i], false, cv::Scalar(0., 255, 0., 255), 1, 8, 0);
    cv::imwrite(DEBUG_LEVEL2"detect-contours.png", dbgImg);
#endif
    if (contours[i].size() > contourComplexity) {
      cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
      cv::Rect appRect(boundingRect(cv::Mat(contours_poly[i])));
      
      int forwardRectIndex = findForwardRect(boundRect, appRect);
      if (forwardRectIndex == -1) {
        boundRect.push_back(appRect);
      } else {
        boundRect.insert(boundRect.begin() + forwardRectIndex, appRect);
      }
    } else {
      cv::approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 3, true);
      cv::Rect appRect(boundingRect(cv::Mat(contours_poly[i])));
      int forwardRectIndex = findJustAroundRect(boundRect, appRect);
      if (forwardRectIndex != -1) {
        boundRect.insert(boundRect.begin() + forwardRectIndex, appRect);
      }
    }
  }
  return boundRect;
}

static inline cv::Rect inflate(cv::Rect rect, float x, float y) {
  return cv::Rect(
    (int) (rect.x - x),
    (int) (rect.y - y),
    (int) (rect.width + x * 2),
    (int) (rect.height + y * 2));
}

cv::Rect normalize(const cv::Mat &img, cv::Rect rect) {
  int x = rect.x < 0 ? 0 : rect.x;
  int y = rect.y < 0 ? 0 : rect.y;
  int w = rect.width + x > img.cols ? img.cols - x : rect.width;
  int h = rect.height + y > img.rows ? img.rows - y : rect.height;
  return cv::Rect(x, y, w, h);
}

static inline 
cv::Mat sobelToBinrayImage(const cv::Mat &img) { // for extract linked text contour.
  cv::Mat img_gray;
  if (img.type() != CV_8U) {
    cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  } else {
    img_gray = img;
  }
  cv::Mat img_sobelX;
  cv::Mat img_sobelY;
  cv::Sobel(img_gray, img_sobelX, CV_8U, 1, 0, 3, 2., 1, cv::BORDER_DEFAULT);
  cv::Sobel(img_gray, img_sobelY, CV_8U, 0, 1, 3, 2., 1, cv::BORDER_DEFAULT);
  cv::Mat img_sobel = img_sobelX + img_sobelY;
  cv::Mat img_threshold;
  cv::threshold(img_sobel, img_threshold, 0, 255, cv::THRESH_OTSU + cv::THRESH_BINARY);
#ifdef DEBUG_LEVEL3
  cv::imwrite(DEBUG_LEVEL3"sobel_src.png", img_gray);
  cv::imwrite(DEBUG_LEVEL3"sobelX.png", img_sobelX);
  cv::imwrite(DEBUG_LEVEL3"sobelY.png", img_sobelY);
  cv::imwrite(DEBUG_LEVEL3"sobel.png", img_sobel);
  cv::imwrite(DEBUG_LEVEL3"threshold.png", img_threshold);
#endif
  cv::morphologyEx(img_threshold, img_threshold, cv::MORPH_CLOSE, getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1))); //Does the trick
  return img_threshold;
}

std::vector<cv::Rect>
detectLetters(const cv::Mat &img) {
  auto letters = findContourBounds(sobelToBinrayImage(img), 40);
  letters = imageUtil::reorganizeText(letters);
  for (auto it = letters.begin(); it != letters.end(); ) {
    cv::Rect letter = *it;
    if (letter.width < letter.height
      || letter.height > 100
      || letter.height < 5
      || letter.width < 2) {
      it = letters.erase(it);
    } else {
      ++it;
    }
  }
  return letters;
}

std::vector<cv::Rect> reorganizeText(const std::vector<cv::Rect> &src) {
  std::vector<cv::Rect> dst;

  for (auto el : src) {
    //cv::Rect rect = inflate(el, el.width * 0.1, el. height * 0.1);
    cv::Rect rect = el;
    if (rect.height > 100) { continue; }

    int distance = INT_MAX;
    int foundIndex = findNearstRect(dst, rect, false, &distance);
    double spacing = rect.height * 0.8;
    if (foundIndex != -1 && abs(distance) < spacing) {
      dst[foundIndex] = rectUtil::mergeRect(dst[foundIndex], rect).toCVRect();
    } else {
      dst.push_back(rect);
    }
  }

  return dst;
}

int Vec2Rgb(cv::Vec3b vec) {
  return vec[2] << 16 | vec[1] << 8 | vec[0];
}

cv::Mat toMat(Image* imgParam) {
  return cv::Mat(cv::Size(imgParam->width, imgParam->height), CV_8UC4, imgParam->samples);
}

std::vector<cv::Rect>
findDiffRange(const cv::Mat &first, const cv::Mat &second) {
  std::vector<cv::Rect> diffRange;
  if (second.empty()) {
    return diffRange;
  }

  cv::Mat prevImageG, imageG;
  cvtColor(first, prevImageG, cv::COLOR_BGR2GRAY);
  cvtColor(second, imageG, cv::COLOR_BGR2GRAY);
  cv::Mat diff = imageG ^ prevImageG;
  if (!diff.empty()) {
    diffRange = imageUtil::findContourBounds(sobelToBinrayImage(diff), 4);
  }
  return diffRange;
}

} // namespace imageUtil