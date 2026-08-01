#pragma once

#include "../../Feature.h"
#include "../../HostServices.h"
#include "../../McRpc.h"

namespace mcrpc {

/**
 * Button feature — registers commands; physical edge detection lives in the
 * board/app (MomentaryButton) and calls notifyPressed().
 */
class ButtonFeature : public Feature {
public:
  explicit ButtonFeature(HostServices& host) : _host(host) {}

  const char* name() const override { return "button"; }

  void registerCommands(Registry& registry) override;

  /** Called by app when MomentaryButton reports a click. */
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
