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
template <typename TOp, typename TInstrContext> class BinaryStoreInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  template <typename TArg0, typename TArg1>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_n, const TArg1 &arg_t, const u32 &imm32) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    ExecFlagsSet eflags{0x0U};
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
    TRY(ExecResult, TOp::Write(ictx, address, rt));

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), offset_addr);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  BinaryStoreInstrWithImm() = delete;

  /// \brief Destructor
  ~BinaryStoreInstrWithImm() = delete;

  /// \brief Copy constructor for MemoBinaryStoreInstrWithImmryRouter.
  /// \param r_src the object to be copied
  BinaryStoreInstrWithImm(const BinaryStoreInstrWithImm &r_src) = default;

  /// \brief Copy assignment operator for BinaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  BinaryStoreInstrWithImm &operator=(const BinaryStoreInstrWithImm &r_src) = delete;

  /// \brief Move constructor for BinaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  BinaryStoreInstrWithImm(BinaryStoreInstrWithImm &&r_src) = delete;

  /// \brief Move assignment operator for BinaryStoreInstrWithImm.
  /// \param r_src the object to be copied
  BinaryStoreInstrWithImm &operator=(BinaryStoreInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu