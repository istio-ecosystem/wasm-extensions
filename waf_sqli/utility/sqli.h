#include "common.h"
#include "libinjection/libinjection.h"
#include "libinjection/libinjection_sqli.h"

/**
 * Detect SQL injection on input string
 * Input
 *  - input: the string to be inspected
 * Output
 *  true if a SQL injection is detected, false if not
 */
bool detectSQLi(std::string input);

/**
 * Pick the keys for SQL injection detection
 * Input
 *  - params: a map of param key value pairs
 *  - include: a boolean
 *      if true, given keys are the only keys to be inspected
 *      if false, all but the given keys will be inspected
 *  - keys: a vector of keys to be included or excluded
 *  - log: a pointer to store the log messages of the process
 * Output
 *  a set of keys for SQL injection detection
 */
Keys pickKeysForDetection(QueryParams params, bool include, Keys keys, std::string* log);

/**
 * Detect SQL injection on given parameter pairs with configuration
 * Input
 *  - params: a map of param key value pairs
 *  - include: a boolean
 *      if true, given keys are the only keys to be inspected
 *      if false, all but the given keys will be inspected
 *  - keys: a vector of keys to be included or excluded
 *  - log: a pointer to store the log messages of the process
 * Output
 *   true if a SQL injection is detected, false if not
 */
bool detectSQLiOnParams(QueryParams params, bool include, Keys keys, std::string* log);
