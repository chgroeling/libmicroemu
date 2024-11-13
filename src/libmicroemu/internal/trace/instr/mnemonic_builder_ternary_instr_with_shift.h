#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderTernaryInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &n, const TArg2 &m,
                    const ImmShiftResults &shift_res) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0U;
    const bool is_supress_shift_type =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType)) !=
        0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');
    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(d.Get())).AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()));

    if (shift_res.value != 0) {
      mctx.builder.AddString(", ");

      if (!is_supress_shift_type) {
        mctx.builder.AddString(shift_res.GetShiftTypeAsStr()).AddChar(' ');
      }
      mctx.builder.AddString("#").AddUInt(shift_res.value);
    }
    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderTernaryInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderTernaryInstrWithShift() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderTernaryInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryInstrWithShift(const MnemonicBuilderTernaryInstrWithShift &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderTernaryInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryInstrWithShift &
  operator=(const MnemonicBuilderTernaryInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderTernaryInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryInstrWithShift(MnemonicBuilderTernaryInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderTernaryInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryInstrWithShift &
  operator=(MnemonicBuilderTernaryInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu