#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderUnaryBranchInstrWithImm {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const u32 &imm) {
    static_cast<void>(bflags);
    static_cast<void>(iflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddString(" #")
        .AddInt(static_cast<int>(imm))
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderUnaryBranchInstrWithImm() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderUnaryBranchInstrWithImm() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderUnaryBranchInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryBranchInstrWithImm(const MnemonicBuilderUnaryBranchInstrWithImm &r_src) =
      default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderUnaryBranchInstrWithImm.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryBranchInstrWithImm &
  operator=(const MnemonicBuilderUnaryBranchInstrWithImm &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderUnaryBranchInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryBranchInstrWithImm(MnemonicBuilderUnaryBranchInstrWithImm &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderUnaryBranchInstrWithImm.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryBranchInstrWithImm &
  operator=(MnemonicBuilderUnaryBranchInstrWithImm &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu