#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryNullInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &rm, const TArg1 &rn,
                    const char *shift_str) {
    static_cast<void>(instr_spec);
    static_cast<void>(bflags);
    static_cast<void>(iflags);

    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddString(" [")
        .AddString(Reg::GetRegisterName(rn.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rm.Get()))
        .AddString(shift_str)
        .AddChar(']');

    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryNullInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryNullInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryNullInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstr(const MnemonicBuilderBinaryNullInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryNullInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstr &operator=(const MnemonicBuilderBinaryNullInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryNullInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstr(MnemonicBuilderBinaryNullInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryNullInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstr &operator=(MnemonicBuilderBinaryNullInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal