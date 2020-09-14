#include "common.h"

// Decode the given string encoded in URI formatting
std::string percentDecode(std::string encoded);

// parse the http path into key-value pairs of parameters
QueryParams parsePath(std::string path);

// parse the http request body into key-value pairs of parameters
QueryParams parseBody(std::string body);

// parse the http cookie header value into key-value pairs
QueryParams parseCookie(std::string cookie);



