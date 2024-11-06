#pragma once

#include "libmicroemu/register_id.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>

/** \brief Register argument constant class
 *
 */
template <microemu::RegisterId Rid> class RArgConst {
public:
  static constexpr auto kRegId = Rid;
  constexpr microemu::RegisterId Get() const { return kRegId; }

  /// \brief Constructor
  constexpr RArgConst() = default;

  /// \brief Destructor
  ~RArgConst() = default;

  /// \brief Copy constructor for ArgConst.
  /// \param r_src the object to be copied
  constexpr RArgConst(const RArgConst &r_src) = default;

  /// \brief Copy assignment operator for ArgConst.
  /// \param r_src the object to be copied
  constexpr RArgConst &operator=(const RArgConst &r_src) = default;

  /// \brief Move constructor for ArgConst.
  /// \param r_src the object to be copied
  constexpr RArgConst(RArgConst &&r_src) = default;

  /// \brief Move assignment operator for ArgConst.
  /// \param r_src the object to be copied
  constexpr RArgConst &operator=(RArgConst &&r_src) = default;

  // Equality operator
  bool operator==(const RArgConst &other) const { return kRegId == other.kRegId; }

  // Inequality operator
  bool operator!=(const RArgConst &other) const { return !(*this == other); }
};

/** \brief Register argument class
 *
 * This class is used to represent a register argument.
 */
template <typename T> class RArg {
public:
  /// \brief Constructor
  RArg(T reg_id) : val_(reg_id) {
    static_assert(std::is_integral<T>::value, "T must be an integral type");
    static_assert(std::is_unsigned<T>::value, "T must be an unsigned type");
    static_assert(std::is_same_v<T, std::underlying_type_t<microemu::RegisterId>>,
                  "RArg only allows underlying type of RegisterId");
  }

  /// \brief Destructor
  ~RArg() = default;

  /// \brief Copy constructor for Arg.
  /// \param r_src the object to be copied
  RArg(const RArg &r_src) = default;

  /// \brief Copy assignment operator for Arg.
  /// \param r_src the object to be copied
  RArg &operator=(const RArg &r_src) = default;

  /// \brief Move constructor for Arg.
  /// \param r_src the object to be copied
  RArg(RArg &&r_src) = default;

  /// \brief Move assignment operator for Arg.
  /// \param r_src the object to be copied
  RArg &operator=(RArg &&r_src) = default;

  inline microemu::RegisterId Get() const { return static_cast<microemu::RegisterId>(val_); }

  // Equality operator
  bool operator==(const RArg &other) const { return val_ == other.val_; }

  // Inequality operator
  bool operator!=(const RArg &other) const { return !(*this == other); }

private:
  T val_;
};