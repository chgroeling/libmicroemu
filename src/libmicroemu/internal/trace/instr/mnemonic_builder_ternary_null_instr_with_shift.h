#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class MnemonicBuilderTernaryNullInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &n, const TArg1 &m,
                    const ImmShiftResults &shift_res, bool supress_type) {

    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()));

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
  MnemonicBuilderTernaryNullInstrWithShift() = delete;

  /// \brief Destructor
  ~MnemonicBuilderTernaryNullInstrWithShift() = delete;

  /// \brief Copy constructor for MnemonicBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  MnemonicBuilderTernaryNullInstrWithShift(const MnemonicBuilderTernaryNullInstrWithShift &r_src) =
      default;

  /// \brief Copy assignment operator for MnemonicBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  MnemonicBuilderTernaryNullInstrWithShift &
  operator=(const MnemonicBuilderTernaryNullInstrWithShift &r_src) = delete;

  /// \brief Move constructor for MnemonicBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  MnemonicBuilderTernaryNullInstrWithShift(MnemonicBuilderTernaryNullInstrWithShift &&r_src) =
      delete;

  /// \brief Move assignment operator for MnemonicBuilderTernaryNullInstrWithShift.
  /// \param r_src the object to be copied
  MnemonicBuilderTernaryNullInstrWithShift &
  operator=(MnemonicBuilderTernaryNullInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu