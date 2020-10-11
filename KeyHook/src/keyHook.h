#pragma once
#include <functional>
#include <map>
#include <thread>

class KeySeq {
public:
  KeySeq(bool ctrl, bool shift, bool alt, int vkCode)
  : ctrl(ctrl),
    shift(shift),
    alt(alt),
    vkCode(vkCode) {
  }

  bool operator<(const KeySeq& other) const {
    unsigned int lhs = (this->ctrl ? 0x01 : 0) + (this->shift ? 0x02 : 0) + (this->alt ? 0x04 : 0) + (this->vkCode << 8);
    unsigned int rhs = (other.ctrl ? 0x01 : 0) + (other.shift ? 0x02 : 0) + (other.alt ? 0x04 : 0) + (other.vkCode << 8);

    return lhs > rhs;
  }

private:
  bool ctrl;
  bool shift;
  bool alt;
  int vkCode;
};

typedef std::function<void()> generalFunc;
class KeyHook {
private:
  static KeyHook* g_inst;
  KeyHook() {}
  ~KeyHook() {}

public:
  static KeyHook* instnace() {
    if (g_inst == nullptr) {
      g_inst = new KeyHook();
    }
    return g_inst;
  }

  void startHook();
  void terminateHook();
  void registryFunction(KeySeq seq, generalFunc func) { funcMap[seq] = func; }
  generalFunc findFunction(KeySeq seq) { return funcMap[seq]; }

private:
  std::map<KeySeq, generalFunc> funcMap;
  std::thread m_hookThread;
};