#include "rect_util.h"

namespace rectUtil {

RectWrapper mergeRect(RectWrapper lhs, RectWrapper rhs) {
  int l = min(lhs.l(), rhs.l());
  int t = min(lhs.t(), rhs.t());
  int r = max(lhs.r(), rhs.r());
  int b = max(lhs.b(), rhs.b());
  return RECT{ l,t,r,b };
}

cv::Rect toCVRect(RECT rect) {
  return cv::Rect(rect.left, rect.top, RctW(rect), RctH(rect));
}

RECT toWinRect(cv::Rect rect) {
  return RECT{ rect.x, rect.y, rect.br().x, rect.br().y };
}


bool contain(RectWrapper lhs, RectWrapper rhs) {
  bool contained = (lhs.l() < rhs.r() && lhs.r() > rhs.l() &&
                    lhs.t() > rhs.b() && lhs.b() < rhs.t());
  return contained;
}

bool intersect(RectWrapper lhs, RectWrapper rhs) {

  bool intersected = (lhs.l() <= rhs.l() && lhs.r() >= rhs.l()
                  || rhs.l() <= lhs.l() && rhs.r() >= lhs.l())
                  && (lhs.t() <= rhs.t() && lhs.b() >= rhs.t()
                  || rhs.t() <= lhs.t() && rhs.b() >= lhs.t());
  return intersected;
}

bool isEmpty(RectWrapper rect) {
  return rect.w() * rect.h() == 0;
}

} // namespace rectUtil

RectWrapper::RectWrapper(RECT rect) {
  m_winRect = rect;
  m_type = WIN_RECT;
}
RectWrapper::RectWrapper(cv::Rect rect) {
  m_cvRect = rect;
  m_type = CV_RECT;
}
int RectWrapper::x() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.left;
  case CV_RECT:  return m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::y() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.top;
  case CV_RECT:  return m_cvRect.y;
  default: return -1;
  }
}

int RectWrapper::w() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.right - m_winRect.left;
  case CV_RECT:  return m_cvRect.width;
  default: return -1;
  }
}

int RectWrapper::h() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.bottom - m_winRect.top;
  case CV_RECT:  return m_cvRect.height;
  default: return -1;
  }
}

int RectWrapper::l() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.left;
  case CV_RECT:  return m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::t() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.top;
  case CV_RECT:  return m_cvRect.y;
  default: return -1;
  }
}

int RectWrapper::r() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.right;
  case CV_RECT:  return m_cvRect.width + m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::b() {
  switch (m_type) {
  case WIN_RECT: return m_winRect.bottom;
  case CV_RECT:  return m_cvRect.height + m_cvRect.y;
  default: return -1;
  }
}

RECT RectWrapper::toWinRect() {
  return RECT{ l(), t(), r(), b() };
}
cv::Rect RectWrapper::toCVRect() {
  return cv::Rect(x(), y(), w(), h());
}