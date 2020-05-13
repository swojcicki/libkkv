/**
 *
 * @file    utils.h
 * @author  damianzim
 * @date    2020-04-16
 *
 */

#ifndef KKV_UTILITY_UTILS_H_
#define KKV_UTILITY_UTILS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

namespace kkv {

namespace math {

inline bool IsPowerOf2(uint32_t a) { return a && !(a & (a - 1)); }

inline uint32_t RoundUpPowerOf2(uint32_t a) {
  a--;
  a |= a >> 1U;
  a |= a >> 2U;
  a |= a >> 4U;
  a |= a >> 8U;
  a |= a >> 16U;
  a++;

  return a;
}

} // namespace math

namespace hash {

uint32_t CRC32(const void* data, size_t length);
bool VerifyCRC32(const void* data, size_t length, uint32_t crc);

} // namespace hash

namespace utils {

static inline bool IsLittleEndian() {
  const union {
    uint32_t u;
    uint8_t c[4];
  } data = {1};

  return data.c[0];
}

inline uint16_t ChangeEndianness16(uint16_t value) {
  uint16_t result = 0;

  result |= (value & 0x00FFU) << 8U;
  result |= (value & 0xFF00U) >> 8U;

  return result;
}

inline uint32_t ChangeEndianness32(uint32_t value) {
  uint32_t result = 0;

  result |= (value & 0x000000FFU) << 24U;
  result |= (value & 0x0000FF00U) << 8U;
  result |= (value & 0x00FF0000U) >> 8U;
  result |= (value & 0xFF000000U) >> 24U;

  return result;
}

template<typename T, class = typename std::enable_if_t<std::is_unsigned_v<T>>>
inline T ChangeUnsignedEndianness(T value) {
  auto size = sizeof(T);

  if (size == 1) return value;

  const auto half_bits = size * 4;
  T result = 0;

  for (; size; size -= 2) {
		result |= (value & (T)0xFF << half_bits - size * 4) << (size - 1) * 8;
    result |= (value & (T)0xFF << half_bits + (size / 2 - 1) * 8)
        >> (size - 1) * 8;
  }

	return result;
}

template<typename T, class = typename std::enable_if_t<std::is_unsigned_v<T>>>
inline std::string UnsignedToString(T value) {
  std::string result;

  auto size = sizeof(T);
  while (size)
    result.push_back((value >> (--size * 8)) & 0xFFU);

  return result;
}

template<typename T, class = typename std::enable_if_t<std::is_unsigned_v<T>>>
std::string UnsignedToLittleEndianString(T value) {
  return UnsignedToString(IsLittleEndian()
                          ? value
                          : ChangeUnsignedEndianness<T>(value));
}

} // namespace utils

} // namespace kkv

#endif  // KKV_UTILITY_UTILS_H_
