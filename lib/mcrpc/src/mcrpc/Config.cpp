#include "Config.h"

namespace mcrpc {

static void copyZ(char* dest, size_t dest_size, const char* src) {
  if (!dest || dest_size == 0) return;
  if (!src) {
    dest[0] = 0;
    return;
  }
  size_t i = 0;
  for (; i + 1 < dest_size && src[i]; i++) dest[i] = src[i];
  dest[i] = 0;
}

void Config::setDefaults(const char* node_name, const char* profile,
                         const char* channel_name, const char* psk_ascii16) {
  memset(&_prefs, 0, sizeof(_prefs));
  _prefs.magic = MCRPC_CFG_MAGIC;
  _prefs.version = MCRPC_CFG_VERSION;
  if (node_name) copyZ(_prefs.node_name, sizeof(_prefs.node_name), node_name);
  if (profile) copyZ(_prefs.profile, sizeof(_prefs.profile), profile);
  if (channel_name) copyZ(_prefs.channel_name, sizeof(_prefs.channel_name), channel_name);
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
  if (name) copyZ(_prefs.node_name, sizeof(_prefs.node_name), name);
}

void Config::setChannelName(const char* name) {
  if (name) copyZ(_prefs.channel_name, sizeof(_prefs.channel_name), name);
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

bool Config::begin(ConfigStore* store) {
  _store = store;
  if (!load()) {
    return save();
  }
  return true;
}

bool Config::load() {
  if (_store == nullptr) return false;
  ConfigPrefs tmp;
  if (!_store->load(tmp)) return false;
  if (tmp.magic != MCRPC_CFG_MAGIC) return false;
  if (tmp.version != MCRPC_CFG_VERSION) return false;
  _prefs = tmp;
  return true;
}

bool Config::save() {
  if (_store == nullptr) return false;
  _prefs.magic = MCRPC_CFG_MAGIC;
  _prefs.version = MCRPC_CFG_VERSION;
  return _store->save(_prefs);
}

bool Config::begin(void* fs) {
  (void)fs;
  // Without an Arduino adapter linked, void* begin is a no-op persist.
  // MeshCore consumer should call begin(ConfigStore*).
  return true;
}

}  // namespace mcrpc
