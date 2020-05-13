/**
 *
 * @file    pair.h
 * @author  damianzim
 * @date    2020-05-09
 *
 */

#ifndef KKV_INFRASTRUCTURE_PAIR_H_
#define KKV_INFRASTRUCTURE_PAIR_H_

#include <string_view>

#include "common/types.h"
#include "kkv/slice.h"

namespace kkv {

namespace constants::pair {

constexpr auto kConfigSize = 1;
constexpr auto kKeyLengthSize = 4;
constexpr auto kValueLengthSize = 4;
constexpr auto kMinKeySize = 1;
constexpr auto kMinValueSize = 0;

constexpr auto kMinPairLength = kConfigSize + kKeyLengthSize +
                                kValueLengthSize + kMinKeySize + kMinValueSize;

} // namespace constants::pair

class PairConfig {
 public:
  PairConfig() : config_(0) {};
  explicit PairConfig(TByte config) : config_(config) {};

  [[nodiscard]] bool IsValueAsBytes() const;
  [[nodiscard]] bool IsDisplacementProduct() const;

  [[nodiscard]] bool IsSectorDisplacement() const;
  [[nodiscard]] bool IsPartitionDisplacement() const;

  [[nodiscard]] bool IsDeleted() const;

  void SetValueAsBytes();
  void SetDisplacementProduct();

  void SetSectorDisplacement();
  void SetPartitionDisplacement();

  void SetDeleted();

 protected:
  [[nodiscard]] TByte GetConfig() const;

 private:
  TByte config_;
};

class Pair : public PairConfig {
 public:
  Pair(const Slice& key, const Slice& value);

  const char* Get();
  [[nodiscard]] size_t Size() const;

  [[nodiscard]] std::string_view Key() const;
  [[nodiscard]] std::string_view Value() const;

 private:
  const uint32_t key_length_;
  const uint32_t value_length_;

  std::string data_;
};

} // namespace kkv

#endif  // KKV_INFRASTRUCTURE_PAIR_H_
