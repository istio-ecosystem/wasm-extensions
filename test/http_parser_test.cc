#include "test_common.h"
#include "../utility/http_parser.h"


/*********
 * TESTS *
 *********/
TEST PercentDecodingTest(void) {
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
  PASS();
}

TEST ParseCookieTest(void) {
  CHECK_CALL(matchParams(parseCookie("abc=def; token=abc123; Expires=Wed, 09 Jun 2021 10:18:14 GMT"),
                         QueryParams({
                           {"abc", "def"},
                           {"token", "abc123"},
                           {"Expires", "Wed, 09 Jun 2021 10:18:14 GMT"}
                         })));

  // cookies with bad formatting
  CHECK_CALL(matchValue(parseCookie("token1=abc123; = "), "token1", "abc123"));
  CHECK_CALL(matchValue(parseCookie("token2=abc123;   "), "token2", "abc123"));
  CHECK_CALL(matchValue(parseCookie("; token3=abc123;"), "token3", "abc123"));
  CHECK_CALL(matchValue(parseCookie("=; token4=\"abc123\""), "token4", "abc123"));

  // cookies with quotes
  CHECK_CALL(matchParams(
      QueryParams({
        {"dquote", "\""},
        {"quoteddquote", "\""},
        {"leadingdquote", "\"foobar"}
      }),
      parseCookie("dquote=\"; quoteddquote=\"\"\"; leadingdquote=\"foobar;")
  ));
  PASS();
}

TEST ParseBodyTest(void) {
  CHECK_CALL(matchParams(parseBody(""), QueryParams()));
  CHECK_CALL(matchParams(parseBody("hello"), QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parseBody("hello="), QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parseBody("hello=&"), QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parseBody("hello=world"),
                         QueryParams({{"hello", "world"}})));
  CHECK_CALL(matchParams(parseBody("hello=&hello2=world2"),
                         QueryParams({{"hello", ""}, {"hello2", "world2"}})));
  CHECK_CALL(matchParams(parseBody("name=admin&level=trace"),
                         QueryParams({{"name", "admin"}, {"level", "trace"}})));
  PASS();
}

TEST ParsePathTest(void) {
  CHECK_CALL(matchParams(parsePath("/hello"), QueryParams()));
  CHECK_CALL(matchParams(parsePath("/hello?"), QueryParams()));
  CHECK_CALL(matchParams(parsePath("/hello?hello"), QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parsePath("/hello?hello="),QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parsePath("/hello?hello=&"),QueryParams({{"hello", ""}})));
  CHECK_CALL(matchParams(parsePath("/hello?hello=world"),
                         QueryParams({{"hello", "world"}})));
  CHECK_CALL(matchParams(parsePath("/hello?hello=&hello2=world2"),
                         QueryParams({{"hello", ""}, {"hello2", "world2"}})));
  CHECK_CALL(matchParams(parsePath("/logging?name=admin&level=trace"),
                         QueryParams({{"name", "admin"}, {"level", "trace"}})));
  PASS();
}

SUITE(HttpParserTest) {
  RUN_TEST(PercentDecodingTest);
  RUN_TEST(ParseCookieTest);
  RUN_TEST(ParseBodyTest);
  RUN_TEST(ParsePathTest);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(HttpParserTest);
  GREATEST_MAIN_END();
}
