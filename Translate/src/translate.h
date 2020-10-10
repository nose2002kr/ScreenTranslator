#pragma once
#include <string>
#include <map>

class Translate {
public:
  static Translate& instance() {
    static Translate instance;
    return instance;
  }
  ~Translate();
  std::string translate(std::string src);
  void pushHistory(std::string key, std::string value) {
    m_transHistory[key] = value;
  }
private:
  Translate();
  std::map<std::string, std::string> m_transHistory;
};