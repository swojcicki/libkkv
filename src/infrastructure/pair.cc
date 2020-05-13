/**
 *
 * @file    pair.cc
 * @author  damianzim
 * @date    2020-05-09
 *
 */

#include "infrastructure/pair.h"

#include <stdexcept>

#include "utility/utils.h"

namespace kkv {

namespace {

constexpr TByte kValueAsBytes          = 0x01U;

constexpr TByte kDisplacementProduct   = 0x02U;

constexpr TByte kPartitionDisplacement = 0x04U;
constexpr TByte kSectorDisplacement    = 0x08U;

constexpr TByte kDeleted               = 0x80U;

} // namespace anonymous

// PairConfig

bool PairConfig::IsValueAsBytes() const {
  return config_ & kValueAsBytes;
}

bool PairConfig::IsDisplacementProduct() const {
  return config_ & kDisplacementProduct;
}

bool PairConfig::IsPartitionDisplacement() const {
  return config_ & kPartitionDisplacement;
}

bool PairConfig::IsSectorDisplacement() const {
  return config_ & kSectorDisplacement;
}

bool PairConfig::IsDeleted() const {
  return config_ & kDeleted;
}

void PairConfig::SetValueAsBytes() {
  config_ |= kValueAsBytes;
}

void PairConfig::SetDisplacementProduct() {
  config_ |= kDisplacementProduct;
}

void PairConfig::SetPartitionDisplacement() {
  config_ |= kPartitionDisplacement;
}

void PairConfig::SetSectorDisplacement() {
  config_ |= kSectorDisplacement;
}

void PairConfig::SetDeleted() {
  config_ |= kDeleted;
}

TByte PairConfig::GetConfig() const { return config_; }

// Pair

Pair::Pair(const Slice& key, const Slice& value) : key_length_(key.Size()),
                                                   value_length_(value.Size()) {
  using namespace utils;
  if (!key_length_)
    throw std::length_error("The key length must be at least 1");

  data_.push_back('\0');

  data_.append(UnsignedToLittleEndianString<uint32_t>(key_length_));
  data_.append(key.Data(), key_length_);

  data_.append(UnsignedToLittleEndianString<uint32_t>(value_length_));
  data_.append(value.Data(), value.Size());
}

const char* Pair::Get() {
  data_.front() = GetConfig();

  return data_.c_str();
}

size_t Pair::Size() const {
  return constants::pair::kMinPairLength - 1 + key_length_ + value_length_;
}

std::string_view Pair::Key() const {
  using namespace constants::pair;
  return std::string_view(data_.c_str() + kConfigSize + kKeyLengthSize,
                          key_length_);
}

std::string_view Pair::Value() const {
  using namespace constants::pair;
  return std::string_view(data_.c_str() + kMinPairLength - 1 + key_length_,
                          value_length_);
}

} // namespace kkv
