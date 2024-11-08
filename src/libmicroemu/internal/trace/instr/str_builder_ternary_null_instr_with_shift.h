#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryNullInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &n, const u8 &m,
                    const ImmShiftResults &shift_res, bool supress_type) {

    static_cast<void>(bflags);
    const bool is_setflags = (iflags & InstrFlags::kSetFlags) != 0u;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(n))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m));

    if (shift_res.value != 0) {
      mctx.builder.AddString(", ");

      if (!supress_type) {
        mctx.builder.AddString(shift_res.GetShiftTypeAsStr()).AddChar(' ');
      }
      mctx.builder.AddString("#").AddUInt(shift_res.value);
    }
    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderTernaryNullInstrWithShift() = delete;

  /// \brief Destructor
  ~StrBuilderTernaryNullInstrWithShift() = delete;

  /// \brief Copy constructor for StrBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryNullInstrWithShift(const StrBuilderTernaryNullInstrWithShift &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryNullInstrWithShift &
  operator=(const StrBuilderTernaryNullInstrWithShift &r_src) = delete;

  /// \brief Move constructor for StrBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryNullInstrWithShift(StrBuilderTernaryNullInstrWithShift &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryNullInstrWithShift &
  operator=(StrBuilderTernaryNullInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu