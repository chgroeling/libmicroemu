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

/// Branch X
/// see Armv7-M Architecture Reference Manual Issue E.e p.205
template <typename TInstrContext> class Bx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc, const u32 &rm) {
    static_cast<void>(ictx);
    static_cast<void>(pc);
    return BranchOpResult{static_cast<me_adr_t>(rm)};
  }
};

/// Branch link X
/// see Armv7-M Architecture Reference Manual Issue E.e p.205
template <typename TInstrContext> class Blx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc, const u32 &rm) {
    static_cast<void>(ictx);
    const me_adr_t next_instr_address = static_cast<me_adr_t>(pc - 2U);

    Reg::WriteRegister(ictx.pstates, RegisterId::kLr,
                       static_cast<uint32_t>((next_instr_address & ~0x1) | 0x1U));
    // TODO BLXWrite instead of BXWrite
    return BranchOpResult{static_cast<me_adr_t>(rm)};
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg0>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg0 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());
    auto result = TOp::Call(ictx, pc, rm);

    It::ITAdvance(ictx.pstates);
    TRY(ExecResult, Pc::BXWritePC(ictx.pstates, ictx.bus, result.new_pc));

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  UnaryBranchInstr() = delete;

  /// \brief Destructor
  ~UnaryBranchInstr() = delete;

  /// \brief Copy constructor for UnaryBranchInstr.
  /// \param r_src the object to be copied
  UnaryBranchInstr(const UnaryBranchInstr &r_src) = default;

  /// \brief Copy assignment operator for UnaryBranchInstr.
  /// \param r_src the object to be copied
  UnaryBranchInstr &operator=(const UnaryBranchInstr &r_src) = delete;

  /// \brief Move constructor for UnaryBranchInstr.
  /// \param r_src the object to be copied
  UnaryBranchInstr(UnaryBranchInstr &&r_src) = delete;

  /// \brief Move assignment operator for UnaryBranchInstr.
  /// \param r_src the object to be copied
  UnaryBranchInstr &operator=(UnaryBranchInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu