#include "plugin.h"

// Boilderplate code to register the extension implementation.
static RegisterContextFactory register_Example(CONTEXT_FACTORY(PluginContext),
                                               ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t) { return true; }

FilterHeadersStatus PluginContext::onResponseHeaders(uint32_t, bool) {
  addResponseHeader("X-Wasm-custom", "foo");
  return FilterHeadersStatus::Continue;
}
