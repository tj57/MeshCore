/* Native host tests for mcRPC parser / registry / dispatcher.
 * Build: g++ -std=c++17 -I../../src -o test_mcrpc test_mcrpc.cpp \
 *          ../../src/mcrpc/Parser.cpp ../../src/mcrpc/Registry.cpp \
 *          ../../src/mcrpc/Dispatcher.cpp FeatureManager.cpp (optional)
 */
#include <mcrpc/Parser.h>
#include <mcrpc/Registry.h>
#include <mcrpc/Dispatcher.h>
#include <mcrpc/FeatureManager.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace mcrpc;

static int g_fail = 0;

#define EXPECT(cond) do { \
  if (!(cond)) { \
    std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    g_fail++; \
  } \
} while (0)

static bool hPing(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append("pong");
  return true;
}

static bool hGps(CommandContext& ctx) {
  ctx.reply->clear();
  ctx.reply->append("gps lat=1.0 lon=2.0");
  return true;
}

static void test_parser() {
  Request r;
  EXPECT(Parser::parse("", r) == ParseResult::Empty);
  EXPECT(Parser::parse("   ", r) == ParseResult::Empty);

  EXPECT(Parser::parse("ha ping", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Named);
  EXPECT(std::strcmp(r.target, "ha") == 0);
  EXPECT(std::strcmp(r.command, "ping") == 0);
  EXPECT(!r.has_request_id);

  EXPECT(Parser::parse("ha#42 ping", r) == ParseResult::Ok);
  EXPECT(r.has_request_id);
  EXPECT(r.request_id == 42);

  EXPECT(Parser::parse("all discover", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::All);

  EXPECT(Parser::parse("group:gps status", r) == ParseResult::Ok);
  EXPECT(r.address_kind == AddressKind::Group);
  EXPECT(std::strcmp(r.target, "gps") == 0);

  EXPECT(Parser::parse("tracker GPS", r) == ParseResult::Ok);
  EXPECT(std::strcmp(r.command, "gps") == 0);  // lowercased

  EXPECT(Parser::parse("tracker set led on", r) == ParseResult::Ok);
  EXPECT(r.argc == 2);
  EXPECT(std::strcmp(r.args[0], "led") == 0);
  EXPECT(std::strcmp(r.args[1], "on") == 0);

  const char* stripped = Parser::stripSenderPrefix("button: ha ping");
  EXPECT(std::strcmp(stripped, "ha ping") == 0);
}

static void test_dispatcher() {
  Registry reg;
  reg.registerCommand("ping", hPing);
  reg.registerCommand("gps", hGps, "gps fix", "gps");

  Dispatcher d(reg);
  d.setNodeName("tracker");
  d.setGroupName("mych");

  ReplyBuffer reply;
  EXPECT(d.dispatch("other ping", reply) == false);  // not addressed

  EXPECT(d.dispatch("tracker ping", reply) == true);
  EXPECT(std::strcmp(reply.data, "pong") == 0);

  EXPECT(d.dispatch("tracker#7 ping", reply) == true);
  EXPECT(std::strcmp(reply.data, "#7 pong") == 0);

  EXPECT(d.dispatch("all ping", reply) == true);
  EXPECT(std::strcmp(reply.data, "pong") == 0);

  EXPECT(d.dispatch("tracker unknown", reply) == true);
  EXPECT(std::strstr(reply.data, "unknown_command") != nullptr);

  EXPECT(d.dispatch("tracker gps", reply) == true);
  EXPECT(std::strstr(reply.data, "lat=") != nullptr);

  // malformed / missing command → ignore
  EXPECT(d.dispatch("tracker", reply) == false);
}

static void test_registry_caps() {
  Registry reg;
  reg.registerCommand("gps", hGps, nullptr, "gps");
  reg.registerCommand("location", hGps, nullptr, "gps");
  const char* caps[8];
  size_t n = reg.collectCapabilities(caps, 8);
  EXPECT(n == 1);
  EXPECT(std::strcmp(caps[0], "gps") == 0);
}

int main() {
  test_parser();
  test_dispatcher();
  test_registry_caps();
  if (g_fail) {
    std::printf("%d FAILURES\n", g_fail);
    return 1;
  }
  std::printf("ALL TESTS PASSED\n");
  return 0;
}
