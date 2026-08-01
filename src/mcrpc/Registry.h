#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

/**
 * Command registry — O(n) linear lookup by design.
 * Firmware typically registers < 32 commands; hash tables are overkill
 * and harder to keep deterministic on MCUs.
 */
class Registry {
public:
  bool registerCommand(const char* name, CommandHandler handler,
                       const char* help = nullptr, const char* capability = nullptr);

  const CommandEntry* find(const char* name) const;

  size_t count() const { return _count; }
  const CommandEntry* at(size_t i) const {
    return i < _count ? &_entries[i] : nullptr;
  }

  /** Append capability names (unique) into dest lines; returns count written. */
  size_t collectCapabilities(const char* dest[], size_t max) const;

private:
  CommandEntry _entries[MCRPC_MAX_COMMANDS];
  size_t _count = 0;
};

}  // namespace mcrpc
