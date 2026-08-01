#pragma once

/**
 * Build outbound mcRPC request/response/event lines (transport-independent).
 * Used by firmware, desktop tools, and Home Assistant integrations.
 */

#include "McRpcTypes.h"
#include <stdio.h>
#include <stdarg.h>

namespace mcrpc {

class OutboundBuilder {
public:
  /** `target[#id] command [args...]` */
  static bool request(ReplyBuffer& out, const char* target, const char* command,
                      bool has_id = false, uint32_t request_id = 0) {
    if (!target || !command) return false;
    out.clear();
    out.append(target);
    if (has_id) {
      char id[16];
      snprintf(id, sizeof(id), "#%lu", (unsigned long)request_id);
      out.append(id);
    }
    out.appendChar(' ');
    return out.append(command);
  }

  static bool requestWithArgs(ReplyBuffer& out, const char* target, const char* command,
                              int argc, const char* const* argv, bool has_id = false,
                              uint32_t request_id = 0) {
    if (!request(out, target, command, has_id, request_id)) return false;
    for (int i = 0; i < argc; i++) {
      if (!argv[i]) continue;
      out.appendChar(' ');
      if (!out.append(argv[i])) return false;
    }
    return true;
  }

  /** `event <name> [kv]` */
  static bool event(ReplyBuffer& out, const char* name, const char* kv = nullptr) {
    if (!name) return false;
    out.clear();
    out.append("event ");
    out.append(name);
    if (kv && kv[0]) {
      out.appendChar(' ');
      out.append(kv);
    }
    return true;
  }

  /** `err <code>` */
  static bool error(ReplyBuffer& out, const char* code) {
    out.clear();
    out.append("err ");
    return out.append(code ? code : "internal");
  }

  /** `ok` or `ok <detail>` */
  static bool ok(ReplyBuffer& out, const char* detail = nullptr) {
    out.clear();
    out.append("ok");
    if (detail && detail[0]) {
      out.appendChar(' ');
      out.append(detail);
    }
    return true;
  }
};

}  // namespace mcrpc
