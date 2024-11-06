#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/logic/reg_ops.h"
#include "libmicroemu/internal/utils/arg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TOp, typename TInstrContext> class BinaryLoadInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg &arg_n, const u32 &imm32, const TArg &arg_t) {
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

    TRY_ASSIGN(data, ExecResult, TOp::Read(ictx, address));

    const bool is_wback = (iflags & kWBack) != 0u;
    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), offset_addr);
    }

    if (arg_t.Get() == RegisterId::kPc) {
      // When the given address was unaligend the behaviour is
      // unpredtictable
      if ((address & 0x3u) == 0u) {
        It::ITAdvance(ictx.pstates);
        TRY(ExecResult, (Pc::LoadWritePC(ictx.pstates, ictx.bus, data)));

        return Ok(ExecResult{eflags});
      } else {
        return Err<ExecResult>(StatusCode::kScExecutorUnpredictable);
      }
    } else {
      Reg::WriteRegister(ictx.pstates, arg_t.Get(), data);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  BinaryLoadInstrWithImm() = delete;

  /// \brief Destructor
  ~BinaryLoadInstrWithImm() = delete;

  /// \brief Copy constructor for BinaryLoadInstrWithImm.
  /// \param r_src the object to be copied
  BinaryLoadInstrWithImm(const BinaryLoadInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for BinaryLoadInstrWithImm.
  /// \param r_src the object to be copied
  BinaryLoadInstrWithImm &operator=(const BinaryLoadInstrWithImm &r_src) = delete;

  /// \brief Move constructor for BinaryLoadInstrWithImm.
  /// \param r_src the object to be copied
  BinaryLoadInstrWithImm(BinaryLoadInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for BinaryLoadInstrWithImm.
  /// \param r_src the object to be copied
  BinaryLoadInstrWithImm &operator=(BinaryLoadInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu