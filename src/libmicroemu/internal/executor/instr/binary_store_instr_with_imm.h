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
template <typename TStoreOp, typename TInstrContext> class BinaryStoreInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_t, const TArg1 &arg_n, const u32 &imm32) {
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const me_adr_t offset_addr = is_add == true ? rn + imm32 : rn - imm32;
    const me_adr_t address = is_index == true ? offset_addr : rn;

    const auto rt = ictx.cpua.ReadRegister(arg_t.Get());
    TRY(InstrExecResult, TStoreOp::Write(ictx, address, rt));

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback) {
      PostExecWriteRegPcExcluded::Call(ictx, arg_n, offset_addr);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryStoreInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryStoreInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MemoBinaryStoreInstrWithImmryRouter.
   * @param r_src the object to be copied
   */
  BinaryStoreInstrWithImm(const BinaryStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryStoreInstrWithImm &operator=(const BinaryStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryStoreInstrWithImm(BinaryStoreInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryStoreInstrWithImm &operator=(BinaryStoreInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
