#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

/** Dedicated capability registry — `caps` is built only from this. */
class CapabilityRegistry {
public:
  bool registerCapability(const char* name);

  size_t count() const { return _count; }
  const char* at(size_t i) const { return i < _count ? _names[i] : nullptr; }

  /** One capability per line into reply. */
  void writeTo(ReplyBuffer& reply) const;

  void clear() { _count = 0; }

private:
  const char* _names[MCRPC_MAX_CAPS];
  size_t _count = 0;
};

}  // namespace mcrpc
