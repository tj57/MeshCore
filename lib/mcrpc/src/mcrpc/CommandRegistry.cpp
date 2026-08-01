#include "CommandRegistry.h"

namespace mcrpc {

bool CommandRegistry::registerCommand(const char* name, CommandHandler handler, const char* help) {
  if (name == nullptr || handler == nullptr) return false;
  if (_count >= MCRPC_MAX_COMMANDS) return false;
  for (size_t i = 0; i < _count; i++) {
    if (ieq(_entries[i].name, name)) {
      _entries[i].handler = handler;
      _entries[i].help = help;
      return true;
    }
  }
  _entries[_count].name = name;
  _entries[_count].handler = handler;
  _entries[_count].help = help;
  _entries[_count].capability = nullptr;
  _count++;
  return true;
}

const CommandEntry* CommandRegistry::find(const char* name) const {
  if (name == nullptr) return nullptr;
  for (size_t i = 0; i < _count; i++) {
    if (ieq(_entries[i].name, name)) return &_entries[i];
  }
  return nullptr;
}

}  // namespace mcrpc
