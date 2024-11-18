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
 * @brief Enum class for status codes
 * Used to indicate the status of an operation
 */
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

/**
 * @brief Converts a status code to a string
 * @param status_code the status code to convert
 * @return a string representation of the status code
 */
static std::string_view StatusCodeToString(const StatusCode status_code) noexcept {
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