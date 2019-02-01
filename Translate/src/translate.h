#pragma once
#include <string>
#include <map>

class Translate {
public:
  Translate();
  ~Translate();
  std::string translate(std::string src);
  void pushHistory(std::string key, std::string value) {
    m_transHistory[key] = value;
  }
private:
  std::map<std::string, std::string> m_transHistory;
};