#include "FeatureManager.h"

namespace mcrpc {

bool FeatureManager::add(Feature* feature) {
  if (feature == nullptr) return false;
  if (_started) return false;  // immutable after start
  if (_count >= MCRPC_MAX_FEATURES) return false;
  for (size_t i = 0; i < _count; i++) {
    if (_features[i] == feature) return true;
  }
  _features[_count++] = feature;
  return true;
}

void FeatureManager::start(CommandRegistry& commands, CapabilityRegistry& capabilities,
                           EventBus& events) {
  if (_started) return;
  _caps = &capabilities;
  _ctx.commands = &commands;
  _ctx.capabilities = &capabilities;
  _ctx.events = &events;
  _ctx.manager = this;

  for (size_t i = 0; i < _count; i++) {
    _features[i]->setup(_ctx);
  }
  for (size_t i = 0; i < _count; i++) {
    _features[i]->registerCommands(commands);
  }
  for (size_t i = 0; i < _count; i++) {
    _features[i]->registerCapabilities(capabilities);
  }
  _started = true;
}

void FeatureManager::loop() {
  if (!_started) return;
  for (size_t i = 0; i < _count; i++) _features[i]->loop();
}

void FeatureManager::stop() {
  if (!_started) return;
  for (size_t i = _count; i > 0; i--) {
    _features[i - 1]->shutdown();
  }
  _started = false;
}

void FeatureManager::collectStatus(StatusBuilder& status) const {
  for (size_t i = 0; i < _count; i++) {
    _features[i]->contributeStatus(status);
  }
}

void FeatureManager::collectDiscover(DiscoverBuilder& discover) const {
  for (size_t i = 0; i < _count; i++) {
    _features[i]->contributeDiscover(discover);
  }
}

void FeatureManager::writeCapabilities(ReplyBuffer& reply) const {
  if (_caps) _caps->writeTo(reply);
  else reply.clear();
}

void FeatureManager::writeHelp(ReplyBuffer& reply, const CommandRegistry& commands) const {
  reply.clear();
  for (size_t i = 0; i < commands.count(); i++) {
    const CommandEntry* e = commands.at(i);
    if (!e) continue;
    if (reply.len > 0) reply.appendChar(' ');
    reply.append(e->name);
  }
}

}  // namespace mcrpc
