#pragma once
#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

class DisplayFeature : public Feature {
public:
  explicit DisplayFeature(HostServices& host) : _host(host) {}
  const char* name() const override { return "display"; }
  void registerCommands(Registry& registry) override;

private:
  HostServices& _host;
  static bool cmdDisplay(CommandContext& ctx);
  static bool cmdText(CommandContext& ctx);
  static bool cmdClear(CommandContext& ctx);
};

}  // namespace mcrpc
