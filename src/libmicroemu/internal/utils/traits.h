#pragma once
#include "libmicroemu/types.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace libmicroemu {
namespace internal {

template <typename T> struct make_signed_type;

template <> struct make_signed_type<u64> {
  using type = i64;
};
template <> struct make_signed_type<u32> {
  using type = i32;
};

template <> struct make_signed_type<u16> {
  using type = i16;
};

template <> struct make_signed_type<u8> {
  using type = i8;
};

template <typename T> using make_signed_type_t = typename make_signed_type<T>::type;

// ----------------------------------

template <typename T> struct no_of_bits;

template <> struct no_of_bits<u64> {
  static constexpr u32 N = 64U;
};

template <> struct no_of_bits<u32> {
  static constexpr u32 N = 32U;
};

template <> struct no_of_bits<u16> {
  static constexpr u16 N = 16U;
};

template <> struct no_of_bits<u8> {
  static constexpr u8 N = 8U;
};

template <typename T> struct next_bigger_type;

template <> struct next_bigger_type<u32> {
  using type = u64;
};

template <> struct next_bigger_type<u16> {
  using type = u32;
};

template <> struct next_bigger_type<u8> {
  using type = u16;
};

// Alias for easy use
template <typename T> using next_bigger_type_t = typename next_bigger_type<T>::type;

} // namespace internal
} // namespace libmicroemu