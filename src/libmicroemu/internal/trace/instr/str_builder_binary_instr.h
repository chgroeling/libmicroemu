#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderBinaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &n, const u8 &m) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0;

    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');
    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(d))).AddString(", ");
    }
    mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)))
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderBinaryInstr() = delete;

  /// \brief Destructor
  ~StrBuilderBinaryInstr() = delete;

  /// \brief Copy constructor for StrBuilderBinaryInstr.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstr(const StrBuilderBinaryInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderBinaryInstr.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstr &operator=(const StrBuilderBinaryInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderBinaryInstr.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstr(StrBuilderBinaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderBinaryInstr.
  /// \param r_src the object to be copied
  StrBuilderBinaryInstr &operator=(StrBuilderBinaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu