/* Expanded host tests for the mcRPC Feature SDK framework. */
#include <mcrpc/Parser.h>
#include <mcrpc/CommandRegistry.h>
#include <mcrpc/CapabilityRegistry.h>
#include <mcrpc/Dispatcher.h>
#include <mcrpc/FeatureManager.h>
#include <mcrpc/EventBus.h>
#include <mcrpc/StatusBuilder.h>
#include <mcrpc/DiscoverBuilder.h>
#include <mcrpc/InboundMessage.h>
#include <mcrpc/McRpc.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace mcrpc;

static int g_fail = 0;
static int g_pass = 0;

#define EXPECT(...) do { \
  if (!(__VA_ARGS__)) { \
    std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #__VA_ARGS__); \
    g_fail++; \
  } else { g_pass++; } \
} while (0)

static bool hPing(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append("pong");
  return true;
}

static bool hEcho(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append("echo");
  if (ctx.request->argc > 0) {
    ctx.reply->appendChar(' ');
    ctx.reply->append(ctx.request->args[0]);
  }
  return true;
}

/* -------------------- Parser -------------------- */

static void test_parser_basics() {
  Request r;
  EXPECT(Parser::parse(nullptr, r) == ParseResult::Empty);
  EXPECT(Parser::parse("", r) == ParseResult::Empty);
  EXPECT(Parser::parse("   \t  ", r) == ParseResult::Empty);
  EXPECT(Parser::parse("\n", r) == ParseResult::Empty);

  EXPECT(Parser::parse("ha ping", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Named);
  EXPECT(std::strcmp(r.target, "ha") == 0);
  EXPECT(std::strcmp(r.command, "ping") == 0);
  EXPECT(!r.has_request_id);
  EXPECT(r.argc == 0);
}

static void test_parser_request_ids() {
  Request r;
  EXPECT(Parser::parse("ha#42 ping", r) == ParseResult::Ok);
  EXPECT(r.has_request_id);
  EXPECT(r.request_id == 42);
  EXPECT(std::strcmp(r.target, "ha") == 0);

  EXPECT(Parser::parse("tracker #99 status", r) == ParseResult::Ok);
  EXPECT(r.has_request_id);
  EXPECT(r.request_id == 99);

  EXPECT(Parser::parse("ha#0 ping", r) == ParseResult::Ok);
  EXPECT(r.has_request_id);
  EXPECT(r.request_id == 0);
}

static void test_parser_addressing() {
  Request r;
  EXPECT(Parser::parse("all discover", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::All);

  EXPECT(Parser::parse("self reboot", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Self);

  EXPECT(Parser::parse("group:gps status", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Group);
  EXPECT(std::strcmp(r.target, "gps") == 0);

  EXPECT(Parser::parse("GROUP:sensors ping", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Group);
}

static void test_parser_case_and_args() {
  Request r;
  EXPECT(Parser::parse("tracker GPS", r) == ParseResult::Ok);
  EXPECT(std::strcmp(r.command, "gps") == 0);

  EXPECT(Parser::parse("tracker set Led ON", r) == ParseResult::Ok);
  EXPECT(std::strcmp(r.command, "set") == 0);
  EXPECT(r.argc == 2);
  EXPECT(std::strcmp(r.args[0], "Led") == 0);  // args preserve case
  EXPECT(std::strcmp(r.args[1], "ON") == 0);

  EXPECT(Parser::parse("  ha   ping   ", r) == ParseResult::Ok);
  EXPECT(std::strcmp(r.command, "ping") == 0);
}

static void test_parser_malformed() {
  Request r;
  EXPECT(Parser::parse("tracker", r) == ParseResult::MissingCommand);
  EXPECT(Parser::parse("ha #", r) != ParseResult::Ok);
}

static void test_parser_sender_prefix() {
  EXPECT(std::strcmp(Parser::stripSenderPrefix("button: ha ping"), "ha ping") == 0);
  EXPECT(std::strcmp(Parser::stripSenderPrefix("ha ping"), "ha ping") == 0);
  EXPECT(std::strcmp(Parser::stripSenderPrefix("a b: x"), "a b: x") == 0);  // space before colon
  EXPECT(std::strcmp(Parser::stripSenderPrefix(nullptr), "") == 0);
}

/* -------------------- Registry / Caps -------------------- */

static void test_command_registry() {
  CommandRegistry reg;
  EXPECT(reg.registerCommand("ping", hPing, "help"));
  EXPECT(reg.registerCommand("echo", hEcho));
  EXPECT(reg.find("PING") != nullptr);
  EXPECT(reg.find("missing") == nullptr);
  EXPECT(reg.count() == 2);
  // replace
  EXPECT(reg.registerCommand("ping", hEcho, "new"));
  EXPECT(reg.count() == 2);
  EXPECT(reg.find("ping")->handler == hEcho);
}

static void test_capability_registry() {
  CapabilityRegistry caps;
  EXPECT(caps.registerCapability("gps"));
  EXPECT(caps.registerCapability("battery"));
  EXPECT(caps.registerCapability("gps"));  // dedupe
  EXPECT(caps.count() == 2);
  ReplyBuffer reply;
  caps.writeTo(reply);
  EXPECT(std::strstr(reply.data, "gps") != nullptr);
  EXPECT(std::strstr(reply.data, "battery") != nullptr);
  EXPECT(std::strchr(reply.data, '\n') != nullptr);
}

/* -------------------- Dispatcher -------------------- */

static void test_dispatcher() {
  CommandRegistry reg;
  reg.registerCommand("ping", hPing);
  reg.registerCommand("echo", hEcho);

  Dispatcher d(reg);
  d.setNodeName("tracker");
  d.setGroupName("mych");

  ReplyBuffer reply;
  EXPECT(d.dispatch("other ping", reply) == false);
  EXPECT(d.dispatch("tracker ping", reply) == true);
  EXPECT(std::strcmp(reply.data, "pong") == 0);

  EXPECT(d.dispatch("tracker#7 ping", reply) == true);
  EXPECT(std::strcmp(reply.data, "#7 pong") == 0);

  EXPECT(d.dispatch("all ping", reply) == true);
  EXPECT(d.dispatch("self ping", reply) == true);
  EXPECT(d.dispatch("group:mych ping", reply) == true);
  EXPECT(d.dispatch("group:other ping", reply) == false);

  EXPECT(d.dispatch("tracker unknown", reply) == true);
  EXPECT(std::strstr(reply.data, "unknown_command") != nullptr);

  EXPECT(d.dispatch("tracker", reply) == false);
  EXPECT(d.dispatch("", reply) == false);
}

/* -------------------- Builders -------------------- */

static void test_status_builder() {
  StatusBuilder b;
  b.add("name", "tracker");
  b.add("battery", 97);
  b.add("uptime", (unsigned long)12345);
  b.add("voltage", 3.85f);
  ReplyBuffer reply;
  b.writeTo(reply);
  EXPECT(std::strncmp(reply.data, "status ", 7) == 0);
  EXPECT(std::strstr(reply.data, "name=tracker") != nullptr);
  EXPECT(std::strstr(reply.data, "battery=97") != nullptr);
  EXPECT(std::strstr(reply.data, "uptime=12345") != nullptr);
}

static void test_discover_builder() {
  DiscoverBuilder b;
  b.setNodeName("tracker");
  b.add("profile", "tracker");
  b.add("fw", "1.0");
  b.add("gps", "yes");
  ReplyBuffer reply;
  b.writeTo(reply);
  EXPECT(std::strncmp(reply.data, "tracker ", 8) == 0);
  EXPECT(std::strstr(reply.data, "gps=yes") != nullptr);
}

/* -------------------- Event bus -------------------- */

static int g_event_hits = 0;
static char g_last_event[64];

static void eventHandler(const char* name, const char* kv, void* ctx) {
  (void)ctx;
  g_event_hits++;
  std::snprintf(g_last_event, sizeof(g_last_event), "%s|%s", name, kv ? kv : "");
}

static void test_event_bus() {
  EventBus bus;
  g_event_hits = 0;
  EXPECT(bus.subscribe(eventHandler, nullptr));
  EXPECT((bus.publish("button_pressed", "count=1")));
  EXPECT(g_event_hits == 1);
  EXPECT(std::strcmp(g_last_event, "button_pressed|count=1") == 0);
  EXPECT((bus.publish("battery_low", nullptr)));
  EXPECT(g_event_hits == 2);
}

/* -------------------- Feature manager -------------------- */

class DummyFeature : public Feature {
public:
  const char* name() const override { return "dummy"; }
  void registerCommands(CommandRegistry& c) override {
    c.registerCommand("dummy", hPing, "dummy");
  }
  void registerCapabilities(CapabilityRegistry& caps) override {
    caps.registerCapability("dummy");
  }
  void contributeStatus(StatusBuilder& s) override { s.add("dummy", "1"); }
  void contributeDiscover(DiscoverBuilder& d) override { d.add("dummy", "yes"); }
  int loops = 0;
  int setups = 0;
  int shutdowns = 0;
  void setup(FeatureContext& ctx) override {
    Feature::setup(ctx);
    setups++;
  }
  void loop() override { loops++; }
  void shutdown() override { shutdowns++; }
};

static void test_feature_manager() {
  CommandRegistry commands;
  CapabilityRegistry caps;
  EventBus events;
  FeatureManager fm;
  DummyFeature f1;
  DummyFeature f2;
  EXPECT(fm.add(&f1));
  EXPECT(fm.add(&f2));
  fm.start(commands, caps, events);
  EXPECT(f1.setups == 1);
  EXPECT(commands.find("dummy") != nullptr);
  EXPECT(caps.count() == 1);  // deduped capability name
  fm.loop();
  EXPECT(f1.loops == 1);
  StatusBuilder status;
  fm.collectStatus(status);
  EXPECT(status.fieldCount() >= 1);
  ReplyBuffer reply;
  fm.writeCapabilities(reply);
  EXPECT(std::strstr(reply.data, "dummy") != nullptr);
  fm.stop();
  EXPECT(f1.shutdowns == 1);
  // cannot add after start until stop — after stop, _started false, add works
  DummyFeature f3;
  EXPECT(fm.add(&f3));
}

/* -------------------- McRpc facade + inbound -------------------- */

static char g_published[256];
static bool capturePublish(const char* text, void* ctx) {
  (void)ctx;
  std::snprintf(g_published, sizeof(g_published), "%s", text);
  return true;
}

static uint32_t fakeUptime(void*) { return 42; }
static int fakeRssi(void*) { return -91; }

static void test_mcrpc_inbound_and_events() {
  McRpc rpc;

  // Minimal: register via a dummy feature
  class PingFeature : public Feature {
  public:
    const char* name() const override { return "pingf"; }
    void registerCommands(CommandRegistry& c) override {
      c.registerCommand("ping", hPing);
    }
  } pingf;

  rpc.features().add(&pingf);
  rpc.setPublishHandler(capturePublish, nullptr);
  rpc.setNodeIdentity("tracker", "mych");
  rpc.setFirmwareVersion("test");
  rpc.setProfile("tracker");
  rpc.setIdentityCallbacks(fakeUptime, fakeRssi, nullptr);
  rpc.begin();

  g_published[0] = 0;
  EXPECT(rpc.handleIncomingText("tracker: tracker ping") == true);
  EXPECT(std::strcmp(g_published, "pong") == 0);

  g_published[0] = 0;
  EXPECT(rpc.handleIncomingText("other ping") == false);

  g_published[0] = 0;
  EXPECT(rpc.events().publish("motion", "x=1") == true);
  EXPECT(std::strstr(g_published, "event motion") != nullptr);
  EXPECT(std::strstr(g_published, "x=1") != nullptr);

  InboundMessage msg;
  msg.text = "tracker#3 ping";
  g_published[0] = 0;
  EXPECT(rpc.handleInbound(msg) == true);
  EXPECT(std::strcmp(g_published, "#3 pong") == 0);

  rpc.shutdown();
}

static void test_status_discover_via_framework() {
  McRpc rpc;
  class BattFeat : public Feature {
  public:
    const char* name() const override { return "battery"; }
    void registerCommands(CommandRegistry&) override {}
    void registerCapabilities(CapabilityRegistry& c) override { c.registerCapability("battery"); }
    void contributeStatus(StatusBuilder& s) override { s.add("battery", 88); }
    void contributeDiscover(DiscoverBuilder& d) override { d.add("battery", "yes"); }
  } batt;

  rpc.features().add(&batt);
  rpc.setNodeIdentity("n1", "ch");
  rpc.setProfile("sensor");
  rpc.setFirmwareVersion("fw1");
  rpc.setIdentityCallbacks(fakeUptime, fakeRssi, nullptr);
  rpc.begin();

  StatusBuilder status;
  rpc.buildStatus(status);
  ReplyBuffer reply;
  status.writeTo(reply);
  EXPECT(std::strstr(reply.data, "name=n1") != nullptr);
  EXPECT(std::strstr(reply.data, "battery=88") != nullptr);
  EXPECT(std::strstr(reply.data, "uptime=42") != nullptr);

  DiscoverBuilder discover;
  rpc.buildDiscover(discover);
  discover.writeTo(reply);
  EXPECT(std::strstr(reply.data, "n1 ") != nullptr);
  EXPECT(std::strstr(reply.data, "battery=yes") != nullptr);

  rpc.buildCaps(reply);
  EXPECT(std::strcmp(reply.data, "battery") == 0);
  rpc.shutdown();
}

int main() {
  test_parser_basics();
  test_parser_request_ids();
  test_parser_addressing();
  test_parser_case_and_args();
  test_parser_malformed();
  test_parser_sender_prefix();
  test_command_registry();
  test_capability_registry();
  test_dispatcher();
  test_status_builder();
  test_discover_builder();
  test_event_bus();
  test_feature_manager();
  test_mcrpc_inbound_and_events();
  test_status_discover_via_framework();

  if (g_fail) {
    std::printf("%d FAILED, %d passed\n", g_fail, g_pass);
    return 1;
  }
  std::printf("ALL TESTS PASSED (%d assertions)\n", g_pass);
  return 0;
}
