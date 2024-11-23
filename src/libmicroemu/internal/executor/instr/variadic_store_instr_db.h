#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/*
 * @brief Store Multiple Decrement Before
 * PUSH - see Armv7-M Architecture Reference Manual Issue E.e p.322
 */
template <typename TInstrContext> class VariadicStoreInstrDb {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  template <typename TArg>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg &rn, const u32 &registers) {
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      PostExecAdvancePcAndIt::Call(ictx, iflags);
      return Ok(InstrExecResult{kNoInstrExecFlags});
    }
    const auto n = ictx.cpua.ReadRegister(rn.Get());
    auto reg_count = Bm32::BitCount(registers);
    u32 address = n - 4U * reg_count;

    for (u32 reg = 0U; reg <= 14U; ++reg) {
      u32 bm = 0x1U << reg;
      if ((registers & bm) != 0U) {
        const auto r = ictx.cpua.ReadRegister(static_cast<RegisterId>(reg));
        TRY(InstrExecResult,
            ictx.bus.template WriteOrRaise<u32>(ictx.cpua, address, r,
                                                BusExceptionType::kRaisePreciseDataBusError));
        address += 4U;
      }
    }

    // For debugging purposes
    // MemoryViewer<TMemAccess> mem_view(bus_);
    // mem_view.Print(0x80000 - 0x32, 0x64);

    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;

    if (is_wback) {
      const auto wback_val = n - 4U * reg_count;

      // Update n register
      PostExecWriteRegPcExcluded::Call(ictx, rn, wback_val);
    }
    PostExecAdvancePcAndIt::Call(ictx, iflags);

    return Ok(InstrExecResult{kNoInstrExecFlags});
  }

private:
  /**
   * @brief Constructor
   */
  VariadicStoreInstrDb() = delete;

  /**
   * @brief Destructor
   */
  ~VariadicStoreInstrDb() = delete;

  /**
   * @brief Copy constructor for VariadicStoreInstrDb.
   * @param r_src the object to be copied
   */
  VariadicStoreInstrDb(const VariadicStoreInstrDb &r_src) = delete;

  /**
   * @brief Copy assignment operator for VariadicStoreInstrDb.
   * @param r_src the object to be copied
   */
  VariadicStoreInstrDb &operator=(const VariadicStoreInstrDb &r_src) = delete;

  /**
   * @brief Move constructor for VariadicStoreInstrDb.
   * @param r_src the object to be moved
   */
  VariadicStoreInstrDb(VariadicStoreInstrDb &&r_src) = delete;

  /**
   * @brief Move assignment operator for  VariadicStoreInstrDb.
   * @param r_src the object to be moved
   */
  VariadicStoreInstrDb &operator=(VariadicStoreInstrDb &&r_src) = delete;
};

} // namespace libmicroemu::internal
