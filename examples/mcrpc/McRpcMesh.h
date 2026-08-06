#pragma once

#include "SensorMesh.h"

#include <helpers/ChannelDetails.h>
#include <mcrpc/McRpc.h>
#include <mcrpc/HostServices.h>
#include <mcrpc/features/core/CoreFeature.h>
#include <mcrpc/features/battery/BatteryFeature.h>
#include <mcrpc/features/button/ButtonFeature.h>
#include <mcrpc/features/gps/GpsFeature.h>
#include "drivers/OnDemandGps.h"
#include "ArduinoFsConfigStore.h"

#ifndef MCRPC_MAX_TEXT
#define MCRPC_MAX_TEXT (10 * 16)
#endif

#ifndef MCRPC_TX_QUEUE
#define MCRPC_TX_QUEUE 8
#endif

#ifndef MCRPC_FW_VERSION
#define MCRPC_FW_VERSION "mcrpc-1.1.0-beta"
#endif

/**
 * Sensor-role mesh node with mcRPC application layer on a private group channel.
 *
 * Layering (merge-friendly):
 *   Radio → MeshCore (SensorMesh) → McRpcMesh transport → mcRPC → Features
 *
 * Upstream SensorMesh / CommonCLI remain intact for admin serial/CLI.
 */
class McRpcMesh : public SensorMesh, public mcrpc::HostServices {
public:
  McRpcMesh(mesh::MainBoard& board, mesh::Radio& radio, mesh::MillisecondClock& ms,
            mesh::RNG& rng, mesh::RTCClock& rtc, mesh::MeshTables& tables);

  void beginMcRpc(FILESYSTEM* fs);
  void loopMcRpc();

  mcrpc::McRpc& rpc() { return _rpc; }
  mcrpc::ButtonFeature& buttonFeature() { return _feat_button; }
  mcrpc::OnDemandGps& onDemandGps() { return _gps_session; }

  bool sendChannelText(const char* text);
  void rebuildChannel();

  // TX pipeline counters (stress / diagnostics)
  uint32_t txOk() const { return _tx_ok; }
  uint32_t txQueued() const { return _tx_queued; }
  uint32_t txDropBusy() const { return _tx_drop_busy; }
  uint32_t txDropAlloc() const { return _tx_drop_alloc; }
  uint32_t txDropQueueFull() const { return _tx_drop_queue_full; }
  uint8_t txQueueDepth() const { return _tx_q_count; }

  // HostServices
  const char* nodeName() override;
  const char* profile() override;
  const char* firmwareVersion() override { return MCRPC_FW_VERSION; }
  uint32_t uptimeSeconds() override;
  int rssi() override;
  bool readBattery(float& volts, int& percent) override;
  bool readGps(mcrpc::GpsFix& fix) override;
  bool requestGpsFix() override;
  bool gpsBusy() override;
  bool readButtonPressed() override;

protected:
  Trigger low_batt, critical_batt;
  TimeSeriesData battery_data;

  void onSensorDataRead() override;
  int querySeriesData(uint32_t start_secs_ago, uint32_t end_secs_ago, MinMaxAvg dest[],
                      int max_num) override;
  int searchChannelsByHash(const uint8_t* hash, mesh::GroupChannel channels[],
                           int max_matches) override;
  void onGroupDataRecv(mesh::Packet* packet, uint8_t type, const mesh::GroupChannel& channel,
                       uint8_t* data, size_t len) override;

private:
  mcrpc::McRpc _rpc;
  mcrpc::ArduinoFsConfigStore _cfg_store;
  mcrpc::CoreFeature _feat_core;
  mcrpc::BatteryFeature _feat_battery;
  mcrpc::ButtonFeature _feat_button;
  mcrpc::GpsFeature _feat_gps;
  mcrpc::OnDemandGps _gps_session;

  ChannelDetails _channel;
  bool _channel_ready;
  uint32_t _boot_ms;
  bool _btn_down;

  // Outbound reply queue — avoids silent drops when radio outbound is busy.
  char _tx_queue[MCRPC_TX_QUEUE][MCRPC_MAX_TEXT + 1];
  uint8_t _tx_q_head;
  uint8_t _tx_q_tail;
  uint8_t _tx_q_count;
  uint32_t _tx_ok;
  uint32_t _tx_queued;
  uint32_t _tx_drop_busy;
  uint32_t _tx_drop_alloc;
  uint32_t _tx_drop_queue_full;

  bool enqueueTx(const char* text);
  bool trySendNow(const char* text);
  void drainTxQueue();

  static bool publishThunk(const char* text, void* ctx);
  static void gpsDoneThunk(bool ok, float lat, float lon, float alt, int sats, float hdop,
                           void* ctx);
  static uint32_t uptimeThunk(void* ctx);
  static int rssiThunk(void* ctx);
};
