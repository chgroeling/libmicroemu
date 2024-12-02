/**
 * @file
 * @brief File contains enum class for status codes
 */
#pragma once

#include "libmicroemu/types.h"
#include <cstdlib>
#include <string_view>
#include <type_traits>

namespace libmicroemu {

/**
 * @brief Enum representing various status codes used throughout the system.
 */
enum class StatusCode : u32 {

  /** @brief Operation succeeded successfully. */
  kSuccess = 0x0U,

  // Unspecified errors

  /** @brief An unspecified error occurred. */
  kError = 0x1U,

  /** @brief An unexpected error occurred. */
  kUnexpected = 0x1002U,

  /** @brief Operation or feature is unsupported. */
  kUnsuporrted = 0x1003U,

  /** @brief Operation or feature is not implemented. */
  kNotImplemented = 0x1004U,

  // Generic errors

  /** @brief Value is out of the allowed range. */
  kOutOfRange = 0x2001U,

  /** @brief Iterator has been exhausted. */
  kIteratorExhausted = 0x2002U,

  /** @brief Provided buffer is too small. */
  kBufferTooSmall = 0x2003U,

  /** @brief Failed to open the file. */
  kOpenFileFailed = 0x2004U,

  // Memory related errors

  /** @brief Memory is inaccessible. */
  kMemInaccesible = 0x3001U,

  /** @brief Memory write operation is not allowed. */
  kMemWriteNotAllowed = 0x3002U,

  // Decoder related errors

  /** @brief Decoder encountered an unknown opcode. */
  kDecoderUnknownOpCode = 0x4001U,

  /**
   * @brief Decoder encountered an unpredictable operation.
   */
  kDecoderUnpredictable = 0x4002U,

  /** @brief Decoder encountered an undefined operation. */
  kDecoderUndefined = 0x4003U,

  // Executor related errors

  /**
   * @brief Executor encountered an unpredictable operation.
   */
  kExecutorUnpredictable = 0x5001U,

  /** @brief Executor encountered an undefined operation. */
  kExecutorUndefined = 0x5002U,

  /** @brief Executor exited with an error. */
  kExecutorExitWithError = 0x5003U,

  // ELF reader related errors

  /** @brief ELF file is not valid. */
  kElfNotValid = 0x6001U,

  /** @brief ELF file has a wrong header. */
  kElfWrongHeader = 0x6002U,

  // ARM related errors

  /** @brief Usage fault occurred. */
  kUsageFault = 0x7001U,

  // Execution result

  /** @brief Maximum instruction count reached. */
  kMaxInstructionsReached = 0x8001U,
};

/**
 * @brief Converts a status code to a string
 * @param status_code the status code to convert
 * @return a string representation of the status code
 */
static std::string_view StatusCodeToString(const StatusCode &status_code) noexcept {
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
} // namespace libmicroemu