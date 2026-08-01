#include "FeatureManager.h"

namespace mcrpc {

bool FeatureManager::add(Feature* feature) {
  if (feature == nullptr) return false;
  if (_count >= MCRPC_MAX_FEATURES) return false;
  for (size_t i = 0; i < _count; i++) {
    if (_features[i] == feature) return true;
  }
  _features[_count++] = feature;
  return true;
}

void FeatureManager::beginAll() {
  for (size_t i = 0; i < _count; i++) _features[i]->begin();
}

void FeatureManager::loopAll() {
  for (size_t i = 0; i < _count; i++) _features[i]->loop();
}

void FeatureManager::registerAll(Registry& registry) {
  for (size_t i = 0; i < _count; i++) _features[i]->registerCommands(registry);
}

void FeatureManager::writeCaps(ReplyBuffer& reply) const {
  reply.clear();
  for (size_t i = 0; i < _count; i++) {
    const char* cap = _features[i]->capability();
    if (cap == nullptr || cap[0] == 0) continue;
    if (reply.len > 0) reply.appendChar('\n');
    reply.append(cap);
  }
  // Spec: capabilities MUST be one per line. If none, empty ok reply path
  // is handled by caller; leave empty here.
}

}  // namespace mcrpc
