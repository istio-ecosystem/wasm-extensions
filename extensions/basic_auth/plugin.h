#include <assert.h>

#include <string>
#include <unordered_set>
#define ASSERT(_X) assert(_X)

static const std::string EMPTY_STRING;

#ifndef NULL_PLUGIN

#include "proxy_wasm_intrinsics.h"

#else

#include "include/proxy-wasm/null_plugin.h"

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

#endif

// PluginRootContext is the root context for all streams processed by the
// thread. It has the same lifetime as the worker thread and acts as target for
// interactions that outlives individual stream, e.g. timer, async calls.
class PluginRootContext : public RootContext {
 public:
  PluginRootContext(uint32_t id, std::string_view root_id)
      : RootContext(id, root_id) {}
  ~PluginRootContext() {}
  bool onConfigure(size_t) override;

  // check() handles the retrieval of certain headers (path,
  // method and authorization) from the HTTP Request Header in order to compare
  // it against the plugin's configuration data and deny or grant access to that
  // requested path.
  FilterHeadersStatus check();

  enum MATCH_TYPE { Prefix, Exact, Suffix };
  struct BasicAuthConfigRule {
    std::string request_path;
    MATCH_TYPE path_pattern;
    std::vector<std::pair<MATCH_TYPE, std::string>> hosts;
    std::unordered_set<std::string> encoded_credentials;
  };

 private:
  bool configure(size_t);

  // The following map holds information regarding the plugin's configuration
  // data. The key will hold the request_method (GET, POST, DELETE for example)
  // The value is a vector of structs holding request_path, match_pattern and
  // encoded_credentials container at each position of the vector for a given
  // request_method. Here is an example layout of the container:
  //{
  // "GET":{
  //    { "/products",
  //      "prefix",
  //      ["YWRtaW46YWRtaW4="]
  //    },
  // },
  // "POST":{
  //     { "/wiki",
  //      "prefix",
  //      ["YWRtaW46YWRtaW4=", "AWRtaW46YWRtaW4="]
  //    }
  //  },
  //}
  std::unordered_map<std::string,
                     std::vector<PluginRootContext::BasicAuthConfigRule>>
      basic_auth_configuration_;
  std::string realm_ = "istio";
  FilterHeadersStatus credentialsCheck(
      const PluginRootContext::BasicAuthConfigRule&, std::string_view);
};

// Per-stream context.
class PluginContext : public Context {
 public:
  explicit PluginContext(uint32_t id, RootContext* root) : Context(id, root) {}
  FilterHeadersStatus onRequestHeaders(uint32_t, bool) override;

 private:
  inline PluginRootContext* rootContext() {
    return dynamic_cast<PluginRootContext*>(this->root());
  }
};

#ifdef NULL_PLUGIN

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm

#endif
