#pragma once

#include <helpers/sensors/EnvironmentSensorManager.h>
#include <Arduino.h>

namespace mcrpc {

/**
 * On-demand GPS power session for battery-powered trackers (LW010 / WisMesh Tag).
 * Powers GPS only while waiting for a fix, then powers off.
 *
 * Uses EnvironmentSensorManager settings ("gps" = 1/0) — same mechanism as
 * MeshCore CommonCLI GPS control — so we do not fork sensor drivers.
 */
class OnDemandGps {
public:
  enum State { Idle = 0, WaitingFix };

  using DoneFn = void (*)(bool ok, float lat, float lon, float alt, int sats, float hdop, void* ctx);

  explicit OnDemandGps(EnvironmentSensorManager& sensors)
    : _sensors(sensors), _state(Idle), _deadline_ms(0), _timeout_ms(90000),
      _done(nullptr), _done_ctx(nullptr), _powered(false) {}

  void setTimeoutMs(uint32_t ms) { _timeout_ms = ms; }
  void setDoneHandler(DoneFn fn, void* ctx) {
    _done = fn;
    _done_ctx = ctx;
  }

  bool isBusy() const { return _state != Idle; }
  bool isPowered() const { return _powered; }

  bool request() {
    if (_state != Idle) return false;
    if (!powerOn()) return false;
    _state = WaitingFix;
    _deadline_ms = millis() + _timeout_ms;
    return true;
  }

  void forceOff() {
    _state = Idle;
    powerOff();
  }

  void loop() {
#if ENV_INCLUDE_GPS
    _sensors.loop();
    if (_state != WaitingFix) return;

    LocationProvider* loc = _sensors.getLocationProvider();
    if (loc != nullptr) {
      loc->loop();
      if (loc->isValid()) {
        float lat = ((double)loc->getLatitude()) / 1000000.0;
        float lon = ((double)loc->getLongitude()) / 1000000.0;
        float alt = ((double)loc->getAltitude()) / 1000.0;
        int sats = (int)loc->satellitesCount();
        complete(true, lat, lon, alt, sats, 0);
        return;
      }
    }
    if ((long)(millis() - _deadline_ms) >= 0) {
      complete(false, 0, 0, 0, 0, 0);
    }
#else
    (void)0;
#endif
  }

  bool readLast(float& lat, float& lon, float& alt, int& sats, float& hdop) const {
    if (!_last_valid) return false;
    lat = _last_lat;
    lon = _last_lon;
    alt = _last_alt;
    sats = _last_sats;
    hdop = _last_hdop;
    return true;
  }

private:
  EnvironmentSensorManager& _sensors;
  State _state;
  uint32_t _deadline_ms;
  uint32_t _timeout_ms;
  DoneFn _done;
  void* _done_ctx;
  bool _powered;
  bool _last_valid = false;
  float _last_lat = 0, _last_lon = 0, _last_alt = 0, _last_hdop = 0;
  int _last_sats = 0;

  bool powerOn() {
#if !ENV_INCLUDE_GPS
    return false;
#else
#if defined(PIN_GPS_TX) && defined(PIN_GPS_RX)
    Serial1.setPins(PIN_GPS_TX, PIN_GPS_RX);
#endif
#ifdef GPS_BAUD_RATE
    Serial1.begin(GPS_BAUD_RATE);
#else
    Serial1.begin(9600);
#endif
    if (!_sensors.setSettingValue("gps", "1")) {
      Serial1.end();
      _powered = false;
      return false;
    }
    _powered = true;
    return true;
#endif
  }

  void powerOff() {
#if ENV_INCLUDE_GPS
    _sensors.setSettingValue("gps", "0");
    LocationProvider* loc = _sensors.getLocationProvider();
    if (loc != nullptr && loc->isEnabled()) loc->stop();
    Serial1.end();
#endif
    _powered = false;
  }

  void complete(bool ok, float lat, float lon, float alt, int sats, float hdop) {
    _state = Idle;
    if (ok) {
      _last_valid = true;
      _last_lat = lat;
      _last_lon = lon;
      _last_alt = alt;
      _last_sats = sats;
      _last_hdop = hdop;
    }
    powerOff();
    if (_done) _done(ok, lat, lon, alt, sats, hdop, _done_ctx);
  }
};

}  // namespace mcrpc
