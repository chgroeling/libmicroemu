#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

template <typename TContext> class MnemonicBuilderVariadicLoadStoreInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &n, const u32 &registers,
                    bool suppress_dest_register) {
    static_cast<void>(bflags);
    const bool is_wback = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kWBack)) != 0U;
    mctx.builder.AddString(instr_spec).AddString(It::GetConditionAsStr(mctx.pstates)).AddChar(' ');

    if (!suppress_dest_register) {
      mctx.builder.AddString(Reg::GetRegisterName(n.Get()));
    }
    if (is_wback) {
      mctx.builder.AddChar('!');
    }
    if (!suppress_dest_register) {
      mctx.builder.AddString(", ");
    }
    mctx.builder.AddChar('{');
    auto regs_cnt = Bm32::BitCount(registers);
    for (u32 pstates = 0U; pstates <= 15U; ++pstates) {
      u32 bm = 0b1U << pstates;
      if ((registers & bm) != 0U) {
        auto r_spec = Reg::GetRegisterName(static_cast<RegisterId>(pstates));

        mctx.builder.AddString(r_spec);
        regs_cnt--;
        if (regs_cnt > 0U) {
          mctx.builder.AddString(", ");
        }
      }
    }
    mctx.builder.AddChar('}');
    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderVariadicLoadStoreInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderVariadicLoadStoreInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderVariadicLoadStoreInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderVariadicLoadStoreInstr(const MnemonicBuilderVariadicLoadStoreInstr &r_src) =
      default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderVariadicLoadStoreInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderVariadicLoadStoreInstr &
  operator=(const MnemonicBuilderVariadicLoadStoreInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderVariadicLoadStoreInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderVariadicLoadStoreInstr(MnemonicBuilderVariadicLoadStoreInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderVariadicLoadStoreInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderVariadicLoadStoreInstr &
  operator=(MnemonicBuilderVariadicLoadStoreInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu