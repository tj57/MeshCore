#pragma once

/**
 * Portable mcRPC configuration (in-memory).
 * Persistence is injected via ConfigStore — Arduino FS, desktop file, HA none, etc.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace mcrpc {

#ifndef MCRPC_NODE_NAME_MAX
#define MCRPC_NODE_NAME_MAX 32
#endif

struct ConfigPrefs {
  uint32_t magic;
  uint16_t version;
  char node_name[MCRPC_NODE_NAME_MAX];
  char profile[24];
  char channel_name[32];
  uint8_t channel_psk[16];
  uint8_t listen_enabled;
  uint8_t debug;
  uint16_t report_interval_s;
  uint8_t feat_gps;
  uint8_t feat_battery;
  uint8_t feat_button;
  uint8_t feat_relay;
  uint8_t feat_display;
  uint8_t feat_led;
  uint8_t reserved[16];
};

static const uint32_t MCRPC_CFG_MAGIC = 0x4D435250;  // 'MCRP'
static const uint16_t MCRPC_CFG_VERSION = 1;

/** Platform persistence backend — implement per consumer. */
class ConfigStore {
public:
  virtual ~ConfigStore() {}
  virtual bool load(ConfigPrefs& prefs) = 0;
  virtual bool save(const ConfigPrefs& prefs) = 0;
};

class Config {
public:
  ConfigPrefs& prefs() { return _prefs; }
  const ConfigPrefs& prefs() const { return _prefs; }

  void setDefaults(const char* node_name, const char* profile,
                   const char* channel_name, const char* psk_ascii16);

  const char* nodeName() const { return _prefs.node_name; }
  const char* profile() const { return _prefs.profile; }
  const char* channelName() const { return _prefs.channel_name; }
  bool listenEnabled() const { return _prefs.listen_enabled != 0; }

  void setNodeName(const char* name);
  void setChannelName(const char* name);
  void setChannelPskAscii(const char* psk16);
  void setListenEnabled(bool on);

  /** Attach store and load (or save defaults if missing). */
  bool begin(ConfigStore* store);
  bool save();
  bool load();

  /**
   * Legacy MeshCore hook: `fs` is FILESYSTEM* when built with Arduino adapter.
   * Prefer begin(ConfigStore*).
   */
  bool begin(void* fs);

private:
  ConfigPrefs _prefs{};
  ConfigStore* _store = nullptr;
};

}  // namespace mcrpc
