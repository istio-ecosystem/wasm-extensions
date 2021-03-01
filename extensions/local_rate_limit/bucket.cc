#include "extensions/local_rate_limit/bucket.h"

// Key for token bucket shared data.
constexpr char localRateLimitTokenBucket[] =
    "wasm_local_rate_limit.token_bucket";

// Key for token bucket last updated time.
constexpr char localRateLimitLastRefilled[] =
    "wasm_local_rate_limit.last_refilled";

bool getToken() {
  WasmDataPtr token_bucket_data;
  uint32_t cas;
  while (true) {
    // Get the current token left with cas (compare-and-swap), which will be
    // used in the set call below.
    if (WasmResult::Ok !=
        getSharedData(localRateLimitTokenBucket, &token_bucket_data, &cas)) {
      return false;
    }
    uint64_t token_left =
        *reinterpret_cast<const uint64_t *>(token_bucket_data->data());

    // If there is no token left, returns false so that request gets 429.
    if (token_left == 0) {
      return false;
    }

    // If there is token left, subtract it by 1, and try set it with cas.
    // If token bucket set fails because of cas mismatch, which indicates the
    // bucket is updated by other VMs, retry the whole process.
    token_left -= 1;
    auto res = setSharedData(
        localRateLimitTokenBucket,
        {reinterpret_cast<const char *>(&token_left), sizeof(token_left)}, cas);
    if (res == WasmResult::Ok) {
      // token bucket is updated successfully, returns true and let the request
      // go through.
      return true;
    }
    if (res == WasmResult::CasMismatch) {
      continue;
    }
    return false;
  }
}

void refillToken(uint64_t tokens_per_refill, uint64_t refill_interval_nanosec,
                 uint64_t max_tokens) {
  // Get last refill time, if it is less than refill interval, which indicates
  // the bucket is going to be refilled or has already been refilled by other
  // VMs.
  uint32_t last_update_cas;
  WasmDataPtr last_update_data;
  if (WasmResult::Ok != getSharedData(localRateLimitLastRefilled,
                                      &last_update_data, &last_update_cas)) {
    LOG_WARN(
        "failed to get last update time of the local rate limit token bucket");
    return;
  }
  uint64_t last_update =
      *reinterpret_cast<const uint64_t *>(last_update_data->data());
  uint64_t now = getCurrentTimeNanoseconds();
  if (now - last_update < refill_interval_nanosec) {
    return;
  }

  // Otherwise, try set last updated time. If updated failed because of cas
  // mismatch, the bucket is going to be refilled by other VMs.
  auto res = setSharedData(localRateLimitLastRefilled,
                           {reinterpret_cast<const char *>(&now), sizeof(now)},
                           last_update_cas);
  if (res == WasmResult::CasMismatch) {
    return;
  }
  if (res != WasmResult::Ok) {
    LOG_WARN("failed to set local rate limit token bucket last update time");
    return;
  }

  // Refill token bucket.
  WasmDataPtr token_bucket_data;
  uint32_t cas;
  while (true) {
    // Get token left with cas.
    if (WasmResult::Ok !=
        getSharedData(localRateLimitTokenBucket, &token_bucket_data, &cas)) {
      LOG_WARN("failed to get current local rate limit token bucket");
      break;
    }
    uint64_t token_left =
        *reinterpret_cast<const uint64_t *>(token_bucket_data->data());

    // Refill tokens, and update bucket with cas. If update failed because of
    // cas mismatch, retry refilling.
    token_left += tokens_per_refill;
    if (token_left > max_tokens) {
      token_left = max_tokens;
    }
    auto res = setSharedData(
        localRateLimitTokenBucket,
        {reinterpret_cast<const char *>(&token_left), sizeof(token_left)}, cas);
    if (res == WasmResult::CasMismatch) {
      continue;
    }
    if (res != WasmResult::Ok) {
      LOG_WARN("failed to refill local rate limit token bucket");
    }
    break;
  }
}

bool initializeTokenBucket(uint64_t initial_tokens) {
  // Check if the bucket is already initialized.
  WasmDataPtr last_update_data;
  if (WasmResult::Ok ==
      getSharedData(localRateLimitLastRefilled, &last_update_data)) {
    return true;
  }
  // If not yet initialized, set last update time to 0 and tokens left to
  // initial_tokens.
  uint64_t init_last_update = 0;
  auto res = setSharedData(localRateLimitLastRefilled,
                           {reinterpret_cast<const char *>(&init_last_update),
                            sizeof(init_last_update)});
  if (res == WasmResult::CasMismatch) {
    return true;
  }
  if (res != WasmResult::Ok) {
    LOG_WARN("failed to set local rate limit token bucket last update time");
    return false;
  }

  res = setSharedData(localRateLimitTokenBucket,
                      {reinterpret_cast<const char *>(&initial_tokens),
                       sizeof(initial_tokens)});
  if (res != WasmResult::Ok) {
    LOG_WARN("failed to initialize token bucket");
    return false;
  }
  return true;
}
