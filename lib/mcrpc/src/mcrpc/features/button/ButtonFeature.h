#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"

namespace mcrpc {

/**
 * Button feature — board edge detection stays in the host (MomentaryButton).
 * Call notifyPressed() from the app; feature publishes via EventBus.
 */
class ButtonFeature : public Feature {
public:
  explicit ButtonFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "button"; }

  void registerCommands(CommandRegistry& commands) override;
  void registerCapabilities(CapabilityRegistry& caps) override;
  void contributeStatus(StatusBuilder& status) override;
  void contributeDiscover(DiscoverBuilder& discover) override;

  void notifyPressed();
  bool lastState() const { return _pressed; }

private:
  HostServices& _host;
  bool _pressed = false;
  uint32_t _press_count = 0;

  static bool cmdButton(CommandContext& ctx);
  static bool cmdButtonState(CommandContext& ctx);
};

}  // namespace mcrpc
