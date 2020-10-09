#include "test/test.h"

#include "common_util.h"

#include "Translate/src/translate.h"
void test::translate() {

  //std::string src = "Hello world!";
  std::string src = "HelloWorld";
  std::string translated = Translate().translate(src);
  writeLog(DEBUG, "Translate test:" + src + " - " + translated);
}
