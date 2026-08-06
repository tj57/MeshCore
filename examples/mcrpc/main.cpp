#include "McRpcMesh.h"

#include <helpers/ui/MomentaryButton.h>

StdRNG fast_rng;
SimpleMeshTables tables;
McRpcMesh the_mesh(board, radio_driver, *new ArduinoMillis(), fast_rng, rtc_clock, tables);

#if defined(PIN_USER_BTN) && (PIN_USER_BTN >= 0)
#ifndef DISPLAY_CLASS
// Boards without DISPLAY_CLASS still need a button instance for mcRPC.
static MomentaryButton user_btn(PIN_USER_BTN, 1000, true);
#endif
#define MCRPC_HAS_USER_BTN 1
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

#if defined(ADVERT_BOOT_DELAY_MS)
  delay(ADVERT_BOOT_DELAY_MS);
#endif
  the_mesh.sendSelfAdvertisement(500, true);
}

void loop() {
  the_mesh.loop();
  the_mesh.loopMcRpc();

#if ENV_INCLUDE_GPS || defined(ENV_INCLUDE_BME680_BSEC)
  sensors.loop();
#endif

#ifdef MCRPC_HAS_USER_BTN
  // Edge detect → event button_down / button_up on the private mcRPC channel.
  static bool was_down = false;
  const bool down = user_btn.isPressed();
  if (down != was_down) {
    the_mesh.setButtonDown(down);
    if (down) {
#ifdef MCRPC_ENABLE_BUTTON
      the_mesh.buttonFeature().notifyDown();
#endif
#ifdef MCRPC_ENABLE_GPS
      // Tracker profile: wake GPS on press; OnDemandGps powers off after fix/timeout.
      if (!the_mesh.onDemandGps().isBusy()) {
        the_mesh.onDemandGps().request();
      }
#endif
    } else {
#ifdef MCRPC_ENABLE_BUTTON
      the_mesh.buttonFeature().notifyUp();
#endif
    }
    was_down = down;
  }

  // Keep click path for legacy event button_pressed (short click after release).
  int ev = user_btn.check();
  if (ev == BUTTON_EVENT_CLICK) {
#ifdef MCRPC_ENABLE_BUTTON
    the_mesh.buttonFeature().notifyPressed();
#endif
  }
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
