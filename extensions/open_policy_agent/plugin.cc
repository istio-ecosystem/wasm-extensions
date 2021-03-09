#include "extensions/open_policy_agent/plugin.h"

#include "absl/strings/str_cat.h"
#include "extensions/common/wasm/json_util.h"

using ::nlohmann::json;
using ::Wasm::Common::JsonArrayIterate;
using ::Wasm::Common::JsonGetField;
using ::Wasm::Common::JsonObjectIterate;
using ::Wasm::Common::JsonValueAs;

static RegisterContextFactory register_Opa(CONTEXT_FACTORY(PluginContext),
                                           ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t configuration_size) {
  if (!parseConfiguration(configuration_size)) {
    return false;
  }

  // Initialize cache valid duration.
  cache_.setValidDuration(cache_valid_for_sec_);

  // Initialize cache stats.
  Metric cache_count(MetricType::Counter, "policy_cache_count",
                     {MetricTag{"wasm_filter", MetricTag::TagType::String},
                      MetricTag{"cache", MetricTag::TagType::String}});
  cache_hits_ = cache_count.resolve("opa_filter", "hit");
  cache_misses_ = cache_count.resolve("opa_filter", "miss");
  return true;
}

FilterHeadersStatus PluginRootContext::check(uint32_t stream_context_id) {
  // Fill in opa check payload.
  Payload payload;
  getValue({"connection", "uri_san_peer_certificate"},
           &payload.source_principal);
  getValue({"node", "metadata", "WORKLOAD_NAME"},
           &payload.destination_workload);
  getValue({"request", "method"}, &payload.request_method);
  getValue({"request", "url_path"}, &payload.request_url_path);

  // Check cache first.
  uint64_t payload_hash = 0;
  bool allowed = false;
  bool cache_hit = checkCache(payload, payload_hash, allowed);

  // If there is valid allopw cache entry, continue the request.
  if (cache_hit && allowed) {
    return FilterHeadersStatus::Continue;
  }

  // If there is valid denial cache entry, return permission denial direclty.
  if (cache_hit && !allowed) {
    sendLocalResponse(403, "OPA policy check denied", "", {});
    return FilterHeadersStatus::StopIteration;
  }

  // Otherwise sending check request to OPA server.
  // Convert payload proto to json string and send it to OPA server.
  Wasm::Common::JsonObject payload_obj = {
      {"input",
       {
           {"source_principal", payload.source_principal},
           {"destination_workload", payload.destination_workload},
           {"request_method", payload.request_method},
           {"request_url_path", payload.request_url_path},
       }}};
  auto json_payload = payload_obj.dump();

  // Construct http call to OPA server.
  HeaderStringPairs headers;
  HeaderStringPairs trailers;
  headers.emplace_back("content-type", "application/json");
  headers.emplace_back(":path", "/v1/data/test/allow");
  headers.emplace_back(":method", "POST");
  headers.emplace_back(":authority", opa_host_);

  auto call_result = httpCall(
      /* envoy service cluster */ opa_cluster_,
      /* headers */ headers, /* body */ json_payload, /* body */ trailers,
      /* timeout milliseconds */ 5000,
      [this, stream_context_id, payload_hash](uint32_t, size_t body_size,
                                              uint32_t) {
        // Callback is triggered inside root context. setEffectiveContext
        // swtich the background context from root context to the current
        // stream context.
        getContext(stream_context_id)->setEffectiveContext();

        auto body =
            getBufferBytes(WasmBufferType::HttpCallResponseBody, 0, body_size);
        // Parse returned result JSON string.
        auto result = ::Wasm::Common::JsonParse(body->view());
        if (!result.has_value()) {
          LOG_DEBUG(absl::StrCat(
              "cannot parse OPA policy response JSON string: ", body->view()));
          sendLocalResponse(500, "OPA policy check failed", "", {});
          return;
        }

        // j is a JsonObject holds configuration data
        auto j = result.value();
        auto it = j.find("result");
        bool check_result = false;
        if (it != j.end()) {
          auto result_val = JsonValueAs<bool>(it.value());
          if (result_val.second != Wasm::Common::JsonParserResultDetail::OK) {
            // Failed to parse OPA response, response with server error.
            LOG_DEBUG(absl::StrCat(
                "cannot parse result in OPA response JSON string: ",
                body->view()));
            sendLocalResponse(500, "OPA policy check failed", "", {});
            return;
          }
          check_result = result_val.first.value();
        } else {
          // no result found in OPA response, response with server error.
          LOG_WARN(absl::StrCat(
              "result must be provided in OPA response JSON string: ",
              body->view()));
          sendLocalResponse(500, "OPA policy check failed", "", {});
          return;
        }
        addCache(payload_hash, check_result);
        if (!check_result) {
          // denied, send direct response.
          sendLocalResponse(403, "OPA policy check denied", "", {});
          return;
        }
        // allowed, continue request.
        continueRequest();
      });

  if (call_result != WasmResult::Ok) {
    LOG_DEBUG("cannot make call to OPA policy server");
    sendLocalResponse(500, "OPA policy check call failed", "", {});
  }

  return FilterHeadersStatus::StopIteration;
}

FilterHeadersStatus PluginContext::onRequestHeaders(uint32_t, bool) {
  return rootContext()->check(id());
}

bool PluginRootContext::parseConfiguration(size_t configuration_size) {
  auto configuration_data = getBufferBytes(WasmBufferType::PluginConfiguration,
                                           0, configuration_size);
  // Parse configuration JSON string.
  auto result = ::Wasm::Common::JsonParse(configuration_data->view());
  if (!result.has_value()) {
    LOG_WARN(absl::StrCat("cannot parse plugin configuration JSON string: ",
                          configuration_data->view()));
    return false;
  }

  // j is a JsonObject holds configuration data
  auto j = result.value();

  // Get OPA extension configuration
  // {
  //   "opa_service_host": "opa.default.svc.cluster.local",
  //   "opa_cluster_name": "outbound|8080||opa.default.svc.cluster.local",
  //   "check_result_cache_valid_sec": 10
  // }
  // Parse and get opa service host.
  auto it = j.find("opa_service_host");
  if (it != j.end()) {
    auto opa_host_val = JsonValueAs<std::string>(it.value());
    if (opa_host_val.second != Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(absl::StrCat(
          "cannot parse opa service host in plugin configuration JSON string: ",
          configuration_data->view()));
      return false;
    }
    opa_host_ = opa_host_val.first.value();
  } else {
    LOG_WARN(
        absl::StrCat("opa service host must be provided in plugin "
                     "configuration JSON string: ",
                     configuration_data->view()));
    return false;
  }

  // Parse and get opa cluster name.
  it = j.find("opa_cluster_name");
  if (it != j.end()) {
    auto opa_cluster_val = JsonValueAs<std::string>(it.value());
    if (opa_cluster_val.second != Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(absl::StrCat(
          "cannot parse opa cluster name in plugin configuration JSON string: ",
          configuration_data->view()));
      return false;
    }
    opa_cluster_ = opa_cluster_val.first.value();
  } else {
    LOG_WARN(
        absl::StrCat("opa cluster name must be provided in plugin "
                     "configuration JSON string: ",
                     configuration_data->view()));
    return false;
  }

  // Parse and get cache valid duraiton.
  // If not provided, result won't be cached.
  it = j.find("check_result_cache_valid_sec");
  if (it != j.end()) {
    auto check_result_cache_valid_sec_val = JsonValueAs<uint64_t>(it.value());
    if (check_result_cache_valid_sec_val.second !=
        Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(
          absl::StrCat("cannot parse cache valid duration in plugin "
                       "configuration JSON string: ",
                       configuration_data->view()));
      return false;
    }
    cache_valid_for_sec_ = check_result_cache_valid_sec_val.first.value();
  }

  return true;
}
