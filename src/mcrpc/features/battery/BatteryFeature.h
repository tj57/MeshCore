#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"
#include "../../McRpc.h"

namespace mcrpc {

class BatteryFeature : public Feature {
public:
  explicit BatteryFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "battery"; }

  void registerCommands(Registry& registry) override;
  void loop() override;

  void setLowThresholdVolts(float v) { _low_v = v; }

private:
  HostServices& _host;
  float _low_v = 3.6f;
  bool _low_latched = false;

  static bool cmdBattery(CommandContext& ctx);
  static bool cmdVoltage(CommandContext& ctx);
  static bool cmdCharging(CommandContext& ctx);
};

}  // namespace mcrpc
