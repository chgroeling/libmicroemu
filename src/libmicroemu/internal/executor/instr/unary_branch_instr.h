#pragma once
#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/instr/op_result.h"
#include "libmicroemu/internal/executor/instr_context.h"
#include "libmicroemu/internal/executor/instr_exec_results.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Branch X
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class Bx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc, const u32 &rm) {
    static_cast<void>(ictx);
    static_cast<void>(pc);
    return BranchOpResult{static_cast<me_adr_t>(rm)};
  }
};

/**
 * @brief Branch link X
 *
 * see Armv7-M Architecture Reference Manual Issue E.e p.205
 */
template <typename TInstrContext> class Blx1Op {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  static inline BranchOpResult Call(const TInstrContext &ictx, const me_adr_t &pc, const u32 &rm) {
    static_cast<void>(ictx);
    const me_adr_t next_instr_address = static_cast<me_adr_t>(pc - 2U);

    ictx.cpua.WriteRegister(RegisterId::kLr,
                            static_cast<uint32_t>((next_instr_address & ~0x1) | 0x1U));
    return BranchOpResult{static_cast<me_adr_t>(rm)};
  }
};

template <typename TOp, typename TInstrContext> class UnaryBranchInstr {
public:
  using It = typename TInstrContext::It;
  using Pc = typename TInstrContext::Pc;

  using ExcTrig = typename TInstrContext::ExcTrig;

  template <typename TArg0>
  static Result<InstrExecResult> Call(TInstrContext &ictx, const InstrFlagsSet &iflags,
                                      const TArg0 &arg_m) {

    const auto is_32bit = (iflags & static_cast<InstrFlagsSet>(InstrFlags::k32Bit)) != 0U;

    InstrExecFlagsSet eflags{0x0U};
    TRY_ASSIGN(condition_passed, InstrExecResult, It::ConditionPassed(ictx.cpua));

    if (!condition_passed) {
      It::ITAdvance(ictx.cpua);
      Pc::AdvanceInstr(ictx.cpua, is_32bit);
      return Ok(InstrExecResult{eflags});
    }

    const me_adr_t pc = static_cast<me_adr_t>(ictx.cpua.template ReadRegister<RegisterId::kPc>());
    const auto rm = ictx.cpua.ReadRegister(arg_m.Get());
    auto result = TOp::Call(ictx, pc, rm);

    It::ITAdvance(ictx.cpua);
    TRY(InstrExecResult, Pc::BXWritePC(ictx.cpua, ictx.bus, result.new_pc));

    return Ok(InstrExecResult{eflags});
  }

private:
  /**
   * @brief Constructor
   */
  UnaryBranchInstr() = delete;

  /**
   * @brief Destructor
   */
  ~UnaryBranchInstr() = delete;

  /**
   * @brief Copy constructor for UnaryBranchInstr.
   * @param r_src the object to be copied
   */
  UnaryBranchInstr(const UnaryBranchInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for UnaryBranchInstr.
   * @param r_src the object to be copied
   */
  UnaryBranchInstr &operator=(const UnaryBranchInstr &r_src) = delete;

  /**
   * @brief Move constructor for UnaryBranchInstr.
   * @param r_src the object to be moved
   */
  UnaryBranchInstr(UnaryBranchInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  UnaryBranchInstr.
   * @param r_src the object to be moved
   */
  UnaryBranchInstr &operator=(UnaryBranchInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu