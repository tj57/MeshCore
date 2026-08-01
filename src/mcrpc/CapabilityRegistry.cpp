#include "CapabilityRegistry.h"

namespace mcrpc {

bool CapabilityRegistry::registerCapability(const char* name) {
  if (name == nullptr || name[0] == 0) return false;
  if (_count >= MCRPC_MAX_CAPS) return false;
  for (size_t i = 0; i < _count; i++) {
    if (ieq(_names[i], name)) return true;
  }
  _names[_count++] = name;
  return true;
}

void CapabilityRegistry::writeTo(ReplyBuffer& reply) const {
  reply.clear();
  for (size_t i = 0; i < _count; i++) {
    if (reply.len > 0) reply.appendChar('\n');
    reply.append(_names[i]);
  }
}

}  // namespace mcrpc
