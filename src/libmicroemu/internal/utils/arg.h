#pragma once

#include "libmicroemu/register_id.h"
#include <cstddef>
#include <cstdint>

template <microemu::RegisterId N> class ArgConst {
public:
  static constexpr auto kVal = N;
  constexpr microemu::RegisterId Get() const { return N; }

  /// \brief Constructor
  constexpr ArgConst() = default;

  /// \brief Destructor
  ~ArgConst() = default;

  /// \brief Copy constructor for ArgConst.
  /// \param r_src the object to be copied
  constexpr ArgConst(const ArgConst &r_src) = default;

  /// \brief Copy assignment operator for ArgConst.
  /// \param r_src the object to be copied
  constexpr ArgConst &operator=(const ArgConst &r_src) = default;

  /// \brief Move constructor for ArgConst.
  /// \param r_src the object to be copied
  constexpr ArgConst(ArgConst &&r_src) = default;

  /// \brief Move assignment operator for ArgConst.
  /// \param r_src the object to be copied
  constexpr ArgConst &operator=(ArgConst &&r_src) = default;

  // Equality operator
  bool operator==(const ArgConst &other) const { return kVal == other.kVal; }

  // Inequality operator
  bool operator!=(const ArgConst &other) const { return !(*this == other); }
};

template <typename T> class Arg {
public:
  /// \brief Constructor
  Arg(T val) : val_(val) {}

  /// \brief Destructor
  ~Arg() = default;

  /// \brief Copy constructor for Arg.
  /// \param r_src the object to be copied
  Arg(const Arg &r_src) = default;

  /// \brief Copy assignment operator for Arg.
  /// \param r_src the object to be copied
  Arg &operator=(const Arg &r_src) = default;

  /// \brief Move constructor for Arg.
  /// \param r_src the object to be copied
  Arg(Arg &&r_src) = default;

  /// \brief Move assignment operator for Arg.
  /// \param r_src the object to be copied
  Arg &operator=(Arg &&r_src) = default;

  inline microemu::RegisterId Get() const { return static_cast<microemu::RegisterId>(val_); }

  // Equality operator
  bool operator==(const Arg &other) const { return val_ == other.val_; }

  // Inequality operator
  bool operator!=(const Arg &other) const { return !(*this == other); }

private:
  T val_;
};