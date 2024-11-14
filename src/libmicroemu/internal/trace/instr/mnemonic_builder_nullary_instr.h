#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderNullaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderNullaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderNullaryInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderNullaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderNullaryInstr(const MnemonicBuilderNullaryInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderNullaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderNullaryInstr &operator=(const MnemonicBuilderNullaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderNullaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderNullaryInstr(MnemonicBuilderNullaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderNullaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderNullaryInstr &operator=(MnemonicBuilderNullaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu