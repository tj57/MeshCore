#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

/** Placeholder — registers commands that return err unsupported until IO lands. */
class RelayFeature : public Feature {
public:
  explicit RelayFeature(HostServices& host) : _host(host) {}
  const char* name() const override { return "relay"; }
  void registerCommands(Registry& registry) override;

private:
  HostServices& _host;
  static bool unsupported(CommandContext& ctx) {
    ctx.reply->clear();
    ctx.reply->append("err unsupported");
    return true;
  }
};

}  // namespace mcrpc
