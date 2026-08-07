#pragma once

#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/MomentaryButton.h>

/**
 * Minimal OLED UI for Heltec mcRPC button firmware.
 * PRG (PIN_USER_BTN) wakes the screen; status refreshes while on.
 *
 * Call SSD1306Display::begin() in setup() before McRpcUi::begin() —
 * DisplayDriver has no begin().
 */
class McRpcUi {
  DisplayDriver* _display;
  MomentaryButton* _btn1;
  MomentaryButton* _btn2;
  unsigned long _next_refresh = 0;
  unsigned long _auto_off = 0;
  const char* _name = "";
  const char* _profile = "";
  const char* _channel = "";
  bool _btn1_down = false;
  bool _btn2_down = false;
  bool _ok = false;

  static constexpr unsigned long AUTO_OFF_MS = 30000;
  static constexpr unsigned long REFRESH_MS = 500;

public:
  McRpcUi(DisplayDriver& display, MomentaryButton* btn1, MomentaryButton* btn2 = nullptr)
    : _display(&display), _btn1(btn1), _btn2(btn2) {}

  void begin(const char* name, const char* profile, const char* channel) {
    _name = name ? name : "";
    _profile = profile ? profile : "";
    _channel = channel ? channel : "";
    // Buttons are begun in setup() unconditionally — do not gate on OLED.
    _display->turnOn();
    _ok = true;
    _auto_off = millis() + AUTO_OFF_MS;
    render();
  }

  void setButtonState(bool btn1_down, bool btn2_down) {
    _btn1_down = btn1_down;
    _btn2_down = btn2_down;
  }

  void poke() {
    if (!_ok) return;
    if (!_display->isOn()) {
      _display->turnOn();
    }
    _auto_off = millis() + AUTO_OFF_MS;
  }

  void loop() {
    if (!_ok || !_display->isOn()) return;
    if (millis() >= _next_refresh) {
      render();
      _next_refresh = millis() + REFRESH_MS;
    }
    if (millis() > _auto_off) {
      _display->turnOff();
    }
  }

private:
  void render() {
    char line[48];
    _display->startFrame();
    _display->setTextSize(1);
    _display->setColor(DisplayDriver::LIGHT);

    _display->setCursor(0, 0);
    _display->print("Heltec_v3a");

    _display->setCursor(0, 14);
    snprintf(line, sizeof(line), "name %s", _name);
    _display->print(line);

    _display->setCursor(0, 26);
    snprintf(line, sizeof(line), "tag  %s", _profile);
    _display->print(line);

    _display->setCursor(0, 38);
    snprintf(line, sizeof(line), "ch   #%s", _channel);
    _display->print(line);

    _display->setCursor(0, 52);
    if (_btn2) {
      snprintf(line, sizeof(line), "b1:%s  b2:%s",
               _btn1_down ? "DOWN" : "up",
               _btn2_down ? "DOWN" : "up");
    } else {
      snprintf(line, sizeof(line), "btn1 %s", _btn1_down ? "DOWN" : "up");
    }
    _display->print(line);
    _display->endFrame();
  }
};
