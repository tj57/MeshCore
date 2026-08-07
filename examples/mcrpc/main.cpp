#include "McRpcMesh.h"

#include <helpers/ui/MomentaryButton.h>

#ifdef DISPLAY_CLASS
#include "McRpcUi.h"
#endif

StdRNG fast_rng;
SimpleMeshTables tables;
McRpcMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

#if defined(PIN_USER_BTN) && (PIN_USER_BTN >= 0)
#ifndef DISPLAY_CLASS
// Boards without DISPLAY_CLASS still need a button instance for mcRPC.
static MomentaryButton user_btn(PIN_USER_BTN, 1000, true, true);
#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
static MomentaryButton user_btn2(PIN_USER_BTN2, 1000, true, true);
#endif
#endif
#define MCRPC_HAS_USER_BTN 1
#endif

#ifdef DISPLAY_CLASS
#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
static McRpcUi ui_task(display, &user_btn, &user_btn2);
#else
static McRpcUi ui_task(display, &user_btn, nullptr);
#endif
#endif

void halt() {
  while (1) {
  }
}

static char command[160];

void setup() {
  Serial.begin(115200);
  delay(1000);

  board.begin();

  // Buttons must init even if OLED fails (1.2 regression: begin was gated on display).
#if defined(MCRPC_HAS_USER_BTN)
  user_btn.begin();
#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
  user_btn2.begin();
#endif
#endif

#ifdef DISPLAY_CLASS
  // Init OLED early (VEXT already on after board.begin) — same order as companion/sensor.
  bool oled_ok = display.begin();
  Serial.println(oled_ok ? "OLED ok" : "OLED FAIL");
  if (oled_ok) {
    display.startFrame();
    display.setTextSize(1);
    display.setColor(DisplayDriver::LIGHT);
    display.setCursor(0, 0);
    display.print("Please wait...");
    display.endFrame();
  }
#endif

  if (!radio_init()) {
    halt();
  }

  fast_rng.begin(radio_driver.getRngSeed());

  FILESYSTEM* fs;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  InternalFS.begin();
  fs = &InternalFS;
  IdentityStore store(InternalFS, "");
#elif defined(ESP32)
  SPIFFS.begin(true);
  fs = &SPIFFS;
  IdentityStore store(SPIFFS, "/identity");
#elif defined(RP2040_PLATFORM)
  LittleFS.begin();
  fs = &LittleFS;
  IdentityStore store(LittleFS, "/identity");
  store.begin();
#else
#error "need to define filesystem"
#endif
  if (!store.load("_main", the_mesh.self_id)) {
    MESH_DEBUG_PRINTLN("Generating new keypair");
    the_mesh.self_id = radio_new_identity();
    int count = 0;
    while (count < 10 &&
           (the_mesh.self_id.pub_key[0] == 0x00 || the_mesh.self_id.pub_key[0] == 0xFF)) {
      the_mesh.self_id = radio_new_identity();
      count++;
    }
    store.save("_main", the_mesh.self_id);
  }

  Serial.print("mcRPC ID: ");
  mesh::Utils::printHex(Serial, the_mesh.self_id.pub_key, PUB_KEY_SIZE);
  Serial.println();

  sensors.begin();
  the_mesh.begin(fs);
  the_mesh.beginMcRpc(fs);

  Serial.print("mcRPC name=");
  Serial.println(the_mesh.nodeName() ? the_mesh.nodeName() : "?");

#ifdef DISPLAY_CLASS
  // UI works when OLED began; buttons already live without it.
  if (oled_ok) {
    ui_task.begin(the_mesh.nodeName(), the_mesh.profile(), the_mesh.rpc().config().channelName());
  }
#endif

#if defined(ADVERT_BOOT_DELAY_MS)
  delay(ADVERT_BOOT_DELAY_MS);
#endif
  the_mesh.sendSelfAdvertisement(500, true);
}

static void handleButtonEdge(MomentaryButton& btn, bool& was_down, uint8_t btn_id) {
  const bool down = btn.isPressed();
  if (down == was_down) return;
  if (btn_id <= 1) {
    the_mesh.setButtonDown(down);
  }
#ifdef MCRPC_ENABLE_BUTTON
  if (down) {
    the_mesh.buttonFeature().notifyDown(btn_id);
  } else {
    the_mesh.buttonFeature().notifyUp(btn_id);
  }
#endif
#ifdef MCRPC_ENABLE_GPS
  if (down && btn_id == 1 && !the_mesh.onDemandGps().isBusy()) {
    the_mesh.onDemandGps().request();
  }
#endif
#ifdef DISPLAY_CLASS
  ui_task.poke();
#endif
  was_down = down;
}

void loop() {
  the_mesh.loop();
  the_mesh.loopMcRpc();

#if ENV_INCLUDE_GPS || defined(ENV_INCLUDE_BME680_BSEC)
  sensors.loop();
#endif

#ifdef MCRPC_HAS_USER_BTN
  static bool was_down1 = false;
  handleButtonEdge(user_btn, was_down1, 1);

#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
  static bool was_down2 = false;
  handleButtonEdge(user_btn2, was_down2, 2);
#endif

#ifdef DISPLAY_CLASS
  ui_task.setButtonState(was_down1,
#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
                         was_down2
#else
                         false
#endif
  );
  ui_task.loop();
#endif

  int ev = user_btn.check();
  if (ev == BUTTON_EVENT_CLICK) {
#ifdef MCRPC_ENABLE_BUTTON
    the_mesh.buttonFeature().notifyPressed(1);
#endif
#ifdef DISPLAY_CLASS
    ui_task.poke();
#endif
  }
#if defined(PIN_USER_BTN2) && (PIN_USER_BTN2 >= 0)
  int ev2 = user_btn2.check();
  if (ev2 == BUTTON_EVENT_CLICK) {
#ifdef MCRPC_ENABLE_BUTTON
    the_mesh.buttonFeature().notifyPressed(2);
#endif
#ifdef DISPLAY_CLASS
    ui_task.poke();
#endif
  }
#endif
#endif

  if (Serial.available()) {
    int len = Serial.readBytesUntil('\n', command, sizeof(command) - 1);
    if (len > 0) {
      command[len] = 0;
      while (len > 0 && (command[len - 1] == '\r' || command[len - 1] == ' ')) {
        command[--len] = 0;
      }
      char reply[160];
      reply[0] = 0;
      the_mesh.handleCommand(0, command, reply);
      if (reply[0]) {
        Serial.print("  -> ");
        Serial.println(reply);
      }
    }
  }
}
