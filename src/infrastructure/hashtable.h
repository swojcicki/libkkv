/**
 *
 * @file    hashtable.cc
 * @author  damianzim
 * @date    2020-05-08
 *
 */

#ifndef KKV_INFRASTRUCTURE_HASHTABLE_H_
#define KKV_INFRASTRUCTURE_HASHTABLE_H_

#include <string_view>

#include "kkv/slice.h"

namespace kkv {

struct TPairDest {
  uint16_t partition;
  uint16_t slot;
};

class HashTable {
 public:
  HashTable(const uint16_t &partitions, const uint16_t &slots)
      : partitions_(partitions), slots_(slots) {};

  TPairDest GetDestination(const Slice& key);
  TPairDest GetDestination(const std::string_view& key);

  TPairDest IndexToPairDest(size_t index);

 private:
  uint16_t Divisor() const;

  const uint16_t &partitions_;
  const uint16_t &slots_;
};

} // namespace kkv

#endif  // KKV_INFRASTRUCTURE_HASHTABLE_H_
