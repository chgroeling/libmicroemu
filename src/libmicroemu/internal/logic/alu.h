#pragma once
#include "libmicroemu/internal/logic/imm_shift_results.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/internal/utils/traits.h"
#include "libmicroemu/types.h"
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace libmicroemu {
namespace internal {

// Alias zur einfacheren Verwendung
template <typename T> using next_bigger_type_t = typename next_bigger_type<T>::type;

template <typename T> class Alu {
  // clang-format off
  static_assert(
    std::is_same<T, u32>::value ||
    std::is_same<T, u16>::value || 
    std::is_same<T, u8>::value,
    "Alu only works for u32, u16 and u8");
  // clang-format on
public:
  // Structure to hold the result of addition with carry
  struct AddWithCarryResult {
    T value;        // The result of the addition
    bool carry_out; // True if there was an unsigned carry out of the most significant bit
    bool overflow;  // True if there was a signed overflow during addition
  };

  // AddWithCarry function that performs addition with a carry-in, and handles both unsigned carry
  // and signed overflow
  static AddWithCarryResult AddWithCarry(const T &x, const T &y, const bool carry_in) {
    // Ensure that the type T is one of the unsigned integer types: u32, u16, or u8
    static_assert(std::is_same<T, u32>::value || std::is_same<T, u16>::value ||
                      std::is_same<T, u8>::value,
                  "AddWithCarry only works for u32, u16, and u8");

    // Define types for signed equivalent of T and the next bigger unsigned and signed types
    using U =
        next_bigger_type_t<T>; // Next bigger unsigned type that can hold the result of T + T + 1

    // Perform unsigned addition using the larger type U, including carry_in
    U unsigned_sum = static_cast<U>(x) + static_cast<U>(y) + static_cast<U>(carry_in);

    // Convert the result back to the original type T
    T result = static_cast<T>(unsigned_sum);

    // Calculate carry_out: if any bit beyond the most significant bit of T is set, carry occurred
    bool carry_out = unsigned_sum > std::numeric_limits<T>::max();

    // Calculate overflow: check if sign bits differ (using two's complement arithmetic rules)
    bool overflow = (((x ^ result) & (y ^ result)) >> (sizeof(T) * 8 - 1)) & 1;

    // Return the result, along with the carry out and overflow flags
    return AddWithCarryResult{result, carry_out, overflow};
  }

  static constexpr T OR(const T &x, const T &y) { return x | y; }
  static constexpr T EOR(const T &x, const T &y) { return x ^ y; }
  static constexpr T AND(const T &x, const T &y) { return x & y; }
  static T LSR(const T &x, u32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift >= 0);
    return x >> shift;
  }
  //------------------------

  static T ASR(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift >= 0);
    return static_cast<T>(static_cast<make_signed_type<T>>(x) >> shift);
  }

  struct ASR_C_Results {
    T result;
    bool carry_out;
  };

  static ASR_C_Results ASR_C(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift > 0);
    using Ts = make_signed_type_t<T>;

    const Ts extended_x = static_cast<Ts>(x);
    const T result = static_cast<T>(extended_x >> shift);

    const bool carry_out = ((extended_x >> (shift - 1U)) & 0x1U) == 1U;
    return ASR_C_Results{result, carry_out};
  }

  //-------------------------
  static T LSL(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift >= 0);
    return x << shift;
  }

  struct LSR_C_Results {
    T result;
    bool carry_out;
  };

  static LSR_C_Results LSR_C(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift > 0);

    using U = next_bigger_type_t<T>;
    const U extended_x = (static_cast<U>(x) << no_of_bits<T>::N) >> shift;

    const T result = static_cast<T>(
        BitManip<U>::template Slice1R<no_of_bits<U>::N - 1U, no_of_bits<T>::N>(extended_x));

    const bool carry_out =
        BitManip<U>::template Slice1R<no_of_bits<T>::N - 1, no_of_bits<T>::N - 1>(extended_x) == 1U;

    return LSR_C_Results{result, carry_out};
  }

  struct LSL_C_Results {
    T result;
    bool carry_out;
  };

  static LSL_C_Results LSL_C(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift > 0);

    using U = next_bigger_type_t<T>;
    const U extended_x = (static_cast<U>(x) << shift);
    const T result =
        static_cast<T>(BitManip<U>::template Slice1R<no_of_bits<T>::N - 1U, 0>(extended_x));

    const bool carry_out =
        BitManip<U>::template Slice1R<no_of_bits<T>::N, no_of_bits<T>::N>(extended_x) == 1U;

    return LSL_C_Results{result, carry_out};
  }

  static T ROR(const T &x, i32 shift) {
    if (shift == 0) {
      return x;
    } else {
      return ROR_C(x, shift).result;
    }
  }

  struct ROR_C_Results {
    T result;
    bool carry_out;
  };

  static ROR_C_Results ROR_C(const T &x, i32 shift) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.26
    assert(shift != 0U);
    constexpr auto N = no_of_bits<T>::N;
    const auto m = shift % N;
    const T result = LSR(x, m) | LSL(x, N - m);
    const auto carry_out = BitManip<T>::template Slice1R<N - 1U, N - 1U>(result);
    return ROR_C_Results{result, carry_out == 0x1U};
  }

  struct RXR_C_Results {
    T result;
    bool carry_out;
  };

  static RXR_C_Results RXR_C(const T &x, bool carry_in) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.27
    const u32 N = no_of_bits<T>::N;
    const auto carry_out = BitManip<T>::template Slice1R<0, 0>(x) == 0x1U;
    const T carry_in_t = carry_in ? static_cast<T>(0x1U) : static_cast<T>(0x0U);
    const T result = (x >> 1U) | (carry_in_t << (N - 1U));

    return RXR_C_Results{result, carry_out};
  }

  static T RXR(const T &x, bool carry_in) { return RXR_C(x, carry_in).result; }
  static ImmShiftResults DecodeImmShift(uint8_t type, uint8_t imm5) {
    switch (type & 0x3U) {
    case 0b00U: {
      return ImmShiftResults{SRType::SRType_LSL, imm5};
    }
    case 0b01U: {
      if (imm5 == 0x0U) {
        return ImmShiftResults{SRType::SRType_LSR, 32U};
      } else {
        return ImmShiftResults{SRType::SRType_LSR, imm5};
      }
    }
    case 0b10U: {
      if (imm5 == 0x0U) {
        return ImmShiftResults{SRType::SRType_ASR, 32U};
      } else {
        return ImmShiftResults{SRType::SRType_ASR, imm5};
      }
    }
    case 0b11U: {
      if (imm5 == 0x0U) {
        return ImmShiftResults{SRType::SRType_RRX, 1U};
      } else {
        return ImmShiftResults{SRType::SRType_ROR, imm5};
      }
    default: {
      assert(false);
      // should not happen
    }
    }
    }
    return ImmShiftResults{SRType::SRType_None, 0U};
  }
  static T Shift(T value, SRType type, T amount, bool carry_in) {
    const auto result = Shift_C(value, type, amount, carry_in);
    return result.result;
  }
  struct ShiftCRes {
    T result;
    bool carry_out;
  };
  static ShiftCRes Shift_C(T value, SRType type, T amount, bool carry_in) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.183
    if (amount == 0U) {
      return ShiftCRes{value, carry_in};
    }
    switch (type) {
    case SRType::SRType_LSL: {
      const auto res = LSL_C(value, amount);
      return ShiftCRes{res.result, res.carry_out};
    }
    case SRType::SRType_LSR: {
      const auto res = LSR_C(value, amount);
      return ShiftCRes{res.result, res.carry_out};
    }
    case SRType::SRType_ASR: {
      const auto res = ASR_C(value, amount);
      return ShiftCRes{res.result, res.carry_out};
    }
    case SRType::SRType_ROR: {
      const auto res = ROR_C(value, amount);
      return ShiftCRes{res.result, res.carry_out};
    }
    case SRType::SRType_RRX: {
      const auto res = RXR_C(value, carry_in);
      return ShiftCRes{res.result, res.carry_out};
      break;
    }
    default: {
      // should not happen
      assert(false);
      break;
    }
    }
    return ShiftCRes{value, carry_in}; // // should not happen
  }

private:
  /// \brief Constructor
  Alu() = delete;

  /// \brief Destructor
  ~Alu() = delete;

  /// \brief Copy constructor for Alu.
  /// \param r_src the object to be copied
  Alu(const Alu &r_src) = delete;

  /// \brief Copy assignment operator for Alu.
  /// \param r_src the object to be copied
  Alu &operator=(const Alu &r_src) = delete;

  /// \brief Move constructor for Alu.
  /// \param r_src the object to be copied
  Alu(Alu &&r_src) = delete;

  /// \brief Move assignment operator for Alu.
  /// \param r_src the object to be copied
  Alu &operator=(Alu &&r_src) = delete;
}; // namespace libmicroemu

using Alu8 = Alu<u8>;
using Alu16 = Alu<u16>;
using Alu32 = Alu<u32>;

} // namespace internal
} // namespace libmicroemu