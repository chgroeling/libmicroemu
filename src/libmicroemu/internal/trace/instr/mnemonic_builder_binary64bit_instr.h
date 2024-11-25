#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Binary instruction with 64-bit result
 */
template <typename TContext> class MnemonicBuilderBinary64bitInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDestLo, typename TDestHi, typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDestLo &d_lo, const TDestHi &d_hi,
                    const TArg0 &rn, const TArg1 &rm) {
    static_cast<void>(bflags);
    static_cast<void>(iflags);

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(d_lo.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(d_hi.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rn.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rm.Get()))
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinary64bitInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinary64bitInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinary64bitInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinary64bitInstr(const MnemonicBuilderBinary64bitInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinary64bitInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinary64bitInstr &operator=(const MnemonicBuilderBinary64bitInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinary64bitInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinary64bitInstr(MnemonicBuilderBinary64bitInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinary64bitInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinary64bitInstr &operator=(MnemonicBuilderBinary64bitInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal