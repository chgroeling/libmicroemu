#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Branch
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class B1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc,
                                    const me_adr_t &imm32) {
    static_cast<void>(ictx);

    return BranchOpResult{pc + imm32};
  }
};

/**
 * @brief Branch Link
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.213
 */
template <typename TInstrContext> class Bl1ImmOp {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc,
                                    const me_adr_t &imm32) {
    static_cast<void>(ictx);

    ictx.cpua.template WriteRegister<RegisterId::kLr>(static_cast<uint32_t>((pc & ~0x1U) | 0x1U));
    return BranchOpResult{pc + imm32};
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const u32 &imm32) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    auto result = TOp::Call(ictx, pc, imm32);

    Pc::BranchWritePC(ictx.cpua, result.new_pc);
    It::ITAdvance(ictx.cpua);

    return Ok(ExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryBranchInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryBranchInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryBranchInstrImm.
   * @param r_src the object to be copied
   */
  UnaryBranchInstrImm(const UnaryBranchInstrImm &r_src) = default;

  /**
   * @brief Copy assignment operator for UnaryBranchInstrImm.
   * @param r_src the object to be copied
   */
  UnaryBranchInstrImm &operator=(const UnaryBranchInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryBranchInstrImm.
   * @param r_src the object to be moved
   */
  UnaryBranchInstrImm(UnaryBranchInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryBranchInstrImm.
   * @param r_src the object to be moved
   */
  UnaryBranchInstrImm &operator=(UnaryBranchInstrImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu