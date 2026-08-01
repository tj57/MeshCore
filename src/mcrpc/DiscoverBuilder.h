#pragma once

#include "McRpcTypes.h"
#include <stdio.h>

namespace mcrpc {

#ifndef MCRPC_MAX_DISCOVER_FIELDS
#define MCRPC_MAX_DISCOVER_FIELDS 16
#endif

/**
 * Assembles `<name> profile=… fw=… [feature contributions]`.
 */
class DiscoverBuilder {
public:
  void reset() { _count = 0; _name[0] = 0; }

  void setNodeName(const char* name) {
    if (name) copyToken(_name, sizeof(_name), name, strlen(name));
    else _name[0] = 0;
  }

  bool add(const char* key, const char* value) {
    if (!key || !value || _count >= MCRPC_MAX_DISCOVER_FIELDS) return false;
    _keys[_count] = key;
    copyToken(_vals[_count], sizeof(_vals[_count]), value, strlen(value));
    _count++;
    return true;
  }

  bool add(const char* key, int value) {
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%d", value);
    return add(key, tmp);
  }

  void writeTo(ReplyBuffer& reply) const {
    reply.clear();
    reply.append(_name[0] ? _name : "node");
    for (size_t i = 0; i < _count; i++) {
      reply.appendChar(' ');
      reply.append(_keys[i]);
      reply.appendChar('=');
      reply.append(_vals[i]);
    }
  }

  size_t fieldCount() const { return _count; }

private:
  char _name[MCRPC_MAX_TOKEN];
  const char* _keys[MCRPC_MAX_DISCOVER_FIELDS];
  char _vals[MCRPC_MAX_DISCOVER_FIELDS][MCRPC_MAX_TOKEN];
  size_t _count = 0;
};

}  // namespace mcrpc
