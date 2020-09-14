#include "test_common.h"
#include "../utility/config_parser.h"

// Check that invalid json formatting will be handled
TEST InvalidFormat(void) {
  std::string log;
  Config config;
  bool result = parseConfig("invalid json format {}", &config, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log, "JSON parse error in configuration");
  PASS();
}

// Check that empty config string corresponds to default setting
TEST EmptyConfig(void) {
  std::string log;
  Config config;
  parseConfig("", &config, &log);

  ASSERT_EQ(config.content_type, URLENCODED);

  ASSERT_EQ(config.param_include, false);
  ASSERT_EQ(config.params.size(), 0);

  ASSERT_EQ(config.header_include, true);
  Keys expected = Keys({"referer", "user-agent"});
  ASSERT_EQUAL_T(&config.headers, &expected, &Keys_info, NULL);

  ASSERT_EQ(config.cookie_include, false);
  ASSERT_EQ(config.cookies.size(), 0);

  PASS();
}

// Check that query param inputs are parsed correctly
TEST QueryParamsConfig(void) {
  std::string log;
  bool result;

  // failure: missing content-type in config string
  Config config_none;
  std::string param_none = R"(
  {
    "query_param": {}
  }
  )";
  result = parseConfig(param_none, &config_none, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log, "missing content-type field under query_param");

  // failure: content-type not supported
  Config config_unsupported;
  std::string param_unsupported = R"(
  {
    "query_param": {
       "content-type": "unsupported-type"
    }
  }
  )";
  result = parseConfig(param_unsupported, &config_unsupported, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log,
    "invalid content type, only application/x-www-form-urlencoded is supported");

  // success: default config when no include/exclude is provided
  Config config_default;
  std::string param_default = R"(
  {
    "query_param": {
      "content-type": "application/x-www-form-urlencoded"
    }
  }
  )";
  result = parseConfig(param_default, &config_default, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_default.param_include, false);
  Keys expected_default = Keys({});
  ASSERT_EQUAL_T(&config_default.params, &expected_default, &Keys_info, NULL);

  // success: include is provided
  Config config_include;
  std::string param_include = R"(
  {
    "query_param": {
      "content-type": "application/x-www-form-urlencoded",
      "include": ["foo", "bar"]
    }
  }
  )";
  result = parseConfig(param_include, &config_include, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_include.param_include, true);
  Keys expected_include = Keys({"foo", "bar"});
  ASSERT_EQUAL_T(&config_include.params, &expected_include, &Keys_info, NULL);

  // success: exclude is provided
  Config config_exclude;
  std::string param_exclude = R"(
  {
    "query_param": {
      "content-type": "application/x-www-form-urlencoded",
      "exclude": ["foo", "bar"]
    }
  }
  )";
  result = parseConfig(param_exclude, &config_exclude, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_exclude.param_include, false);
  Keys expected_exclude = Keys({"foo", "bar"});
  ASSERT_EQUAL_T(&config_exclude.params, &expected_exclude, &Keys_info, NULL);

  // failure: both include and exclude are provided
  Config config_both;
  std::string param_both = R"(
  {
    "query_param": {
      "content-type": "application/x-www-form-urlencoded",
      "exclude": ["foo", "bar"],
      "include": []
    }
  }
  )";
  result = parseConfig(param_both, &config_both, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log, "include and exclude cannot both be present");

  PASS();
}

TEST HeaderConfig(void) {
  // note that the config objects passed into the parser are initialized to
  // default value. In default, the header field will include "user-agent" and
  // "referer" according to ModSecurity rule 942100.

  bool result;
  std::string log;

  // success: default config when no include/exclude is provided
  Config config_default;
  std::string param_default = R"(
  {
    "header": {}
  }
  )";
  result = parseConfig(param_default, &config_default, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_default.header_include, true);
  Keys expected_default = Keys({"user-agent", "referer"});
  ASSERT_EQUAL_T(&config_default.headers, &expected_default, &Keys_info, NULL);

  // success: include is provided
  Config config_include;
  std::string param_include = R"(
  {
    "header": {
      "include": ["foo"]
    }
  }
  )";
  result = parseConfig(param_include, &config_include, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_include.header_include, true);
  Keys expected_include = Keys({"foo", "referer", "user-agent"});
  ASSERT_EQUAL_T(&config_include.headers, &expected_include, &Keys_info, NULL);

  // success: exclude is provided
  Config config_exclude;
  std::string param_exclude = R"(
  {
    "header": {
      "exclude": ["foo", "bar", "user-agent"]
    }
  }
  )";
  result = parseConfig(param_exclude, &config_exclude, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_exclude.param_include, false);
  Keys expected_exclude = Keys({"foo", "bar", "user-agent"});
  ASSERT_EQUAL_T(&config_exclude.headers, &expected_exclude, &Keys_info, NULL);

  // failure: both include and exclude are provided
  Config config_both;
  std::string param_both = R"(
  {
    "header": {
      "exclude": ["foo", "bar"],
      "include": ["user-agent", "referer"]
    }
  }
  )";
  result = parseConfig(param_both, &config_both, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log, "include and exclude cannot both be present");

  PASS();
}

TEST CookieConfig(void) {
  bool result;
  std::string log;

  // success: default config when no include/exclude is provided
  Config config_default;
  std::string param_default = R"(
  {
    "cookie": {}
  }
  )";
  result = parseConfig(param_default, &config_default, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_default.cookie_include, false);
  Keys expected_default = Keys({});
  ASSERT_EQUAL_T(&config_default.cookies, &expected_default, &Keys_info, NULL);

  // success: include is provided
  Config config_include;
  std::string param_include = R"(
  {
    "cookie": {
      "include": ["foo", "bar"]
    }
  }
  )";
  result = parseConfig(param_include, &config_include, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_include.cookie_include, true);
  Keys expected_include = Keys({"foo", "bar"});
  ASSERT_EQUAL_T(&config_include.cookies, &expected_include, &Keys_info, NULL);

  // success: exclude is provided
  Config config_exclude;
  std::string param_exclude = R"(
  {
    "cookie": {
      "exclude": ["foo", "bar"]
    }
  }
  )";
  result = parseConfig(param_exclude, &config_exclude, &log);
  ASSERT_EQ(result, true);
  ASSERT_EQ(config_exclude.cookie_include, false);
  Keys expected_exclude = Keys({"foo", "bar"});
  ASSERT_EQUAL_T(&config_exclude.cookies, &expected_exclude, &Keys_info, NULL);

  // failure: both include and exclude are provided
  Config config_both;
  std::string param_both = R"(
  {
    "cookie": {
      "exclude": ["foo", "bar"],
      "include": ["foo"]
    }
  }
  )";
  result = parseConfig(param_both, &config_both, &log);
  ASSERT_EQ(result, false);
  ASSERT_EQ(log, "include and exclude cannot both be present");

  PASS();
}

SUITE(configTests) {
  RUN_TEST(InvalidFormat);
  RUN_TEST(EmptyConfig);
  RUN_TEST(QueryParamsConfig);
  RUN_TEST(HeaderConfig);
  RUN_TEST(CookieConfig);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv){
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(configTests);
  GREATEST_MAIN_END();
}
