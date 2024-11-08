#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

/// Load from immediate adr to register
template <typename TContext> class StrBuilderTernaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &d, const u8 &n, const u8 &m,
                    const u8 &a) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ');

    if ((d != n) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(d)).data())
          .AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(static_cast<RegisterId>(n)).data())
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(m)).data())
        .AddString(", ")
        .AddString(Reg::GetRegisterName(static_cast<RegisterId>(a)).data());
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
} // namespace microemu