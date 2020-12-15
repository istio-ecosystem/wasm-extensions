#include "extensions/scaffold/plugin.h"

static RegisterContextFactory register_Scaffold(
    CONTEXT_FACTORY(PluginContext), ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t) { return true; }
