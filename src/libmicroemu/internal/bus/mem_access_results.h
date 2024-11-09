#pragma once
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

enum class ReadStatusCode : u32 {
  kOk = 0U,
  kReadNotAllowed = 1U,
};

template <typename T> struct ReadResult {
  T content;
  ReadStatusCode status_code;
};

enum class WriteStatusCode : u32 {
  kOk = 0U,
  kWriteNotAllowed = 1U,
};

template <typename T> struct WriteResult {
  WriteStatusCode status_code;
};

} // namespace libmicroemu::internal