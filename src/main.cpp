#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "index.h"
#include "fonts.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN    5

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char* ssid = "ESP32-Matrix";
const char* password = "12345678";
WebServer server(80);

char msgTop[64];
char msgBtm[256];
String webTop = "Привіт";
String webBtm = "Україна";
bool animTop = false; // верхній рядок за замовчуванням статичний
bool animBtm = true;  // нижній рядок за замовчуванням біжить

String convertToFontCode(String text) {
  String out = "";
  size_t i = 0;

  while (i < text.length()) {
    uint8_t b1 = text[i];

    // Звичайний ASCII (латиниця, цифри, пробіл, апостроф) - код співпадає напряму
    if (b1 < 0x80) {
      out += (char)b1;
      i++;
      continue;
    }

    // Кириличний символ у UTF-8 завжди займає 2 байти: 110xxxxx 10xxxxxx
    if ((b1 & 0xE0) == 0xC0 && i + 1 < text.length()) {
      uint8_t b2 = text[i + 1];
      uint16_t codepoint = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
      i += 2;

      uint8_t code = ' '; // якщо символ невідомий - виводимо пробіл

      if (codepoint == 0x0406)      code = 73;   // І -> латинська I (форма ідентична)
      else if (codepoint == 0x0456) code = 105;  // і -> латинська i
      else if (codepoint == 0x0407) code = 208;  // Ї
      else if (codepoint == 0x0457) code = 209;  // ї
      else if (codepoint == 0x0404) code = 210;  // Є
      else if (codepoint == 0x0454) code = 211;  // є
      else if (codepoint == 0x0490) code = 212;  // Ґ
      else if (codepoint == 0x0491) code = 213;  // ґ
      else if (codepoint == 0x0401) code = 192;  // Ё
      else if (codepoint == 0x0451) code = 193;  // ё
      else if (codepoint >= 0x0410 && codepoint <= 0x042F) code = codepoint - 0x0410 + 144; // А-Я
      else if (codepoint >= 0x0430 && codepoint <= 0x043F) code = codepoint - 0x0430 + 176; // а-п
      else if (codepoint >= 0x0440 && codepoint <= 0x044F) code = codepoint - 0x0440 + 128; // р-я

      out += (char)code;
    } else {
      i++; // невідомий/пошкоджений байт - пропускаємо
    }
  }
  return out;
}

void updateDisplay() {
  snprintf(msgTop, sizeof(msgTop), "%s", convertToFontCode(webTop).c_str());
  snprintf(msgBtm, sizeof(msgBtm), "%s", convertToFontCode(webBtm).c_str());

  // ВАЖЛИВО: без повторного displayZoneText новий текст на екран не потрапить

  if (animTop) {
    P.displayZoneText(0, msgTop, PA_LEFT, 60, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } else {
    P.displayZoneText(0, msgTop, PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
  }

  if (animBtm) {
    P.displayZoneText(1, msgBtm, PA_LEFT, 60, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  } else {
    P.displayZoneText(1, msgBtm, PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
  }

  P.displayReset(0);
  P.displayReset(1);
}

void handleRoot() {
  String html = String(index_html);
  html.replace("%TOP_VAL%", webTop);
  html.replace("%BTM_VAL%", webBtm);
  html.replace("%TOP_ANIM_CHECKED%", animTop ? "checked" : "");
  html.replace("%BTM_ANIM_CHECKED%", animBtm ? "checked" : "");
  server.send(200, "text/html", html);
}

void handleGetText() {
  if (server.hasArg("top_text")) webTop = server.arg("top_text");
  if (server.hasArg("btm_text")) webBtm = server.arg("btm_text");

  // Невідмічений чекбокс браузер взагалі не надсилає в запиті,
  // тому наявність параметра в запиті = чекбокс був відмічений
  animTop = server.hasArg("anim_top");
  animBtm = server.hasArg("anim_btm");

  updateDisplay();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  P.begin(2);
  P.setZone(0, 4, 7); // Верхня панель
  P.setZone(1, 0, 3); // Нижня панель

  P.setFont(0, _5bite_rus);
  P.setFont(1, _5bite_rus);

  P.setIntensity(0, 2);
  P.setIntensity(1, 2);

  updateDisplay();

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/get", HTTP_GET, handleGetText);
  server.begin();
}

void loop() {
  P.displayAnimate();
  server.handleClient();

  if (P.getZoneStatus(0) && animTop) {
    P.displayReset(0);
  }
  if (P.getZoneStatus(1) && animBtm) {
    P.displayReset(1);
  }
}