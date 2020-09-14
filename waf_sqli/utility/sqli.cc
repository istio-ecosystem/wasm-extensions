#include "sqli.h"

bool detectSQLi(std::string input) {
  struct libinjection_sqli_state state;
  char* input_char_str = const_cast<char*>(input.c_str());
  libinjection_sqli_init(&state, input_char_str, input.length(), FLAG_NONE);
  return libinjection_is_sqli(&state);
}

Keys pickKeysForDetection(QueryParams params, bool include, Keys keys, std::string* log) {
  Keys keys_to_inspect;
  if (include) {
    // include the given keys for detection
    keys_to_inspect = keys;
  } else {
    // include all but the given keys for detection
    for (auto param : params) {
      if (keys.find(param.first) == keys.end()) {
        keys_to_inspect.insert(param.first);
      }
    }
  }
  return keys_to_inspect;
}

bool detectSQLiOnParams(QueryParams params, bool include, Keys keys, std::string* log) {
  Keys keys_to_inspect = pickKeysForDetection(params, include, keys, log);
  for (auto key : keys_to_inspect) {
    auto param = params.find(key);

    if (param == params.end()) {
      log->append("key [" + key + "] not in params\n");
      continue;
    }

    if (detectSQLi(param->second) || detectSQLi(param->first)) {
      log->append("-- SQL injection detected");
      return true;
    }

    log->append("key [" + key + "] passed detection\n");
  }
  return false;
}
