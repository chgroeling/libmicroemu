#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &m,
                    const ImmShiftResults &shift_res) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_supress_shift_type =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType)) !=
        0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
        .AddString(", ");

    if (!is_supress_shift_type) {
      mctx.builder.AddString(shift_res.GetShiftTypeAsStr()).AddChar(' ');
    }

    mctx.builder.AddString("#").AddUInt(shift_res.value).Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryInstrWithShift() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryInstrWithShift() = delete;

  /// \brief Copy constructor for StrBuilderBinaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithShift(const StrBuilderBinaryInstrWithShift &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithShift &operator=(const StrBuilderBinaryInstrWithShift &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithShift(StrBuilderBinaryInstrWithShift &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithShift &operator=(StrBuilderBinaryInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu