#include "greatest.h"
#include "../utility/common.h"

/*********************************************
 * Definitions for checking equality of Keys *
 *********************************************/
int KeysEq(const void* expected, const void* result, void* udata) {
  const Keys* expected_keys = static_cast<const Keys*>(expected);
  const Keys* resulted_keys = static_cast<const Keys*>(result);
  if (expected_keys->size() != resulted_keys->size()) {
    return false;
  }
  for (auto key : *expected_keys) {
    if (resulted_keys->find(key) == resulted_keys->end()) {
      return false;
    }
  }
  return true;
}

int KeysPrint(const void* arg, void* udata) {
  return 0;
}

static greatest_type_info Keys_info = {
  KeysEq,
  KeysPrint,
};

TEST matchKeys(Keys arg1, Keys arg2) {
  ASSERT_EQUAL_T(&arg1, &arg2, &Keys_info, NULL);
  PASS();
}


/****************************************************
 * Definitions for checking equality of QueryParams *
 ****************************************************/

// check equality of two query params objects
static int ParamsEq(const void* expected, const void* resulted, void *udata) {
  const QueryParams* expected_params = static_cast<const QueryParams*>(expected);
  const QueryParams* resulted_params = static_cast<const QueryParams*>(resulted);
  if (expected_params->size() != resulted_params->size()) {
    return false;
  }
  for (auto param : *expected_params) {
    auto match = resulted_params->find(param.first);
    if (match == resulted_params->end() || match->second != param.second) {
      return false;
    }
  }
  return true;
}

// print a query params object
static int ParamsPrint(const void* t, void* udata) {
  return 0;
}

// pack the query params functions to pass in ASSERT_EQUAL_T
static greatest_type_info ParamsInfo = {
  ParamsEq,
  ParamsPrint,
};

// check if a key value pair exists in a query params object
TEST matchValue(QueryParams params, std::string key, std::string val) {
  ASSERT(params.find(key) != params.end());
  ASSERT_EQ(params.find(key)->second, val);
  PASS();
}

// TEST for checking equality of two query params objects
TEST matchParams(QueryParams arg1, QueryParams arg2) {
  ASSERT_EQUAL_T(&arg1, &arg2, &ParamsInfo, NULL);
  PASS();
}


