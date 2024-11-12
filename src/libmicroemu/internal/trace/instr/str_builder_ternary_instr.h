#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2, typename TArg3>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &d, const TArg1 &n, const TArg2 &m,
                    const TArg3 &a) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');

    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(d.Get())).AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(a.Get()));
    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderTernaryInstr() = delete;

  /// \brief Destructor
  ~StrBuilderTernaryInstr() = delete;

  /// \brief Copy constructor for StrBuilderTernaryInstr.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstr(const StrBuilderTernaryInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderTernaryInstr.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstr &operator=(const StrBuilderTernaryInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderTernaryInstr.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstr(StrBuilderTernaryInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderTernaryInstr.
  /// \param r_src the object to be copied
  StrBuilderTernaryInstr &operator=(StrBuilderTernaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu