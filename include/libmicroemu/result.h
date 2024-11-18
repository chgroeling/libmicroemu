#pragma once
#include "libmicroemu/status_code.h"
#include "libmicroemu/types.h"
#include <cstdlib>
#include <string_view>
#include <type_traits>

namespace libmicroemu {

template <typename T, typename TStatusCode = StatusCode> struct Result {
  using Content = T;
  using StatusCode = TStatusCode;

  const StatusCode status_code;
  const Content content;

  constexpr bool IsOk() const noexcept { return status_code == StatusCode::kSuccess; }
  constexpr bool IsErr() const noexcept { return status_code != StatusCode::kSuccess; }

  constexpr bool operator==(Result<T> const &rhs) const noexcept {
    return (rhs.status_code == this->status_code) && (rhs.content == this->content);
  }

  constexpr const Content &Unwrap() const noexcept {
    if (IsErr()) {
      abort();
    }
    return content;
  }

  std::string_view ToString() const noexcept { return StatusCodeToString(status_code); }

  explicit constexpr Result(StatusCode status_code, Content content) noexcept
      : status_code(status_code), content(content) {}

  // Move Constructor
  constexpr Result(Result &&r) noexcept = default;

  // Move Assignment
  constexpr Result &operator=(Result &&r) = delete; // not assignable

  // Copy Constructor
  constexpr Result(const Result &r) noexcept = default;

  // Copy Assignment
  constexpr Result &operator=(const Result &r) = delete;
};

template <> struct Result<void> {
  using Content = void;

  const StatusCode status_code;

  constexpr bool IsOk() const noexcept { return status_code == StatusCode::kSuccess; }
  constexpr bool IsErr() const noexcept { return status_code != StatusCode::kSuccess; }

  constexpr bool operator==(Result<void> const &rhs) const noexcept {
    return (rhs.status_code == this->status_code);
  }

  constexpr void Unwrap() const noexcept {
    if (IsErr()) {
      abort();
    }
  }

  constexpr std::string_view ToString() const noexcept { return StatusCodeToString(status_code); }

  constexpr Result(StatusCode status_code) noexcept : status_code(status_code) {}

  // Move Constructor
  constexpr Result(Result &&r) noexcept = default;

  // Move Assignment
  constexpr Result &operator=(Result &&r) = delete; // not assignable

  // Copy Constructor
  constexpr Result(const Result &r) noexcept = default;

  // Copy Assignment
  constexpr Result &operator=(const Result &r) = delete; // not assignable
};

// convience function
template <typename T> static constexpr Result<T> Ok(const T &content) noexcept {
  return Result<T>(StatusCode::kSuccess, content);
}
static constexpr Result<void> Ok() noexcept { return Result<void>(StatusCode::kSuccess); }

// convience function
template <typename T> static constexpr Result<T> Err(const StatusCode &sc) noexcept {
  return Result<T>(sc, T());
}

static constexpr Result<void> Err(const StatusCode &sc) noexcept { return Result<void>(sc); }

template <typename TIn, typename TRet>
static constexpr std::enable_if_t<!std::is_same_v<TRet, void>, Result<TRet>>
Err(const Result<TIn> &res) noexcept {
  return Result<TRet>(res.status_code, TRet());
}

template <typename TIn, typename TRet>
static constexpr std::enable_if_t<std::is_same_v<TRet, void>, Result<TRet>>
Err(const Result<TIn> &res) noexcept {
  return Result<TRet>(res.status_code);
}

#define TRY_ASSIGN(NAME, OUT_TYPE, CALL)                                                           \
  auto r_##NAME = CALL;                                                                            \
  if (r_##NAME.IsErr()) {                                                                          \
    using t_ret_decl = decltype(CALL);                                                             \
    using t_ret = typename t_ret_decl::Content;                                                    \
    return Err<t_ret, OUT_TYPE>(r_##NAME);                                                         \
  };                                                                                               \
  const auto &NAME = r_##NAME.content;

#define TRY(OUT_TYPE, CALL)                                                                        \
  {                                                                                                \
    auto r_tmp = CALL;                                                                             \
    if (r_tmp.IsErr()) {                                                                           \
      using t_ret_decl = decltype(CALL);                                                           \
      using t_ret = typename t_ret_decl::Content;                                                  \
      return Err<t_ret, OUT_TYPE>(r_tmp);                                                          \
    };                                                                                             \
  }

} // namespace libmicroemu