#pragma once

#include "Dispatcher.h"
#include "FeatureManager.h"
#include "Parser.h"
#include "CommandRegistry.h"
#include "CapabilityRegistry.h"
#include "EventBus.h"
#include "StatusBuilder.h"
#include "DiscoverBuilder.h"
#include "InboundMessage.h"
#include "Config.h"

namespace mcrpc {

using PublishFn = bool (*)(const char* text, void* ctx);

/**
 * Framework facade.
 *
 * Features depend on Feature / FeatureContext / HostServices only.
 * Transport depends on handleInbound / EventBus subscription.
 */
class McRpc {
public:
  McRpc() : _dispatcher(_commands) {}

  CommandRegistry& commands() { return _commands; }
  /** @deprecated use commands() */
  CommandRegistry& registry() { return _commands; }
  CapabilityRegistry& capabilities() { return _capabilities; }
  EventBus& events() { return _events; }
  Dispatcher& dispatcher() { return _dispatcher; }
  FeatureManager& features() { return _features; }
  Config& config() { return _config; }

  void setPublishHandler(PublishFn fn, void* ctx) {
    _publish = fn;
    _publish_ctx = ctx;
  }

  void setNodeIdentity(const char* node_name, const char* group_name) {
    _node_name = node_name ? node_name : "";
    _dispatcher.setNodeName(_node_name);
    _dispatcher.setGroupName(group_name);
  }

  void setFirmwareVersion(const char* fw) { _firmware = fw ? fw : ""; }
  void setProfile(const char* profile) { _profile = profile ? profile : ""; }

  using UptimeFn = uint32_t (*)(void*);
  using RssiFn = int (*)(void*);
  void setIdentityCallbacks(UptimeFn uptime, RssiFn rssi, void* ctx) {
    _uptime_fn = uptime;
    _rssi_fn = rssi;
    _id_ctx = ctx;
  }

  void setUserData(void* user) { _dispatcher.setUserData(user); }

  void begin();
  void loop();
  void shutdown();

  bool handleIncomingText(const char* text);
  bool handleInbound(const InboundMessage& msg);
  bool publishRaw(const char* text);

  void buildStatus(StatusBuilder& status);
  void buildDiscover(DiscoverBuilder& discover);
  void buildCaps(ReplyBuffer& reply);
  void buildHelp(ReplyBuffer& reply);

private:
  static void onEventThunk(const char* name, const char* kv, void* ctx);
  void onEvent(const char* name, const char* kv);

  CommandRegistry _commands;
  CapabilityRegistry _capabilities;
  EventBus _events;
  Dispatcher _dispatcher;
  FeatureManager _features;
  Config _config;
  PublishFn _publish = nullptr;
  void* _publish_ctx = nullptr;
  const char* _node_name = "";
  const char* _profile = "";
  const char* _firmware = "";
  UptimeFn _uptime_fn = nullptr;
  RssiFn _rssi_fn = nullptr;
  void* _id_ctx = nullptr;
};

}  // namespace mcrpc
