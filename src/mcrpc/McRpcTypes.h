#pragma once

/**
 * mcRPC core types — hardware-agnostic.
 *
 * Protocol: doc/mcRPC-CORE.md (derived from mcRPC Specification v1).
 * No Arduino / MeshCore / board includes here.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace mcrpc {

#ifndef MCRPC_MAX_ARGS
#define MCRPC_MAX_ARGS 8
#endif

#ifndef MCRPC_MAX_TOKEN
#define MCRPC_MAX_TOKEN 48
#endif

#ifndef MCRPC_MAX_REPLY
#define MCRPC_MAX_REPLY 192
#endif

#ifndef MCRPC_MAX_COMMANDS
#define MCRPC_MAX_COMMANDS 48
#endif

#ifndef MCRPC_MAX_FEATURES
#define MCRPC_MAX_FEATURES 16
#endif

#ifndef MCRPC_MAX_CAPS
#define MCRPC_MAX_CAPS 24
#endif

enum class ParseResult : uint8_t {
  Ok = 0,
  Empty,
  MissingTarget,
  MissingCommand,
  Malformed,
};

enum class AddressKind : uint8_t {
  Named = 0,
  All,
  Self,
  Group,
};

struct Request {
  AddressKind address_kind;
  char target[MCRPC_MAX_TOKEN];
  bool has_request_id;
  uint32_t request_id;
  char command[MCRPC_MAX_TOKEN];
  char args[MCRPC_MAX_ARGS][MCRPC_MAX_TOKEN];
  uint8_t argc;
};

struct ReplyBuffer {
  char data[MCRPC_MAX_REPLY] = {0};
  size_t len = 0;

  void clear() {
    data[0] = 0;
    len = 0;
  }

  bool append(const char* s) {
    if (s == nullptr) return false;
    size_t n = strlen(s);
    if (len + n >= sizeof(data)) return false;
    memcpy(data + len, s, n);
    len += n;
    data[len] = 0;
    return true;
  }

  bool appendChar(char c) {
    if (len + 1 >= sizeof(data)) return false;
    data[len++] = c;
    data[len] = 0;
    return true;
  }

  bool printf(const char* fmt, ...);
};

/** Context passed to every command handler. */
struct CommandContext {
  const Request* request;
  ReplyBuffer* reply;
  void* user;  // optional app/feature private pointer
};

using CommandHandler = bool (*)(CommandContext& ctx);

struct CommandEntry {
  const char* name;
  CommandHandler handler;
  const char* help;       // short help string (may be null)
  const char* capability; // caps line (may be null); core cmds omit
};

inline bool ieq(const char* a, const char* b) {
  if (a == nullptr || b == nullptr) return a == b;
  while (*a && *b) {
    char ca = *a, cb = *b;
    if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
    if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
    if (ca != cb) return false;
    ++a;
    ++b;
  }
  return *a == 0 && *b == 0;
}

inline void copyToken(char* dest, size_t dest_size, const char* src, size_t n) {
  if (dest_size == 0) return;
  if (n >= dest_size) n = dest_size - 1;
  memcpy(dest, src, n);
  dest[n] = 0;
}

}  // namespace mcrpc
