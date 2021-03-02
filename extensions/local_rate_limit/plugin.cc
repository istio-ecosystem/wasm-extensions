#include "extensions/local_rate_limit/plugin.h"

#include "absl/strings/str_cat.h"
#include "extensions/common/wasm/json_util.h"
#include "extensions/local_rate_limit/bucket.h"

using ::nlohmann::json;
using ::Wasm::Common::JsonArrayIterate;
using ::Wasm::Common::JsonGetField;
using ::Wasm::Common::JsonObjectIterate;
using ::Wasm::Common::JsonValueAs;

namespace {

// tooManyRequest returns a 429 response code.
void tooManyRequest() {
  sendLocalResponse(429, "Too many requests", "rate_limited", {});
}

bool shouldRateLimit() {
  /*
    Add any rate limit condition here, such as host, path, etc.
  */
  return true;
}

}  // namespace

static RegisterContextFactory register_LocalRateLimit(
    CONTEXT_FACTORY(PluginContext), ROOT_FACTORY(PluginRootContext));

bool PluginRootContext::onConfigure(size_t configuration_size) {
  if (!parseConfiguration(configuration_size)) {
    return false;
  }

  // Initialize token bucket.
  if (!initializeTokenBucket(tokens_per_refill_)) {
    return false;
  }

  // Start ticker, which will trigger token bucket refill.
  proxy_set_tick_period_milliseconds(refill_interval_nanosec_ / 1000000);

  return true;
}

void PluginRootContext::onTick() {
  refillToken(tokens_per_refill_, refill_interval_nanosec_, max_tokens_);
}

FilterHeadersStatus PluginContext::onRequestHeaders(uint32_t, bool) {
  if (shouldRateLimit() && !getToken()) {
    tooManyRequest();
    return FilterHeadersStatus::StopIteration;
  }
  return FilterHeadersStatus::Continue;
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

  // Get token bucket configuration
  // {
  //   "max_tokens": 100,
  //   "tokens_per_fill": 50,
  //   "fill_interval_seconds": 10
  // }
  // Parse and get max tokens.
  auto it = j.find("max_tokens");
  if (it != j.end()) {
    auto max_tokens_val = JsonValueAs<uint64_t>(it.value());
    if (max_tokens_val.second != Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(absl::StrCat(
          "cannot parse max token in plugin configuration JSON string: ",
          configuration_data->view()));
      return false;
    }
    max_tokens_ = max_tokens_val.first.value();
  } else {
    LOG_WARN(absl::StrCat(
        "max token must be provided in plugin configuration JSON string: ",
        configuration_data->view()));
    return false;
  }

  // Parse and get tokens per refill.
  it = j.find("tokens_per_refill");
  if (it != j.end()) {
    auto tokens_per_refill_val = JsonValueAs<uint64_t>(it.value());
    if (tokens_per_refill_val.second !=
        Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(
          absl::StrCat("cannot parse tokens per refill in plugin configuration "
                       "JSON string: ",
                       configuration_data->view()));
      return false;
    }
    tokens_per_refill_ = tokens_per_refill_val.first.value();
  } else {
    LOG_WARN(
        absl::StrCat("tokens per refill must be provided in plugin "
                     "configuration JSON string: ",
                     configuration_data->view()));
    return false;
  }

  // Parse and get refill interval.
  it = j.find("refill_interval_sec");
  if (it != j.end()) {
    auto refill_interval_sec_val = JsonValueAs<uint64_t>(it.value());
    if (refill_interval_sec_val.second !=
        Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(absl::StrCat(
          "cannot parse refill interval in plugin configuration JSON string: ",
          configuration_data->view()));
      return false;
    }
    refill_interval_nanosec_ =
        refill_interval_sec_val.first.value() * 1000000000;
  } else {
    LOG_WARN(
        absl::StrCat("refill interval must be provided in plugin configuration "
                     "JSON string: ",
                     configuration_data->view()));
    return false;
  }

  return true;
}
