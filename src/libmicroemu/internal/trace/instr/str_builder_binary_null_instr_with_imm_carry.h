#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryNullInstrWithImmCarry {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &m,
                    const ThumbImmediateResult &imm_carry) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates).data())
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)).data())
        .AddString(", #")
        .AddUInt(imm_carry.out)
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryNullInstrWithImmCarry() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryNullInstrWithImmCarry() = delete;

  /// \brief Copy constructor for StrBuilderBinaryNullInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImmCarry(const StrBuilderBinaryNullInstrWithImmCarry &r_src) =
      default;

  /// \brief Copy assignment operator for StrBuilderBinaryNullInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImmCarry &
  operator=(const StrBuilderBinaryNullInstrWithImmCarry &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryNullInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImmCarry(StrBuilderBinaryNullInstrWithImmCarry &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryNullInstrWithImmCarry.
  /// \param r_src the object to be copied
  StrBuilderBinaryNullInstrWithImmCarry &
  operator=(StrBuilderBinaryNullInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace microemu