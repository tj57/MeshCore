#include "GpsFeature.h"

#include "../../McRpc.h"
#include <stdio.h>

namespace mcrpc {

static GpsFeature* g_gps = nullptr;

void GpsFeature::registerCommands(Registry& registry) {
  g_gps = this;
  registry.registerCommand("gps", &GpsFeature::cmdGps, "get GPS fix", "gps");
  registry.registerCommand("location", &GpsFeature::cmdLocation, "alias for gps", "gps");
  registry.registerCommand("track", &GpsFeature::cmdTrack, "request/track fix", "gps");
}

bool GpsFeature::writeFix(CommandContext& ctx, bool request_if_missing) {
  if (!g_gps) return false;
  GpsFix fix;
  memset(&fix, 0, sizeof(fix));
  if (!g_gps->_host.readGps(fix) || !fix.valid) {
    if (request_if_missing) {
      if (g_gps->_host.gpsBusy()) {
        ctx.reply->clear();
        ctx.reply->append("err busy");
        return true;
      }
      if (g_gps->_host.requestGpsFix()) {
        ctx.reply->clear();
        ctx.reply->append("ok pending");
        return true;
      }
    }
    ctx.reply->clear();
    ctx.reply->append("err gps_no_fix");
    return true;
  }
  ctx.reply->clear();
  ctx.reply->printf("gps lat=%.6f lon=%.6f alt=%.1f sat=%d", (double)fix.lat, (double)fix.lon,
                    (double)fix.alt, fix.sats);
  if (fix.hdop > 0) ctx.reply->printf(" hdop=%.1f", (double)fix.hdop);
  return true;
}

bool GpsFeature::cmdGps(CommandContext& ctx) {
  // Optional arg: "on" | "off" | "status" | (none = read/request)
  if (ctx.request->argc >= 1) {
    if (ieq(ctx.request->args[0], "status")) {
      GpsFix fix;
      memset(&fix, 0, sizeof(fix));
      bool ok = g_gps && g_gps->_host.readGps(fix);
      ctx.reply->clear();
      if (ok && fix.valid) {
        ctx.reply->printf("gps status=fix sat=%d", fix.sats);
      } else if (g_gps && g_gps->_host.gpsBusy()) {
        ctx.reply->append("gps status=busy");
      } else {
        ctx.reply->append("gps status=nofix");
      }
      return true;
    }
  }
  return writeFix(ctx, true);
}

bool GpsFeature::cmdLocation(CommandContext& ctx) {
  return writeFix(ctx, true);
}

bool GpsFeature::cmdTrack(CommandContext& ctx) {
  (void)ctx;
  if (!g_gps) return false;
  if (g_gps->_host.gpsBusy()) {
    ctx.reply->clear();
    ctx.reply->append("err busy");
    return true;
  }
  if (!g_gps->_host.requestGpsFix()) {
    ctx.reply->clear();
    ctx.reply->append("err unsupported");
    return true;
  }
  ctx.reply->clear();
  ctx.reply->append("ok pending");
  return true;
}

}  // namespace mcrpc
