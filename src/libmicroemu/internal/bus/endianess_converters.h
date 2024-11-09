/// \file
/// \copyright Festo SE & Co. KG, Esslingen. All rights reserved.
/// \brief Contains the declaration of the BigToLittleEndianConverter class.

#pragma once

#include <cstddef>
#include <cstdint>

#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// \brief This class contains a function to convert the endianess from big
/// endian to little endian
class BigToLittleEndianConverter {
public:
  /// \brief Converts an arbitrary value from big endian representation to
  /// little endian representation.
  ///
  /// \tparam T The type of the value to be converted.
  /// \param value The value to be converted from big endian to little endian
  ///   representation.
  /// \return T the converted value
  template <typename T> static constexpr inline T Convert(const T &value) = delete;

private:
  /// \brief Constructor
  BigToLittleEndianConverter() = delete;

  /// \brief Destructor
  ~BigToLittleEndianConverter() = delete;

  /// \brief Copy constructor for BigToLittleEndianConverter.
  /// \param r_src the object to be copied
  BigToLittleEndianConverter(const BigToLittleEndianConverter &r_src) = delete;

  /// \brief Copy assignment operator for BigToLittleEndianConverter.
  /// \param r_src the object to be copied
  BigToLittleEndianConverter &operator=(const BigToLittleEndianConverter &r_src) = delete;

  /// \brief Move constructor for BigToLittleEndianConverter.
  /// \param r_src the object to be copied
  BigToLittleEndianConverter(BigToLittleEndianConverter &&r_src) = delete;

  /// \brief Move assignment operator for BigToLittleEndianConverter.
  /// \param r_src the object to be copied
  BigToLittleEndianConverter &operator=(BigToLittleEndianConverter &&r_src) = delete;
};

/// \brief Specialization for converting a u8 value from big endian to
/// little endian representation.
template <> constexpr inline u8 BigToLittleEndianConverter::Convert(const u8 &value) {
  return value;
}

/// \brief Specialization for converting a u16 value from big endian to
/// little endian representation.
template <> constexpr inline u16 BigToLittleEndianConverter::Convert<u16>(const u16 &value) {
  return ((value << 8U) & 0xFF00U) | ((value >> 8U) & 0x00FFU);
}

/// \brief Specialization for converting a u32 value from big endian to
/// little endian representation.
template <> constexpr inline u32 BigToLittleEndianConverter::Convert<u32>(const u32 &value) {
  return ((value << 24U) & 0xFF000000U) | ((value << 8U) & 0x00FF0000U) |
         ((value >> 8U) & 0x0000FF00U) | ((value >> 24U) & 0x000000FFU);
}

/// \brief Specialization for converting a u64 value from big endian to
/// little endian representation.
template <> constexpr inline u64 BigToLittleEndianConverter::Convert<u64>(const u64 &value) {
  return ((value << 56U) & 0xFF00000000000000U) | ((value << 40U) & 0x00FF000000000000U) |
         ((value << 24U) & 0x0000FF0000000000U) | ((value << 8U) & 0x000000FF00000000U) |
         ((value >> 8U) & 0x00000000FF000000U) | ((value >> 24U) & 0x0000000000FF0000U) |
         ((value >> 40U) & 0x000000000000FF00U) | ((value >> 56U) & 0x00000000000000FFU);
}

// -----------------------------------------------------------------------------

/// \brief This class contains a function to convert the endianess from little
/// endian to little endian.
///
/// For obvious reasons the mentioned conversion does nothing.
class LittleToLittleEndianConverter {
public:
  /// \brief Converts an arbitrary value from little endian representation to
  /// little endian representation.
  ///
  /// For obvious reasons the mentioned conversion does nothing.
  /// \tparam T The type of the value to be converted.
  /// \param value The value to be converted from little endian to little endian
  ///   representation.
  /// \return T the converted value
  template <typename T> static constexpr inline T Convert(const T &value) = delete;

private:
  /// \brief Constructor
  LittleToLittleEndianConverter() = delete;

  /// \brief Destructor
  ~LittleToLittleEndianConverter() = delete;

  /// \brief Copy constructor for LittleToLittleEndianConverter.
  /// \param r_src the object to be copied
  LittleToLittleEndianConverter(const LittleToLittleEndianConverter &r_src) = delete;

  /// \brief Copy assignment operator for LittleToLittleEndianConverter.
  /// \param r_src the object to be copied
  LittleToLittleEndianConverter &operator=(const LittleToLittleEndianConverter &r_src) = delete;

  /// \brief Move constructor for LittleToLittleEndianConverter.
  /// \param r_src the object to be copied
  LittleToLittleEndianConverter(LittleToLittleEndianConverter &&r_src) = delete;

  /// \brief Move assignment operator for LittleToLittleEndianConverter.
  /// \param r_src the object to be copied
  LittleToLittleEndianConverter &operator=(LittleToLittleEndianConverter &&r_src) = delete;
};

/// \brief Specialization for converting a u8 value from little endian to
/// little endian representation.
template <> constexpr inline u8 LittleToLittleEndianConverter::Convert(const u8 &value) {
  return value;
}

/// \brief Specialization for converting a u16 value from little endian to
/// little endian representation.
template <> constexpr inline u16 LittleToLittleEndianConverter::Convert<u16>(const u16 &value) {
  return value;
}

/// \brief Specialization for converting a u32 value from little endian to
/// little endian representation.
template <> constexpr inline u32 LittleToLittleEndianConverter::Convert<u32>(const u32 &value) {
  return value;
}

/// \brief Specialization for converting a u64 value from little endian to
/// little endian representation.
template <> constexpr inline u64 LittleToLittleEndianConverter::Convert<u64>(const u64 &value) {
  return value;
}

} // namespace internal
} // namespace libmicroemu