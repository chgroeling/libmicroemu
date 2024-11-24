#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/logic/reg_ops.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TLoadOp, typename TInstrContext> class BinaryLoadInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TTgt, typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TTgt &rt, const TArg0 &rn, const u32 &imm32) {
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const me_adr_t offset_addr = is_add == true ? n + imm32 : n - imm32;
    const me_adr_t address = is_index == true ? offset_addr : n;

    TRY_ASSIGN(data, InstrExecResult, TLoadOp::Read(ictx, address));

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback) {
      PostExecWriteRegPcExcluded::Call(ictx, rn, offset_addr);
    }
    const bool is_aligned = (address & 0x3U) == 0U;
    TRY(InstrExecResult, PostExecWriteRegPcIncluded::Call(ictx, iflags, rt, data, is_aligned));
    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  BinaryLoadInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~BinaryLoadInstrWithImm() = delete;

  /**
   * @brief Copy constructor for BinaryLoadInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryLoadInstrWithImm(const BinaryLoadInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for BinaryLoadInstrWithImm.
   * @param r_src the object to be copied
   */
  BinaryLoadInstrWithImm &operator=(const BinaryLoadInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for BinaryLoadInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryLoadInstrWithImm(BinaryLoadInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  BinaryLoadInstrWithImm.
   * @param r_src the object to be moved
   */
  BinaryLoadInstrWithImm &operator=(BinaryLoadInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
