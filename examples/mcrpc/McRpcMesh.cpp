#include "McRpcMesh.h"

#include <stdio.h>
#include <ctype.h>

McRpcMesh::McRpcMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms,
                     mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables)
  : SensorMesh(board, radio, ms, rng, rtc, tables),
    battery_data(12 * 24, 5 * 60),
    _feat_core(*this),
    _feat_battery(*this),
    _feat_button(*this),
    _feat_gps(*this),
    _gps_session(sensors),
    _channel_ready(false),
    _boot_ms(0),
    _btn_down(false) {
  memset(&_channel, 0, sizeof(_channel));
}

void McRpcMesh::beginMcRpc(FILESYSTEM* fs) {
  _boot_ms = millis();
  engine = &_rpc;

#ifndef MCRPC_DEFAULT_NAME
#define MCRPC_DEFAULT_NAME "node"
#endif
#ifndef MCRPC_DEFAULT_PROFILE
#define MCRPC_DEFAULT_PROFILE "sensor"
#endif
#ifndef MCRPC_DEFAULT_CHANNEL
#define MCRPC_DEFAULT_CHANNEL "mychan"
#endif
#ifndef MCRPC_DEFAULT_PSK
#define MCRPC_DEFAULT_PSK "mychan-gps-chan!"  // 16 chars; change in field
#endif

  _rpc.config().setDefaults(MCRPC_DEFAULT_NAME, MCRPC_DEFAULT_PROFILE, MCRPC_DEFAULT_CHANNEL,
                            MCRPC_DEFAULT_PSK);

#ifdef MCRPC_ENABLE_GPS
  _rpc.config().prefs().feat_gps = 1;
#endif
#ifdef MCRPC_ENABLE_BUTTON
  _rpc.config().prefs().feat_button = 1;
#endif
  _rpc.config().prefs().feat_battery = 1;

  _rpc.config().begin(fs);
  // Prefer MeshCore node name from NodePrefs when set
  if (getNodeName() && getNodeName()[0]) {
    _rpc.config().setNodeName(getNodeName());
  }

  rebuildChannel();

  _rpc.setPublishHandler(&McRpcMesh::publishThunk, this);
  _rpc.setFirmwareVersion(MCRPC_FW_VERSION);
  _rpc.setProfile(_rpc.config().profile());
  _rpc.setNodeIdentity(_rpc.config().nodeName(), _rpc.config().channelName());
  _rpc.setIdentityCallbacks(&McRpcMesh::uptimeThunk, &McRpcMesh::rssiThunk, this);
  _rpc.setUserData(this);

  // FeatureManager owns lifecycle — features must not register outside add()+begin().
  _rpc.features().add(&_feat_core);
  _rpc.features().add(&_feat_battery);
#ifdef MCRPC_ENABLE_BUTTON
  _rpc.features().add(&_feat_button);
#endif
#ifdef MCRPC_ENABLE_GPS
  _rpc.features().add(&_feat_gps);
  _gps_session.setDoneHandler(&McRpcMesh::gpsDoneThunk, this);
#endif

  _rpc.begin();
  MESH_DEBUG_PRINTLN("mcRPC ready name=%s profile=%s channel=#%s", _rpc.config().nodeName(),
                     _rpc.config().profile(), _rpc.config().channelName());
}

void McRpcMesh::loopMcRpc() {
  _rpc.loop();
#ifdef MCRPC_ENABLE_GPS
  _gps_session.loop();
#endif
}

void McRpcMesh::rebuildChannel() {
  memset(&_channel, 0, sizeof(_channel));
  StrHelper::strncpy(_channel.name, _rpc.config().channelName(), sizeof(_channel.name));
  memset(_channel.channel.secret, 0, sizeof(_channel.channel.secret));
  memcpy(_channel.channel.secret, _rpc.config().prefs().channel_psk, 16);
  mesh::Utils::sha256(_channel.channel.hash, sizeof(_channel.channel.hash),
                      _channel.channel.secret, 16);
  _channel_ready = (_channel.name[0] != 0);
  _rpc.setNodeIdentity(_rpc.config().nodeName(), _rpc.config().channelName());
}

bool McRpcMesh::sendChannelText(const char* text) {
  if (!_channel_ready || text == nullptr || text[0] == 0) return false;
  if (_mgr->getOutboundTotal() > 0) return false;

  uint8_t temp[5 + MCRPC_MAX_TEXT + 32];
  uint32_t timestamp = getRTCClock()->getCurrentTimeUnique();
  memcpy(temp, &timestamp, 4);
  temp[4] = 0;  // TXT_TYPE_PLAIN

  const char* sender = _rpc.config().nodeName();
  if (sender == nullptr || sender[0] == 0) sender = "mcrpc";

  int prefix_len = snprintf((char*)&temp[5], MCRPC_MAX_TEXT, "%s: ", sender);
  if (prefix_len < 0) return false;

  int text_len = (int)strlen(text);
  if (prefix_len + text_len > MCRPC_MAX_TEXT) text_len = MCRPC_MAX_TEXT - prefix_len;
  if (text_len < 0) return false;
  memcpy((char*)&temp[5] + prefix_len, text, text_len);
  ((char*)&temp[5])[prefix_len + text_len] = 0;

  // If reply starts with #id, rewrite to "name#id body" per protocol examples
  // Transport already has "name: " prefix from MeshCore convention; body may
  // include "#42 pong". Spec examples use "ha#42 pong" as the full message —
  // acceptable on channel as "tracker: #42 pong".

  mesh::Packet* pkt =
      createGroupDatagram(PAYLOAD_TYPE_GRP_TXT, _channel.channel, temp,
                          (size_t)(5 + prefix_len + text_len));
  if (pkt == nullptr) return false;
  sendFlood(pkt);
  return true;
}

bool McRpcMesh::publishThunk(const char* text, void* ctx) {
  return static_cast<McRpcMesh*>(ctx)->sendChannelText(text);
}

void McRpcMesh::gpsDoneThunk(bool ok, float lat, float lon, float alt, int sats, float hdop,
                             void* ctx) {
  McRpcMesh* self = static_cast<McRpcMesh*>(ctx);
  char buf[128];
  if (!ok) {
    self->_rpc.publishRaw("err gps_no_fix");
    self->_rpc.events().publish("gps_nofix", nullptr);
    return;
  }
  snprintf(buf, sizeof(buf), "gps lat=%.6f lon=%.6f alt=%.1f sat=%d", (double)lat, (double)lon,
           (double)alt, sats);
  if (hdop > 0) {
    char extra[32];
    snprintf(extra, sizeof(extra), " hdop=%.1f", (double)hdop);
    strncat(buf, extra, sizeof(buf) - strlen(buf) - 1);
  }
  self->_rpc.publishRaw(buf);
  self->_rpc.events().publish("gps_fix", nullptr);
}

uint32_t McRpcMesh::uptimeThunk(void* ctx) {
  return static_cast<McRpcMesh*>(ctx)->uptimeSeconds();
}

int McRpcMesh::rssiThunk(void* ctx) {
  return static_cast<McRpcMesh*>(ctx)->rssi();
}

const char* McRpcMesh::nodeName() {
  return _rpc.config().nodeName();
}

const char* McRpcMesh::profile() {
  return _rpc.config().profile();
}

uint32_t McRpcMesh::uptimeSeconds() {
  return (millis() - _boot_ms) / 1000UL;
}

int McRpcMesh::rssi() {
  return (int)radio_driver.getLastRSSI();
}

bool McRpcMesh::readBattery(float& volts, int& percent) {
  volts = getVoltage(TELEM_CHANNEL_SELF);
  if (volts <= 0.1f) return false;
  // crude LiPo estimate
  percent = (int)((volts - 3.3f) / (4.2f - 3.3f) * 100.0f);
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return true;
}

bool McRpcMesh::readGps(mcrpc::GpsFix& fix) {
  memset(&fix, 0, sizeof(fix));
  float lat, lon, alt;
  int sats;
  float hdop;
  if (_gps_session.readLast(lat, lon, alt, sats, hdop)) {
    fix.valid = true;
    fix.lat = lat;
    fix.lon = lon;
    fix.alt = alt;
    fix.sats = sats;
    fix.hdop = hdop;
    return true;
  }
  return false;
}

bool McRpcMesh::requestGpsFix() {
#ifdef MCRPC_ENABLE_GPS
  return _gps_session.request();
#else
  return false;
#endif
}

bool McRpcMesh::gpsBusy() {
#ifdef MCRPC_ENABLE_GPS
  return _gps_session.isBusy();
#else
  return false;
#endif
}

bool McRpcMesh::readButtonPressed() {
  return _btn_down;
}

void McRpcMesh::onSensorDataRead() {
  float batt_voltage = getVoltage(TELEM_CHANNEL_SELF);
  battery_data.recordData(getRTCClock(), batt_voltage);
  alertIf(batt_voltage < 3.4f, critical_batt, HIGH_PRI_ALERT, "Battery is critical!");
  alertIf(batt_voltage < 3.6f, low_batt, LOW_PRI_ALERT, "Battery is low");
}

int McRpcMesh::querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[],
                               int max_num) {
  (void)max_num;
  battery_data.calcMinMaxAvg(getRTCClock(), start_secs_ago, end_secs_ago, &dest[0],
                             TELEM_CHANNEL_SELF, LPP_VOLTAGE);
  return 1;
}

int McRpcMesh::searchChannelsByHash(const uint8_t* hash, mesh::GroupChannel channels[],
                                    int max_matches) {
  if (!_channel_ready || !_rpc.config().listenEnabled()) return 0;
  if (max_matches < 1) return 0;
  if (_channel.channel.hash[0] != hash[0]) return 0;
  channels[0] = _channel.channel;
  return 1;
}

void McRpcMesh::onGroupDataRecv(mesh::Packet* packet, uint8_t type, const mesh::GroupChannel& channel,
                                uint8_t* data, size_t len) {
  (void)packet;
  (void)channel;
  if (!_rpc.config().listenEnabled()) return;
  if (type != PAYLOAD_TYPE_GRP_TXT) return;
  if (len < 5) return;

  uint8_t txt_type = data[4];
  if ((txt_type >> 2) != 0) return;  // plain only

  data[len] = 0;
  const char* text = (const char*)&data[5];
  _rpc.handleIncomingText(text);
}
