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
  return RectWrapper(rect).toCVRect();
}

RECT toWinRect(cv::Rect rect) {
  return RectWrapper(rect).toWinRect();
}


bool contains(RectWrapper super, RectWrapper sub) {
  bool contains = (super.l() < sub.l() && super.r() > sub.r() &&
                    super.t() < sub.t() && super.b() > sub.b());
  return contains;
}

bool intersected(RectWrapper lhs, RectWrapper rhs) {

  bool intersected = (lhs.l() <= rhs.l() && lhs.r() >= rhs.l()
                  || rhs.l() <= lhs.l() && rhs.r() >= lhs.l())
                  && (lhs.t() <= rhs.t() && lhs.b() >= rhs.t()
                  || rhs.t() <= lhs.t() && rhs.b() >= lhs.t());
  return intersected;
}

RectWrapper intersect(RectWrapper lhs, RectWrapper rhs) {
  RECT ret;
  ret.left =   max(lhs.l(), rhs.l());
  ret.top =    max(lhs.t(), rhs.t());
  ret.right =  min(lhs.r(), rhs.r());
  ret.bottom = min(lhs.b(), rhs.b());
  return (ret.right < ret.left || ret.bottom < ret.top) ? RECT{ 0, } : ret;
}

bool isEmpty(RectWrapper rect) {
  return rect.w() * rect.h() == 0;
}

std::string
toString(RectWrapper rect) {
  return std::to_string(rect.l()) + ", "
       + std::to_string(rect.t()) + ", "
       + std::to_string(rect.r()) + ", "
       + std::to_string(rect.b());
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
int RectWrapper::x() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.left;
  case CV_RECT:  return m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::y() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.top;
  case CV_RECT:  return m_cvRect.y;
  default: return -1;
  }
}

int RectWrapper::w() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.right - m_winRect.left;
  case CV_RECT:  return m_cvRect.width;
  default: return -1;
  }
}

int RectWrapper::h() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.bottom - m_winRect.top;
  case CV_RECT:  return m_cvRect.height;
  default: return -1;
  }
}

int RectWrapper::l() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.left;
  case CV_RECT:  return m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::t() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.top;
  case CV_RECT:  return m_cvRect.y;
  default: return -1;
  }
}

int RectWrapper::r() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.right;
  case CV_RECT:  return m_cvRect.width + m_cvRect.x;
  default: return -1;
  }
}

int RectWrapper::b() const {
  switch (m_type) {
  case WIN_RECT: return m_winRect.bottom;
  case CV_RECT:  return m_cvRect.height + m_cvRect.y;
  default: return -1;
  }
}

RECT RectWrapper::toWinRect() const {
  return RECT{ l(), t(), r(), b() };
}
cv::Rect RectWrapper::toCVRect() const {
  return cv::Rect(x(), y(), w(), h());
}
