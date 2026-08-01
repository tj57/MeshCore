#include "Registry.h"

namespace mcrpc {

bool Registry::registerCommand(const char* name, CommandHandler handler,
                               const char* help, const char* capability) {
  if (name == nullptr || handler == nullptr) return false;
  if (_count >= MCRPC_MAX_COMMANDS) return false;
  // Replace if already registered (allows feature re-init)
  for (size_t i = 0; i < _count; i++) {
    if (ieq(_entries[i].name, name)) {
      _entries[i].handler = handler;
      _entries[i].help = help;
      _entries[i].capability = capability;
      return true;
    }
  }
  _entries[_count].name = name;
  _entries[_count].handler = handler;
  _entries[_count].help = help;
  _entries[_count].capability = capability;
  _count++;
  return true;
}

const CommandEntry* Registry::find(const char* name) const {
  if (name == nullptr) return nullptr;
  for (size_t i = 0; i < _count; i++) {
    if (ieq(_entries[i].name, name)) return &_entries[i];
  }
  return nullptr;
}

size_t Registry::collectCapabilities(const char* dest[], size_t max) const {
  size_t n = 0;
  for (size_t i = 0; i < _count && n < max; i++) {
    const char* cap = _entries[i].capability;
    if (cap == nullptr || cap[0] == 0) continue;
    bool dup = false;
    for (size_t j = 0; j < n; j++) {
      if (ieq(dest[j], cap)) {
        dup = true;
        break;
      }
    }
    if (!dup) dest[n++] = cap;
  }
  return n;
}

}  // namespace mcrpc
