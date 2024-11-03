#pragma once
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

enum class SRType : uint8_t {
  SRType_None = 0u,
  SRType_LSL = 1u,
  SRType_LSR = 2u,
  SRType_ASR = 3u,
  SRType_ROR = 4u,
  SRType_RRX = 5u
};

struct ImmShiftResults {
  SRType type;
  uint8_t value;
  // ----
  const char *GetShiftTypeAsStr() const { return TypeToStr(type); }
  static const char *TypeToStr(SRType shift_type) {
    switch (shift_type) {
    case SRType::SRType_None: {
      return "NONE";
    }
    case SRType::SRType_LSL: {
      return "LSL";
    }
    case SRType::SRType_LSR: {
      return "LSR";
    }
    case SRType::SRType_ASR: {
      return "ASR";
    }
    case SRType::SRType_ROR: {
      return "ROR";
    }
    case SRType::SRType_RRX: {
      return "RRX";
    }
    default:
      break;
    }
    return "invalid";
  }
};
} // namespace internal
} // namespace microemu