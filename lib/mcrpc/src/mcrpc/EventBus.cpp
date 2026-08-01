#include "EventBus.h"

namespace mcrpc {

bool EventBus::subscribe(Handler handler, void* ctx) {
  if (handler == nullptr) return false;
  if (_count >= MCRPC_MAX_EVENT_SUBS) return false;
  _subs[_count].handler = handler;
  _subs[_count].ctx = ctx;
  _count++;
  return true;
}

bool EventBus::publish(const char* event_name, const char* kv) {
  if (event_name == nullptr || event_name[0] == 0) return false;
  bool any = false;
  for (size_t i = 0; i < _count; i++) {
    if (_subs[i].handler) {
      _subs[i].handler(event_name, kv, _subs[i].ctx);
      any = true;
    }
  }
  return any;
}

}  // namespace mcrpc
