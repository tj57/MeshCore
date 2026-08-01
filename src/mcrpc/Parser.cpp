#include "Parser.h"

#include <ctype.h>
#include <stdlib.h>

namespace mcrpc {

static void skipSpaces(const char*& p) {
  while (*p == ' ' || *p == '\t') ++p;
}

static bool isIdentChar(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || c == '_' || c == '-';
}

static bool readToken(const char*& p, char* dest, size_t dest_size, bool ident_only) {
  skipSpaces(p);
  if (*p == 0) return false;
  const char* start = p;
  if (ident_only) {
    while (isIdentChar(*p)) ++p;
  } else {
    while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') ++p;
  }
  if (p == start) return false;
  copyToken(dest, dest_size, start, (size_t)(p - start));
  return true;
}

const char* Parser::stripSenderPrefix(const char* text) {
  if (text == nullptr) return "";
  const char* colon = strchr(text, ':');
  if (colon == nullptr) return text;
  // Heuristic: MeshCore channel text is "Name: payload"
  // Only strip if the prefix looks like a short sender name (no spaces before colon,
  // and something follows).
  const char* p = text;
  while (p < colon) {
    if (*p == ' ' || *p == '\t') return text;
    ++p;
  }
  const char* msg = colon + 1;
  while (*msg == ' ' || *msg == '\t') ++msg;
  return msg;
}

ParseResult Parser::parse(const char* input, Request& out) {
  memset(&out, 0, sizeof(out));
  if (input == nullptr) return ParseResult::Empty;

  const char* p = input;
  skipSpaces(p);
  // trim trailing whitespace (work on a local view)
  while (*p == '\r' || *p == '\n') ++p;
  if (*p == 0) return ParseResult::Empty;

  // target (may be group:name)
  skipSpaces(p);
  if (*p == 0) return ParseResult::MissingTarget;

  if (strncmp(p, "group:", 6) == 0 || strncmp(p, "GROUP:", 6) == 0) {
    out.address_kind = AddressKind::Group;
    p += 6;
    if (!readToken(p, out.target, sizeof(out.target), true)) {
      return ParseResult::Malformed;
    }
  } else {
    if (!readToken(p, out.target, sizeof(out.target), true)) {
      return ParseResult::Malformed;
    }
    if (ieq(out.target, "all")) {
      out.address_kind = AddressKind::All;
    } else if (ieq(out.target, "self")) {
      out.address_kind = AddressKind::Self;
    } else {
      out.address_kind = AddressKind::Named;
    }
  }

  // optional #request_id glued to target OR as separate token after target
  // Spec: target [request] SP command  where request = "#" 1*DIGIT
  // In human typing this appears as "ha#42 ping" (no space) which we already
  // partially handled if # is part of the token — re-parse.
  {
    char* hash = strchr(out.target, '#');
    if (hash != nullptr && hash[1] != 0) {
      bool all_digits = true;
      for (const char* d = hash + 1; *d; ++d) {
        if (!isdigit((unsigned char)*d)) {
          all_digits = false;
          break;
        }
      }
      if (all_digits) {
        out.has_request_id = true;
        out.request_id = (uint32_t)strtoul(hash + 1, nullptr, 10);
        *hash = 0;
      }
    }
  }

  // Also accept "ha #42 ping"
  skipSpaces(p);
  if (*p == '#') {
    ++p;
    char idbuf[16];
    size_t i = 0;
    while (isdigit((unsigned char)*p) && i + 1 < sizeof(idbuf)) {
      idbuf[i++] = *p++;
    }
    idbuf[i] = 0;
    if (i == 0) return ParseResult::Malformed;
    out.has_request_id = true;
    out.request_id = (uint32_t)strtoul(idbuf, nullptr, 10);
  }

  if (!readToken(p, out.command, sizeof(out.command), true)) {
    return ParseResult::MissingCommand;
  }

  // lowercase command for registry lookup convenience (store lower)
  for (char* c = out.command; *c; ++c) {
    if (*c >= 'A' && *c <= 'Z') *c = (char)(*c - 'A' + 'a');
  }

  out.argc = 0;
  while (out.argc < MCRPC_MAX_ARGS) {
    if (!readToken(p, out.args[out.argc], sizeof(out.args[out.argc]), false)) break;
    out.argc++;
  }

  return ParseResult::Ok;
}

}  // namespace mcrpc
