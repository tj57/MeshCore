#pragma once

/**
 * HostServices — board/app callbacks used by features.
 * Keeps feature code free of MeshCore and pin-level APIs.
 */
#include <stdint.h>

namespace mcrpc {

struct GpsFix {
  bool valid;
  float lat;
  float lon;
  float alt;
  int sats;
  float hdop;
};

struct HostServices {
  virtual ~HostServices() {}

  virtual const char* nodeName() = 0;
  virtual const char* profile() = 0;
  virtual const char* firmwareVersion() = 0;
  virtual uint32_t uptimeSeconds() = 0;
  virtual int rssi() { return 0; }

  virtual bool readBattery(float& volts, int& percent) {
    (void)volts;
    (void)percent;
    return false;
  }
  virtual bool isCharging() { return false; }

  virtual bool readGps(GpsFix& fix) {
    (void)fix;
    return false;
  }
  /** Request an on-demand GPS acquisition (may be async). */
  virtual bool requestGpsFix() { return false; }
  virtual bool gpsBusy() { return false; }

  virtual bool readButtonPressed() { return false; }
  virtual bool setRelay(bool on) {
    (void)on;
    return false;
  }
  virtual bool getRelay() { return false; }

  virtual bool displayText(const char* text) {
    (void)text;
    return false;
  }
  virtual bool displayClear() { return false; }

  /** Access to McRpc for event publishing from features (set by app). */
  class McRpc* engine = nullptr;
};

}  // namespace mcrpc
