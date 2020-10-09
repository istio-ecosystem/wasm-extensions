#include "extensions/scaffold/plugin.h"

#ifdef NULL_PLUGIN

namespace proxy_wasm {
namespace null_plugin {
namespace scaffold {

PROXY_WASM_NULL_PLUGIN_REGISTRY

#endif

static RegisterContextFactory register_Scaffold(
    CONTEXT_FACTORY(PluginContext), ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t) { return true; }

#ifdef NULL_PLUGIN

}  // scaffold
}  // namespace null_plugin
}  // namespace proxy_wasm

#endif
