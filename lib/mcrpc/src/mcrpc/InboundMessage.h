#pragma once

#include "McRpcTypes.h"

namespace mcrpc {

/**
 * Transport-agnostic inbound message.
 *
 * MeshCore (or serial/BLE) unwraps framing into plain text before this.
 * Parser never sees MeshCore Packet types.
 */
struct InboundMessage {
  const char* text = nullptr;  // may include MeshCore "Sender: " prefix
  int8_t rssi = 0;             // optional metadata (0 if unknown)
  uint8_t channel_hash = 0;    // optional
  bool has_rssi = false;
};

/** Result of turning inbound text into a command object. */
struct CommandObject {
  Request request;
  ParseResult parse_result = ParseResult::Empty;
  bool addressed = false;
};

}  // namespace mcrpc
