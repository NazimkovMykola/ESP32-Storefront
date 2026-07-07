#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "index.h"
#include "fonts.h" // Підключаємо наш файл шрифтів

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8 
#define CS_PIN    5

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char* ssid = "ESP32-Matrix";
const char* password = "12345678";

AsyncWebServer server(80);

// Буфери для відображення на екрані (у форматі CP1251)
char msgTop[64] = "ПРИВIТ";
char msgBtm[256] = "як справи, друже? текст українською!";

bool updateTop = false;
bool updateBtm = false;

// === ФУНКЦІЯ КОНВЕРТАЦІЇ UTF-8 В CP1251 ===
String utf8_to_cp1251(const String &utf8) {
  String cp1251 = "";
  int i = 0;
  while (i < utf8.length()) {
    uint8_t c = utf8[i];
    if (c < 0x80) {
      cp1251 += (char)c;
      i++;
    } else if (c == 0xD0 || c == 0xD1) { // Кириличні блоки
      uint8_t next_c = utf8[i + 1];
      if (c == 0xD0) {
        if (next_c >= 0x90 && next_c <= 0xBF) cp1251 += (char)(next_c + 0x30); // А-П
        else if (next_c == 0x81) cp1251 += (char)168; // Ё -> Є (тимчасово)
        else if (next_c == 0x86) cp1251 += (char)178; // І
        else if (next_c == 0x87) cp1251 += (char)175; // Ї
        else if (next_c == 0x84) cp1251 += (char)170; // Є
      } else if (c == 0xD1) {
        if (next_c >= 0x80 && next_c <= 0x8F) cp1251 += (char)(next_c + 0x70); // р-я
        else if (next_c == 0x91) cp1251 += (char)184; // ё -> є
        else if (next_c == 0x96) cp1251 += (char)186; // і
        else if (next_c == 0x97) cp1251 += (char)187; // ї
        else if (next_c == 0x94) cp1251 += (char)186; // є
      }
      i += 2;
    } else {
      i++; // Ігноруємо інші спецсимволи
    }
  }
  return cp1251;
}

String processor(const String& var) {
  // На сторінці показуємо текст у нормальному UTF-8 (але ми тут зберігаємо вже конвертований, 
  // тому для простоти форми поки лишимо дефолтні змінні або латиницю)
  if (var == "TOP_VAL") return "ПРИВIТ";
  if (var == "BTM_VAL") return "як справи, друже?";
  return String();
}

void setup() {
  Serial.begin(115200);

  P.begin(2); 
  P.setZone(0, 0, 3); // Нижня
  P.setZone(1, 4, 7); // Верхня
  
  // !!! ПІДКЛЮЧАЄМО НАШ КИРИЛИЧНИЙ ШРИФТ ДЛЯ ОБОХ ЗОН !!!
  P.setFont(0, cyrillicFont);
  P.setFont(1, cyrillicFont);

  P.setIntensity(0, 3);
  P.setIntensity(1, 3);

  // Конвертуємо початковий текст
  String startTop = utf8_to_cp1251("ПРИВIТ");
  String startBtm = utf8_to_cp1251("як справи, друже? все працює!");
  
  snprintf(msgTop, sizeof(msgTop), "%s", startTop.c_str());
  snprintf(msgBtm, sizeof(msgBtm), "%s", startBtm.c_str());

  P.displayZoneText(1, msgTop, PA_CENTER, 0, 0, PA_PRINT, PA_PRINT);
  P.displayZoneText(0, msgBtm, PA_LEFT, 60, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  WiFi.softAP(ssid, password);
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("top_text")) {
      String t = request->getParam("top_text")->value();
      String converted = utf8_to_cp1251(t); // Перекодовуємо при отриманні
      snprintf(msgTop, sizeof(msgTop), "%s", converted.c_str());
      updateTop = true;
    }
    if (request->hasParam("btm_text")) {
      String b = request->getParam("btm_text")->value();
      String converted = utf8_to_cp1251(b); // Перекодовуємо при отриманні
      snprintf(msgBtm, sizeof(msgBtm), "%s", converted.c_str());
      updateBtm = true;
    }
    request->redirect("/");
  });

  server.begin();
}

void loop() {
  if (P.displayAnimate()) {
    if (updateTop) {
      P.displayReset(1);
      updateTop = false;
    }
    if (updateBtm) {
      P.displayReset(0);
      updateBtm = false;
    }
    P.displayReset(0);
  }
}