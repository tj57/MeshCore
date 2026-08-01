#pragma once

/**
 * Arduino / MeshCore filesystem ConfigStore for /mcrpc_cfg.
 * Lives in the MeshCore consumer — not required for desktop/HA builds.
 */

#include <mcrpc/Config.h>
#include <helpers/IdentityStore.h>

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
#include <InternalFileSystem.h>
#elif defined(RP2040_PLATFORM)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif

#ifndef MCRPC_CFG_FILE
#define MCRPC_CFG_FILE "/mcrpc_cfg"
#endif

namespace mcrpc {

class ArduinoFsConfigStore : public ConfigStore {
public:
  ArduinoFsConfigStore() : _fs(nullptr) {}
  explicit ArduinoFsConfigStore(FILESYSTEM* fs) : _fs(fs) {}

  void setFilesystem(FILESYSTEM* fs) { _fs = fs; }

  bool load(ConfigPrefs& prefs) override {
    if (_fs == nullptr || !_fs->exists(MCRPC_CFG_FILE)) return false;
#if defined(RP2040_PLATFORM)
    File f = _fs->open(MCRPC_CFG_FILE, "r");
#else
    File f = _fs->open(MCRPC_CFG_FILE);
#endif
    if (!f) return false;
    size_t n = f.read((uint8_t*)&prefs, sizeof(prefs));
    f.close();
    return n >= sizeof(prefs);
  }

  bool save(const ConfigPrefs& prefs) override {
    if (_fs == nullptr) return false;
#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
    _fs->remove(MCRPC_CFG_FILE);
    File f = _fs->open(MCRPC_CFG_FILE, FILE_O_WRITE);
#elif defined(RP2040_PLATFORM)
    File f = _fs->open(MCRPC_CFG_FILE, "w");
#else
    File f = _fs->open(MCRPC_CFG_FILE, "w", true);
#endif
    if (!f) return false;
    size_t w = f.write((const uint8_t*)&prefs, sizeof(prefs));
    f.close();
    return w == sizeof(prefs);
  }

private:
  FILESYSTEM* _fs;
};

}  // namespace mcrpc
