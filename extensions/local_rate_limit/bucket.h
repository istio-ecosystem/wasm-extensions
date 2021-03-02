#include "proxy_wasm_intrinsics.h"

// getToken try fetch a token from the local rate limit token buckets.
// Returns false if no token left, or any error returns when accessing the token
// bucket.
bool getToken();

// Refill token bucket.
void refillToken(uint64_t tokens_per_refill, uint64_t refill_interval_nanosec,
                 uint64_t max_tokens);

// Initialize token buckets.
bool initializeTokenBucket(uint64_t initial_tokens);
