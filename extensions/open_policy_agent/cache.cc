#include "extensions/open_policy_agent/cache.h"

const uint64_t MAX_NUM_ENTRY = 1000;

namespace {

uint64_t computeHash(const Payload &payload) {
  const uint64_t kMul = static_cast<uint64_t>(0x9ddfea08eb382d69);
  uint64_t h = 0;
  h += std::hash<std::string>()(payload.source_principal) * kMul;
  h += std::hash<std::string>()(payload.destination_workload) * kMul;
  h += std::hash<std::string>()(payload.request_method) * kMul;
  h += std::hash<std::string>()(payload.request_url_path) * kMul;
  return h;
}

}  // namespace

bool ResultCache::check(const Payload &param, uint64_t &hash, bool &allowed,
                        uint64_t timestamp) {
  hash = computeHash(param);
  auto iter = result_cache_.find(hash);
  if (iter == result_cache_.end()) {
    return false;
  }
  const auto &entry = iter->second;
  if (entry.second + valid_for_nanosec_ > timestamp) {
    use(hash);
    allowed = entry.first;
    return true;
  }
  auto recent_iter = pos_.find(hash);
  recent_.erase(recent_iter->second);
  pos_.erase(hash);
  result_cache_.erase(hash);
  return false;
}

void ResultCache::add(const uint64_t hash, bool result, uint64_t timestamp) {
  use(hash);
  result_cache_.emplace(hash, std::make_pair(result, timestamp));
}

void ResultCache::use(const uint64_t hash) {
  if (pos_.find(hash) != pos_.end()) {
    recent_.erase(pos_[hash]);
  } else if (recent_.size() >= MAX_NUM_ENTRY) {
    int old = recent_.back();
    recent_.pop_back();
    result_cache_.erase(old);
    pos_.erase(old);
  }
  recent_.push_front(hash);
  pos_[hash] = recent_.begin();
}
