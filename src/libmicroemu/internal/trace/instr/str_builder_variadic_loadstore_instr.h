#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

template <typename TContext> class StrBuilderVariadicLoadStoreInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u8 &n, const u32 &registers,
                    bool suppress_dest_register) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & InstrFlags::kWBack) != 0u;
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.pstates)).AddChar(' ');

    if (!suppress_dest_register) {
      mctx.builder.AddString(Reg::GetRegisterName(n));
    }
    if (is_wback) {
      mctx.builder.AddChar('!');
    }
    if (!suppress_dest_register) {
      mctx.builder.AddString(", ");
    }
    mctx.builder.AddChar('{');
    auto regs_cnt = Bm32::BitCount(registers);
    for (u32 pstates = 0u; pstates <= 15u; ++pstates) {
      u32 bm = 0b1u << pstates;
      if ((registers & bm) != 0u) {
        auto r_spec = Reg::GetRegisterName(pstates);

        mctx.builder.AddString(r_spec);
        regs_cnt--;
        if (regs_cnt > 0u) {
          mctx.builder.AddString(", ");
        }
      }
    }
    mctx.builder.AddChar('}');
    mctx.builder.Terminate();
  }

private:
  /// \brief Constructor
  StrBuilderVariadicLoadStoreInstr() = delete;

  /// \brief Destructor
  ~StrBuilderVariadicLoadStoreInstr() = delete;

  /// \brief Copy constructor for StrBuilderVariadicLoadStoreInstr.
  /// \param r_src the object to be copied
  StrBuilderVariadicLoadStoreInstr(const StrBuilderVariadicLoadStoreInstr &r_src) = default;

  /// \brief Copy assignment operator for StrBuilderVariadicLoadStoreInstr.
  /// \param r_src the object to be copied
  StrBuilderVariadicLoadStoreInstr &
  operator=(const StrBuilderVariadicLoadStoreInstr &r_src) = delete;

  /// \brief Move constructor for StrBuilderVariadicLoadStoreInstr.
  /// \param r_src the object to be copied
  StrBuilderVariadicLoadStoreInstr(StrBuilderVariadicLoadStoreInstr &&r_src) = delete;

  /// \brief Move assignment operator for StrBuilderVariadicLoadStoreInstr.
  /// \param r_src the object to be copied
  StrBuilderVariadicLoadStoreInstr &operator=(StrBuilderVariadicLoadStoreInstr &&r_src) = delete;
};

} // namespace internal
} // namespace microemu