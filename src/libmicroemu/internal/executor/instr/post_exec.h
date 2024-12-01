#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

struct OpResult {
  u32 value;
  bool carry_out;
  bool overflow;
};

class PostExecSetFlags {
public:
  template <typename TInstrContext, typename OpResult>
  static inline void Call(const TInstrContext &ictx, const OpResult &result) {
    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    // Clear N, Z, C, V flags
    apsr &=
        ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

    apsr |= ((result.value >> 31U) & 0x1U) << ApsrRegister::kNPos;           // N
    apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;            // Z
    apsr |= (result.carry_out == true ? 0x1U : 0x0U) << ApsrRegister::kCPos; // C
    apsr |= (result.overflow == true ? 0x1U : 0x0U) << ApsrRegister::kVPos;  // V
    ictx.cpua.template WriteRegister<SpecialRegisterId::kApsr>(apsr);
  }
};

class PostExecOptionalSetFlags {
public:
  template <typename TInstrContext, typename OpResult>
  static inline void Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                          const OpResult &result) {
    if ((iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U) {
      PostExecSetFlags::Call(ictx, result);
    }
  }
};

class PostExecAdvancePcAndIt {
public:
  template <typename TInstrContext>
  static inline void Call(const TInstrContext &ictx, const InstrFlagsSet &iflags) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);
  }
};

class PostExecBranch {
public:
  template <typename TInstrContext>
  static inline void Call(const TInstrContext &ictx, const me_adr_t &new_pc) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    Pc::BranchWritePC(ictx.cpua, new_pc);
    It::ITAdvance(ictx.cpua);
  }
};

class PostExecLoadWritePc {
public:
  template <typename TInstrContext>
  static inline Result<void> Call(const TInstrContext &ictx, const me_adr_t &new_pc) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    TRY(void, Pc::LoadWritePC(ictx.cpua, ictx.bus, new_pc));
    It::ITAdvance(ictx.cpua);
    return Ok();
  }
};

class PostExecBxWritePc {
public:
  template <typename TInstrContext>
  static inline Result<void> Call(const TInstrContext &ictx, const me_adr_t &new_pc) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    TRY(void, Pc::BXWritePC(ictx.cpua, ictx.bus, new_pc));
    It::ITAdvance(ictx.cpua);
    return Ok();
  }
};

class PostExecBlxWritePc {
public:
  template <typename TInstrContext>
  static inline void Call(const TInstrContext &ictx, const me_adr_t &new_pc) {
    using It = typename TInstrContext::It;
    using Pc = typename TInstrContext::Pc;

    Pc::BLXWritePC(ictx.cpua, new_pc);
    It::ITAdvance(ictx.cpua);
  }
};

class PostExecWriteRegPcExcluded {
public:
  template <typename TInstrContext, typename TArg>
  static void Call(const TInstrContext &ictx, const TArg &arg, const u32 &value) {
    assert(arg.Get() != RegisterId::kPc);
    ictx.cpua.WriteRegister(arg.Get(), value);
  }
};

class PostExecWriteRegPcIncluded {
public:
  template <typename TInstrContext, typename TArg>
  static Result<void> Call(const TInstrContext &ictx, const InstrFlagsSet &iflags, const TArg &arg,
                           const u32 &value, const bool is_aligned) {
    // This instruction is a branch instruction
    // It fails if the address from which is was loaded is not aligned
    if (arg.Get() == RegisterId::kPc) {
      if (is_aligned) {
        TRY(void, PostExecLoadWritePc::Call(ictx, value));
      } else {
        return Err(StatusCode::kExecutorUnpredictable);
      }
    } else {
      ictx.cpua.WriteRegister(arg.Get(), value);
      PostExecAdvancePcAndIt::Call(ictx, iflags);
    }
    return Ok();
  }
};

} // namespace libmicroemu::internal
