#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryInstrWithRotation {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &m, const u8 &rotation) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)));

    if (rotation != 0U) {
      mctx.builder.AddString(", ROR #").AddUInt(rotation);
    }

    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryInstrWithRotation() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryInstrWithRotation() = delete;

  /// \brief Copy constructor for StrBuilderBinaryInstrWithRotation.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithRotation(const StrBuilderBinaryInstrWithRotation &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryInstrWithRotation.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithRotation &
  operator=(const StrBuilderBinaryInstrWithRotation &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryInstrWithRotation.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithRotation(StrBuilderBinaryInstrWithRotation &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryInstrWithRotation.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstrWithRotation &operator=(StrBuilderBinaryInstrWithRotation &&r_src) = delete;
};

} // namespace internal
} // namespace microemu