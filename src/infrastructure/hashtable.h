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
  HashTable(uint16_t partitions, uint16_t slots)
      : slots_(slots), divisor_(partitions * slots) {};

  TPairDest GetDestination(const Slice& key);
  TPairDest GetDestination(const std::string_view& key);

  TPairDest IndexToPairDest(size_t index);

 private:
  uint16_t slots_;
  uint16_t divisor_;
};

} // namespace kkv

#endif  // KKV_INFRASTRUCTURE_HASHTABLE_H_
