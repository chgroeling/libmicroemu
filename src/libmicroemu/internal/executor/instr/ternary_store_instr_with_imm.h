#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

template <typename TStoreOp, typename TInstrContext> class TernaryStoreInstrWithImm {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TDest, typename TTgt, typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TDest &rd, const TTgt &rt, const TArg0 &rn,
                                      const u32 &imm32) {
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    InstrExecFlagsSet eflags{kNoInstrExecFlags};
    const auto n = ictx.cpua.ReadRegister(rn.Get());

    const me_adr_t offset_addr = is_add == true ? n + imm32 : n - imm32;
    const me_adr_t address = is_index == true ? offset_addr : n;

    const auto t = ictx.cpua.ReadRegister(rt.Get());
    u32 d{0U};

    TRY(InstrExecResult, TStoreOp::Write(ictx, address, t, d));

    // write back if write succeeded
    ictx.cpua.WriteRegister(rd.Get(), d);

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback) {
      PostExecWriteRegPcExcluded::Call(ictx, rn, offset_addr);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);
    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  TernaryStoreInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryStoreInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MemoTernaryStoreInstrWithImmryRouter.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithImm(const TernaryStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Copy assignment operator for TernaryStoreInstrWithImm.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithImm &operator=(const TernaryStoreInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for TernaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithImm(TernaryStoreInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryStoreInstrWithImm.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithImm &operator=(TernaryStoreInstrWithImm &&r_src) = delete;
};

} // namespace libmicroemu::internal
