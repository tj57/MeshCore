#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

/**
 * Pure text parser for mcRPC messages.
 *
 * Grammar (see doc/mcRPC-CORE.md):
 *   message = target [ "#" DIGITS ] SP command *(SP argument)
 *
 * Never touches hardware. Case-insensitive commands; args preserve case.
 */
class Parser {
public:
  static ParseResult parse(const char* input, Request& out);

  /** Strip MeshCore "Sender: " prefix if present; returns pointer into input. */
  static const char* stripSenderPrefix(const char* text);
};

}  // namespace mcrpc
