#include "translate.h"

#include <stdio.h>
#include <memory>
#include <sstream>
#include <iomanip>

#define CURL_STATICLIB
#define USE_OPENSSL
#include <curl/curl.h>

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

Translate::Translate() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

Translate::~Translate() {
  curl_global_cleanup();
}

std::string url_encode(const std::string &value) {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
    std::string::value_type c = (*i);

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

std::string 
Translate::translate(std::string src) {
  CURL *curl = curl_easy_init();
  CURLcode res;
  std::string s;
  if (curl) {
    char error_buffer[CURL_ERROR_SIZE] = {};

    curl_easy_setopt(curl, CURLOPT_URL, "https://translation.googleapis.com/language/translate/v2?q=" + url_encode(src) + "&target=ko&format=text&source=en&key=AIzaSyB3MXUyZEcDiw4VQ8tucT3lcvOyr5VbIh0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  
  return s;
}

int main() {
  


  return 0;
}
// 
