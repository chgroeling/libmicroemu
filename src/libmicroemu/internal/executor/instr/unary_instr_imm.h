#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/post_exec.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Add to pc
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.190
 */
template <typename TInstrContext> class AddToPcImmOp {
public:
  template <typename TDest>
  static Result<InstrExecFlagsSet> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                        const TDest &rd, const u32 &imm32) {
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const auto apc = Bm32::AlignDown<4>(pc);
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;
    const auto result = is_add != false ? apc + imm32 : apc - imm32;
    const auto op_res = OpResult{result, false, false};
    PostExecWriteRegPcExcluded::Call(ictx, rd, op_res.value);
    PostExecOptionalSetFlags::Call(ictx, iflags, op_res);
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(kNoInstrExecFlags);
  }
};

template <typename TOp, typename TInstrContext> class UnaryInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const u32 &imm32) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    TRY_ASSIGN(eflags, InstrExecResult, TOp::Call(ictx, iflags, rd, imm32));
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryInstrImm.
   * @param r_src the object to be copied
   */
  UnaryInstrImm(const UnaryInstrImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryInstrImm.
   * @param r_src the object to be copied
   */
  UnaryInstrImm &operator=(const UnaryInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryInstrImm.
   * @param r_src the object to be moved
   */
  UnaryInstrImm(UnaryInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryInstrImm.
   * @param r_src the object to be moved
   */
  UnaryInstrImm &operator=(UnaryInstrImm &&r_src) = delete;
};

} // namespace libmicroemu::internal