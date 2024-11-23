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
template <typename TLoadOp, typename TInstrContext> class TernaryLoadInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg0, typename TArg1, typename TArg2>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &rt, const TArg1 &rn, const TArg2 &rm,
                                      const ImmShiftResults &shift_res) {
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));
    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    const auto m = ictx.cpua.ReadRegister(rm.Get());

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();
    auto offset = Alu32::Shift(m, shift_res.type, shift_res.value,
                               (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    const u32 offset_addr = is_add == true ? n + offset : n - offset;
    const u32 address = is_index == true ? offset_addr : n;

    TRY_ASSIGN(data, InstrExecResult, TLoadOp::Read(ictx, address));
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
  TernaryLoadInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~TernaryLoadInstrWithShift() = delete;

  /**
   * @brief Copy constructor for TernaryLoadInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryLoadInstrWithShift(const TernaryLoadInstrWithShift &r_src) = delete;

  /**
   * @brief Copy assignment operator for TernaryLoadInstrWithShift.
   * @param r_src the object to be copied
   */
  TernaryLoadInstrWithShift &operator=(const TernaryLoadInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for TernaryLoadInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryLoadInstrWithShift(TernaryLoadInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  TernaryLoadInstrWithShift.
   * @param r_src the object to be moved
   */
  TernaryLoadInstrWithShift &operator=(TernaryLoadInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu