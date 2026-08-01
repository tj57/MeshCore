#include "RelayFeature.h"

namespace mcrpc {

void RelayFeature::registerCommands(CommandRegistry& commands) {
  (void)_host;
  commands.registerCommand("relay", &RelayFeature::unsupported, "relay control");
  commands.registerCommand("toggle", &RelayFeature::unsupported, "toggle relay");
  commands.registerCommand("power", &RelayFeature::unsupported, "relay power");
}

}  // namespace mcrpc
