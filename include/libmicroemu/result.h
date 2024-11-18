#pragma once
#include "libmicroemu/types.h"
#include <cstdlib>

#include <type_traits>

namespace libmicroemu {
enum class StatusCode : u32 {
  // clang-format off
  kSuccess = 0x0U, // Operation succedded successfully 
  
  // Unspecified errors
  kError = 0x1U,      // An unspecified error occured
  kUnexpected = 0x1002U,
  kUnsuporrted = 0x1003U,
  kNotImplemented = 0x1004U,

  // Generic errors
  kOutOfRange = 0x2001U,
  kIteratorExhausted = 0x2002U,
  kBufferTooSmall = 0x2003U,
  kOpenFileFailed = 0x2004U,
  
  // Memory related errors
  kMemInaccesible = 0x3001U, 
  kMemWriteNotAllowed = 0x3002U,

  // Decoder related errors
  kDecoderUnknownOpCode = 0x4001U,
  kDecoderUnpredictable = 0x4002U, // see Armv7-M Architecture Reference Manual Issue E.e p.818
  kDecoderUndefined = 0x4003U,

  // Executor related errors
  kExecutorUnpredictable = 0x5001U, // see Armv7-M Architecture Reference Manual Issue E.e p.818
  kExecutorUndefined = 0x5002U,
  kExecutorExitWithError = 0x5003U,
  
  // Elf reader related errors
  kElfNotValid = 0x6001U,
  kElfWrongHeader = 0x6002U,

  // Arm related errors
  kUsageFault = 0x7001U,

  // Execution result
  kMaxInstructionsReached = 0x8001U,

  // clang-format on
};

static constexpr const char *StatusCodeToString(const StatusCode status_code) noexcept {
  switch (status_code) {
  case StatusCode::kSuccess: {
    return "Success";
  }
  case StatusCode::kError: {
    return "Error";
  }
  case StatusCode::kOutOfRange: {
    return "OutOfRange";
  }
  case StatusCode::kMemInaccesible: {
    return "MemInaccesible";
  }
  case StatusCode::kIteratorExhausted: {
    return "IteratorExhausted";
  }
  case StatusCode::kDecoderUnknownOpCode: {
    return "DecoderUnknownOpCode";
  }
  case StatusCode::kDecoderUnpredictable: {
    return "DecoderUnpredictable";
  }
  case StatusCode::kDecoderUndefined: {
    return "DecoderUndefined";
  }
  case StatusCode::kExecutorUnpredictable: {
    return "ExecutorUnpredictable";
  }
  case StatusCode::kMemWriteNotAllowed: {
    return "MemWriteNotAllowed";
  }
  case StatusCode::kBufferTooSmall: {
    return "BufferTooSmall";
  }
  case StatusCode::kElfWrongHeader: {
    return "ElfWrongHeader";
  }
  case StatusCode::kElfNotValid: {
    return "ElfNotValid";
  }
  case StatusCode::kOpenFileFailed: {
    return "OpenFileFailed";
  }
  case StatusCode::kNotImplemented: {
    return "NotImplemented";
  }
  case StatusCode::kUnexpected: {
    return "Unexpected";
  }
  case StatusCode::kExecutorExitWithError: {
    return "ExecutorExitWithError";
  }
  case StatusCode::kExecutorUndefined: {
    return "ExecutorUndefined";
  }
  case StatusCode::kUsageFault: {
    return "UsageFault";
  }
  case StatusCode::kUnsuporrted: {
    return "Unsuporrted";
  }

  case StatusCode::kMaxInstructionsReached: {
    return "MaxInstructionsReached";
  }

  default: {
    return "UnknownStatusCode";
  }
  }
  return "UnknownStatusCode";
}

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