#include "http_parser.h"
#include <ctype.h>

std::string percentDecode(std::string encoded) {
  std::string decoded;
  decoded.reserve(encoded.size());
  for (size_t i = 0; i < encoded.size(); ++i) {
    char ch = encoded[i];
    if (ch == '+') {
      ch = ' ';
    }
    if (ch == '%' && i + 2 < encoded.size()) {
      const char& hi = encoded[i + 1];
      const char& lo = encoded[i + 2];
      if (isdigit(hi)) {
        ch = hi - '0';
      } else {
        ch = toupper(hi) - 'A' + 10;
      }

      ch *= 16;
      if (isdigit(lo)) {
        ch += lo - '0';
      } else {
        ch += toupper(lo) - 'A' + 10;
      }
      i += 2;
    }
    decoded.push_back(ch);
  }
  return decoded;
}

inline std::string slice(std::string str, int start, int end) {
  return str.substr(start, end - start);
}

QueryParams parseParameters(std::string data, size_t start, bool cookie = false) {
  QueryParams params;
  std::string delim = "&";
  if (cookie) {
    delim = ";";
  }

  while (start < data.size()) {
    size_t end = data.find(delim, start);
    if (end == std::string::npos) {
      end = data.size();
    }
    std::string param = slice(data, start, end);

    const size_t equal = param.find('=');
    if (equal != std::string::npos) {
      std::string key = percentDecode(slice(data, start, start + equal));
      std::string val = percentDecode(slice(data, start + equal + 1, end));

      // if the data is cookie, remove outer quotes
      if (cookie && val.size() >= 2 && val.back() == '"' && val[0] == '"') {
        val = slice(val, 1, val.size() - 1);
      }

      params.emplace(key, val);
    } else {
      std::string key = percentDecode(slice(data, start, end));
      params.emplace(key, "");
    }

    if (cookie) {
      start = data.find_first_not_of(" ", end + 1);
    } else {
      start = end + 1;
    }
  }
  return params;
}

QueryParams parsePath(std::string path) {
  size_t start = path.find('?');
  if (start == std::string::npos) {
    QueryParams params;
    return params;
  }
  start++;
  return parseParameters(path, start);
}

QueryParams parseBody(std::string body) {
  return parseParameters(body, 0);
}

QueryParams parseCookie(std::string cookie) {
  return parseParameters(cookie, 0, true);
}

