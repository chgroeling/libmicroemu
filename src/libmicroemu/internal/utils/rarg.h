#pragma once

#include "libmicroemu/register_id.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>

/** \brief Register argument constant class
 *
 */
template <libmicroemu::RegisterId Rid> class RArgConst {
public:
  static constexpr auto kRegId = Rid;
  constexpr libmicroemu::RegisterId Get() const { return kRegId; }

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
  template <typename R> bool operator==(const R &other) const { return Get() == other.Get(); }

  // Inequality operator
  template <typename R> bool operator!=(const R &other) const { return !(Get() == other.Get()); }
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
    static_assert(std::is_same_v<T, std::underlying_type_t<libmicroemu::RegisterId>>,
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

  inline libmicroemu::RegisterId Get() const { return static_cast<libmicroemu::RegisterId>(val_); }

  // Equality operator
  template <typename R> bool operator==(const R &other) const { return Get() == other.Get(); }

  // Inequality operator
  template <typename R> bool operator!=(const R &other) const { return !(Get() == other.Get()); }

private:
  T val_;
};