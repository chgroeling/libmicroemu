#pragma once

#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_instr_with_rotation.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_loadstore_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_null_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_binary_null_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_nullary_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_special_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_ternary_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_ternary_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_ternary_loadstore_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_ternary_loadstore_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_ternary_null_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_unary_branch_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_unary_branch_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_unary_instr.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_unary_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_unary_null_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/mnemonic_builder_variadic_loadstore_instr.h"
#include "libmicroemu/internal/trace/mnemonic_builder_context.h"
#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/types.h"
#include <cstddef>

namespace libmicroemu {
namespace internal {

class MnemonicBuilder {
public:
  template <typename TCpuAccessor, typename TItOps, typename TRegOps, typename TSpecRegOps>
  static void Build(const TCpuAccessor &cpua, const Instr &instr, char *buf, std::size_t buf_len) {

    auto cstr_builder = ConstStringBuilder(buf, buf_len);
    using TMnemonicBuilderContext =
        MnemonicBuilderContext<TCpuAccessor, TItOps, TRegOps, TSpecRegOps>;
    auto mctx = TMnemonicBuilderContext{cpua, cstr_builder};
    MnemonicBuilderFlagsSet bflags =
        static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kNone);
    switch (instr.id) {
    case InstrId::kLdrLiteral: {
      const auto &iargs = instr.ldr_literal;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrLiteral(
          "LDR", mctx, bflags, iargs.flags, RArg(iargs.t), iargs.imm32);
      break;
    }
    case InstrId::kLdrsbImmediate: {
      const auto &iargs = instr.ldrsb_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRSB", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kLdrbImmediate: {
      const auto &iargs = instr.ldrb_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRB", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kLdrshImmediate: {
      const auto &iargs = instr.ldrsh_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRSH", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kLdrhImmediate: {
      const auto &iargs = instr.ldrh_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRH", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kLdrImmediate: {
      const auto &iargs = instr.ldr_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDR", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kLdrex: {
      const auto &iargs = instr.ldr_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDREX", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kMsr: {
      const auto &iargs = instr.msr;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildMsr(
          "MSR", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.mask, iargs.SYSm);
      break;
    }
    case InstrId::kMrs: {
      const auto &iargs = instr.mrs;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildMrs(
          "MRS", mctx, bflags, iargs.flags, RArg(iargs.d), iargs.mask, iargs.SYSm);
      break;
    }
    case InstrId::kUmlal: {
      const auto &iargs = instr.umlal;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "UMLAL", mctx, bflags, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi), RArg(iargs.n),
          RArg(iargs.m));
      break;
    }
    case InstrId::kUmull: {
      const auto &iargs = instr.umull;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "UMULL", mctx, bflags, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi), RArg(iargs.n),
          RArg(iargs.m));
      break;
    }
    case InstrId::kSmull: {
      const auto &iargs = instr.umull;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "SMULL", mctx, bflags, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi), RArg(iargs.n),
          RArg(iargs.m));
      break;
    }
    case InstrId::kLdrdImmediate: {
      const auto &iargs = instr.ldrd_immediate;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrdStrd(
          "LDRD", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.t2), RArg(iargs.n),
          iargs.imm32);
      break;
    }
    case InstrId::kCmpImmediate: {
      const auto &iargs = instr.cmp_immediate;
      MnemonicBuilderBinaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "CMP", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kCmnImmediate: {
      const auto &iargs = instr.cmn_immediate;
      MnemonicBuilderBinaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "CMN", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kIt: {
      const auto &iargs = instr.it;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildIt("IT", mctx, bflags, iargs.flags,
                                                                    iargs.firstcond, iargs.mask);
      break;
    }
    case InstrId::kMovImmediate: {
      const auto &iargs = instr.mov_immediate;
      MnemonicBuilderUnaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "MOV", mctx, bflags, iargs.flags, RArg(iargs.d), iargs.imm32_carry);
      break;
    }
    case InstrId::kMvnImmediate: {
      const auto &iargs = instr.mvn_immediate;
      MnemonicBuilderUnaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "MVN", mctx, bflags, iargs.flags, RArg(iargs.d), iargs.imm32_carry);
      break;
    }
    case InstrId::kMovRegister: {
      const auto &iargs = instr.mov_register;
      MnemonicBuilderUnaryInstr<TMnemonicBuilderContext>::Build("MOV", mctx, bflags, iargs.flags,
                                                                RArg(iargs.d), RArg(iargs.m));
      break;
    }
    case InstrId::kRrx: {
      const auto &iargs = instr.rrx;
      MnemonicBuilderUnaryInstr<TMnemonicBuilderContext>::Build("RRX", mctx, bflags, iargs.flags,
                                                                RArg(iargs.d), RArg(iargs.m));
      break;
    }
    case InstrId::kBl: {
      const auto &iargs = instr.bl;
      MnemonicBuilderUnaryBranchInstrWithImm<TMnemonicBuilderContext>::Build(
          "BL", mctx, bflags, iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kBx: {
      const auto &iargs = instr.bx;
      MnemonicBuilderUnaryBranchInstr<TMnemonicBuilderContext>::Build("BX", mctx, bflags,
                                                                      iargs.flags, RArg(iargs.m));
      break;
    }
    case InstrId::kBlx: {
      const auto &iargs = instr.blx;
      MnemonicBuilderUnaryBranchInstr<TMnemonicBuilderContext>::Build("BLX", mctx, bflags,
                                                                      iargs.flags, RArg(iargs.m));
      break;
    }
    case InstrId::kBCond: {
      const auto &iargs = instr.b_cond;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildBCond(
          "B", mctx, bflags, iargs.flags, iargs.cond, iargs.imm32);
      break;
    }
    case InstrId::kB: {
      const auto &iargs = instr.b;
      MnemonicBuilderUnaryBranchInstrWithImm<TMnemonicBuilderContext>::Build(
          "B", mctx, bflags, iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kSubImmediate: {
      const auto &iargs = instr.sub_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "SUB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kSbcImmediate: {
      const auto &iargs = instr.sbc_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "SBC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kRsbImmediate: {
      const auto &iargs = instr.rsb_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "RSB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kSubSpMinusImmediate: {
      const auto &iargs = instr.sub_sp_minus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "SUB", mctx, bflags, iargs.flags, RArg(iargs.d), RArgConst<RegisterId::kSp>(),
          iargs.imm32);
      break;
    }
    case InstrId::kAddSpPlusImmediate: {
      const auto &iargs = instr.add_sp_plus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, RArg(iargs.d), RArgConst<RegisterId::kSp>(),
          iargs.imm32);
      break;
    }
    case InstrId::kAddImmediate: {
      const auto &iargs = instr.add_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kAdcImmediate: {
      const auto &iargs = instr.adc_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kAddPcPlusImmediate: {
      const auto &iargs = instr.add_pc_plus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, RArg(iargs.d), RArgConst<RegisterId::kPc>(),
          iargs.imm32);
      break;
    }
    case InstrId::kClz: {
      const auto &iargs = instr.clz;
      MnemonicBuilderUnaryInstr<TMnemonicBuilderContext>::Build("CLZ", mctx, bflags, iargs.flags,
                                                                RArg(iargs.d), RArg(iargs.m));
      break;
    }
    case InstrId::kSubRegister: {
      const auto &iargs = instr.sub_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "SUB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kRsbRegister: {
      const auto &iargs = instr.rsb_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "RSB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kUdiv: {
      const auto &iargs = instr.udiv;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "UDIV", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kSdiv: {
      const auto &iargs = instr.sdiv;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "SDIV", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kMls: {
      const auto &iargs = instr.mls;
      MnemonicBuilderTernaryInstr<TMnemonicBuilderContext>::Build("MLS", mctx, bflags, iargs.flags,
                                                                  RArg(iargs.d), RArg(iargs.n),
                                                                  RArg(iargs.m), RArg(iargs.a));
      break;
    }
    case InstrId::kMla: {
      const auto &iargs = instr.mla;
      MnemonicBuilderTernaryInstr<TMnemonicBuilderContext>::Build("MLA", mctx, bflags, iargs.flags,
                                                                  RArg(iargs.d), RArg(iargs.n),
                                                                  RArg(iargs.m), RArg(iargs.a));
      break;
    }
    case InstrId::kMul: {
      const auto &iargs = instr.mul;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "MUL", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kAddRegister: {
      const auto &iargs = instr.add_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kAdcRegister: {
      const auto &iargs = instr.adc_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ADC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kMvnRegister: {
      const auto &iargs = instr.mvn_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "MVN", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.shift_res);
      break;
    }
    case InstrId::kLsrImmediate: {
      const auto &iargs = instr.lsr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      MnemonicBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "LSR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.shift_res);
      break;
    }
    case InstrId::kAsrImmediate: {
      const auto &iargs = instr.asr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      MnemonicBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ASR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.shift_res);
      break;
    }
    case InstrId::kLslImmediate: {
      const auto &iargs = instr.lsl_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      MnemonicBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "LSL", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.shift_res);
      break;
    }
    case InstrId::kLsrRegister: {
      const auto &iargs = instr.lsr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "LSR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kAsrRegister: {
      const auto &iargs = instr.asr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "ASR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kLslRegister: {
      const auto &iargs = instr.lsl_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstr<TMnemonicBuilderContext>::Build(
          "LSL", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kStm: {
      const auto &iargs = instr.stm;
      MnemonicBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "STM", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.registers, false);
      break;
    }
    case InstrId::kPush: {
      const auto &iargs = instr.push;
      MnemonicBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "PUSH", mctx, bflags, iargs.flags, RArgConst<RegisterId::kSp>(), iargs.registers, true);
      break;
    }
    case InstrId::kLdm: {
      const auto &iargs = instr.ldm;
      MnemonicBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "LDM", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.registers, false);
      break;
    }
    case InstrId::kPop: {
      const auto &iargs = instr.pop;
      MnemonicBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "POP", mctx, bflags, iargs.flags, RArgConst<RegisterId::kSp>(), iargs.registers, true);
      break;
    }
    case InstrId::kSxtb: {
      const auto &iargs = instr.sxtb;
      MnemonicBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "SXTB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.rotation);
      break;
    }
    case InstrId::kUxtb: {
      const auto &iargs = instr.uxtb;
      MnemonicBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "UXTB", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.rotation);
      break;
    }
    case InstrId::kSxth: {
      const auto &iargs = instr.sxth;
      MnemonicBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "SXTH", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.rotation);
      break;
    }
    case InstrId::kUxth: {
      const auto &iargs = instr.uxth;
      MnemonicBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "UXTH", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.m), iargs.rotation);
      break;
    }
    case InstrId::kUbfx: {
      const auto &iargs = instr.ubfx;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUbfx(
          "UBFX", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.lsbit,
          iargs.widthminus1);
      break;
    }
    case InstrId::kBfi: {
      const auto &iargs = instr.bfi;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildBfi(
          "BFI", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.lsbit, iargs.msbit);
      break;
    }
    case InstrId::kSbcRegister: {
      const auto &iargs = instr.sbc_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "SBC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kEorRegister: {
      const auto &iargs = instr.eor_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "EOR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kOrrRegister: {
      const auto &iargs = instr.orr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ORR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kAndRegister: {
      const auto &iargs = instr.and_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "AND", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kBicRegister: {
      const auto &iargs = instr.bic_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "BIC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kBicImmediate: {
      const auto &iargs = instr.bic_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "BIC", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kStrdImmediate: {
      const auto &iargs = instr.strd_immediate;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrdStrd(
          "STRD", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.t2), RArg(iargs.n),
          iargs.imm32);
      break;
    }
    case InstrId::kCmpRegister: {
      const auto &iargs = instr.cmp_register;
      MnemonicBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "CMP", mctx, bflags, iargs.flags, RArg(iargs.n), RArg(iargs.m), iargs.shift_res, false);
      break;
    }
    case InstrId::kTeqImmediate: {
      const auto &iargs = instr.teq_immediate;
      MnemonicBuilderBinaryNullInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "TEQ", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kTstImmediate: {
      const auto &iargs = instr.tst_immediate;
      MnemonicBuilderBinaryNullInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "TST", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kTstRegister: {
      const auto &iargs = instr.tst_register;
      MnemonicBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "TST", mctx, bflags, iargs.flags, RArg(iargs.n), RArg(iargs.m), iargs.shift_res, false);
      break;
    }
    case InstrId::kTeqRegister: {
      const auto &iargs = instr.teq_register;
      MnemonicBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "TEQ", mctx, bflags, iargs.flags, RArg(iargs.n), RArg(iargs.m), iargs.shift_res, false);
      break;
    }
    case InstrId::kEorImmediate: {
      const auto &iargs = instr.eor_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "EOR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kOrrImmediate: {
      const auto &iargs = instr.orr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "ORR", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kAndImmediate: {
      const auto &iargs = instr.and_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      MnemonicBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "AND", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.n), iargs.imm32_carry);
      break;
    }
    case InstrId::kStrhImmediate: {
      const auto &iargs = instr.strh_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STRH", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kTbbH: {
      const auto &iargs = instr.tbb_h;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildTbbH(
          "TBH", mctx, bflags, iargs.flags, RArg(iargs.n), RArg(iargs.m));
      break;
    }
    case InstrId::kStrbImmediate: {
      const auto &iargs = instr.strb_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STRB", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kStrImmediate: {
      const auto &iargs = instr.str_immediate;
      MnemonicBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STR", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kStrex: {
      const auto &iargs = instr.strex;
      MnemonicBuilderTernaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STREX", mctx, bflags, iargs.flags, RArg(iargs.d), RArg(iargs.t), RArg(iargs.n),
          iargs.imm32);
      break;
    }
    case InstrId::kLdrhRegister: {
      const auto &iargs = instr.ldrh_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDRH", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kLdrbRegister: {
      const auto &iargs = instr.ldrb_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDRB", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kLdrRegister: {
      const auto &iargs = instr.ldr_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDR", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kStrRegister: {
      const auto &iargs = instr.str_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STR", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kStrbRegister: {
      const auto &iargs = instr.strb_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STRB", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kStrhRegister: {
      const auto &iargs = instr.strh_register;
      MnemonicBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STRH", mctx, bflags, iargs.flags, RArg(iargs.t), RArg(iargs.n), RArg(iargs.m),
          iargs.shift_res);
      break;
    }
    case InstrId::kCbNZ: {
      const auto &iargs = instr.cb_n_z;
      MnemonicBuilderSpecialInstr<TMnemonicBuilderContext>::BuildCbNZ(
          "CBNZ", mctx, bflags, iargs.flags, RArg(iargs.n), iargs.imm32);
      break;
    }
    case InstrId::kSvc: {
      const auto &iargs = instr.svc;
      MnemonicBuilderUnaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "SVC", mctx, bflags, iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kBkpt: {
      const auto &iargs = instr.bkpt;
      MnemonicBuilderUnaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "BKPT", mctx, bflags, iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kNop: {
      const auto &iargs = instr.nop;
      MnemonicBuilderNullaryInstr<TMnemonicBuilderContext>::Build("NOP", mctx, bflags, iargs.flags);
      break;
    }
    case InstrId::kDmb: {
      const auto &iargs = instr.dmb;
      MnemonicBuilderNullaryInstr<TMnemonicBuilderContext>::Build("DMB", mctx, bflags, iargs.flags);
      break;
    }
    default:
      ConstStringBuilder(buf, sizeof(buf)).AddString("invalid op").Terminate();
      break;
    };
  }

private:
  /**
   * @brief Constructs a MnemonicBuilder object
   */
  MnemonicBuilder() = delete;

  /**
   * @brief Destructor
   */
  ~MnemonicBuilder() = delete;

  /**
   * @brief Copy constructor for MnemonicBuilder.
   * @param r_src the object to be copied
   */
  MnemonicBuilder(MnemonicBuilder &r_src) = delete;

  /**
   * @brief Copy assignment operator for MnemonicBuilder.
   * @param r_src the object to be copied
   */
  MnemonicBuilder &operator=(const MnemonicBuilder &r_src) = delete;

  /**
   * @brief Move constructor for MnemonicBuilder.
   * @param r_src the object to be moved
   */
  MnemonicBuilder(MnemonicBuilder &&r_src) = delete;

  /**
   * @brief Move assignment operator for  MnemonicBuilder.
   * @param r_src the object to be moved
   */
  MnemonicBuilder &operator=(MnemonicBuilder &&r_src) = delete;
};

} // namespace internal
} // namespace libmicroemu