#pragma once
#include "libmicroemu/types.h"

namespace microemu::internal {

enum class ReadStatusCode : u32 {
  kOk = 0u,
  kReadNotAllowed = 1u,
};

template <typename T> struct ReadResult {
  T content;
  ReadStatusCode status_code;
};

enum class WriteStatusCode : u32 {
  kOk = 0u,
  kWriteNotAllowed = 1u,
};

template <typename T> struct WriteResult {
  WriteStatusCode status_code;
};

} // namespace microemu::internal