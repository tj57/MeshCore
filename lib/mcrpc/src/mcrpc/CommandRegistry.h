#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

/**
 * Command registry — parser/dispatcher look up handlers here.
 * Capabilities are NOT stored here (see CapabilityRegistry).
 */
class CommandRegistry {
public:
  bool registerCommand(const char* name, CommandHandler handler, const char* help = nullptr);

  const CommandEntry* find(const char* name) const;

  size_t count() const { return _count; }
  const CommandEntry* at(size_t i) const {
    return i < _count ? &_entries[i] : nullptr;
  }

  void clear() { _count = 0; }

private:
  CommandEntry _entries[MCRPC_MAX_COMMANDS];
  size_t _count = 0;
};

/** Back-compat alias used during migration. */
using Registry = CommandRegistry;

}  // namespace mcrpc
