#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderTernaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDest, typename TArg0, typename TArg1, typename TArg3>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDest &rd, const TArg0 &rn, const TArg1 &rm,
                    const TArg3 &a) {

    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    const bool is_reduced_rd =
        (bflags & static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ');

    if ((rd != rn) || (!is_reduced_rd)) {
      mctx.builder.AddString(Reg::GetRegisterName(rd.Get())).AddString(", ");
    }

    mctx.builder.AddString(Reg::GetRegisterName(rn.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rm.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(a.Get()));
    mctx.builder.Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderTernaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderTernaryInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderTernaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryInstr(const MnemonicBuilderTernaryInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderTernaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryInstr &operator=(const MnemonicBuilderTernaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderTernaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryInstr(MnemonicBuilderTernaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderTernaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryInstr &operator=(MnemonicBuilderTernaryInstr &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu