#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryLoadStoreInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &n, const u8 &m, const u8 &t,
                    const ImmShiftResults &shift_res) {

    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(t)))
        .AddString(", [")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)));

    if (shift_res.value != 0U) {
      mctx.builder.AddString(", LSL #").AddUInt(shift_res.value);
    }
    mctx.builder.AddChar(']').Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderTernaryLoadStoreInstrWithShift() = delete;

  /// \brief Destructor
  ~StrBuilderTernaryLoadStoreInstrWithShift() = delete;

  /// \brief Copy constructor for StrBuilderTernaryLoadStoreInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithShift(const StrBuilderTernaryLoadStoreInstrWithShift &r_src) =
      default;

  /// \brief Copy assignment operator for StrBuilderTernaryLoadStoreInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithShift &
  operator=(const StrBuilderTernaryLoadStoreInstrWithShift &r_src) = delete;

  /// \brief Move constructor for StrBuilderTernaryLoadStoreInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithShift(StrBuilderTernaryLoadStoreInstrWithShift &&r_src) =
      delete;

  /// \brief Move assignment operator for StrBuilderTernaryLoadStoreInstrWithShift.
  /// \param r_src the object to be copied
  StrBuilderTernaryLoadStoreInstrWithShift &
  operator=(StrBuilderTernaryLoadStoreInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace microemu