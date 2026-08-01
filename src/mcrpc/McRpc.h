#pragma once

#include "Dispatcher.h"
#include "FeatureManager.h"
#include "Parser.h"
#include "Registry.h"
#include "Config.h"

namespace mcrpc {

/**
 * Facade: Parser → Dispatcher → Registry → Handlers.
 *
 * Event publishing goes through a callback so the transport layer can
 * flood group text without the core knowing about MeshCore.
 */
using PublishFn = bool (*)(const char* text, void* ctx);

class McRpc {
public:
  McRpc() : _dispatcher(_registry) {}

  Registry& registry() { return _registry; }
  Dispatcher& dispatcher() { return _dispatcher; }
  FeatureManager& features() { return _features; }
  Config& config() { return _config; }

  void setPublishHandler(PublishFn fn, void* ctx) {
    _publish = fn;
    _publish_ctx = ctx;
  }

  void setNodeIdentity(const char* node_name, const char* group_name) {
    _dispatcher.setNodeName(node_name);
    _dispatcher.setGroupName(group_name);
  }

  void setUserData(void* user) { _dispatcher.setUserData(user); }

  void begin() {
    _features.registerAll(_registry);
    _features.beginAll();
  }

  void loop() { _features.loopAll(); }

  /**
   * Handle inbound channel text (MeshCore "Sender: body" or raw mcRPC line).
   * If a reply is produced, publishes it via PublishFn.
   */
  bool handleIncomingText(const char* text) {
    const char* line = Parser::stripSenderPrefix(text);
    ReplyBuffer reply;
    if (!_dispatcher.dispatch(line, reply)) return false;
    if (_publish) return _publish(reply.data, _publish_ctx);
    return true;
  }

  /** Publish an asynchronous event: "event <name> [key=value ...]" */
  bool publishEvent(const char* event_name, const char* kv = nullptr) {
    ReplyBuffer buf;
    buf.append("event ");
    buf.append(event_name);
    if (kv && kv[0]) {
      buf.appendChar(' ');
      buf.append(kv);
    }
    if (_publish) return _publish(buf.data, _publish_ctx);
    return false;
  }

  bool publishRaw(const char* text) {
    if (_publish) return _publish(text, _publish_ctx);
    return false;
  }

private:
  Registry _registry;
  Dispatcher _dispatcher;
  FeatureManager _features;
  Config _config;
  PublishFn _publish = nullptr;
  void* _publish_ctx = nullptr;
};

}  // namespace mcrpc
