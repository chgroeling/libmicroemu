#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderUnaryInstrWithImmCarry {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TDest>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TDest &rd,
                    const ThumbImmediateResult &imm_carry) {

    static_cast<void>(bflags);
    const bool is_setflags = (iflags & static_cast<InstrFlagsSet>(InstrFlags::kSetFlags)) != 0U;
    mctx.builder.AddString(instr_spec)
        .AddString(is_setflags == true ? "S" : "")
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(rd.Get()))
        .AddString(", #")
        .AddUInt(imm_carry.out)
        .Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderUnaryInstrWithImmCarry() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderUnaryInstrWithImmCarry() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderUnaryInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryInstrWithImmCarry(const MnemonicBuilderUnaryInstrWithImmCarry &r_src) =
      default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderUnaryInstrWithImmCarry.
   * @param r_src the object to be copied
   */
  MnemonicBuilderUnaryInstrWithImmCarry &
  operator=(const MnemonicBuilderUnaryInstrWithImmCarry &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderUnaryInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryInstrWithImmCarry(MnemonicBuilderUnaryInstrWithImmCarry &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderUnaryInstrWithImmCarry.
   * @param r_src the object to be moved
   */
  MnemonicBuilderUnaryInstrWithImmCarry &
  operator=(MnemonicBuilderUnaryInstrWithImmCarry &&r_src) = delete;
};

} // namespace libmicroemu::internal