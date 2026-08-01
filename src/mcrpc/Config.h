#pragma once

/**
 * Unified mcRPC node configuration.
 *
 * Reuses MeshCore channel crypto model (16/32-byte PSK → SHA256 → 1-byte hash).
 * Does NOT invent a parallel secrets store for mesh admin passwords — those
 * remain in NodePrefs / CommonCLI (/com_prefs).
 *
 * This file stores only mcRPC-specific prefs in /mcrpc_cfg.
 */

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
  char profile[24];          // tracker | switch | sensor | ...
  char channel_name[32];     // mesh group channel name (without #)
  uint8_t channel_psk[16];   // 128-bit PSK (raw bytes)
  uint8_t listen_enabled;    // 1 = process mcRPC on channel
  uint8_t debug;
  uint16_t report_interval_s;
  // Feature enable flags
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

  /** Load/save via FILESYSTEM* — implemented in Config.cpp with Arduino FS. */
  bool begin(void* fs);
  bool save();
  bool load();

private:
  ConfigPrefs _prefs;
  void* _fs = nullptr;
};

}  // namespace mcrpc
