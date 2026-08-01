#include "Config.h"

#include <Arduino.h>
#include <helpers/IdentityStore.h>
#include <helpers/TxtDataHelpers.h>
#include <string.h>

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
#include <InternalFileSystem.h>
#elif defined(RP2040_PLATFORM)
#include <LittleFS.h>
#elif defined(ESP32)
#include <SPIFFS.h>
#endif

#define MCRPC_CFG_FILE "/mcrpc_cfg"

namespace mcrpc {

void Config::setDefaults(const char* node_name, const char* profile,
                         const char* channel_name, const char* psk_ascii16) {
  memset(&_prefs, 0, sizeof(_prefs));
  _prefs.magic = MCRPC_CFG_MAGIC;
  _prefs.version = MCRPC_CFG_VERSION;
  if (node_name) StrHelper::strncpy(_prefs.node_name, node_name, sizeof(_prefs.node_name));
  if (profile) StrHelper::strncpy(_prefs.profile, profile, sizeof(_prefs.profile));
  if (channel_name) StrHelper::strncpy(_prefs.channel_name, channel_name, sizeof(_prefs.channel_name));
  if (psk_ascii16) {
    memset(_prefs.channel_psk, 0, sizeof(_prefs.channel_psk));
    size_t n = strlen(psk_ascii16);
    if (n > 16) n = 16;
    memcpy(_prefs.channel_psk, psk_ascii16, n);
  }
  _prefs.listen_enabled = 1;
  _prefs.debug = 0;
  _prefs.report_interval_s = 0;
  _prefs.feat_gps = 0;
  _prefs.feat_battery = 1;
  _prefs.feat_button = 0;
  _prefs.feat_relay = 0;
  _prefs.feat_display = 0;
  _prefs.feat_led = 0;
}

void Config::setNodeName(const char* name) {
  if (name) StrHelper::strncpy(_prefs.node_name, name, sizeof(_prefs.node_name));
}

void Config::setChannelName(const char* name) {
  if (name) StrHelper::strncpy(_prefs.channel_name, name, sizeof(_prefs.channel_name));
}

void Config::setChannelPskAscii(const char* psk16) {
  memset(_prefs.channel_psk, 0, sizeof(_prefs.channel_psk));
  if (!psk16) return;
  size_t n = strlen(psk16);
  if (n > 16) n = 16;
  memcpy(_prefs.channel_psk, psk16, n);
}

void Config::setListenEnabled(bool on) {
  _prefs.listen_enabled = on ? 1 : 0;
}

bool Config::begin(void* fs) {
  _fs = fs;
  if (!load()) {
    return save();
  }
  return true;
}

bool Config::load() {
  if (_fs == nullptr) return false;
  FILESYSTEM* fs = (FILESYSTEM*)_fs;
  if (!fs->exists(MCRPC_CFG_FILE)) return false;

#if defined(RP2040_PLATFORM)
  File f = fs->open(MCRPC_CFG_FILE, "r");
#else
  File f = fs->open(MCRPC_CFG_FILE);
#endif
  if (!f) return false;
  ConfigPrefs tmp;
  size_t n = f.read((uint8_t*)&tmp, sizeof(tmp));
  f.close();
  if (n < sizeof(tmp)) return false;
  if (tmp.magic != MCRPC_CFG_MAGIC) return false;
  if (tmp.version != MCRPC_CFG_VERSION) return false;
  _prefs = tmp;
  return true;
}

bool Config::save() {
  if (_fs == nullptr) return false;
  FILESYSTEM* fs = (FILESYSTEM*)_fs;
  _prefs.magic = MCRPC_CFG_MAGIC;
  _prefs.version = MCRPC_CFG_VERSION;

#if defined(NRF52_PLATFORM) || defined(STM32_PLATFORM)
  fs->remove(MCRPC_CFG_FILE);
  File f = fs->open(MCRPC_CFG_FILE, FILE_O_WRITE);
#elif defined(RP2040_PLATFORM)
  File f = fs->open(MCRPC_CFG_FILE, "w");
#else
  File f = fs->open(MCRPC_CFG_FILE, "w", true);
#endif
  if (!f) return false;
  size_t w = f.write((uint8_t*)&_prefs, sizeof(_prefs));
  f.close();
  return w == sizeof(_prefs);
}

}  // namespace mcrpc
