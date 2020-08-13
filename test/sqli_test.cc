#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "utility/sqli.h"

using ::testing::Eq;
using ::testing::UnorderedElementsAreArray;
using ::testing::IsEmpty;

// Sanity check that detectSQLiTest is working properly
TEST(SQLiTest, detectSQLiTest) {
  bool result;

  std::string valid_input = "valid-input";
  result = detectSQLi(valid_input);
  ASSERT_EQ(result, false);

  std::string invalid_input = "=1' AND 1=1";
  result = detectSQLi(invalid_input);
  ASSERT_EQ(result, true);
}

// Check that correct keys are picked for detection
TEST(SQLiTest, pickKeysTest) {
  std::string log;
  QueryParams params ({
    {"param1", "value1"},
    {"param2", "value2"},
    {"param3", "value3"}
  });

  // empty inclusion
  Keys keys0 ({});
  bool include0 = true;
  Keys picked0 = pickKeysForDetection(params, include0, keys0, &log);
  ASSERT_THAT(picked0, IsEmpty());

  // inclusion
  Keys keys1 ({"param1", "param3"});
  bool include1 = true;
  Keys picked1 = pickKeysForDetection(params, include1, keys1, &log);
  ASSERT_THAT(picked1, UnorderedElementsAreArray({"param1", "param3"}));

  // inclusion with missing key
  Keys keys2 ({"param1", "param5"});
  bool include2 = true;
  Keys picked2 = pickKeysForDetection(params, include2, keys2, &log);
  ASSERT_THAT(picked2, UnorderedElementsAreArray({"param1", "param5"}));

  // empty exclusion
  Keys keys3 ({});
  bool include3 = false;
  Keys picked3 = pickKeysForDetection(params, include3, keys3, &log);
  ASSERT_THAT(picked3, UnorderedElementsAreArray({"param1", "param2", "param3"}));

  // exclusion
  Keys keys4 ({"param1", "param3"});
  bool include4 = false;
  Keys picked4 = pickKeysForDetection(params, include4, keys4, &log);
  ASSERT_THAT(picked4, UnorderedElementsAreArray({"param2"}));

  // exclusion with missing key
  Keys keys5 ({"param1", "param5"});
  bool include5 = false;
  Keys picked5 = pickKeysForDetection(params, include5, keys5, &log);
  ASSERT_THAT(picked5, UnorderedElementsAreArray({"param2", "param3"}));
}

TEST(SQLiTest, detectSQLiOnParamsTest) {
  std::string log;
  std::string invalid_input = "=1' AND 1=1";
  QueryParams params ({
    {"param1", "value1"},
    {"param2", "value2"},
    {"param3", invalid_input},
  });

  // include: all included values are safe
  Keys keys0 ({"param1", "param2"});
  bool include0 = true;
  bool issqli0 = detectSQLiOnParams(params, include0, keys0, &log);
  ASSERT_THAT(issqli0, false);

  // include: some included values are unsafe
  Keys keys1 ({"param1", "param3"});
  bool include1 = true;
  bool issqli1 = detectSQLiOnParams(params, include1, keys1, &log);
  ASSERT_THAT(issqli1, true);

  // exclude: all included values are safe
  Keys keys2 ({"param1", "param3"});
  bool include2 = false;
  bool issqli2 = detectSQLiOnParams(params, include2, keys2, &log);
  ASSERT_THAT(issqli2, false);

  // exclude: some included values are unsafe
  Keys keys3 ({"param1", "param2"});
  bool include3 = false;
  bool issqli3 = detectSQLiOnParams(params, include3, keys3, &log);
  ASSERT_THAT(issqli3, true);

  // exclude: some included keys are unsafe
  QueryParams params_unsafe_key ({
    {"param1", "value1"},
    {"param2", "value2"},
    {"param3", invalid_input},
    {invalid_input, "value4"},
    {invalid_input, invalid_input},
  });
  Keys keys4 ({"param1", "param3"});
  bool include4 = false;
  bool issqli4 = detectSQLiOnParams(params_unsafe_key, include4, keys4, &log);
  ASSERT_THAT(issqli4, true);
}

