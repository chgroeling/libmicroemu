#pragma once

#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryInstr {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDest, typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDest &rd, const TArg0 &rn,
                    const TArg1 &rm) {

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
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryInstr() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryInstr() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstr(const MnemonicBuilderBinaryInstr &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryInstr.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryInstr &operator=(const MnemonicBuilderBinaryInstr &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstr(MnemonicBuilderBinaryInstr &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryInstr.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryInstr &operator=(MnemonicBuilderBinaryInstr &&r_src) = delete;
};

} // namespace libmicroemu::internal