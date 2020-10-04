#include "messageHandler.h"

#include "common_util.h"

#include "OCR/src/ocr.h"
#include "TextOverlay/src/overlay.h"

void 
MessageHandler::handle() {
  while (true) {
    if (g_msg.empty()) {
      ::Sleep(100);
      continue;
    }

    switch (g_msg.front()) {
    case MESSAGE_TO_UPDATE_CANVAS_WINDOW: TextOverlay::instnace()->updateCanvasWindow(); break;
    case MESSAGE_OCR_CANCEL: clearTextInfo(); OCR::instnace()->cancel(); break;
    }
    g_msg.pop();
  }
}