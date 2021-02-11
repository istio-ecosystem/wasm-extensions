#include "extensions/basic_auth/plugin.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "extensions/common/wasm/base64.h"
#include "extensions/common/wasm/json_util.h"

using ::nlohmann::json;
using ::Wasm::Common::JsonArrayIterate;
using ::Wasm::Common::JsonGetField;
using ::Wasm::Common::JsonObjectIterate;
using ::Wasm::Common::JsonValueAs;

#ifdef NULL_PLUGIN

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

PROXY_WASM_NULL_PLUGIN_REGISTRY

#endif

static RegisterContextFactory register_BasicAuth(
    CONTEXT_FACTORY(PluginContext), ROOT_FACTORY(PluginRootContext));

namespace {

void deniedNoBasicAuthData(const std::string& realm) {
  sendLocalResponse(
      401,
      "Request denied by Basic Auth check. No Basic "
      "Authentication information found.",
      "", {{"WWW-Authenticate", absl::StrCat("Basic realm=", realm)}});
}

void deniedInvalidCredentials(const std::string& realm) {
  sendLocalResponse(
      401,
      "Request denied by Basic Auth check. Invalid "
      "username and/or password",
      "", {{"WWW-Authenticate", absl::StrCat("Basic realm=", realm)}});
}

bool extractBasicAuthRule(
    const json& configuration,
    std::unordered_map<std::string,
                       std::vector<PluginRootContext::BasicAuthConfigRule>>*
        rules) {
  std::string prefix;
  std::string suffix;
  std::string exact;
  std::vector<std::string> request_methods;

  // Build a basic auth config rule
  struct PluginRootContext::BasicAuthConfigRule rule;

  // Example expected json object:
  // {
  //   "prefix": "/api",
  //   "request_methods":[ "GET", "POST" ],
  //   "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ]
  //  }

  // Extract out request path field: prefix, suffix, and exact.
  int32_t count = 0;
  auto it = configuration.find("prefix");
  if (it != configuration.end()) {
    count += 1;
    auto parse_result = JsonValueAs<std::string>(it.value());
    if (parse_result.second != Wasm::Common::JsonParserResultDetail::OK ||
        !parse_result.first.has_value()) {
      LOG_WARN("failed to parse 'prefix' field in filter configuration.");
      return false;
    }
    prefix = parse_result.first.value();
  }
  it = configuration.find("exact");
  if (it != configuration.end()) {
    count += 1;
    auto parse_result = JsonValueAs<std::string>(it.value());
    if (parse_result.second != Wasm::Common::JsonParserResultDetail::OK ||
        !parse_result.first.has_value()) {
      LOG_WARN("failed to parse 'exact' field in filter configuration.");
      return false;
    }
    exact = parse_result.first.value();
  }

  it = configuration.find("suffix");
  if (it != configuration.end()) {
    count += 1;
    auto parse_result = JsonValueAs<std::string>(it.value());
    if (parse_result.second != Wasm::Common::JsonParserResultDetail::OK ||
        !parse_result.first.has_value()) {
      LOG_WARN("failed to parse 'suffix' field in filter configuration.");
      return false;
    }
    suffix = parse_result.first.value();
  }

  // Exact one of prefix, suffix, and exact is allowed in basic auth rule
  if (count != 1) {
    LOG_WARN(
        "exactly one of 'prefix', 'suffix', and 'exact' has to present in "
        "basic "
        "auth filter configuration.");
    return false;
  }

  // iterate over methods that rule should be applied on.
  if (!JsonArrayIterate(
          configuration, "request_methods", [&](const json& method) -> bool {
            auto method_string = JsonValueAs<std::string>(method);
            if (method_string.second !=
                Wasm::Common::JsonParserResultDetail::OK) {
              return false;
            }
            request_methods.push_back(method_string.first.value());
            return true;
          })) {
    LOG_WARN("failed to parse configuration for request methods.");
    return false;
  }
  if (request_methods.size() <= 0) {
    LOG_WARN("at least one method has to be configured for a rule.");
    return false;
  }

  if (!JsonArrayIterate(
          configuration, "credentials", [&](const json& credentials) -> bool {
            auto credential = JsonValueAs<std::string>(credentials);
            if (credential.second != Wasm::Common::JsonParserResultDetail::OK) {
              return false;
            }
            // Check if credential has `:` in it. If it has, it needs to be
            // base64 encoded.
            if (absl::StrContains(credential.first.value(), ":")) {
              rule.encoded_credentials.insert(
                  Base64::encode(credential.first.value().data(),
                                 credential.first.value().size()));
              return true;
            }
            // Otherwise, try base64 decode and insert into credential list if
            // it can be decoded.
            if (!Base64::decodeWithoutPadding(credential.first.value())
                     .empty()) {
              rule.encoded_credentials.insert(credential.first.value());
              return true;
            }
            return false;
          })) {
    LOG_WARN("failed to parse configuration for credentials.");
    return false;
  }
  if (rule.encoded_credentials.size() <= 0) {
    LOG_WARN("at least one credential has to be configured for a rule.");
    return false;
  }

  if (!prefix.empty()) {
    rule.pattern = PluginRootContext::Prefix;
    rule.request_path = prefix;
  } else if (!exact.empty()) {
    rule.pattern = PluginRootContext::Exact;
    rule.request_path = exact;
  } else if (!suffix.empty()) {
    rule.pattern = PluginRootContext::Suffix;
    rule.request_path = suffix;
  }
  for (auto& method : request_methods) {
    (*rules)[method].push_back(rule);
  }
  return true;
}

}  // namespace

FilterHeadersStatus PluginRootContext::credentialsCheck(
    const PluginRootContext::BasicAuthConfigRule& rule,
    std::string_view authorization_header) {
  // Check if the Basic auth header starts with "Basic "
  if (!absl::StartsWith(authorization_header, "Basic ")) {
    deniedNoBasicAuthData(realm_);
    return FilterHeadersStatus::StopIteration;
  }
  std::string_view authorization_header_strip =
      absl::StripPrefix(authorization_header, "Basic ");

  auto auth_credential_iter =
      rule.encoded_credentials.find(std::string(authorization_header_strip));
  // Check if encoded credential is part of the encoded_credentials
  // set from our container to grant or deny access.
  if (auth_credential_iter == rule.encoded_credentials.end()) {
    deniedInvalidCredentials(realm_);
    return FilterHeadersStatus::StopIteration;
  }

  return FilterHeadersStatus::Continue;
}

bool PluginRootContext::onConfigure(size_t size) {
  // Parse configuration JSON string.
  if (size > 0 && !configure(size)) {
    LOG_WARN("configuration has errors initialization will not continue.");
    return false;
  }
  return true;
}

bool PluginRootContext::configure(size_t configuration_size) {
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
  if (!JsonArrayIterate(j, "basic_auth_rules",
                        [&](const json& configuration) -> bool {
                          return extractBasicAuthRule(
                              configuration, &basic_auth_configuration_);
                        })) {
    LOG_WARN(absl::StrCat("cannot parse plugin configuration JSON string: ",
                          configuration_data->view()));
    return false;
  }
  auto it = j.find("realm");
  if (it != j.end()) {
    auto realm_string = JsonValueAs<std::string>(it.value());
    if (realm_string.second != Wasm::Common::JsonParserResultDetail::OK) {
      LOG_WARN(absl::StrCat(
          "cannot parse realm in plugin configuration JSON string: ",
          configuration_data->view()));
      return false;
    }
    realm_ = realm_string.first.value();
  }
  return true;
}

FilterHeadersStatus PluginRootContext::check() {
  auto request_path_header = getRequestHeader(":path");
  std::string_view request_path = request_path_header->view();
  auto method = getRequestHeader(":method")->toString();
  auto method_iter = basic_auth_configuration_.find(method);
  // First we check if the request method is present in our container
  if (method_iter != basic_auth_configuration_.end()) {
    // We iterate through our vector of struct in order to find if the
    // request_path according to given match pattern, is part of the plugin's
    // configuration data. If that's the case we check the credentials
    FilterHeadersStatus header_status = FilterHeadersStatus::Continue;
    auto authorization_header = getRequestHeader("authorization");
    std::string_view authorization = authorization_header->view();
    for (auto& rules : basic_auth_configuration_[method]) {
      if (rules.pattern == MATCH_TYPE::Prefix) {
        if (absl::StartsWith(request_path, rules.request_path)) {
          header_status = credentialsCheck(rules, authorization);
        }
      } else if (rules.pattern == MATCH_TYPE::Exact) {
        if (rules.request_path == request_path) {
          header_status = credentialsCheck(rules, authorization);
        }
      } else if (rules.pattern == MATCH_TYPE::Suffix) {
        if (absl::EndsWith(request_path, rules.request_path)) {
          header_status = credentialsCheck(rules, authorization);
        }
      }
      if (header_status == FilterHeadersStatus::StopIteration) {
        return FilterHeadersStatus::StopIteration;
      }
    }
  }
  // If there's no match against the request method or request path it means
  // that they don't have any basic auth restriction.
  return FilterHeadersStatus::Continue;
}

FilterHeadersStatus PluginContext::onRequestHeaders(uint32_t, bool) {
  return rootContext()->check();
}

#ifdef NULL_PLUGIN

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm

#endif
