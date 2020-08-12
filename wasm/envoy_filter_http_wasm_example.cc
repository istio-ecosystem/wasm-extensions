// NOLINT(namespace-envoy)
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "proxy_wasm_intrinsics.h"
#include "wasm/utility/config.h"
#include "wasm/utility/http_parser.h"
#include "wasm/utility/sqli.h"

void onSQLi(std::string part) {
  LOG_ERROR("SQL injection detected");
  std::string response_body = "SQL injection detected";
  std::string response_log = "SQLi at " + part;
  sendLocalResponse(403, response_log, response_body, {});
}

std::string printParams(QueryParams params) {
  std::string str;
  for (auto param : params) {
    str += param.first + " -> " + param.second;
  }
  return str;
}

class ExampleRootContext : public RootContext {
public:
  explicit ExampleRootContext(uint32_t id, StringView root_id) : RootContext(id, root_id) {}

  bool onConfigure(size_t config_size) override;
  bool onStart(size_t) override;
  Config getConfig() { return config_; }

private:
  struct Config config_;
};

class ExampleContext : public Context {
public:
  explicit ExampleContext(uint32_t id, RootContext* root) : Context(id, root) {}

  void onCreate() override;
  FilterHeadersStatus onRequestHeaders(uint32_t headers, bool end_of_stream) override;
  FilterDataStatus onRequestBody(size_t body_buffer_length, bool end_of_stream) override;
  FilterHeadersStatus onResponseHeaders(uint32_t headers, bool end_of_stream) override;
  void onDone() override;
  void onLog() override;
  void onDelete() override;

private:
  std::string content_type_;
  struct Config config_;
};
static RegisterContextFactory register_ExampleContext(CONTEXT_FACTORY(ExampleContext),
                                                      ROOT_FACTORY(ExampleRootContext),
                                                      "my_root_id");

bool ExampleRootContext::onStart(size_t) {
  LOG_TRACE("onStart");
  return true;
}

bool ExampleRootContext::onConfigure(size_t config_size) {
  if (config_size == 0) {
    return true;
  }

  // read configuration string from buffer
  auto configuration_data = getBufferBytes(WasmBufferType::PluginConfiguration, 0, config_size);
  std::string configuration = configuration_data->toString();

  // parse configuration string into Config
  std::string log;
  if (!parseConfig(configuration, &config_, &log)) {
    LOG_ERROR("onConfigure: " + log);
    return false;
  }
  LOG_TRACE("onConfigure: " + log);
  return true;
}

void ExampleContext::onCreate() {
  LOG_WARN(std::string("onCreate " + std::to_string(id())));

  // get config from root
  ExampleRootContext* root = dynamic_cast<ExampleRootContext*>(this->root());
  config_ = root->getConfig();
  LOG_TRACE("onCreate: config loaded from root context ->" + config_.to_string());
}

FilterHeadersStatus ExampleContext::onRequestHeaders(uint32_t, bool) {
  // get header pairs
  LOG_DEBUG(std::string("onRequestHeaders ") + std::to_string(id()));
  auto result = getRequestHeaderPairs();
  auto pairs = result->pairs();

  // log all headers
  QueryParams headers;
  LOG_INFO(std::string("headers: ") + std::to_string(pairs.size()));
  for (auto& p : pairs) {
    LOG_INFO(std::string(p.first) + std::string(" -> ") + std::string(p.second));
    headers.emplace(p.first, p.second);
  }
  LOG_TRACE("all headers printed");

  std::string log;

  // detect SQL injection in headers
  if (detectSQLiOnParams(headers, config_.header_include, config_.headers, &log)) {
    onSQLi("Header");
    return FilterHeadersStatus::StopIteration;
  }
  LOG_TRACE("headers:\n" + log);

  // detect SQL injection in cookies
  std::string cookie_str = getRequestHeader("Cookie")->toString();
  QueryParams cookies = parseCookie(cookie_str);
  LOG_TRACE("Cookies parsed: " + printParams(cookies));
  if (detectSQLiOnParams(cookies, config_.cookie_include, config_.cookies, &log)) {
    onSQLi("cookie");
    return FilterHeadersStatus::StopIteration;
  }
  LOG_TRACE("cookies:\n" + log);

  // detect SQL injection in path
  std::string path = getRequestHeader(":path")->toString();
  QueryParams path_params = parsePath(path);
  LOG_TRACE("Path parsed: " + printParams(path_params));
  if (detectSQLiOnParams(cookies, false, {}, &log)) {
    onSQLi("path");
    return FilterHeadersStatus::StopIteration;
  }
  LOG_TRACE("path:\n" + log);

  // record body content type to context
  content_type_ = getRequestHeader("content-type")->toString();

  return FilterHeadersStatus::Continue;
}

FilterDataStatus ExampleContext::onRequestBody(size_t body_buffer_length, bool end_of_stream) {
  auto body = getBufferBytes(WasmBufferType::HttpRequestBody, 0, body_buffer_length);
  auto body_str = std::string(body->view());
  LOG_ERROR(std::string("onRequestBody ") + body_str);

  if (content_type_.compare("application/x-www-form-urlencoded") != 0) {
    return FilterDataStatus::Continue;
  }

  // detect SQL injection in query parameters
  std::string log;
  auto query_params = parseBody(body_str);
  LOG_TRACE("query params parsed: " + printParams(query_params));
  if (detectSQLiOnParams(query_params, config_.param_include, config_.params, &log)) {
    onSQLi("body query params");
    return FilterDataStatus::StopIterationAndBuffer;
  }
  LOG_TRACE("body sqli detection finished:\n" + log);
  return FilterDataStatus::Continue;
}

FilterHeadersStatus ExampleContext::onResponseHeaders(uint32_t, bool) {
  LOG_DEBUG(std::string("onResponseHeaders ") + std::to_string(id()));
  auto result = getResponseHeaderPairs();
  auto pairs = result->pairs();
  LOG_INFO(std::string("headers: ") + std::to_string(pairs.size()));
  for (auto& p : pairs) {
    LOG_INFO(std::string(p.first) + std::string(" -> ") + std::string(p.second));
  }
  addResponseHeader("branch", "libinjection-config");
  replaceResponseHeader("location", "envoy-wasm");
  return FilterHeadersStatus::Continue;
}


void ExampleContext::onDone() { LOG_WARN(std::string("onDone " + std::to_string(id()))); }

void ExampleContext::onLog() { LOG_WARN(std::string("onLog " + std::to_string(id()))); }

void ExampleContext::onDelete() { LOG_WARN(std::string("onDelete " + std::to_string(id()))); }
