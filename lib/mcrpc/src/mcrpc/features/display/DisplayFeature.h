#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

class DisplayFeature : public Feature {
public:
  explicit DisplayFeature(HostServices& host) : _host(host) {}
  const char* name() const override { return "display"; }
  void registerCommands(CommandRegistry& commands) override;
  void registerCapabilities(CapabilityRegistry& caps) override {
    caps.registerCapability("display");
  }
  void contributeDiscover(DiscoverBuilder& discover) override {
    discover.add("display", "yes");
  }

private:
  HostServices& _host;
  static bool cmdDisplay(CommandContext& ctx);
  static bool cmdText(CommandContext& ctx);
  static bool cmdClear(CommandContext& ctx);
};

}  // namespace mcrpc
