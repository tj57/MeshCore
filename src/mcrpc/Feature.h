#pragma once

/**
 * Feature SDK — stable surface for all future mcRPC feature modules.
 *
 * Features MUST depend only on these types + HostServices.
 * They MUST NOT include Parser, Dispatcher, MeshCore, or board headers.
 */

#include "McRpcTypes.h"
#include "CommandRegistry.h"
#include "CapabilityRegistry.h"
#include "EventBus.h"
#include "StatusBuilder.h"
#include "DiscoverBuilder.h"

namespace mcrpc {

class FeatureManager;

/**
 * Injected once during FeatureManager::start().
 * Store references you need; do not keep a dangling FeatureContext pointer
 * beyond the feature's lifetime (manager outlives features).
 */
struct FeatureContext {
  CommandRegistry* commands = nullptr;
  CapabilityRegistry* capabilities = nullptr;
  EventBus* events = nullptr;
  FeatureManager* manager = nullptr;
};

/**
 * Stable Feature API (v1).
 *
 * Lifecycle owned exclusively by FeatureManager:
 *   construct → add → start{ setup → registerCommands → registerCapabilities }
 *            → loop* → stop{ shutdown }
 */
class Feature {
public:
  virtual ~Feature() {}

  virtual const char* name() const = 0;

  /** Acquire EventBus / registries. Default stores context pointers. */
  virtual void setup(FeatureContext& ctx) { _ctx = &ctx; }

  /** Register command handlers only. */
  virtual void registerCommands(CommandRegistry& commands) = 0;

  /** Register capability names for `caps`. */
  virtual void registerCapabilities(CapabilityRegistry& caps) { (void)caps; }

  /** Contribute key=value fields to `status`. */
  virtual void contributeStatus(StatusBuilder& status) { (void)status; }

  /** Contribute key=value fields to `discover`. */
  virtual void contributeDiscover(DiscoverBuilder& discover) { (void)discover; }

  virtual void loop() {}
  virtual void shutdown() {}

protected:
  FeatureContext* context() const { return _ctx; }
  EventBus* events() const { return _ctx ? _ctx->events : nullptr; }
  CommandRegistry* commands() const { return _ctx ? _ctx->commands : nullptr; }
  CapabilityRegistry* capabilities() const { return _ctx ? _ctx->capabilities : nullptr; }
  FeatureManager* manager() const { return _ctx ? _ctx->manager : nullptr; }

  /** Convenience: publish without knowing subscribers. */
  bool publishEvent(const char* name, const char* kv = nullptr) {
    if (!events()) return false;
    return events()->publish(name, kv);
  }

private:
  FeatureContext* _ctx = nullptr;
};

}  // namespace mcrpc
