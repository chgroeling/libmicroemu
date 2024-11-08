#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from register adress to register
template <typename TOp, typename TInstrContext> class TernaryStoreInstrWithShift {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg &arg_n, const TArg &arg_m, const TArg &arg_t,
                                 const ImmShiftResults &shift_res) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;
    const bool is_index = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kIndex)) != 0U;
    const bool is_add = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kAdd)) != 0U;

    ExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    const auto rm = Reg::ReadRegister(ictx.pstates, arg_m.Get());

    auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(ictx.pstates);

    auto offset = Alu32::Shift(rm, shift_res.type, shift_res.value,
                               (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk);

    assert(is_add == true); // other mode is currently not supported
    const u32 offset_address = is_add == true ? rn + offset : rn - offset;
    const u32 address = is_index == true ? offset_address : rn;

    const auto rt = Reg::ReadRegister(ictx.pstates, arg_t.Get());
    TRY(ExecResult, TOp::Write(ictx, address, rt));

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    if (is_wback == true) {
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), offset_address);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  TernaryStoreInstrWithShift() = delete;

  /// \brief Destructor
  ~TernaryStoreInstrWithShift() = delete;

  /// \brief Copy constructor for TernaryStoreInstrWithShift.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithShift(const TernaryStoreInstrWithShift &r_src) = default;

  /// \brief Copy assignment operator for TernaryStoreInstrWithShift.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithShift &operator=(const TernaryStoreInstrWithShift &r_src) = delete;

  /// \brief Move constructor for TernaryStoreInstrWithShift.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithShift(TernaryStoreInstrWithShift &&r_src) = delete;

  /// \brief Move assignment operator for TernaryStoreInstrWithShift.
  /// \param r_src the object to be copied
  TernaryStoreInstrWithShift &operator=(TernaryStoreInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu