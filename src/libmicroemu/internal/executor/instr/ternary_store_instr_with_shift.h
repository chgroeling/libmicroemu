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
template <typename TOp, typename TInstrContext> class TernaryStoreInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg &arg_n, const TArg &arg_m, const TArg &arg_t,
                                      const ImmShiftResults &shift_res) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const auto rn = ictx.cpua.ReadRegister(arg_n.Get());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());

    auto apsr = ictx.cpua.template ReadRegister<SpecialRegisterId::kApsr>();

    auto offset = Alu32::Shift(rm, shift_res.type, shift_res.value,
                               (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    assert(is_add == true); // other mode is currently not supported
    const u32 offset_address = is_add == true ? rn + offset : rn - offset;
    const u32 address = is_index == true ? offset_address : rn;

    const auto rt = ictx.cpua.ReadRegister(arg_t.Get());
    TRY(InstrExecResult, TOp::Write(ictx, address, rt));

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback == true) {
      ictx.cpua.WriteRegister(arg_n.Get(), offset_address);
    }
    It::ITAdvance(ictx.cpua);
    Pc::AdvanceInstr(ictx.cpua, is_32bit);

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
  TernaryStoreInstrWithShift(const TernaryStoreInstrWithShift &r_src) = default;

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