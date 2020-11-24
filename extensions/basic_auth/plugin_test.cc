#include "extensions/basic_auth/plugin.h"

#include "extensions/common/wasm/base64.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/proxy-wasm/context.h"
#include "include/proxy-wasm/null.h"

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

NullPluginRegistry* context_registry_;
RegisterNullVmPluginFactory register_basic_auth_plugin("basic_auth", []() {
  return std::make_unique<NullPlugin>(basic_auth::context_registry_);
});

class MockContext : public proxy_wasm::ContextBase {
 public:
  MockContext(WasmBase* wasm) : ContextBase(wasm) {}

  MOCK_METHOD(BufferInterface*, getBuffer, (WasmBufferType));
  MOCK_METHOD(WasmResult, log, (uint32_t, std::string_view));
  MOCK_METHOD(WasmResult, getHeaderMapValue,
              (WasmHeaderMapType /* type */, std::string_view /* key */,
               std::string_view* /*result */));
  MOCK_METHOD(WasmResult, sendLocalResponse,
              (uint32_t /* response_code */, std::string_view /* body */,
               Pairs /* additional_headers */, uint32_t /* grpc_status */,
               std::string_view /* details */));
};

class BasicAuthTest : public ::testing::Test {
 protected:
  BasicAuthTest() {
    // Initialize test VM
    test_vm_ = createNullVm();
    wasm_base_ =
        std::make_unique<WasmBase>(std::move(test_vm_), "test-vm", "", "");
    wasm_base_->initialize("basic_auth");

    // Initialize host side context
    mock_context_ = std::make_unique<MockContext>(wasm_base_.get());
    current_context_ = mock_context_.get();

    ON_CALL(*mock_context_, log(testing::_, testing::_))
        .WillByDefault([](uint32_t, std::string_view m) {
          std::cerr << m << "\n";
          return WasmResult::Ok;
        });

    ON_CALL(*mock_context_, getHeaderMapValue(WasmHeaderMapType::RequestHeaders,
                                              testing::_, testing::_))
        .WillByDefault([&](WasmHeaderMapType, std::string_view header,
                           std::string_view* result) {
          if (header == ":path") {
            *result = path_;
          }
          if (header == ":method") {
            *result = method_;
          }
          if (header == "authorization") {
            *result = authorization_header_;
          }
          return WasmResult::Ok;
        });

    // Initialize Wasm sandbox context
    root_context_ = std::make_unique<PluginRootContext>(0, "");
    context_ = std::make_unique<PluginContext>(1, root_context_.get());
  }
  ~BasicAuthTest() override {}

  std::unique_ptr<WasmBase> wasm_base_;
  std::unique_ptr<WasmVm> test_vm_;
  std::unique_ptr<MockContext> mock_context_;

  std::unique_ptr<PluginRootContext> root_context_;
  std::unique_ptr<PluginContext> context_;

  std::string path_;
  std::string method_;
  std::string cred_;
  std::string authorization_header_;
};

TEST_F(BasicAuthTest, OnConfigureSuccess) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    },
    { 
      "exact": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "admin:admin", "admin2:admin2", "ok:test" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureNoRequestPath) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    {
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_FALSE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureMoreThanOneRequestPath) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "exact": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_FALSE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureNoRequestMethod) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_FALSE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureWrongRequestMethodType) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods": "GET",
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_FALSE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureNoCredentials) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_FALSE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, OnConfigureEmptyConfig) {
  std::string configuration = "{}";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));
}

TEST_F(BasicAuthTest, PrefixAllow) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/api/test";
  method_ = "GET";
  cred_ = "ok:test";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);

  path_ = "/api/config";
  method_ = "POST";
  cred_ = "admin2:admin2";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);

  path_ = "/api/config";
  method_ = "PUT";
  cred_ = "admin2:admin2";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);

  path_ = "/config";
  method_ = "PUT";
  cred_ = "admin2:admin2";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);
}

TEST_F(BasicAuthTest, PrefixDeny) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/api/test";
  method_ = "GET";
  cred_ = "wrong-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);

  path_ = "/api/config";
  method_ = "POST";
  cred_ = "admin2:admin2";
  authorization_header_ = Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);
}

TEST_F(BasicAuthTest, ExactAllow) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "exact": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/api";
  method_ = "GET";
  cred_ = "ok:test";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);

  path_ = "/api/config";
  method_ = "something";
  cred_ = "some-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);
}

TEST_F(BasicAuthTest, ExactDeny) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "exact": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/api";
  method_ = "GET";
  cred_ = "wrong-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);

  path_ = "/api";
  method_ = "POST";
  cred_ = "admin2:admin2";
  authorization_header_ = Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);
}

TEST_F(BasicAuthTest, SuffixAllow) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "suffix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/test/api";
  method_ = "GET";
  cred_ = "ok:test";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);

  path_ = "/api";
  method_ = "something";
  cred_ = "some-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::Continue);
}

TEST_F(BasicAuthTest, SuffixDeny) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "suffix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/test/api";
  method_ = "GET";
  cred_ = "wrong-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);

  path_ = "/test/api";
  method_ = "POST";
  cred_ = "admin2:admin2";
  authorization_header_ = Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);
}

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm
