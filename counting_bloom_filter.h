#ifndef COUNTER_BLOOM_FILTER_H_
#define COUNTER_BLOOM_FILTER_H_

#include "bloom_filter_base.h"
#include "glog/logging.h"

namespace bloom {

static const int32_t kCounterWidth = 4;

template <class Hasher>
class CountingBloomFilter : public BloomFilterBase<Hasher> {
  static const typename Hasher::ResultType InitialSeed = 0;
 public:
  CountingBloomFilter(uint64_t element_capacity, double false_positive_prob) {
    InitialClear();
    Initialize(element_capacity, false_positive_prob);
  }

  CountingBloomFilter(uint64_t bitmap_byte_size, uint32_t num_hashes) {
    InitialClear();
    Initialize(bitmap_byte_size, num_hashes);
  }

  virtual ~CountingBloomFilter() {
    Destroy();
  }

  /// try insert an unique key
  /// @retval true key doesn't exist before insert
  /// @retval false key exist or false positive (conflict) before insert
  bool Insert(const void *key, uint32_t len) {
    typename Hasher::ResultType seed = InitialSeed;

    bool ret = MayContain(key, len);
    if (ret == false) {
      for (uint32_t i = 0; i < this->num_hash_functions_; ++i) {
        seed = hasher_(key, len, seed);
        typename Hasher::ResultType hash = seed % num_counters_;
        Incre(hash);
      }
      this->element_count_++;
    }
    return ret;
  }

  bool Insert(const char* key) {
    return Insert(key, strlen(key));
  }

  bool Insert(const std::string& key) {
    return Insert(key.c_str(), key.length());
  }

  bool MayContain(const void *key, uint32_t len) const {
    typename Hasher::ResultType seed = InitialSeed;

    for (uint32_t i = 0; i < this->num_hash_functions_; ++i) {
      seed = hasher_(key, len, seed);

      typename Hasher::ResultType hash = seed % num_counters_;
      // LOG(INFO) << "Hash pos: " << hash;
      if (Exist(hash) == false) {
        return false;
      }
    }
    return true;
  }

  bool MayContain(const std::string& key) const {
    return MayContain(key.c_str(), key.length());
  }

  bool MayContain(const char* key) const {
    return MayContain(key, strlen(key));
  }

  void Delete(const void* key, uint32_t len) {
    typename Hasher::ResultType seed = InitialSeed;

    bool ret = MayContain(key, len);
    if (ret == true) {
      for (int64_t i = 0; i < this->num_hash_functions_; ++i) {
        seed = hasher_(key, len, seed);
        typename Hasher::ResultType hash = seed % num_counters_;
        Decre(hash);
      }
      this->element_count_--;
    }
  }

  void Delete(const char* key) {
    Delete(key, strlen(key));
  }

  void Delete(const std::string &key) {
    Delete(key.c_str(), key.length());
  }

  /// clear all keys
  void Clear() {
    assert(this->initialized_);
    memset(this->bitmap_, 0, this->num_bytes_);
    this->element_count_ = 0;
  }

  /// total counter count
  uint64 TotalCounters() const {
    assert(this->initialized_);
    return num_counters_;
  }
 private:
  /// helper function used by ctors to initialize all members
  void InitialClear() {
    this->element_capacity_ = 0;
    this->false_positive_prob_ = 1.0;
    this->num_hash_functions_ = 0;
    this->bitmap_ = NULL;
    this->num_bits_ = 0;
    this->num_counters_ = 0;
    this->num_bytes_ = 0;
    this->element_count_ = 0;
    this->initialized_ = false;
  }

  void Destroy() {
    if (this->bitmap_) {
      delete[] this->bitmap_;
    }
    InitialClear();
  }

  void UncheckedInitialize(uint64_t bitmap_byte_size, uint32_t num_hashes) {
    this->bitmap_ = new uint8_t[bitmap_byte_size];
    this->num_bytes_ = bitmap_byte_size;
    this->num_counters_ = bitmap_byte_size * CHAR_BIT / kCounterWidth;
    this->num_hash_functions_ = num_hashes;
    this->false_positive_prob_ = exp(-log(2.0) * num_hashes);
    this->element_capacity_ = static_cast<uint64_t>(
        this->num_counters_ * log(2.0) / this->num_hash_functions_);
    this->initialized_ = true;

    LOG(INFO) << "Bloom filter bytes: " << this->num_bytes_;
    LOG(INFO) << "Bloom filter hash fun number: " << this->num_hash_functions_;
    LOG(INFO) << "Bloom filter false positive prob: " << this->false_positive_prob_;
    LOG(INFO) << "Bloom filter element capacity: " << this->element_capacity_;
  }

 private:
  CountingBloomFilter(const CountingBloomFilter&);
  CountingBloomFilter& operator=(const CountingBloomFilter&);
  void Initialize(uint64_t element_capacity, double false_positive_prob) {
    double num_hashes = -log(false_positive_prob) / log(2.0);
    uint32_t num_hash_functions = static_cast<uint32_t>(ceil(num_hashes + 0.001));
    uint64_t num_counters = static_cast<uint64_t>(
        element_capacity * num_hash_functions / log(2.0));
    uint64_t num_bytes = (num_counters * kCounterWidth + CHAR_BIT - 1) / CHAR_BIT;  // round up
    num_counters = num_bytes * CHAR_BIT / kCounterWidth;

    if (num_counters == 0) {
      char message[128];
      snprintf(message, sizeof(message),
          "Num elements=%lu false_positive_prob=%g",
          element_capacity, false_positive_prob);
      throw std::runtime_error(message);
    }

    const uint64_t kMaxNumBytes =
        0x7FFFFFFFFFFFFFFF;
    if (num_bytes > kMaxNumBytes) {
      char message[128];
      snprintf(message, sizeof(message),
          "Bitmap too large, size=%lu, exceed int64 limitation", num_bytes);
      throw std::runtime_error(message);
    }

    Initialize(num_bytes, num_hash_functions);
  }

  void Initialize(uint64_t bitmap_byte_size, uint32_t num_hashes) {
    Destroy();
    UncheckedInitialize(bitmap_byte_size, num_hashes);
  }

  void Incre(uint64 index) {
    uint8_t* byte = &(this->bitmap_)[index * kCounterWidth / CHAR_BIT];
    bool highBit = (index * kCounterWidth % CHAR_BIT) > 0;

    if (highBit == true) {
      *byte += 0x10;
    } else {
      *byte += 0x01;
    }
  }

  void Decre(uint64 index) {
    uint8_t* byte = &(this->bitmap_)[index * kCounterWidth / CHAR_BIT];
    bool highBit = (index * kCounterWidth % CHAR_BIT) > 0;

    if ((highBit == true) && ((*byte & 0xf0) != 0)) {
      *byte -= 0x10;
    } else if ((highBit == false) && ((*byte & 0x0f) != 0)) {
      *byte -= 0x01;
    }
  }

  bool Exist(uint64 index) const {
    uint8_t* byte = &(this->bitmap_)[index * kCounterWidth / CHAR_BIT];
    bool highBit = (index * kCounterWidth % CHAR_BIT) > 0;

    uint8_t val = 0;
    if (highBit == true) {
      val = *byte & 0xf0;
    } else {
      val = *byte & 0x0f;
    }
    return val != 0;
  }

 private:
  uint64_t  num_counters_;
};

}  // namespace bloom

#endif  // COUNTER_BLOOM_FILTER_H_
