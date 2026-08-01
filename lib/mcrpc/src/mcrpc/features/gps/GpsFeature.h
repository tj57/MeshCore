#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

/** GPS feature — depends only on HostServices (no board ifdefs). */
class GpsFeature : public Feature {
public:
  explicit GpsFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "gps"; }

  void registerCommands(CommandRegistry& commands) override;
  void registerCapabilities(CapabilityRegistry& caps) override;
  void contributeStatus(StatusBuilder& status) override;
  void contributeDiscover(DiscoverBuilder& discover) override;

private:
  HostServices& _host;

  static bool cmdGps(CommandContext& ctx);
  static bool cmdLocation(CommandContext& ctx);
  static bool cmdTrack(CommandContext& ctx);
  static bool writeFix(CommandContext& ctx, bool request_if_missing);
};

}  // namespace mcrpc
