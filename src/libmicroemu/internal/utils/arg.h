#pragma once

#include <cstddef>
#include <cstdint>

template <typename T, int N> class ArgConst {
public:
  constexpr T Get() const { return static_cast<T>(N); }

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

  inline T Get() const { return val_; }

private:
  T val_;
};