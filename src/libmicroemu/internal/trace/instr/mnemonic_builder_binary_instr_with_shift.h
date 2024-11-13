#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &m,
                    const ImmShiftResults &shift_res) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_supress_shift_type =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType)) !=
        0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()))
        .AddString(", ");

    if (!is_supress_shift_type) {
      mctx.builder.AddString(shift_res.GetShiftTypeAsStr()).AddChar(' ');
    }

    mctx.builder.AddString("#").AddUInt(shift_res.value).Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryInstrWithShift() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithShift(const MnemonicBuilderBinaryInstrWithShift &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstrWithShift &
  operator=(const MnemonicBuilderBinaryInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithShift(MnemonicBuilderBinaryInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstrWithShift &
  operator=(MnemonicBuilderBinaryInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu