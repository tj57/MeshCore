#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

/** Mandatory commands — assembled via Status/Discover/Capability builders. */
class CoreFeature : public Feature {
public:
  explicit CoreFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "core"; }

  void registerCommands(CommandRegistry& commands) override;
  // core is not listed in caps

private:
  HostServices& _host;

  static bool cmdPing(CommandContext& ctx);
  static bool cmdStatus(CommandContext& ctx);
  static bool cmdDiscover(CommandContext& ctx);
  static bool cmdHelp(CommandContext& ctx);
  static bool cmdCaps(CommandContext& ctx);
};

}  // namespace mcrpc
