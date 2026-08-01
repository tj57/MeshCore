#pragma once

#include "Feature.h"
#include "CommandRegistry.h"
#include "CapabilityRegistry.h"
#include "EventBus.h"
#include "StatusBuilder.h"
#include "DiscoverBuilder.h"

namespace mcrpc {

/**
 * Owns feature lifecycle. Features must not self-register outside this manager.
 *
 * start(): setup → registerCommands → registerCapabilities
 * loop():  feature.loop()
 * stop():  shutdown (reverse order)
 */
class FeatureManager {
public:
  bool add(Feature* feature);

  void start(CommandRegistry& commands, CapabilityRegistry& capabilities, EventBus& events);
  void loop();
  void stop();

  size_t count() const { return _count; }
  Feature* at(size_t i) const { return i < _count ? _features[i] : nullptr; }

  void collectStatus(StatusBuilder& status) const;
  void collectDiscover(DiscoverBuilder& discover) const;
  void writeCapabilities(ReplyBuffer& reply) const;
  void writeHelp(ReplyBuffer& reply, const CommandRegistry& commands) const;

  bool started() const { return _started; }

private:
  Feature* _features[MCRPC_MAX_FEATURES];
  size_t _count = 0;
  FeatureContext _ctx;
  CapabilityRegistry* _caps = nullptr;
  bool _started = false;
};

}  // namespace mcrpc
