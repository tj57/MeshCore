#include "McRpc.h"

#include <stdio.h>

namespace mcrpc {

void McRpc::begin() {
  _events.subscribe(&McRpc::onEventThunk, this);
  _features.start(_commands, _capabilities, _events);
}

void McRpc::loop() { _features.loop(); }

void McRpc::shutdown() {
  _features.stop();
  _events.clear();
}

bool McRpc::handleIncomingText(const char* text) {
  InboundMessage msg;
  msg.text = text;
  return handleInbound(msg);
}

bool McRpc::handleInbound(const InboundMessage& msg) {
  if (msg.text == nullptr) return false;
  const char* line = Parser::stripSenderPrefix(msg.text);
  ReplyBuffer reply;
  if (!_dispatcher.dispatch(line, reply)) return false;
  return publishRaw(reply.data);
}

bool McRpc::publishRaw(const char* text) {
  if (!text || !text[0]) return false;
  if (_publish) return _publish(text, _publish_ctx);
  return false;
}

void McRpc::buildStatus(StatusBuilder& status) {
  status.reset();
  status.add("name", _node_name);
  status.add("profile", _profile);
  status.add("fw", _firmware);
  if (_uptime_fn) status.add("uptime", (unsigned long)_uptime_fn(_id_ctx));
  if (_rssi_fn) status.add("rssi", _rssi_fn(_id_ctx));
  _features.collectStatus(status);
}

void McRpc::buildDiscover(DiscoverBuilder& discover) {
  discover.reset();
  discover.setNodeName(_node_name);
  discover.add("profile", _profile);
  discover.add("fw", _firmware);
  _features.collectDiscover(discover);
}

void McRpc::buildCaps(ReplyBuffer& reply) { _features.writeCapabilities(reply); }

void McRpc::buildHelp(ReplyBuffer& reply) { _features.writeHelp(reply, _commands); }

void McRpc::onEventThunk(const char* name, const char* kv, void* ctx) {
  static_cast<McRpc*>(ctx)->onEvent(name, kv);
}

void McRpc::onEvent(const char* name, const char* kv) {
  ReplyBuffer buf;
  buf.clear();
  buf.append("event ");
  buf.append(name);
  if (kv && kv[0]) {
    buf.appendChar(' ');
    buf.append(kv);
  }
  publishRaw(buf.data);
}

}  // namespace mcrpc
