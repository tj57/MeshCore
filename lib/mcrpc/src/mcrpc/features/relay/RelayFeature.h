#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

class RelayFeature : public Feature {
public:
  explicit RelayFeature(HostServices& host) : _host(host) {}
  const char* name() const override { return "relay"; }
  void registerCommands(CommandRegistry& commands) override;
  void registerCapabilities(CapabilityRegistry& caps) override {
    caps.registerCapability("relay");
  }
  void contributeDiscover(DiscoverBuilder& discover) override {
    discover.add("relay", "yes");
  }

private:
  HostServices& _host;
  static bool unsupported(CommandContext& ctx) {
    ctx.reply->clear();
    ctx.reply->append("err unsupported");
    return true;
  }
};

}  // namespace mcrpc
