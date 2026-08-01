#include "DisplayFeature.h"

namespace mcrpc {

static DisplayFeature* g_disp = nullptr;

void DisplayFeature::registerCommands(Registry& registry) {
  g_disp = this;
  registry.registerCommand("display", &DisplayFeature::cmdDisplay, "display status", "display");
  registry.registerCommand("text", &DisplayFeature::cmdText, "show text", "display");
  registry.registerCommand("clear", &DisplayFeature::cmdClear, "clear display", "display");
}

bool DisplayFeature::cmdDisplay(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append(g_disp && g_disp->_host.displayClear() ? "ok" : "err unsupported");
  return true;
}

bool DisplayFeature::cmdText(CommandContext& ctx) {
  if (!g_disp || ctx.request->argc < 1) {
    ctx.reply->clear();
    ctx.reply->append("err invalid_argument");
    return true;
  }
  // Join args into one string (simple: first token only for v1)
  bool ok = g_disp->_host.displayText(ctx.request->args[0]);
  ctx.reply->clear();
  ctx.reply->append(ok ? "ok" : "err unsupported");
  return true;
}

bool DisplayFeature::cmdClear(CommandContext& ctx) {
  bool ok = g_disp && g_disp->_host.displayClear();
  ctx.reply->clear();
  ctx.reply->append(ok ? "ok" : "err unsupported");
  return true;
}

}  // namespace mcrpc
