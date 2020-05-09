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
  uint16_t space_index = hash::CRC32(key.Data(), key.Size()) % divisor_;

  return {
    static_cast<uint16_t>(space_index / slots_),
    static_cast<uint16_t>(space_index % slots_)
  };
}

} // namespace kkv
