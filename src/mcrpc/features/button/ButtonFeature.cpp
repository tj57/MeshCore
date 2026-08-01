#include "ButtonFeature.h"

#include <stdio.h>

namespace mcrpc {

static ButtonFeature* g_btn = nullptr;

void ButtonFeature::registerCommands(Registry& registry) {
  g_btn = this;
  registry.registerCommand("button", &ButtonFeature::cmdButton, "button info", "button");
  registry.registerCommand("button_state", &ButtonFeature::cmdButtonState, "pressed?", "button");
}

void ButtonFeature::notifyPressed() {
  _pressed = true;
  _press_count++;
  if (_host.engine) {
    char kv[40];
    snprintf(kv, sizeof(kv), "count=%lu", (unsigned long)_press_count);
    _host.engine->publishEvent("button_pressed", kv);
  }
  _pressed = false;
}

bool ButtonFeature::cmdButton(CommandContext& ctx) {
  if (!g_btn) return false;
  ctx.reply->clear();
  ctx.reply->printf("button count=%lu", (unsigned long)g_btn->_press_count);
  return true;
}

bool ButtonFeature::cmdButtonState(CommandContext& ctx) {
  if (!g_btn) return false;
  ctx.reply->clear();
  ctx.reply->printf("button_state value=%s", g_btn->_host.readButtonPressed() ? "1" : "0");
  return true;
}

}  // namespace mcrpc
