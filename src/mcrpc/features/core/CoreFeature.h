#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

/** Mandatory commands: ping, status, discover, help, caps */
class CoreFeature : public Feature {
public:
  explicit CoreFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "core"; }
  const char* capability() const override { return nullptr; }  // not listed in caps

  void registerCommands(Registry& registry) override;

private:
  HostServices& _host;

  static bool cmdPing(CommandContext& ctx);
  static bool cmdStatus(CommandContext& ctx);
  static bool cmdDiscover(CommandContext& ctx);
  static bool cmdHelp(CommandContext& ctx);
  static bool cmdCaps(CommandContext& ctx);
};

}  // namespace mcrpc
