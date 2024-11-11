#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load multiple
/// LDM - see Armv7-M Architecture Reference Manual Issue E.e p.242
/// POP - see Armv7-M Architecture Reference Manual Issue E.e p.319
template <typename TInstrContext> class VariadicLoadInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;
  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg &arg_n, const u32 &registers) {
    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    ExecFlagsSet eflags = 0x0U;
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }
    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto address = static_cast<me_adr_t>(rn);

    for (u32 rid = 0U; rid <= 14U; ++rid) {
      const u32 bm = 0x1U << rid;
      if ((registers & bm) != 0U) {
        TRY_ASSIGN(rdat, ExecResult,
                   ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address,
                                                      BusExceptionType::kRaisePreciseDataBusError));
        Reg::WriteRegister(ictx.pstates, static_cast<RegisterId>(rid), rdat);
        address += static_cast<me_adr_t>(4U);
      }
    }

    if (Bm32::Slice1R<15U, 15U>(registers) == 0x1U) {
      TRY_ASSIGN(rdat, ExecResult,
                 ictx.bus.template ReadOrRaise<u32>(ictx.pstates, address,
                                                    BusExceptionType::kRaisePreciseDataBusError));
      It::ITAdvance(ictx.pstates);
      TRY(ExecResult, Pc::LoadWritePC(ictx.pstates, ictx.bus, rdat));

      //  do not advance pc
    } else {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
    }

    // For debugging purposes
    // MemoryViewer<TMemAccess> mem_view(mem_acs_);
    // mem_view.Print(0x80000 - 0x32, 0x64);

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    const auto n_mask = 1U << static_cast<u32>(arg_n.Get());
    const bool is_rn_in_set = (registers & n_mask) != 0;

    if ((is_wback) && (!is_rn_in_set)) {
      auto regcount = Bm32::BitCount(registers);
      const auto wback_val = rn + 4U * regcount;

      // Update n register
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), wback_val);
    }

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  VariadicLoadInstr() = delete;

  /// \brief Destructor
  ~VariadicLoadInstr() = delete;

  /// \brief Copy constructor for VariadicLoadInstr.
  /// \param r_src the object to be copied
  VariadicLoadInstr(const VariadicLoadInstr &r_src) = default;

  /// \brief Copy assignment operator for VariadicLoadInstr.
  /// \param r_src the object to be copied
  VariadicLoadInstr &operator=(const VariadicLoadInstr &r_src) = delete;

  /// \brief Move constructor for VariadicLoadInstr.
  /// \param r_src the object to be copied
  VariadicLoadInstr(VariadicLoadInstr &&r_src) = delete;

  /// \brief Move assignment operator for VariadicLoadInstr.
  /// \param r_src the object to be copied
  VariadicLoadInstr &operator=(VariadicLoadInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu