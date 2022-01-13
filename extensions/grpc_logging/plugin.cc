#include "extensions/grpc_logging/plugin.h"

#include "absl/strings/str_cat.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/util/time_util.h"

using google::protobuf::util::JsonParseOptions;
using google::protobuf::util::Status;
using istio_ecosystem::wasm_extensions::grpc_logging::PluginConfig;
using istio_ecosystem::wasm_extensions::grpc_logging::WriteLogRequest;

static RegisterContextFactory register_gRPCLogging(
    CONTEXT_FACTORY(PluginContext), ROOT_FACTORY(PluginRootContext));

PluginRootContext::PluginRootContext(uint32_t id, std::string_view root_id)
    : RootContext(id, root_id) {
  cur_log_req_ = std::make_unique<WriteLogRequest>();
  log_entry_count_ = 0;
}

bool PluginRootContext::onConfigure(size_t configuration_size) {
  // Parse configuration JSON string.
  std::string configuration = "{}";
  if (configuration_size > 0) {
    auto configuration_data = getBufferBytes(
        WasmBufferType::PluginConfiguration, 0, configuration_size);
    configuration = configuration_data->toString();
  }

  JsonParseOptions json_options;
  json_options.ignore_unknown_fields = true;
  PluginConfig config;
  Status status = JsonStringToMessage(configuration, &config, json_options);
  if (!status.ok()) {
    LOG_WARN(
        absl::StrCat("cannot parse logging plugin configuration JSON string ",
                     configuration, ", ", status.message().ToString()));
    return false;
  }

  if (config.logging_service().empty()) {
    LOG_WARN(absl::StrCat(
        "logging service has to be provided in the logging plugin config ",
        configuration));
    return false;
  }

  // Init gRPC call configuration.
  logging_service_address_ = config.logging_service();
  GrpcService grpc_service;
  grpc_service.mutable_google_grpc()->set_target_uri(logging_service_address_);
  grpc_service.SerializeToString(&grpc_service_);

  // Init gRPC callbacks.
  success_callback_ = [this](size_t) {
    LOG_DEBUG("successfully sent loggin request");
    in_flight_export_call_ -= 1;
    if (in_flight_export_call_ < 0) {
      LOG_WARN("in flight report call should not be negative");
    }
    if (in_flight_export_call_ <= 0 && is_on_done_) {
      // All works have been finished. The plugin is safe to be destroyed.
      proxy_done();
    }
  };

  failure_callback_ = [this](GrpcStatus status) {
    LOG_WARN(absl::StrCat(
        "Logging call error: ", std::to_string(static_cast<int>(status)),
        getStatus().second->toString()));
    in_flight_export_call_ -= 1;
    if (in_flight_export_call_ < 0) {
      LOG_WARN("in flight report call should not be negative");
    }
    if (in_flight_export_call_ <= 0 && is_on_done_) {
      proxy_done();
    }
  };

  // Start timer, which will trigger log report every 10s.
  proxy_set_tick_period_milliseconds(10000 /* milliseconds */);

  return true;
}

bool PluginRootContext::onDone() {
  if (req_buffer_.empty() && in_flight_export_call_ == 0) {
    // returning true to signal that the plugin still has pending work to be
    // done.
    return true;
  }
  // Flush out all log entries
  flushLogBuffer();
  sendLogRequest(/* ondone */ true);
  // returning false to signal that the plugin has finished all the works and is
  // safe to be destroyed.
  return false;
}

void PluginRootContext::onTick() {
  // Flush out all log entries
  flushLogBuffer();
  if (req_buffer_.empty()) {
    return;
  }
  sendLogRequest(/* ondone */ false);
}

void PluginRootContext::addLogEntry(PluginContext* stream) {
  auto* log_entries = cur_log_req_->mutable_log_entries();
  auto* new_entry = log_entries->Add();

  // Add log labels. Note the following logic assumes this extension
  // is running at a server sidecar.

  // Workload attributes.
  getValue({"source", "address"}, new_entry->mutable_source_address());
  getValue({"destination", "address"},
           new_entry->mutable_destination_address());
  getValue({"node", "metadata", "WORKLOAD_NAME"},
           new_entry->mutable_destination_workload());
  getValue({"node", "metadata", "NAMESPACE"},
           new_entry->mutable_destination_namespace());

  // Request attributes.
  int64_t response_code, timestamp, duration;
  getValue({"request", "time"}, &timestamp);
  getValue({"request", "duration"}, &duration);
  getValue({"request", "id"}, new_entry->mutable_request_id());
  getValue({"request", "host"}, new_entry->mutable_host());
  getValue({"request", "url_path"}, new_entry->mutable_path());
  getValue({"response", "code"}, &response_code);
  getValue({"request", "referer"}, new_entry->mutable_referer());
  getValue({"request", "user_agent"}, new_entry->mutable_user_agent());

  *new_entry->mutable_timestamp() =
      google::protobuf::util::TimeUtil::NanosecondsToTimestamp(timestamp);
  *new_entry->mutable_latency() =
      google::protobuf::util::TimeUtil::NanosecondsToDuration(duration);
  new_entry->set_response_code(response_code);

  log_entry_count_ += 1;
  if (log_entry_count_ >= 500) {
    flushLogBuffer();
  }
}

void PluginRootContext::flushLogBuffer() {
  if (log_entry_count_ <= 0) {
    return;
  }
  auto new_log_req = std::make_unique<WriteLogRequest>();
  cur_log_req_.swap(new_log_req);
  req_buffer_.emplace_back(std::move(new_log_req));
  log_entry_count_ = 0;
}

void PluginRootContext::sendLogRequest(bool ondone) {
    is_on_done_ = ondone;
    HeaderStringPairs initial_metadata;

    std::vector<std::unique_ptr<WriteLogRequest>>::iterator itr = req_buffer_.begin();
    while (itr != req_buffer_.end()) {
        const auto &req = *itr;
        auto result = grpcSimpleCall(
                grpc_service_,
                /* service name */
                "istio_ecosystem.wasm_extensions.grpc_logging.LoggingService",
                /* method name */ "WriteLog", initial_metadata, *req,
                /* time out in milliseconds */ 5000, success_callback_,
                failure_callback_);
        if (result != WasmResult::Ok) {
            LOG_WARN("failed to make stackdriver logging export call");
            break;
        }
        in_flight_export_call_ += 1;
        // delete req_buffer_ req data;
        itr = req_buffer_.erase(itr);
    }
}

void PluginContext::onLog() { rootContext()->addLogEntry(this); }
