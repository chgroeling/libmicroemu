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

/// Load from immediate adr to register
template <typename TOp, typename TInstrContext> class UnaryLoadInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;
  template <typename TArg>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags, const u32 &imm32,
                                 const TArg &arg_t) {
    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const bool is_add = (iflags & kAdd) != 0u;

    // unary loads always refer to the pc
    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));

    const me_adr_t base = Bm32::AlignDown<4>(pc);
    const me_adr_t address = is_add == true ? base + imm32 : base - imm32;

    TRY_ASSIGN(data, ExecResult, TOp::Read(ictx, address));

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
  UnaryLoadInstrImm() = delete;

  /// \brief Destructor
  ~UnaryLoadInstrImm() = delete;

  /// \brief Copy constructor for UnaryLoadInstrImm.
  /// \param r_src the object to be copied
  UnaryLoadInstrImm(const UnaryLoadInstrImm &r_src) = default;

  /// \brief Copy assignment operator for UnaryLoadInstrImm.
  /// \param r_src the object to be copied
  UnaryLoadInstrImm &operator=(const UnaryLoadInstrImm &r_src) = delete;

  /// \brief Move constructor for UnaryLoadInstrImm.
  /// \param r_src the object to be copied
  UnaryLoadInstrImm(UnaryLoadInstrImm &&r_src) = delete;

  /// \brief Move assignment operator for UnaryLoadInstrImm.
  /// \param r_src the object to be copied
  UnaryLoadInstrImm &operator=(UnaryLoadInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu