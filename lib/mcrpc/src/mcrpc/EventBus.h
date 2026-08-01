#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

#ifndef MCRPC_MAX_EVENT_SUBS
#define MCRPC_MAX_EVENT_SUBS 8
#endif

/**
 * Internal event bus. Features publish; they never know the consumers.
 *
 * Default subscriber (wired by McRpc) formats `event <name> [kv]` for mesh.
 * Future: HA bridge, logger, display, BLE, storage.
 */
class EventBus {
public:
  using Handler = void (*)(const char* event_name, const char* kv, void* ctx);

  bool subscribe(Handler handler, void* ctx);
  bool publish(const char* event_name, const char* kv = nullptr);

  size_t subscriberCount() const { return _count; }
  void clear() { _count = 0; }

private:
  struct Sub {
    Handler handler;
    void* ctx;
  };
  Sub _subs[MCRPC_MAX_EVENT_SUBS];
  size_t _count = 0;
};

}  // namespace mcrpc
