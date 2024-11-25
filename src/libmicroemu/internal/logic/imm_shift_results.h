#pragma once
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

enum class SRType : uint8_t {
  SRType_None = 0U,
  SRType_LSL = 1U,
  SRType_LSR = 2U,
  SRType_ASR = 3U,
  SRType_ROR = 4U,
  SRType_RRX = 5U
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

} // namespace libmicroemu::internal