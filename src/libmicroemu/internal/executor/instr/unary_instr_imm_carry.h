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

/// Mov
/// see Armv7-M Architecture Reference Manual Issue E.e p.291
template <typename TInstrContext> class MovImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    return OpResult{imm_carry.out, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

/// Mvn
/// see Armv7-M Architecture Reference Manual Issue E.e p.291
template <typename TInstrContext> class MvnImmCarryOp {
public:
  using SReg = typename TInstrContext::SReg;

  static inline OpResult Call(const TInstrContext &ictx, const ThumbImmediateResult &imm_carry) {
    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);
    return OpResult{~imm_carry.out, imm_carry.carry_out,
                    (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk};
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstrImmCarry {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const ThumbImmediateResult &imm_carry) {

    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    auto result = TOp::Call(ictx, imm_carry);

    Reg::WriteRegister(ictx.pstates, arg_d.Get(), result.value);

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
  UnaryInstrImmCarry() = delete;

  /// \brief Destructor
  ~UnaryInstrImmCarry() = delete;

  /// \brief Copy constructor for UnaryInstrImmCarry.
  /// \param r_src the object to be copied
  UnaryInstrImmCarry(const UnaryInstrImmCarry &r_src) = default;

  /// \brief Copy assignment operator for UnaryInstrImmCarry.
  /// \param r_src the object to be copied
  UnaryInstrImmCarry &operator=(const UnaryInstrImmCarry &r_src) = delete;

  /// \brief Move constructor for UnaryInstrImmCarry.
  /// \param r_src the object to be copied
  UnaryInstrImmCarry(UnaryInstrImmCarry &&r_src) = delete;

  /// \brief Move assignment operator for UnaryInstrImmCarry.
  /// \param r_src the object to be copied
  UnaryInstrImmCarry &operator=(UnaryInstrImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace microemu