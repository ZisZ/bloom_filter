#ifndef BLOOM_FILTER_BASE_H_
#define BLOOM_FILTER_BASE_H_

#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace bloom {

class BloomFilterInterface {
 public:
  virtual bool MayContain(const void* key, uint32_t len) const = 0;

  bool MayContain(const std::string& key) const {
    return MayContain(key.c_str(), key.length());
  }

  bool MayContain(const char* key) const {
    return MayContain(key, strlen(key));
  }

  /// try insert an unique key
  /// @retval true key doesn't exist before insert
  /// @retval false key exist or false positive (conflict) before insert
  virtual bool Insert(const void *key, uint32_t len) = 0;

  bool Insert(const char* key) {
    return Insert(key, strlen(key));
  }

  bool Insert(const std::string& key) {
    return Insert(key.c_str(), key.length());
  }
};

/**
 * A space-efficent probabilistic set for membership test,
 * false postives are possible, but false negatives are not.
 */

template <class Hasher>
class BloomFilterBase : public BloomFilterInterface {
  static const typename Hasher::ResultType InitialSeed = 0;
 public:
  virtual ~BloomFilterBase() {};

  /// total bit count
  uint64_t TotalBits() const {
    assert(initialized_);
    return num_bits_;
  }

  /// total memory used, in bytes
  uint64_t MemorySize() const {
    return GetBitmapSize();
  }

  uint64_t GetBitmapSize() const {
    assert(initialized_);
    return num_bytes_;
  }

  uint32_t HashNumber() const {
    assert(initialized_);
    return num_hash_functions_;
  }

  uint8_t* GetBitmap() {
    assert(initialized_);
    return bitmap_;
  }

  const uint8_t* GetBitmap() const {
    assert(initialized_);
    return bitmap_;
  }

  uint64_t Capacity() const {
    assert(initialized_);
    return element_capacity_;
  }

  double FalsePositiveProb() const {
    assert(initialized_);
    return false_positive_prob_;
  }

  int64_t GetElementCount() {
    assert(initialized_);
    return element_count_;
  }

  void SetElementCount(int64_t count) {
    assert(initialized_);
    element_count_ = count;
  }
 protected:
  virtual void Initialize(uint64_t element_capacity, double false_positive_prob) = 0;
  virtual void Initialize(uint64_t bitmap_byte_size, uint32_t num_hashes) = 0;

 protected:
  Hasher    hasher_;
  uint64_t  element_capacity_;
  double    false_positive_prob_;
  uint32_t  num_hash_functions_;
  uint8_t*  bitmap_;
  uint64_t  num_bits_;
  uint64_t  num_bytes_;
  int64_t   element_count_;
  bool      initialized_;
};

}  // namespace bloom

#endif  // BLOOM_FILTER_BASE_H_
