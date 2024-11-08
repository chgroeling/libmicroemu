#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderNullaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & InstrFlags::kSetFlags) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderNullaryInstr() = delete;

  /// \brief Destructor
  ~StrBuilderNullaryInstr() = delete;

  /// \brief Copy constructor for StrBuilderNullaryInstr.
  /// \param r_src the object to be copied
  StrBuilderNullaryInstr(const StrBuilderNullaryInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderNullaryInstr.
  /// \param r_src the object to be copied
  StrBuilderNullaryInstr &operator=(const StrBuilderNullaryInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderNullaryInstr.
  /// \param r_src the object to be copied
  StrBuilderNullaryInstr(StrBuilderNullaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderNullaryInstr.
  /// \param r_src the object to be copied
  StrBuilderNullaryInstr &operator=(StrBuilderNullaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu