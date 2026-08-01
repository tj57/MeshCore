#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

class LedFeature : public Feature {
public:
  explicit LedFeature(HostServices& host) : _host(host) { (void)_host; }
  const char* name() const override { return "led"; }
  void registerCommands(Registry& registry) override {
    registry.registerCommand(
        "led",
        [](CommandContext& ctx) -> bool {
          ctx.reply->clear();
          ctx.reply->append("err unsupported");
          return true;
        },
        "led control", "led");
  }

private:
  HostServices& _host;
};

}  // namespace mcrpc
