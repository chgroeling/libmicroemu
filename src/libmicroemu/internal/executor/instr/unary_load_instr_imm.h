#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TLoadOp, typename TInstrContext> class UnaryLoadInstrImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const u32 &imm32, const TArg &arg_t) {

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    // unary loads always refer to the pc
    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const me_adr_t base = Bm32::AlignDown<4>(pc);
    const me_adr_t address = is_add == true ? base + imm32 : base - imm32;

    TRY_ASSIGN(data, InstrExecResult, TLoadOp::Read(ictx, address));

    const bool is_aligned = (address & 0x3U) == 0U;
    TRY(InstrExecResult, PostExecWriteRegPcIncluded::Call(ictx, iflags, arg_t, data, is_aligned));
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryLoadInstrImm() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryLoadInstrImm() = delete;

  /**
   * @brief Copy constructor for UnaryLoadInstrImm.
   * @param r_src the object to be copied
   */
  UnaryLoadInstrImm(const UnaryLoadInstrImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for UnaryLoadInstrImm.
   * @param r_src the object to be copied
   */
  UnaryLoadInstrImm &operator=(const UnaryLoadInstrImm &r_src) = delete;

  /**
   * @brief Move constructor for UnaryLoadInstrImm.
   * @param r_src the object to be moved
   */
  UnaryLoadInstrImm(UnaryLoadInstrImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryLoadInstrImm.
   * @param r_src the object to be moved
   */
  UnaryLoadInstrImm &operator=(UnaryLoadInstrImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
