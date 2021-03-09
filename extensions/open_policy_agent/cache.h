#include <list>
#include <string>
#include <unordered_map>

struct Payload {
  // Principal of source workload.
  std::string source_principal;

  // destination workload name.
  std::string destination_workload;

  // Request method.
  std::string request_method;

  // URL path of the request.
  std::string request_url_path;
};

// LRU cache for OPA policy check result.
class ResultCache {
 public:
  void setValidDuration(uint64_t valid_for_sec) {
    valid_for_nanosec_ = valid_for_sec * 1000000000;
  }

  // Check if a payload is in the cache. This will update last touched time.
  bool check(const Payload &payload, uint64_t &hash, bool &allowed,
             uint64_t timestamp);

  // Add an entry to check cache.
  void add(const uint64_t hash, bool result, uint64_t timestamp);

 private:
  void use(const uint64_t hash);

  uint64_t valid_for_nanosec_ = 10000000000;

  // LRU cache for OPA check result.
  std::unordered_map<
      uint64_t /* payload hash */,
      std::pair<bool /* result */, uint64_t /* insertion timestamp */>>
      result_cache_;
  std::list<uint64_t /* hash */> recent_;
  std::unordered_map<uint64_t /* hash */, std::list<uint64_t>::iterator> pos_;
};
