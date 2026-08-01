#pragma once

#include "McRpcTypes.h"
#include <stdio.h>

namespace mcrpc {

#ifndef MCRPC_MAX_STATUS_FIELDS
#define MCRPC_MAX_STATUS_FIELDS 16
#endif

/**
 * Assembles `status key=value ...` from framework + feature contributions.
 */
class StatusBuilder {
public:
  void reset() {
    _count = 0;
    _buf.clear();
  }

  bool add(const char* key, const char* value) {
    if (!key || !value || _count >= MCRPC_MAX_STATUS_FIELDS) return false;
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

  bool add(const char* key, long value) {
    char tmp[24];
    snprintf(tmp, sizeof(tmp), "%ld", value);
    return add(key, tmp);
  }

  bool add(const char* key, unsigned long value) {
    char tmp[24];
    snprintf(tmp, sizeof(tmp), "%lu", value);
    return add(key, tmp);
  }

  bool add(const char* key, float value, int decimals = 2) {
    char tmp[24];
    if (decimals <= 0) snprintf(tmp, sizeof(tmp), "%.0f", (double)value);
    else if (decimals == 1) snprintf(tmp, sizeof(tmp), "%.1f", (double)value);
    else snprintf(tmp, sizeof(tmp), "%.2f", (double)value);
    return add(key, tmp);
  }

  /** Write final protocol line into reply (clears reply first). */
  void writeTo(ReplyBuffer& reply) const {
    reply.clear();
    reply.append("status");
    for (size_t i = 0; i < _count; i++) {
      reply.appendChar(' ');
      reply.append(_keys[i]);
      reply.appendChar('=');
      reply.append(_vals[i]);
    }
  }

  size_t fieldCount() const { return _count; }
  const char* keyAt(size_t i) const { return i < _count ? _keys[i] : nullptr; }
  const char* valueAt(size_t i) const { return i < _count ? _vals[i] : nullptr; }

private:
  const char* _keys[MCRPC_MAX_STATUS_FIELDS];
  char _vals[MCRPC_MAX_STATUS_FIELDS][MCRPC_MAX_TOKEN];
  size_t _count = 0;
  ReplyBuffer _buf;  // unused scratch reserved for future
};

}  // namespace mcrpc
