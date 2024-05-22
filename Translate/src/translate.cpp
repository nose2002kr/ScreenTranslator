#include "translate.h"

#include <stdio.h>
#include <memory>
#include <sstream>
#include <iomanip>

#include "libjson.h"
#include "common_util.h"

#define CURL_STATICLIB
#define USE_OPENSSL
#include <curl/curl.h>

#define TRANSLATE_HISTORY_FILE "translation.history"
#define TRANSLATE_HISTORY_DELIMITER " >>>> "

extern "C"
{
    FILE __iob_func[3] = { stdin ,stdout,stderr };
}

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
  size_t newLength = size * nmemb;
  size_t oldLength = s->size();
  try
  {
    s->resize(oldLength + newLength);
  } catch (std::bad_alloc &e)
  {
    //handle memory problem
    return 0;
  }

  std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);
  return size * nmemb;
}
#include <fstream>

Translate::Translate() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  std::ifstream inFile(TRANSLATE_HISTORY_FILE);
  char buf[1024];
  while (!inFile.eof()) {
    inFile.getline(buf, 1024);
    if (strlen(buf) == 0) break;
    std::string src(buf);
    std::string key = src.substr(0, src.find(TRANSLATE_HISTORY_DELIMITER));
    std::string val = src.substr(src.find(TRANSLATE_HISTORY_DELIMITER) + 6);
    m_transHistory[key] = val;
  }
  inFile.close();
}

Translate::~Translate() {
  std::ofstream outFile(TRANSLATE_HISTORY_FILE);
  for (auto elem : m_transHistory) {
    outFile << elem.first << TRANSLATE_HISTORY_DELIMITER << elem.second << "\n";
  }
  outFile.close();
  curl_global_cleanup();
}

void setPapagoNmtAPIHeader(CURL* curl, struct curl_slist* &header, std::string param) {
  std::string postdata = std::string("source=en&target=ko&text=") + param;
  std::string headerClientId = std::string("X-Naver-Client-Id: ") + std::getenv("TRANSLATE_PAPAGO_API_ID");
  std::string headerClientSecret = std::string("X-Naver-Client-Secret: ") + std::getenv("TRANSLATE_PAPAGO_API_SECRET");
  curl_easy_setopt(curl, CURLOPT_URL, "https://openapi.naver.com/v1/papago/n2mt");
  header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
  header = curl_slist_append(header, headerClientId.c_str());
  header = curl_slist_append(header, headerClientSecret.c_str());
  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
}

void setGoogleTranslationAPIHeader(CURL* curl, struct curl_slist*& header, std::string param) {
  // https://translation.googleapis.com/language/translate/v2?q={0}&target={1}&key={2}'.format(text, translation_language, api_key)
  std::string baseurl = "https://translation.googleapis.com/language/translate/v2";
  std::string key = std::getenv("GOOGLE_TRANSLATE_API_KEY");
  std::string url = baseurl;
  url += "?key=" + key;
  url += "&q=" + param;
  url += "&target=ko";
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

std::string 
Translate::translate(std::string src) {
  if (!m_transHistory[src].empty()) {
    return m_transHistory[src];
  }

  CURL *curl = curl_easy_init();
  CURLcode res;
  if (curl) {
    std::string s;
    char error_buffer[CURL_ERROR_SIZE] = {};
    src = ANSIToUTF8(src);
    std::string encodedSrc = curl_easy_escape(curl, src.c_str(), src.length());

    struct curl_slist* header = nullptr;
    //setPapagoNmtAPIHeader(curl, header, encodedSrc);
    setGoogleTranslationAPIHeader(curl, header, encodedSrc);
   
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);


    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (header) {
      curl_slist_free_all(header);
    }
    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
    
    JSONNode node = libjson::parse(s);
    if (node.find("error") != node.end()) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", s);
      return "";
    } else {
      //std::string translatedText = node["message"]["result"]["translatedText"].as_string();
      std::string translatedText = node["data"]["translations"][0]["translatedText"].as_string();
      pushHistory(src, translatedText);
      return translatedText;
    }
  }
  
  return "";
}
