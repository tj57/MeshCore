#pragma once

/**
 * Umbrella header for the stable mcRPC Feature SDK.
 *
 * Feature modules should include this — not Parser, Dispatcher, or MeshCore.
 */

#include "Feature.h"
#include "CommandRegistry.h"
#include "CapabilityRegistry.h"
#include "EventBus.h"
#include "StatusBuilder.h"
#include "DiscoverBuilder.h"
#include "HostServices.h"
#include "InboundMessage.h"
