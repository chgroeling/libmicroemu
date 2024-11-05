#pragma once
#include "libmicroemu/types.h"
#include <cstdlib>

#include <type_traits>

namespace microemu {
enum class StatusCode : u32 {
  // clang-format off
  kScSuccess = 0x0u, // Operation succedded successfully 
  
  // Unspecified errors
  kScError = 0x1u,      // An unspecified error occured
  kScUnexpected = 0x1002u,
  kScUnsuporrted = 0x1003u,
  kScNotImplemented = 0x1004u,

  // Generic errors
  kScOutOfRange = 0x2001u,
  kScIteratorExhausted = 0x2002u,
  kScBufferTooSmall = 0x2003u,
  kScOpenFileFailed = 0x2004u,
  
  // Memory related errors
  kScMemInaccesible = 0x3001u, 
  kScMemWriteNotAllowed = 0x3002u,

  // Decoder related errors
  kScDecoderUnknownOpCode = 0x4001u,
  kScDecoderUnpredictable = 0x4002u, // see Armv7-M Architecture Reference Manual Issue E.e p.818
  kScDecoderUndefined = 0x4003u,

  // Executor related errors
  kScExecutorUnpredictable = 0x5001u, // see Armv7-M Architecture Reference Manual Issue E.e p.818
  kScExecutorUndefined = 0x5002u,
  kScExecutorExitWithError = 0x5003u,
  
  // Elf reader related errors
  kScElfNotValid = 0x6001u,
  kScElfWrongHeader = 0x6002u,

  // Arm related errors
  kScUsageFault = 0x7001u,

  // clang-format on
};

static constexpr const char *StatusCodeToString(const StatusCode &status_code) noexcept {
  switch (status_code) {
  case StatusCode::kScSuccess: {
    return "Success";
  }
  case StatusCode::kScError: {
    return "Error";
  }
  case StatusCode::kScOutOfRange: {
    return "OutOfRange";
  }
  case StatusCode::kScMemInaccesible: {
    return "MemInaccesible";
  }
  case StatusCode::kScIteratorExhausted: {
    return "IteratorExhausted";
  }
  case StatusCode::kScDecoderUnknownOpCode: {
    return "DecoderUnknownOpCode";
  }
  case StatusCode::kScDecoderUnpredictable: {
    return "DecoderUnpredictable";
  }
  case StatusCode::kScDecoderUndefined: {
    return "DecoderUndefined";
  }
  case StatusCode::kScExecutorUnpredictable: {
    return "ExecutorUnpredictable";
  }
  case StatusCode::kScMemWriteNotAllowed: {
    return "MemWriteNotAllowed";
  }
  case StatusCode::kScBufferTooSmall: {
    return "BufferTooSmall";
  }
  case StatusCode::kScElfWrongHeader: {
    return "ElfWrongHeader";
  }
  case StatusCode::kScElfNotValid: {
    return "ElfNotValid";
  }
  case StatusCode::kScOpenFileFailed: {
    return "OpenFileFailed";
  }
  case StatusCode::kScNotImplemented: {
    return "NotImplemented";
  }
  case StatusCode::kScUnexpected: {
    return "Unexpected";
  }
  case StatusCode::kScExecutorExitWithError: {
    return "ExecutorExitWithError";
  }
  case StatusCode::kScExecutorUndefined: {
    return "ExecutorUndefined";
  }
  case StatusCode::kScUsageFault: {
    return "UsageFault";
  }
  case StatusCode::kScUnsuporrted: {
    return "Unsuporrted";
  }
  }
  return "UnknownStatusCode";
}

template <typename T, typename TStatusCode = StatusCode> struct Result {
  using Content = T;
  using StatusCode = TStatusCode;

  const StatusCode status_code;
  const Content content;

  constexpr bool IsOk() const noexcept { return status_code == StatusCode::kScSuccess; }
  constexpr bool IsErr() const noexcept { return status_code != StatusCode::kScSuccess; }

  constexpr bool operator==(Result<T> const &rhs) const noexcept {
    return (rhs.status_code == this->status_code) && (rhs.content == this->content);
  }

  constexpr const Content &Unwrap() const noexcept {
    if (IsErr()) {
      abort();
    }
    return content;
  }

  const char *ToString() const noexcept { return StatusCodeToString(status_code); }

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

  constexpr bool IsOk() const noexcept { return status_code == StatusCode::kScSuccess; }
  constexpr bool IsErr() const noexcept { return status_code != StatusCode::kScSuccess; }

  constexpr bool operator==(Result<void> const &rhs) const noexcept {
    return (rhs.status_code == this->status_code);
  }

  constexpr void Unwrap() const noexcept {
    if (IsErr()) {
      abort();
    }
  }

  constexpr const char *ToString() const noexcept { return StatusCodeToString(status_code); }

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
  return Result<T>(StatusCode::kScSuccess, content);
}
static constexpr Result<void> Ok() noexcept { return Result<void>(StatusCode::kScSuccess); }

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

} // namespace microemu