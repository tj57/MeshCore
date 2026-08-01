#include "Dispatcher.h"

#include <stdio.h>
#include <stdarg.h>

namespace mcrpc {

bool ReplyBuffer::printf(const char* fmt, ...) {
  char tmp[MCRPC_MAX_REPLY];
  va_list ap;
  va_start(ap, fmt);
  int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  if (n < 0) return false;
  return append(tmp);
}

bool Dispatcher::isAddressedToUs(const Request& req) const {
  switch (req.address_kind) {
    case AddressKind::All:
      return true;
    case AddressKind::Self:
      return true;
    case AddressKind::Group:
      return _group_name[0] != 0 && ieq(req.target, _group_name);
    case AddressKind::Named:
    default:
      return _node_name[0] != 0 && ieq(req.target, _node_name);
  }
}

void Dispatcher::writePrefixed(ReplyBuffer& reply, const Request& req, const char* body) {
  reply.clear();
  // Responses are emitted as body only at the transport layer for channel text;
  // prefix with request id when present so callers can correlate:
  // Spec examples show "ha#42 pong" — the transport adds the node name prefix.
  if (req.has_request_id) {
    reply.printf("#%lu ", (unsigned long)req.request_id);
  }
  reply.append(body);
}

bool Dispatcher::dispatch(const char* line, ReplyBuffer& reply) {
  reply.clear();
  Request req;
  ParseResult pr = Parser::parse(line, req);
  if (pr == ParseResult::Empty) return false;
  if (pr != ParseResult::Ok) {
    // Malformed packets addressed potentially to us — still ignore silently
    // unless we can identify the target. Spec: unknown → err unsupported only
    // for well-formed unknown commands.
    return false;
  }

  if (!isAddressedToUs(req)) return false;

  const CommandEntry* entry = _registry.find(req.command);
  if (entry == nullptr) {
    writePrefixed(reply, req, "err unknown_command");
    return true;
  }

  ReplyBuffer body;
  body.clear();
  CommandContext ctx;
  ctx.request = &req;
  ctx.reply = &body;
  ctx.user = _user;

  bool ok = entry->handler(ctx);
  if (!ok && body.len == 0) {
    body.append("err internal");
  }
  if (body.len == 0) {
    body.append("ok");
  }

  writePrefixed(reply, req, body.data);
  return true;
}

}  // namespace mcrpc
