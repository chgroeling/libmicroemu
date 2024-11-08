#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryInstrWithImmCarry {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  /// TODO: Introduce  TArg0, TArg1 for every builder in this directory
  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &n,
                    const ThumbImmediateResult &imm_carry) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates).data())
        .AddChar(' ');

    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(d.Get()).data()).AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(n.Get()).data())
        .AddString(", #")
        .AddUInt(imm_carry.out)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryInstrWithImmCarry() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryInstrWithImmCarry() = delete;

  /// \brief Copy constructor for StrBuilderBinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImmCarry(const StrBuilderBinaryInstrWithImmCarry &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImmCarry &
  operator=(const StrBuilderBinaryInstrWithImmCarry &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImmCarry(StrBuilderBinaryInstrWithImmCarry &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithImmCarry &operator=(StrBuilderBinaryInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace microemu