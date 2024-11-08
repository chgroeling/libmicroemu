#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &n, const u8 &m,
                    const ImmShiftResults &shift_res) {

    const bool is_setflags = (iflags & InstrFlags::kSetFlags) != 0u;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0;
    const bool is_supress_shift_type =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType)) !=
        0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');
    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)).data())
          .AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)).data())
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)).data());

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
  /// \brief Constructor
  StrBuilderTernaryInstrWithShift() = delete;

  /// \brief Destructor
  ~StrBuilderTernaryInstrWithShift() = delete;

  /// \brief Copy constructor for StrBuilderTernaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstrWithShift(const StrBuilderTernaryInstrWithShift &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderTernaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstrWithShift &operator=(const StrBuilderTernaryInstrWithShift &r_src) = delete;

  /// \brief Move constructor for StrBuilderTernaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstrWithShift(StrBuilderTernaryInstrWithShift &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderTernaryInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstrWithShift &operator=(StrBuilderTernaryInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu