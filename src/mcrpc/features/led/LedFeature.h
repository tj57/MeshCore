#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

class LedFeature : public Feature {
public:
  explicit LedFeature(HostServices& host) : _host(host) { (void)_host; }
  const char* name() const override { return "led"; }
  void registerCommands(CommandRegistry& commands) override {
    commands.registerCommand(
        "led",
        [](CommandContext& ctx) -> bool {
          ctx.reply->clear();
          ctx.reply->append("err unsupported");
          return true;
        },
        "led control");
  }
  void registerCapabilities(CapabilityRegistry& caps) override {
    caps.registerCapability("led");
  }
  void contributeDiscover(DiscoverBuilder& discover) override {
    discover.add("led", "yes");
  }

private:
  HostServices& _host;
};

}  // namespace mcrpc
