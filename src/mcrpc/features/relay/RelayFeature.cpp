#include "RelayFeature.h"

namespace mcrpc {

void RelayFeature::registerCommands(Registry& registry) {
  (void)_host;
  registry.registerCommand("relay", &RelayFeature::unsupported, "relay control", "relay");
  registry.registerCommand("toggle", &RelayFeature::unsupported, "toggle relay", "relay");
  registry.registerCommand("power", &RelayFeature::unsupported, "relay power", "relay");
}

}  // namespace mcrpc
