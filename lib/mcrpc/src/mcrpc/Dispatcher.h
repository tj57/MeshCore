#pragma once

#include "McRpcTypes.h"
#include "CommandRegistry.h"
#include "Parser.h"

namespace mcrpc {

/**
 * Addressing policy + registry lookup.
 * Does not know about radios, boards, or MeshCore packets.
 */
class Dispatcher {
public:
  explicit Dispatcher(CommandRegistry& registry) : _registry(registry) {}

  void setNodeName(const char* name) { _node_name = name ? name : ""; }
  void setGroupName(const char* name) { _group_name = name ? name : ""; }
  void setUserData(void* user) { _user = user; }

  /**
   * Process one inbound mcRPC line (already stripped of MeshCore sender prefix).
   * Returns true if a reply was written into `reply` (caller must transmit).
   * Returns false if the message was ignored (not addressed to us / empty).
   *
   * Dispatcher knows only the CommandRegistry — never Feature types.
   */
  bool dispatch(const char* line, ReplyBuffer& reply);

  CommandRegistry& registry() { return _registry; }

private:
  bool isAddressedToUs(const Request& req) const;
  void writePrefixed(ReplyBuffer& reply, const Request& req, const char* body);

  CommandRegistry& _registry;
  const char* _node_name = "";
  const char* _group_name = "";
  void* _user = nullptr;
};

}  // namespace mcrpc
