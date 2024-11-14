#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderBinaryNullInstrWithImmCarry {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &m,
                    const ThumbImmediateResult &imm_carry) {
    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(m.Get()))
        .AddString(", #")
        .AddUInt(imm_carry.out)
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderBinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderBinaryNullInstrWithImmCarry() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderBinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstrWithImmCarry(
      const MnemonicBuilderBinaryNullInstrWithImmCarry &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderBinaryNullInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  MnemonicBuilderBinaryNullInstrWithImmCarry &
  operator=(const MnemonicBuilderBinaryNullInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderBinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstrWithImmCarry(MnemonicBuilderBinaryNullInstrWithImmCarry &&r_src) =
      delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderBinaryNullInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  MnemonicBuilderBinaryNullInstrWithImmCarry &
  operator=(MnemonicBuilderBinaryNullInstrWithImmCarry &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu