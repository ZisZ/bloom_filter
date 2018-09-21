//
//

#ifndef MURMUR_HASHER_H_
#define MURMUR_HASHER_H_

#include <stdint.h>

uint64_t MurmurHash64A(const void* key, int len, uint64_t seed);

namespace bloom {

struct Murmur2Hasher64A {
  typedef uint64_t ResultType;
  uint64_t operator()(const void* key, int len, uint64_t seed) const {
    return MurmurHash64A(key, len, seed);
  }
};

}  // namespace bloom

#endif  // MURMUR_HASHER_H_
