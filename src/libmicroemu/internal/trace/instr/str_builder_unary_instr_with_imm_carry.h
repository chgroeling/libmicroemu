#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderUnaryInstrWithImmCarry {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d,
                    const ThumbImmediateResult &imm_carry) {

    static_cast<void>(bflags);
    const bool is_setflags = (iflags & InstrFlags::kSetFlags) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)).data())
        .AddString(", #")
        .AddUInt(imm_carry.out)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderUnaryInstrWithImmCarry() = delete;

  /// \brief Destructor
  ~StrBuilderUnaryInstrWithImmCarry() = delete;

  /// \brief Copy constructor for StrBuilderUnaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstrWithImmCarry(const StrBuilderUnaryInstrWithImmCarry &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderUnaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstrWithImmCarry &
  operator=(const StrBuilderUnaryInstrWithImmCarry &r_src) = delete;

  /// \brief Move constructor for StrBuilderUnaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstrWithImmCarry(StrBuilderUnaryInstrWithImmCarry &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderUnaryInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderUnaryInstrWithImmCarry &operator=(StrBuilderUnaryInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace microemu