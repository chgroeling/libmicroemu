#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/arg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Branch
/// see Armv7-M Architecture Reference Manual Issue E.e p.205
template <typename TInstrContext> class B1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc,
                                    const me_adr_t &imm32) {
    static_cast<void>(ictx);

    return BranchOpResult{pc + imm32};
  }
};

/// Branch Link
/// see Armv7-M Architecture Reference Manual Issue E.e p.213
template <typename TInstrContext> class Bl1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc,
                                    const me_adr_t &imm32) {
    static_cast<void>(ictx);

    Reg::WriteRegister(ictx.pstates, RegisterId::kLr, static_cast<uint32_t>((pc & ~0x1u) | 0x1u));
    return BranchOpResult{pc + imm32};
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const u32 &imm32) {

    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(Reg::ReadPC(ictx.pstates));
    auto result = TOp::Call(ictx, pc, imm32);

    Pc::BranchWritePC(ictx.pstates, result.new_pc);
    It::ITAdvance(ictx.pstates);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  UnaryBranchInstrImm() = delete;

  /// \brief Destructor
  ~UnaryBranchInstrImm() = delete;

  /// \brief Copy constructor for UnaryBranchInstrImm.
  /// \param r_src the object to be copied
  UnaryBranchInstrImm(const UnaryBranchInstrImm &r_src) = default;

  /// \brief Copy assignment operator for UnaryBranchInstrImm.
  /// \param r_src the object to be copied
  UnaryBranchInstrImm &operator=(const UnaryBranchInstrImm &r_src) = delete;

  /// \brief Move constructor for UnaryBranchInstrImm.
  /// \param r_src the object to be copied
  UnaryBranchInstrImm(UnaryBranchInstrImm &&r_src) = delete;

  /// \brief Move assignment operator for UnaryBranchInstrImm.
  /// \param r_src the object to be copied
  UnaryBranchInstrImm &operator=(UnaryBranchInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace microemu