#pragma once
#include "libmicroemu/internal/utils/traits.h"
#include "libmicroemu/types.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace libmicroemu {
namespace internal {

template <typename T> class BitManip {
  // clang-format off
    static_assert(
        std::is_same<T, u64>::value ||
        std::is_same<T, u32>::value ||
        std::is_same<T, u16>::value || 
        std::is_same<T, u8>::value,
        "BitManip only works for u32, u16 and u8");
  // clang-format on
public:
  /**
   * @brief Aligns the given address down to the nearest multiple of the
   * specified alignment.
   *
   * This function takes an address and aligns it downwards to the nearest
   * multiple of the alignment. If the alignment is 0, it simply returns the
   * original address.
   *
   * @tparam alignment The alignment value, must be a power of 2.
   * @tparam T The type of the address, typically an integer type.
   * @param address The address to be aligned.
   * @return The address aligned down to the nearest multiple of the specified
   * alignment.
   */
  template <T alignment> static constexpr T AlignDown(const T &address) {
    if (alignment == 0U)
      return address;
    return address & ~(alignment - 1U);
  }

  /**
   * @brief Aligns the given address up to the nearest multiple of the specified
   * alignment.
   *
   * This function takes an address and aligns it upwards to the nearest
   * multiple of the alignment. If the alignment is 0, it simply returns the
   * original address.
   *
   * @tparam alignment The alignment value, must be a power of 2.
   * @tparam T The type of the address, typically an integer type.
   * @param address The address to be aligned.
   * @return The address aligned up to the nearest multiple of the specified
   * alignment.
   */
  template <T alignment> static constexpr T AlignUp(const T &address) {
    if (alignment == 0U) {
      return address;
    }
    return (address + (alignment - 1U)) & ~(alignment - 1U);
  }

  /**
   * @brief Generates a mask for the bit 'bit'
   * @tparam bit The bit position.
   * @return The generated mask.
   */
  template <unsigned bit> static constexpr T GenerateBitMask() {
    return static_cast<T>(static_cast<T>(1U) << bit);
  }

  /**
   * @brief Generates a mask of 1 bits in the region [last_bit:first_bit]
   * @tparam last_bit The last bit position.
   * @tparam first_bit The first bit position.
   * @return The generated mask.
   */
  template <unsigned last_bit, unsigned first_bit> static constexpr T GenerateBitMask() {

    if constexpr (last_bit == sizeof(T) * 8U - 1U) {
      T constexpr msk = ~static_cast<T>(0U);
      return static_cast<T>(msk << first_bit);
    } else {
      auto constexpr shift_range = last_bit - first_bit + 1U;
      T constexpr one = static_cast<T>(1U);
      T constexpr msk = (one << shift_range) - one;
      return static_cast<T>(msk << first_bit);
    }
  }

  /**
   * @brief Generates a mask of 1 bits in the region [last_bit:first_bit]
   * @param last_bit The last bit position.
   * @param first_bit The first bit position.
   * @return The generated mask.
   */
  static constexpr T GenerateBitMask(unsigned last_bit, unsigned first_bit) {
    if (last_bit == sizeof(T) * 8U - 1U) {
      T constexpr msk = ~static_cast<T>(0U);
      return static_cast<T>(msk << first_bit);
    } else {
      auto shift_range = last_bit - first_bit + 1U;
      T constexpr one = static_cast<T>(1U);
      T msk = (one << shift_range) - one;
      return static_cast<T>(msk << first_bit);
    }
  }

  /**
   * @brief Isolates the given bit "bit" in the value
   * @tparam bit The bit position.
   * @param value The value to isolate the bit from.
   * @return The isolated bit.
   */
  template <unsigned bit> static constexpr T IsolateBit(const T &value) {
    return (value & (static_cast<T>(1U) << bit)) >> bit;
  }

  /**
   * @brief Checks if the given bit "bit" is set in the value
   * @tparam bit The bit position.
   * @param value The value to check.
   * @return True if the bit is set, false otherwise.
   */
  template <unsigned bit> static constexpr bool IsBitSet(const T &value) {
    return (value & (static_cast<T>(1U) << bit)) != 0;
  }

  /**
   * @brief Extracts bits from value in the region [last_bit:first_bit]
   * @tparam last_bit The last bit position.
   * @tparam first_bit The first bit position.
   * @param value The value to extract bits from.
   * @return The extracted bits.
   */
  template <unsigned last_bit, unsigned first_bit>
  static constexpr T ExtractBits1R(const T &value) {
    return (value & GenerateBitMask<last_bit, first_bit>()) >> first_bit;
  }

  /**
   * @brief Extracts bits from an value and returns the result as the specified
   * type.
   *
   * This function takes an value and extracts bits starting from the specified
   * byte position. The extracted bits are then casted to the specified type and returned.
   *
   * @tparam U The type to cast the extracted bits to.
   * @param value The value to extract bits from.
   * @param start_byte The starting byte position of the bits to extract.
   * @return The extracted bits casted to the specified type.
   */
  template <typename U> static U ExtractType(T value, u32 start_byte) {
    assert(start_byte >= 0U && start_byte <= (sizeof(T) - 1U));
    auto bitpos = start_byte * 8U;
    auto constexpr bitmask = GenerateBitMask<sizeof(U) * 8U - 1U, 0U>() - 1U;
    return static_cast<U>((value >> bitpos) & bitmask);
  }

  /**
   * @brief Inserts bits of a shorter type into a value at the specified byte position.
   * @tparam U The type of the bits to insert.
   * @param value The value to insert the bits into.
   * @param start_byte The starting byte position to insert the value.
   * @param insert_val The value of the type to be inserted.
   * @return The value with the inserted bits.
   */
  template <typename U> static T InsertType(T value, u32 start_byte, U insert_val) {
    assert(start_byte >= 0U && start_byte <= (sizeof(T) - 1U));
    auto start_pos = start_byte * 8U;
    auto end_pos = start_pos + sizeof(U) * 8 - 1U;

    auto mask = GenerateBitMask(end_pos, start_pos);
    auto shifted_bits = static_cast<T>(insert_val) << start_pos;
    return (value & (~mask)) | (shifted_bits & mask);
  }

  /**
   * @brief Extracts bits from value in 2 bit regions and concatenates them.
   * @tparam r2_last_bit The last bit position of the second region.
   * @tparam r2_first_bit The first bit position of the second region.
   * @tparam r1_last_bit The last bit position of the first region.
   * @tparam r1_first_bit The first bit position of the first region.
   * @param value The value to extract bits from.
   * @return The concatenated bits.
   */
  template <unsigned r2_last_bit, unsigned r2_first_bit, unsigned r1_last_bit,
            unsigned r1_first_bit>
  static constexpr T ExtractBits2R(const T &value) {
    return ((value & GenerateBitMask<r2_last_bit, r2_first_bit>()) >>
            (r2_first_bit - r1_last_bit + r1_first_bit - 1U)) +
           ((value & GenerateBitMask<r1_last_bit, r1_first_bit>()) >> (r1_first_bit));
  }

  /**
   * @brief Takes a value of type T and converts it to type U by extending the missing bits with 0.
   * @tparam U The type to convert to.
   * @param value The value to convert.
   * @return The converted value.
   */
  template <typename U> static inline U ZeroExtend(const T &value) {
    static_assert(std::is_same<U, u32>::value || std::is_same<U, u16>::value ||
                      std::is_same<U, u8>::value,
                  "ZeroExtend only works for u32, u16 and u8");
    return static_cast<U>(value);
  }

  /**
   * @brief Takes a value of type T and converts it to type U by extending the given top bit.
   * @tparam U The type to convert to.
   * @tparam top_bit The top bit position.
   * @param value The value to convert.
   * @return The converted value.
   */
  template <typename U, unsigned top_bit> static inline U SignExtend(const T &value) {
    static_assert(std::is_same<U, u32>::value || std::is_same<U, u16>::value ||
                      std::is_same<U, u8>::value,
                  "ZeroExtend only works for u32, u16 and u8");
    T s = (value >> top_bit) & 0x1U;
    U bitmask = BitManip<U>::template GenerateBitMask<no_of_bits<U>::N, top_bit>();
    U value_u = static_cast<U>(value);
    if (s) {
      value_u = value_u | bitmask;
    } else {
      value_u = value_u & ~bitmask;
    }
    return value_u;
  }

  /**
   * @brief Checks if the given value is zero.
   * @param value The value to check.
   * @return 1 if the value is zero, 0 otherwise.
   */
  static T IsZeroBit(const T &value) {
    return value == static_cast<T>(0U) ? static_cast<T>(1U) : static_cast<T>(0U);
  }

  /**
   * @brief Counts the number of bits set in the given value.
   * @param value The value to count the bits.
   * @return The number of bits set.
   */
  static T BitCount(const T &value) {
    T v{value};
    T c{0U}; // c accumulates the total bits set in v
    for (; v; c++) {
      v &= v - 1U; // clear the least significant bit set
    }
    return c;
  }

  /**
   * @brief Finds the position of the lowest bit set in the given value.
   * @param value The value to find the lowest bit set.
   * @return The position of the lowest bit set.
   */
  static T LowestBitSet(const T &value) {
    T x{value};
    if (x == 0)
      return no_of_bits<T>::N;

    T result = 0;
    while ((x & 1) == 0) {
      x >>= 1;
      result++;
    }

    return result;
  }
  static T CountLeadingZeros(const T &value) {
    int n{32U};
    u32 y{0U};
    u32 x{value};

    y = x >> 16U;
    if (y != 0U) {
      n = n - 16U;
      x = y;
    }
    y = x >> 8U;
    if (y != 0U) {
      n = n - 8U;
      x = y;
    }
    y = x >> 4U;
    if (y != 0U) {
      n = n - 4U;
      x = y;
    }
    y = x >> 2U;
    if (y != 0U) {
      n = n - 2U;
      x = y;
    }
    y = x >> 1U;
    if (y != 0U) {
      return n - 2U;
    }

    return n - x;
  }

private:
  /// \brief Constructor
  BitManip() = delete;

  /// \brief Destructor
  ~BitManip() = delete;

  /// \brief Copy constructor for BitManip.
  /// \param r_src the object to be copied
  BitManip(const BitManip &r_src) = delete;

  /// \brief Copy assignment operator for BitManip.
  /// \param r_src the object to be copied
  BitManip &operator=(const BitManip &r_src) = delete;

  /// \brief Move constructor for BitManip.
  /// \param r_src the object to be copied
  BitManip(BitManip &&r_src) = delete;

  /// \brief Move assignment operator for BitManip.
  /// \param r_src the object to be copied
  BitManip &operator=(BitManip &&r_src) = delete;
};

using Bm8 = BitManip<u8>;
using Bm16 = BitManip<u16>;
using Bm32 = BitManip<u32>;

} // namespace internal
} // namespace libmicroemu