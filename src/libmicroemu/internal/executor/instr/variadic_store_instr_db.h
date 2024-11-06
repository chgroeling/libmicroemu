#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/utils/arg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Store Multiple Decrement Before
/// PUSH - see Armv7-M Architecture Reference Manual Issue E.e p.322
template <typename TInstrContext> class VariadicStoreInstrDb {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;
  using Reg = typename TInstrContext::Reg;
  using SReg = typename TInstrContext::SReg;

  template <typename TArg>
  static Result<ExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                 const TArg &arg_n, const u32 &registers) {
    const auto is_32bit = (iflags & k32Bit) != 0u;

    ExecFlagsSet eflags{0x0u};
    TRY_ASSIGN(condition_passed, ExecResult, It::ConditionPassed(ictx.pstates));

    if (!condition_passed) {
      It::ITAdvance(ictx.pstates);
      Pc::AdvanceInstr(ictx.pstates, is_32bit);
      return Ok(ExecResult{eflags});
    }

    const auto rn = Reg::ReadRegister(ictx.pstates, arg_n.Get());
    auto reg_count = Bm32::BitCount(registers);
    u32 address = rn - 4u * reg_count;

    for (u32 reg = 0u; reg <= 14u; ++reg) {
      u32 bm = 0x1u << reg;
      if ((registers & bm) != 0u) {
        const auto r = Reg::ReadRegister(ictx.pstates, static_cast<RegisterId>(reg));
        TRY(ExecResult, ictx.bus.template WriteOrRaise<u32>(
                            ictx.pstates, address, r, BusExceptionType::kRaisePreciseDataBusError));
        address += 4u;
      }
    }

    // For debugging purposes
    // MemoryViewer<TMemAccess> mem_view(bus_);
    // mem_view.print(0x80000 - 0x32, 0x64);

    const bool is_wback = (iflags & kWBack) != 0u;

    if (is_wback) {
      const auto wback_val = rn - 4u * reg_count;

      // Update n register
      Reg::WriteRegister(ictx.pstates, arg_n.Get(), wback_val);
    }
    It::ITAdvance(ictx.pstates);
    Pc::AdvanceInstr(ictx.pstates, is_32bit);

    return Ok(ExecResult{eflags});
  }

private:
  /// \brief Constructor
  VariadicStoreInstrDb() = delete;

  /// \brief Destructor
  ~VariadicStoreInstrDb() = delete;

  /// \brief Copy constructor for VariadicStoreInstrDb.
  /// \param r_src the object to be copied
  VariadicStoreInstrDb(const VariadicStoreInstrDb &r_src) = default;

  /// \brief Copy assignment operator for VariadicStoreInstrDb.
  /// \param r_src the object to be copied
  VariadicStoreInstrDb &operator=(const VariadicStoreInstrDb &r_src) = delete;

  /// \brief Move constructor for VariadicStoreInstrDb.
  /// \param r_src the object to be copied
  VariadicStoreInstrDb(VariadicStoreInstrDb &&r_src) = delete;

  /// \brief Move assignment operator for VariadicStoreInstrDb.
  /// \param r_src the object to be copied
  VariadicStoreInstrDb &operator=(VariadicStoreInstrDb &&r_src) = delete;
};

} // namespace internal
} // namespace microemu