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

/// Add to pc
/// see Armv7-M Architecture Reference Manual Issue E.e p.190
template <typename TInstrContext> class AddToPcImmOp {
public:
  using Reg = typename TInstrContext::Reg;
  static inline OpResult Call(const TInstrContext &ictx, const InstrFlagsSet &iflags,
                              const u32 &imm32) {
    static_cast<void>(ictx);
    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));
    const auto apc = Bm32::AlignDown<4>(pc);
    const bool is_add = (iflags & kAdd) != 0u;
    const auto result = is_add != false ? apc + imm32 : apc - imm32;
    return OpResult{result, false, false};
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg0>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_d, const u32 &imm32) {

    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    auto result = TOp::Call(ictx, iflags, imm32);

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
  UnaryInstrImm() = delete;

  /// \brief Destructor
  ~UnaryInstrImm() = delete;

  /// \brief Copy constructor for UnaryInstrImm.
  /// \param r_src the object to be copied
  UnaryInstrImm(const UnaryInstrImm &r_src) = default;

  /// \brief Copy assignment operator for UnaryInstrImm.
  /// \param r_src the object to be copied
  UnaryInstrImm &operator=(const UnaryInstrImm &r_src) = delete;

  /// \brief Move constructor for UnaryInstrImm.
  /// \param r_src the object to be copied
  UnaryInstrImm(UnaryInstrImm &&r_src) = delete;

  /// \brief Move assignment operator for UnaryInstrImm.
  /// \param r_src the object to be copied
  UnaryInstrImm &operator=(UnaryInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu