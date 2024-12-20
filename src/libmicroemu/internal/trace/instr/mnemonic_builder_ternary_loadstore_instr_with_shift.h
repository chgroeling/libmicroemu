#pragma once

#include "libmicroemu/internal/result.h"
#include "libmicroemu/types.h"

namespace libmicroemu::internal {

/**
 * @brief Load from immediate adr to register
 */
template <typename TContext> class MnemonicBuilderTernaryLoadStoreInstrWithShift {
public:
  using It = typename TContext::It;
  using Reg = typename TContext::Reg;
  using SReg = typename TContext::SReg;

  template <typename TTgt, typename TArg0, typename TArg1>
  static void Build(const char *instr_spec, TContext &mctx, const MnemonicBuilderFlagsSet &bflags,
                    const InstrFlagsSet &iflags, const TTgt &rt, const TArg0 &rn, const TArg1 &rm,
                    const ImmShiftResults &shift_res) {

    static_cast<void>(iflags);
    static_cast<void>(bflags);
    mctx.builder.AddString(instr_spec)
        .AddString(It::GetConditionAsStr(mctx.cpua))
        .AddChar(' ')
        .AddString(Reg::GetRegisterName(rt.Get()))
        .AddString(", [")
        .AddString(Reg::GetRegisterName(rn.Get()))
        .AddString(", ")
        .AddString(Reg::GetRegisterName(rm.Get()));

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

} // namespace libmicroemu::internal