#pragma once

#include "Feature.h"
#include "Registry.h"

namespace mcrpc {

class FeatureManager {
public:
  bool add(Feature* feature);
  void beginAll();
  void loopAll();
  void registerAll(Registry& registry);

  size_t count() const { return _count; }
  Feature* at(size_t i) const { return i < _count ? _features[i] : nullptr; }

  /** Write capability list into reply (one per line). */
  void writeCaps(ReplyBuffer& reply) const;

private:
  Feature* _features[MCRPC_MAX_FEATURES];
  size_t _count = 0;
};

}  // namespace mcrpc
