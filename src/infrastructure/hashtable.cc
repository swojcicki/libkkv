/**
 *
 * @file    hashtable.cc
 * @author  damianzim
 * @date    2020-05-08
 *
 */

#include "infrastructure/hashtable.h"

#include "utility/utils.h"

namespace kkv {

TPairDest HashTable::GetDestination(const Slice& key) {
  return IndexToPairDest(GetSlotIndex(key));
}

size_t HashTable::GetSlotIndex(const Slice& key) {
  return hash::CRC32(key.Data(), key.Size()) % divisor_;
}

TPairDest HashTable::IndexToPairDest(const size_t index) {
  return {
    static_cast<uint16_t>(index / slots_),
    static_cast<uint16_t>(index % slots_)
  };
}

size_t HashTable::PairDestToIndex(const TPairDest& pair_dest) {
  return slots_ * pair_dest.partition + pair_dest.slot;
}

} // namespace kkv
