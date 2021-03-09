#include "extensions/open_policy_agent/cache.h"
#include "proxy_wasm_intrinsics.h"

// OPA filter root context.
class PluginRootContext : public RootContext {
 public:
  explicit PluginRootContext(uint32_t id, std::string_view root_id)
      : RootContext(id, root_id) {}

  bool onConfigure(size_t) override;

  // Check sends out a HTTP check call to OPA server for the given stream.
  FilterHeadersStatus check(uint32_t stream_context_id);

 private:
  bool parseConfiguration(size_t);

  // Cache operations.
  bool checkCache(const Payload &payload, uint64_t &hash, bool &allowed) {
    bool hit =
        cache_.check(payload, hash, allowed, getCurrentTimeNanoseconds());
    incrementMetric((hit ? cache_hits_ : cache_misses_), 1);
    return hit;
  }
  void addCache(const uint64_t hash, bool result) {
    cache_.add(hash, result, getCurrentTimeNanoseconds());
  }

  // LRU cache for OPA check results.
  ResultCache cache_;
  // Duration that cache entry is valid for.
  uint64_t cache_valid_for_sec_ = 0;

  // Host for OPA check call. This will be used as host header.
  std::string opa_host_;
  // Envoy cluster for OPA HTTP call.
  std::string opa_cluster_;

  // Handler for cache stats.
  uint32_t cache_hits_;
  uint32_t cache_misses_;
};

// OPA filter stream context.
class PluginContext : public Context {
 public:
  explicit PluginContext(uint32_t id, RootContext *root) : Context(id, root) {}

  FilterHeadersStatus onRequestHeaders(uint32_t, bool) override;

 private:
  inline PluginRootContext *rootContext() {
    return dynamic_cast<PluginRootContext *>(this->root());
  }
};
