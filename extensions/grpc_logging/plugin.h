#include "extensions/grpc_logging/config.pb.h"
#include "extensions/grpc_logging/log.pb.h"
#include "proxy_wasm_intrinsics_lite.h"

class PluginContext;

// gRPC logging plugin root context.
class PluginRootContext : public RootContext {
 public:
  PluginRootContext(uint32_t id, std::string_view root_id);
  ~PluginRootContext() override = default;

  bool onConfigure(size_t) override;
  void onTick() override;
  bool onDone() override;

  void addLogEntry(PluginContext *stream_context);
  void flushLogBuffer();
  void sendLogRequest(bool ondone);

 private:
  std::string logging_service_address_;

  // Log request that is being written currently.
  std::unique_ptr<
      istio_ecosystem::wasm_extensions::grpc_logging::WriteLogRequest>
      cur_log_req_;

  // Count of buffered log entries.
  int log_entry_count_;

  // Log request that are buffered to be sent.
  std::vector<std::unique_ptr<
      istio_ecosystem::wasm_extensions::grpc_logging::WriteLogRequest>>
      req_buffer_;

  // gRPC service string contains gRPC call configuration for Wasm gRPC call.
  std::string grpc_service_;

  // gRPC callback handler functions.
  std::function<void(size_t)> success_callback_;
  std::function<void(GrpcStatus)> failure_callback_;

  // Record in flight export calls. When ondone is triggered, export call needs
  // to be zero before calling proxy_done.
  int in_flight_export_call_ = 0;

  // Indicates if the current exporting is triggered by root context onDone. If
  // this is true, gRPC callback needs to call proxy_done to indicate that async
  // call finishes.
  bool is_on_done_ = false;
};

// Per-stream context.
class PluginContext : public Context {
 public:
  explicit PluginContext(uint32_t id, ::RootContext *root)
      : Context(id, root) {}

  void onLog() override;

 private:
  inline PluginRootContext *rootContext() {
    return dynamic_cast<PluginRootContext *>(this->root());
  }
};
