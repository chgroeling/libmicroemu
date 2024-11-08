#pragma once

#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

enum class Condition : u32 {
  kCondEq = 0b0000u, // Equal
  kCondNe = 0b0001u, // Not equal

  kCondCs = 0b0010u, // Carry set
  kCondCc = 0b0011u, // Carry clear

  kCondMi = 0b0100u, // Minus, negative
  kCondPl = 0b0101u, // Plus, positive or zero

  kCondVs = 0b0110u, // Overflow
  kCondVc = 0b0111u, // No overflow

  kCondHi = 0b1000u, // Unsigned higher
  kCondLs = 0b1001u, // Unsigned lower or same

  kCondGe = 0b1010u, // Signed greater than or equal
  kCondLt = 0b1011u, // Signed less than

  kCondGt = 0b1100u, // Signed greater than
  kCondLe = 0b1101u, // Signed less than or

  kCondAl = 0b1110u,    // Always
  kUndefined = 0b1111u, // Undefined
};

template <typename TProcessorStates, typename TSpecRegOps> class IfThenOps {

public:
  // TODO: Move constants up here

  /// \brief Constructs a IfThenOps object
  IfThenOps() = delete;

  /// \brief Destructor
  ~IfThenOps() = delete;

  /// \brief Copy constructor for IfThenOps.
  /// \param r_src the object to be copied
  IfThenOps(IfThenOps &r_src) = delete;

  /// \brief Copy assignment operator for IfThenOps.
  /// \param r_src the object to be copied
  IfThenOps &operator=(const IfThenOps &r_src) = delete;

  /// \brief Move constructor for IfThenOps.
  /// \param r_src the object to be copied
  IfThenOps(IfThenOps &&r_src) = delete;

  /// \brief Move assignment operator for IfThenOps.
  /// \param r_src the object to be copied
  IfThenOps &operator=(IfThenOps &&r_src) = delete;

  static inline bool InITBlock(const TProcessorStates &pstates) {
    using SReg = TSpecRegOps;
    auto istate = SReg::template ReadRegister<SpecialRegisterId::kIstate>(pstates);

    return ((istate & IstateRegister::kItBit3to0Msk) != 0b0000u);
  }

  static inline bool LastInITBlock(TProcessorStates &pstates) {
    using SReg = TSpecRegOps;
    auto istate = SReg::template ReadRegister<SpecialRegisterId::kIstate>(pstates);
    return ((istate & IstateRegister::kItBit3to0Msk) == 0b1000u);
  }

  static inline void ITAdvance(TProcessorStates &pstates) {
    using SReg = TSpecRegOps;

    auto istate = SReg::template ReadRegister<SpecialRegisterId::kIstate>(pstates);
    const auto istate_2_0 = istate & IstateRegister::kItBit2to0Msk;

    if (istate_2_0 == 0x0u) {
      istate = 0x0u;
    } else {
      const auto next_istate_4_0 =
          ((istate & IstateRegister::kItBit4to0Msk) << 1u) & IstateRegister::kItBit4to0Msk;
      istate = (istate & ~IstateRegister::kItBit4to0Msk) | next_istate_4_0;
    }

    SReg::template WriteRegister<SpecialRegisterId::kIstate>(pstates, istate);
  }

  static inline Result<u32> CurrentCond(const TProcessorStates &pstates) {
    using SReg = TSpecRegOps;

    // • For the T1 and T3 encodings of the Branch instruction shown in B on
    //   page A7-205, it returns the 4-bit cond field of the encoding.
    // --> This is handled by the encoder by Using ConiditionPassed(cond)

    // • For all other Thumb instructions:
    auto istate = SReg::template ReadRegister<SpecialRegisterId::kIstate>(pstates);

    const auto it_3_0 = istate & IstateRegister::kItBit3to0Msk;

    if (it_3_0 != 0x0u) {
      const auto it_7_4 = (istate & IstateRegister::kItBit7to4Msk) >> IstateRegister::kItBit4Pos;
      return Ok<u32>(it_7_4);
    }

    if (istate == 0x0u) {
      return Ok<u32>(0xEu); // b1110
    }

    // Otherwise, execution of the instruction is UNPREDICTABLE.
    return Err<u32>(StatusCode::kScExecutorUnpredictable);
  }

  static inline Condition DecodeCondition(u8 cond) {
    return static_cast<Condition>(cond & IstateRegister::kItBit3to0Msk);
  }

  static const char *ConditionToString(u8 cond) {
    switch (DecodeCondition(cond)) {

    case Condition::kCondEq: // Equal
      return "EQ";
    case Condition::kCondNe: // Not equal
      return "NE";
    case Condition::kCondCs: // Carry set
      return "CS";
    case Condition::kCondCc: // Carry clear
      return "CC";
    case Condition::kCondMi: // Minus, negative
      return "MI";
    case Condition::kCondPl: // Plus, positive or zero
      return "PL";
    case Condition::kCondVs: // Overflow
      return "VS";
    case Condition::kCondVc: // No overflow
      return "VC";
    case Condition::kCondHi: // Unsigned higher
      return "HI";
    case Condition::kCondLs: // Unsigned lower or same
      return "LS";
    case Condition::kCondGe: // Signed greater than or equal
      return "GE";
    case Condition::kCondLt: // Signed less than
      return "LT";
    case Condition::kCondGt: // Signed greater than
      return "GT";
    case Condition::kCondLe: // Signed less than or
      return "LE";
    case Condition::kCondAl: // Always
      return "";
    case Condition::kUndefined: // Undefined
    default:
      break;
    }
    return "invalid";
  }

  static const char *GetConditionAsStr(const TProcessorStates &pstates) {
    const auto r_cond = CurrentCond(pstates);
    if (r_cond.IsErr()) {
      return "invalid";
    }

    const auto &cond = r_cond.content;
    return ConditionToString(cond);
  }

  static inline bool ConditionPassed(const TProcessorStates &pstates, u8 cond) {

    using SReg = TSpecRegOps;

    // see Armv7-M Architecture Reference Manual Issue E.e p.178 - 179
    bool result{false};

    auto cond_3_1 = (cond & IstateRegister::kItBit3to1Msk) >> IstateRegister::kItBit1Pos;
    switch (cond_3_1) {
    case 0b000u: { // EQ - Equal or NE - Not equal
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = (apsr & ApsrRegister::kZMsk) == ApsrRegister::kZMsk;
      break;
    }
    case 0b001u: { // CS - Carry set  or CC - Carry clear
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = (apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk;
      break;
    }
    case 0b010u: { // MI - Minus, negative  or PL - Plus, positive or zero
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = (apsr & ApsrRegister::kNMsk) == ApsrRegister::kNMsk;
      break;
    }
    case 0b011u: { // VS - Overflow or VC - No overflow
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = (apsr & ApsrRegister::kVMsk) == ApsrRegister::kVMsk;
      break;
    }
    case 0b100u: { // HI - Unsigned higher or LS - Unsigned lower or same
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = ((apsr & ApsrRegister::kCMsk) == ApsrRegister::kCMsk) &&
               ((apsr & ApsrRegister::kZMsk) != ApsrRegister::kZMsk);
      break;
    }
    case 0b101u: { // GE - Signed greater than or equal or LT - Signed less than
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = ((apsr & ApsrRegister::kNMsk) >> ApsrRegister::kNPos) ==
               ((apsr & ApsrRegister::kVMsk) >> ApsrRegister::kVPos);
      break;
    }
    case 0b110u: { // GT - Signed greater than  or LE - Signed less than or
                   // equal
      auto apsr = SReg::template ReadRegister<SpecialRegisterId::kApsr>(pstates);
      result = (((apsr & ApsrRegister::kNMsk) >> ApsrRegister::kNPos) ==
                ((apsr & ApsrRegister::kVMsk) >> ApsrRegister::kVPos)) &&
               ((apsr & ApsrRegister::kZMsk) != ApsrRegister::kZMsk);
      break;
    }
    case 0b111u: { // AL - Always
      result = true;
      break;
    }
    default: {
      // should not happen
      assert(false);
    }
    }

    // Condition flag values in the set '111x' indicate the instruction is
    // always executed Otherwise, invert condition if necessary.

    // if cond<0> == '1' && cond != '1111' then result = !result;
    if (((cond & 0x1u) == 0x1u) && (cond != 0xFu)) {
      result = !result;
    }

    return result;
  }

  static inline Result<bool> ConditionPassed(const TProcessorStates &pstates) {
    // see Armv7-M Architecture Reference Manual Issue E.e p.179

    const auto r_cond = CurrentCond(pstates);
    if (r_cond.IsErr()) {
      return Err<u32, bool>(r_cond);
    }

    const auto &cond = r_cond.content;

    return Ok(ConditionPassed(pstates, cond));
  }
};

} // namespace internal
} // namespace microemu
