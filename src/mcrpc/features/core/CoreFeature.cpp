#include "CoreFeature.h"

#include "../../McRpc.h"
#include <stdio.h>

namespace mcrpc {

static CoreFeature* g_core = nullptr;

void CoreFeature::registerCommands(Registry& registry) {
  g_core = this;
  registry.registerCommand("ping", &CoreFeature::cmdPing, "connectivity test", nullptr);
  registry.registerCommand("status", &CoreFeature::cmdStatus, "node status", nullptr);
  registry.registerCommand("discover", &CoreFeature::cmdDiscover, "discovery info", nullptr);
  registry.registerCommand("help", &CoreFeature::cmdHelp, "list commands", nullptr);
  registry.registerCommand("caps", &CoreFeature::cmdCaps, "list capabilities", nullptr);
}

bool CoreFeature::cmdPing(CommandContext& ctx) {
  (void)ctx;
  ctx.reply->clear();
  ctx.reply->append("pong");
  return true;
}

bool CoreFeature::cmdStatus(CommandContext& ctx) {
  if (!g_core) return false;
  float volts = 0;
  int pct = -1;
  bool has_batt = g_core->_host.readBattery(volts, pct);

  ctx.reply->clear();
  ctx.reply->printf(
      "status name=%s profile=%s fw=%s uptime=%lu rssi=%d",
      g_core->_host.nodeName(), g_core->_host.profile(), g_core->_host.firmwareVersion(),
      (unsigned long)g_core->_host.uptimeSeconds(), g_core->_host.rssi());
  if (has_batt) {
    ctx.reply->printf(" voltage=%.2f", (double)volts);
    if (pct >= 0) ctx.reply->printf(" battery=%d", pct);
  }
  return true;
}

bool CoreFeature::cmdDiscover(CommandContext& ctx) {
  if (!g_core) return false;
  ctx.reply->clear();
  ctx.reply->printf("%s profile=%s fw=%s", g_core->_host.nodeName(), g_core->_host.profile(),
                    g_core->_host.firmwareVersion());
  return true;
}

bool CoreFeature::cmdHelp(CommandContext& ctx) {
  if (!g_core) return false;
  ctx.reply->clear();
  if (g_core->_host.engine == nullptr) {
    ctx.reply->append("ping status discover help caps");
    return true;
  }
  Registry& reg = g_core->_host.engine->registry();
  for (size_t i = 0; i < reg.count(); i++) {
    const CommandEntry* e = reg.at(i);
    if (!e) continue;
    if (ctx.reply->len > 0) ctx.reply->appendChar(' ');
    ctx.reply->append(e->name);
  }
  return true;
}

bool CoreFeature::cmdCaps(CommandContext& ctx) {
  if (!g_core) return false;
  ctx.reply->clear();
  if (g_core->_host.engine != nullptr) {
    g_core->_host.engine->features().writeCaps(*ctx.reply);
    if (ctx.reply->len > 0) return true;
    const char* caps[MCRPC_MAX_CAPS];
    size_t n = g_core->_host.engine->registry().collectCapabilities(caps, MCRPC_MAX_CAPS);
    for (size_t i = 0; i < n; i++) {
      if (ctx.reply->len > 0) ctx.reply->appendChar('\n');
      ctx.reply->append(caps[i]);
    }
  }
  return true;
}

}  // namespace mcrpc
