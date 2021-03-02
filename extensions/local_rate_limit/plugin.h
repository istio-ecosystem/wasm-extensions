#include "proxy_wasm_intrinsics.h"

class PluginRootContext : public RootContext {
 public:
  explicit PluginRootContext(uint32_t id, std::string_view root_id)
      : RootContext(id, root_id) {}

  bool onConfigure(size_t) override;

  // onTick will trigger token bucket refill.
  void onTick() override;

 private:
  bool parseConfiguration(size_t);

  uint64_t max_tokens_;
  uint64_t tokens_per_refill_;
  uint64_t refill_interval_nanosec_;
};

class PluginContext : public Context {
 public:
  explicit PluginContext(uint32_t id, RootContext* root) : Context(id, root) {}
  FilterHeadersStatus onRequestHeaders(uint32_t, bool) override;

 private:
  inline PluginRootContext* rootContext() {
    return dynamic_cast<PluginRootContext*>(this->root());
  }
};
