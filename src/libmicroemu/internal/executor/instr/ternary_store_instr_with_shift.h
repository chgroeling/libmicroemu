#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from register adress to register
 */
template <typename TStoreOp, typename TInstrContext> class TernaryStoreInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rt, const TArg1 &rn, const TArg2 &rm,
                                      const ImmShiftResults &shift_res) {
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }

    InstrExecFlagsSet eflags{kNoInstrExecFlags};
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    auto offset = Alu32::Shift(m, shift_res.type, shift_res.value,
                               (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    assert(is_add == true); // other mode is currently not supported
    const u32 offset_addr = is_add == true ? n + offset : n - offset;
    const u32 address = is_index == true ? offset_addr : n;

    const auto t = ictx.cpua.ReadRegister(rt.Get());
    TRY(InstrExecResult, TStoreOp::Write(ictx, address, t));

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
  TernaryStoreInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryStoreInstrWithShift() = delete;

  /**
   * @brief Copy constructor for TernaryStoreInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithShift(const TernaryStoreInstrWithShift &r_src) = delete;

  /**
   * @brief Copy assignment operator for TernaryStoreInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryStoreInstrWithShift &operator=(const TernaryStoreInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for TernaryStoreInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithShift(TernaryStoreInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryStoreInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryStoreInstrWithShift &operator=(TernaryStoreInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu