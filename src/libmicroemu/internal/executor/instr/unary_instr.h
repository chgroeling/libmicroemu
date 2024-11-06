#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Clz
/// see Armv7-M Architecture Reference Manual Issue E.e p.220
template <typename TInstrContext> class Clz1Op {
public:
  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {
    static_cast<void>(ictx);
    const u32 result = Bm32::CountLeadingZeros(rm);
    return OpResult{result, false, false};
  }
};

/// Mov
/// see Armv7-M Architecture Reference Manual Issue E.e p.293
template <typename TInstrContext> class Mov1Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    return OpResult{rm, (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Rrx
/// see Armv7-M Architecture Reference Manual Issue E.e p.340
template <typename TInstrContext> class Rrx1Op {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const u32 &rm) {

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    auto r_rrx = Alu32::Shift_C(rm, SRType::SRType_RRX, 1u,
                                (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    return OpResult{r_rrx.result, r_rrx.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const TArg1 &arg_m) {

    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    auto result = TOp::Call(ictx, rm);

    if (arg_d.Get() == RegisterId::kPc) {
      Pc::ALUWritePC(ictx.pstates, result.value);
      It::ITAdvance(ictx.pstates);
      return Ok(ExecResult{eflags});
    } else {
      Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);
    }
    if ((iflags & InstrFlags::kSetFlags) != 0u) {
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);

      // Clear N, Z, C, V flags
      apsr &=
          ~(ApsrRegister::kNMsk | ApsrRegister::kZMsk | ApsrRegister::kCMsk | ApsrRegister::kVMsk);

      apsr |= ((result.value >> 31u) & 0x1u) << ApsrRegister::kNPos;       // N
      apsr |= Bm32::IsZeroBit(result.value) << ApsrRegister::kZPos;        // Z
      apsr |= (result.carry_out == true ? 1u : 0u) << ApsrRegister::kCPos; // C
      apsr |= (result.overflow == true ? 1u : 0u) << ApsrRegister::kVPos;  // V
      SReg::template WriteRegister<SpecialRegisterId::kApsr>(ictx.pstates, apsr);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  UnaryInstr() = delete;

  /// \brief Destructor
  ~UnaryInstr() = delete;

  /// \brief Copy constructor for UnaryInstr.
  /// \param r_src the object to be copied
  UnaryInstr(const UnaryInstr &r_src) = default;

  /// \brief Copy assignment operator for UnaryInstr.
  /// \param r_src the object to be copied
  UnaryInstr &operator=(const UnaryInstr &r_src) = delete;

  /// \brief Move constructor for UnaryInstr.
  /// \param r_src the object to be copied
  UnaryInstr(UnaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for UnaryInstr.
  /// \param r_src the object to be copied
  UnaryInstr &operator=(UnaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu