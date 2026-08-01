#include "CoreFeature.h"

#include "../../McRpc.h"

namespace mcrpc {

static CoreFeature* g_core = nullptr;

void CoreFeature::registerCommands(CommandRegistry& commands) {
  g_core = this;
  commands.registerCommand("ping", &CoreFeature::cmdPing, "connectivity test");
  commands.registerCommand("status", &CoreFeature::cmdStatus, "node status");
  commands.registerCommand("discover", &CoreFeature::cmdDiscover, "discovery info");
  commands.registerCommand("help", &CoreFeature::cmdHelp, "list commands");
  commands.registerCommand("caps", &CoreFeature::cmdCaps, "list capabilities");
}

bool CoreFeature::cmdPing(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append("pong");
  return true;
}

bool CoreFeature::cmdStatus(CommandContext& ctx) {
  if (!g_core || !g_core->_host.engine) return false;
  StatusBuilder status;
  g_core->_host.engine->buildStatus(status);
  status.writeTo(*ctx.reply);
  return true;
}

bool CoreFeature::cmdDiscover(CommandContext& ctx) {
  if (!g_core || !g_core->_host.engine) return false;
  DiscoverBuilder discover;
  g_core->_host.engine->buildDiscover(discover);
  discover.writeTo(*ctx.reply);
  return true;
}

bool CoreFeature::cmdHelp(CommandContext& ctx) {
  if (!g_core || !g_core->_host.engine) {
    ctx.reply->clear();
    ctx.reply->append("ping status discover help caps");
    return true;
  }
  g_core->_host.engine->buildHelp(*ctx.reply);
  return true;
}

bool CoreFeature::cmdCaps(CommandContext& ctx) {
  if (!g_core || !g_core->_host.engine) {
    ctx.reply->clear();
    return true;
  }
  g_core->_host.engine->buildCaps(*ctx.reply);
  return true;
}

}  // namespace mcrpc
