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
  return IndexToPairDest(hash::CRC32(key.Data(), key.Size()) % Divisor());
}

TPairDest HashTable::GetDestination(const std::string_view& key) {
	return IndexToPairDest(hash::CRC32(key.data(), key.size()) % Divisor());
}

TPairDest HashTable::IndexToPairDest(const size_t index) {
  return {
    static_cast<uint16_t>(index / slots_),
    static_cast<uint16_t>(index % slots_)
  };
}

uint16_t HashTable::Divisor() const { return partitions_ * slots_; }

} // namespace kkv
