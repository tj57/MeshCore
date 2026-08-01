#pragma once

#include "McRpcTypes.h"
#include "Registry.h"

namespace mcrpc {

class McRpc;  // forward

/**
 * Feature interface — each feature owns its commands, events, and docs.
 * Features never parse raw mesh packets; they only register handlers.
 */
class Feature {
public:
  virtual ~Feature() {}

  virtual const char* name() const = 0;
  virtual const char* capability() const { return name(); }

  /** Register commands into the shared registry. */
  virtual void registerCommands(Registry& registry) = 0;

  /** Optional periodic work (GPS polling, button debounce helpers, etc.). */
  virtual void loop() {}

  virtual void begin() {}
};

}  // namespace mcrpc
