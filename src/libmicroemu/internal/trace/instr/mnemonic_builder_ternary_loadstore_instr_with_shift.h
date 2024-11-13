#pragma once

#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu {
namespace internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderTernaryLoadStoreInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TArg0, typename TArg1, typename TArg2>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TArg0 &n, const TArg1 &m, const TArg2 &t,
                    const ImmShiftResults &shift_res) {

    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.pstates))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(t.Get()))
        .AddString(", [")
        .AddString(Reg::GetRegisterName(n.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(m.Get()));

    if (shift_res.value != 0U) {
      mctx.builder.AddString(", LSL #").AddUInt(shift_res.value);
    }
    mctx.builder.AddChar(']').Terminate();
  }

private:
  /**
   * @brief Constructor
   */
  MnemonicBuilderTernaryLoadStoreInstrWithShift() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilderTernaryLoadStoreInstrWithShift() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilderTernaryLoadStoreInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryLoadStoreInstrWithShift(
      const MnemonicBuilderTernaryLoadStoreInstrWithShift &r_src) = default;

  /**
   * @brief Copy assignment operator for MnemonicBuilderTernaryLoadStoreInstrWithShift.
   * @param r_src the object to be copied
   */
  MnemonicBuilderTernaryLoadStoreInstrWithShift &
  operator=(const MnemonicBuilderTernaryLoadStoreInstrWithShift &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilderTernaryLoadStoreInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryLoadStoreInstrWithShift(
      MnemonicBuilderTernaryLoadStoreInstrWithShift &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilderTernaryLoadStoreInstrWithShift.
   * @param r_src the object to be moved
   */
  MnemonicBuilderTernaryLoadStoreInstrWithShift &
  operator=(MnemonicBuilderTernaryLoadStoreInstrWithShift &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu