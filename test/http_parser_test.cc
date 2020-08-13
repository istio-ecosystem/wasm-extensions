#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "utility/http_parser.h"

using ::testing::Eq;
using ::testing::UnorderedElementsAreArray;
using ::testing::IsEmpty;

MATCHER_P(ParamsEq, expected_params, "params does not match") {
  if (expected_params.size() != arg.size()) {
    return false;
  }
  for (auto param : expected_params) {
    auto match = arg.find(param.first);
    if (match == arg.end() || match->second != param.second) {
      return false;
    }
  }
  return true;
}

TEST(HttpParserTest, PercentDecodingTest) {
  ASSERT_EQ(percentDecode(""), "");
  ASSERT_EQ(percentDecode("not encoded"), "not encoded");
  ASSERT_EQ(percentDecode("_not-encoded_"), "_not-encoded_");

  const std::string json_string = R"EOF(
{
    "error": {
        "code": 401,
        "message": "Unauthorized"
    }
}
  )EOF";
  ASSERT_EQ(percentDecode("%0A{%0A    \"error\": {%0A        \"code\": 401,%0A   "
                          "     \"message\": \"Unauthorized\"%0A    }%0A}%0A  "), json_string);

  ASSERT_EQ(percentDecode("too%20lar%20"), "too lar ");
  ASSERT_EQ(percentDecode("too%20larg%e"), "too larg%e");
  ASSERT_EQ(percentDecode("too%20large%"), "too large%");
  ASSERT_EQ(percentDecode("too%25large"), "too%large");
  ASSERT_EQ(percentDecode("too%25!large/"), "too%!large/");
}

TEST(HttpParserTest, ParseCookieTest) {
  ASSERT_THAT(parseCookie("abc=def; token=abc123; Expires=Wed, 09 Jun 2021 10:18:14 GMT"),
              ParamsEq(QueryParams({
                {"abc", "def"},
                {"token", "abc123"},
                {"Expires", "Wed, 09 Jun 2021 10:18:14 GMT"}
              })));

  // cookies with bad formatting
  ASSERT_EQ(parseCookie("token1=abc123; = ").find("token1")->second, "abc123");
  ASSERT_EQ(parseCookie("token2=abc123;   ").find("token2")->second, "abc123");
  ASSERT_EQ(parseCookie("; token3=abc123;").find("token3")->second, "abc123");
  ASSERT_EQ(parseCookie("=; token4=\"abc123\"").find("token4")->second, "abc123");

  // cookies with quotes
  ASSERT_THAT(parseCookie("dquote=\"; quoteddquote=\"\"\"; leadingdquote=\"foobar;"),
              ParamsEq(QueryParams({
                {"dquote", "\""},
                {"quoteddquote", "\""},
                {"leadingdquote", "\"foobar"}})));
}

TEST(HttpParserTest, ParseBodyTest) {
  ASSERT_THAT(parseBody(""), ParamsEq(QueryParams()));
  ASSERT_THAT(parseBody("hello"), ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parseBody("hello="),ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parseBody("hello=&"),ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parseBody("hello=world"),
              ParamsEq(QueryParams({{"hello", "world"}})));
  ASSERT_THAT(parseBody("hello=&hello2=world2"),
              ParamsEq(QueryParams({{"hello", ""}, {"hello2", "world2"}})));
  ASSERT_THAT(parseBody("name=admin&level=trace"),
              ParamsEq(QueryParams({{"name", "admin"}, {"level", "trace"}})));
}

// Check that path queries are parsed properly
TEST(HttpParserTest, ParsePathTest) {
  ASSERT_THAT(parsePath("/hello"), ParamsEq(QueryParams()));
  ASSERT_THAT(parsePath("/hello?"), ParamsEq(QueryParams()));
  ASSERT_THAT(parsePath("/hello?hello"), ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parsePath("/hello?hello="),ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parsePath("/hello?hello=&"),ParamsEq(QueryParams({{"hello", ""}})));
  ASSERT_THAT(parsePath("/hello?hello=world"),
              ParamsEq(QueryParams({{"hello", "world"}})));
  ASSERT_THAT(parsePath("/hello?hello=&hello2=world2"),
              ParamsEq(QueryParams({{"hello", ""}, {"hello2", "world2"}})));
  ASSERT_THAT(parsePath("/logging?name=admin&level=trace"),
              ParamsEq(QueryParams({{"name", "admin"}, {"level", "trace"}})));
}
