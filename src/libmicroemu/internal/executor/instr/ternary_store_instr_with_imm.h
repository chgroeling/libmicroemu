#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

template <typename TOp, typename TInstrContext> class TernaryStoreInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_t, const TArg2 &arg_d,
                                 const u32 &imm32) {
    const auto is_32bit = (iflags & k32Bit) != 0u;
    const bool is_index = (iflags & kIndex) != 0u;
    const bool is_add = (iflags & kAdd) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());

    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    const auto rt = Reg::ReadRegister(ictx.pstates, arg_t.Get());
    u32 rd{0u};

    TRY(ExecResult, TOp::Write(ictx, address, rt, rd));

    // write back if write succeeded
    Reg::WriteRegister(ictx.pstates, arg_d.Get(), rd);

    const bool is_wback = (iflags & kWBack) != 0u;
    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), offset_addr);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  TernaryStoreInstrWithImm() = delete;

  /// \brief Destructor
  ~TernaryStoreInstrWithImm() = delete;

  /// \brief Copy constructor for MemoTernaryStoreInstrWithImmryRouter.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithImm(const TernaryStoreInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for TernaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithImm &operator=(const TernaryStoreInstrWithImm &r_src) = delete;

  /// \brief Move constructor for TernaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithImm(TernaryStoreInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for TernaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithImm &operator=(TernaryStoreInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu