#include "BatteryFeature.h"

#include <stdio.h>

namespace mcrpc {

static BatteryFeature* g_batt = nullptr;

void BatteryFeature::registerCommands(Registry& registry) {
  g_batt = this;
  registry.registerCommand("battery", &BatteryFeature::cmdBattery, "battery percent", "battery");
  registry.registerCommand("voltage", &BatteryFeature::cmdVoltage, "battery voltage", "battery");
  registry.registerCommand("charging", &BatteryFeature::cmdCharging, "charging state", "battery");
}

void BatteryFeature::loop() {
  float v = 0;
  int pct = -1;
  if (!_host.readBattery(v, pct)) return;
  if (v > 0 && v < _low_v) {
    if (!_low_latched && _host.engine) {
      char kv[32];
      snprintf(kv, sizeof(kv), "voltage=%.2f", (double)v);
      _host.engine->publishEvent("battery_low", kv);
      _low_latched = true;
    }
  } else if (v >= _low_v + 0.1f) {
    _low_latched = false;
  }
}

bool BatteryFeature::cmdBattery(CommandContext& ctx) {
  if (!g_batt) return false;
  float v = 0;
  int pct = -1;
  if (!g_batt->_host.readBattery(v, pct)) {
    ctx.reply->clear();
    ctx.reply->append("err unsupported");
    return true;
  }
  ctx.reply->clear();
  if (pct >= 0) ctx.reply->printf("battery value=%d", pct);
  else ctx.reply->printf("battery voltage=%.2f", (double)v);
  return true;
}

bool BatteryFeature::cmdVoltage(CommandContext& ctx) {
  if (!g_batt) return false;
  float v = 0;
  int pct = -1;
  if (!g_batt->_host.readBattery(v, pct)) {
    ctx.reply->clear();
    ctx.reply->append("err unsupported");
    return true;
  }
  ctx.reply->clear();
  ctx.reply->printf("voltage value=%.2f", (double)v);
  return true;
}

bool BatteryFeature::cmdCharging(CommandContext& ctx) {
  if (!g_batt) return false;
  ctx.reply->clear();
  ctx.reply->printf("charging value=%s", g_batt->_host.isCharging() ? "1" : "0");
  return true;
}

}  // namespace mcrpc
