#pragma once

#include "libmicroemu/internal/trace/instr/str_builder_binary_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_instr_with_rotation.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_loadstore_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_null_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_binary_null_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/str_builder_nullary_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_special_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_ternary_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_ternary_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/str_builder_ternary_loadstore_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_ternary_loadstore_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/str_builder_ternary_null_instr_with_shift.h"
#include "libmicroemu/internal/trace/instr/str_builder_unary_branch_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_unary_branch_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_unary_instr.h"
#include "libmicroemu/internal/trace/instr/str_builder_unary_instr_with_imm_carry.h"
#include "libmicroemu/internal/trace/instr/str_builder_unary_null_instr_with_imm.h"
#include "libmicroemu/internal/trace/instr/str_builder_variadic_loadstore_instr.h"
#include "libmicroemu/internal/trace/mnemonic_builder_context.h"
#include "libmicroemu/internal/trace/mnemonic_builder_flags.h"
#include "libmicroemu/types.h"

namespace microemu {
namespace internal {

class MnemonicBuilder {
public:
  template <typename TProcessorStates, typename TItOps, typename TRegOps, typename TSpecRegOps>
  static void Build(const TProcessorStates &pstates, const Instr &instr, char *buf,
                    size_t buf_len) {
    auto cstr_builder = ConstStringBuilder(buf, buf_len);
    using TMnemonicBuilderContext =
        MnemonicBuilderContext<TProcessorStates, TItOps, TRegOps, TSpecRegOps>;
    auto mctx = TMnemonicBuilderContext{pstates, cstr_builder};
    MnemonicBuilderFlagsSet bflags =
        static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kNone);
    switch (instr.id) {
    case InstrId::kLdrLiteral: {
      const auto &iargs = instr.ldr_literal;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrLiteral(
          "LDR", mctx, bflags, iargs.flags, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrsbImmediate: {
      const auto &iargs = instr.ldrsb_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRSB", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrbImmediate: {
      const auto &iargs = instr.ldrb_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRB", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrshImmediate: {
      const auto &iargs = instr.ldrsh_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRSH", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrhImmediate: {
      const auto &iargs = instr.ldrh_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDRH", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrImmediate: {
      const auto &iargs = instr.ldr_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDR", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kLdrex: {
      const auto &iargs = instr.ldr_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "LDREX", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kMsr: {
      const auto &iargs = instr.msr;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildMsr("MSR", mctx, bflags, iargs.flags,
                                                                iargs.n, iargs.mask, iargs.SYSm);
      break;
    }
    case InstrId::kMrs: {
      const auto &iargs = instr.mrs;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildMrs("MRS", mctx, bflags, iargs.flags,
                                                                iargs.d, iargs.mask, iargs.SYSm);
      break;
    }
    case InstrId::kUmlal: {
      const auto &iargs = instr.umlal;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "UMLAL", mctx, bflags, iargs.flags, iargs.dLo, iargs.dHi, iargs.n, iargs.m);
      break;
    }
    case InstrId::kUmull: {
      const auto &iargs = instr.umull;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "UMULL", mctx, bflags, iargs.flags, iargs.dLo, iargs.dHi, iargs.n, iargs.m);
      break;
    }
    case InstrId::kSmull: {
      const auto &iargs = instr.umull;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUmull(
          "SMULL", mctx, bflags, iargs.flags, iargs.dLo, iargs.dHi, iargs.n, iargs.m);
      break;
    }
    case InstrId::kLdrdImmediate: {
      const auto &iargs = instr.ldrd_immediate;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrdStrd(
          "LDRD", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.t2, iargs.imm32);
      break;
    }
    case InstrId::kCmpImmediate: {
      const auto &iargs = instr.cmp_immediate;
      StrBuilderBinaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "CMP", mctx, bflags, iargs.flags, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kCmnImmediate: {
      const auto &iargs = instr.cmn_immediate;
      StrBuilderBinaryNullInstrWithImm<TMnemonicBuilderContext>::Build(
          "CMN", mctx, bflags, iargs.flags, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kIt: {
      const auto &iargs = instr.it;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildIt("IT", mctx, bflags, iargs.flags,
                                                               iargs.firstcond, iargs.mask);
      break;
    }
    case InstrId::kMovImmediate: {
      const auto &iargs = instr.mov_immediate;
      StrBuilderUnaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "MOV", mctx, bflags, iargs.flags, iargs.d, iargs.imm32_carry);
      break;
    }
    case InstrId::kMvnImmediate: {
      const auto &iargs = instr.mvn_immediate;
      StrBuilderUnaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "MVN", mctx, bflags, iargs.flags, iargs.d, iargs.imm32_carry);
      break;
    }
    case InstrId::kMovRegister: {
      const auto &iargs = instr.mov_register;
      StrBuilderUnaryInstr<TMnemonicBuilderContext>::Build("MOV", mctx, bflags, iargs.flags,
                                                           iargs.d, iargs.m);
      break;
    }
    case InstrId::kRrx: {
      const auto &iargs = instr.rrx;
      StrBuilderUnaryInstr<TMnemonicBuilderContext>::Build("RRX", mctx, bflags, iargs.flags,
                                                           iargs.d, iargs.m);
      break;
    }
    case InstrId::kBl: {
      const auto &iargs = instr.bl;
      StrBuilderUnaryBranchInstrWithImm<TMnemonicBuilderContext>::Build("BL", mctx, bflags,
                                                                        iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kBx: {
      const auto &iargs = instr.bx;
      StrBuilderUnaryBranchInstr<TMnemonicBuilderContext>::Build("BX", mctx, bflags, iargs.flags,
                                                                 iargs.m);
      break;
    }
    case InstrId::kBlx: {
      const auto &iargs = instr.blx;
      StrBuilderUnaryBranchInstr<TMnemonicBuilderContext>::Build("BLX", mctx, bflags, iargs.flags,
                                                                 iargs.m);
      break;
    }
    case InstrId::kBCond: {
      const auto &iargs = instr.b_cond;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildBCond("B", mctx, bflags, iargs.flags,
                                                                  iargs.cond, iargs.imm32);
      break;
    }
    case InstrId::kB: {
      const auto &iargs = instr.b;
      StrBuilderUnaryBranchInstrWithImm<TMnemonicBuilderContext>::Build("B", mctx, bflags,
                                                                        iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kSubImmediate: {
      const auto &iargs = instr.sub_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build("SUB", mctx, bflags, iargs.flags,
                                                                   iargs.d, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kSbcImmediate: {
      const auto &iargs = instr.sbc_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build("SBC", mctx, bflags, iargs.flags,
                                                                   iargs.d, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kRsbImmediate: {
      const auto &iargs = instr.rsb_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build("RSB", mctx, bflags, iargs.flags,
                                                                   iargs.d, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kSubSpMinusImmediate: {
      const auto &iargs = instr.sub_sp_minus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "SUB", mctx, bflags, iargs.flags, iargs.d, static_cast<u32>(RegisterId::kSp),
          iargs.imm32);
      break;
    }
    case InstrId::kAddSpPlusImmediate: {
      const auto &iargs = instr.add_sp_plus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, iargs.d, static_cast<u32>(RegisterId::kSp),
          iargs.imm32);
      break;
    }
    case InstrId::kAddImmediate: {
      const auto &iargs = instr.add_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build("ADD", mctx, bflags, iargs.flags,
                                                                   iargs.d, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kAdcImmediate: {
      const auto &iargs = instr.adc_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build("ADC", mctx, bflags, iargs.flags,
                                                                   iargs.d, iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kAddPcPlusImmediate: {
      const auto &iargs = instr.add_pc_plus_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImm<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, iargs.d, static_cast<u32>(RegisterId::kPc),
          iargs.imm32);
      break;
    }
    case InstrId::kClz: {
      const auto &iargs = instr.clz;
      StrBuilderUnaryInstr<TMnemonicBuilderContext>::Build("CLZ", mctx, bflags, iargs.flags,
                                                           iargs.d, iargs.m);
      break;
    }
    case InstrId::kSubRegister: {
      const auto &iargs = instr.sub_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "SUB", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kRsbRegister: {
      const auto &iargs = instr.rsb_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "RSB", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kUdiv: {
      const auto &iargs = instr.udiv;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("UDIV", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kSdiv: {
      const auto &iargs = instr.sdiv;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("SDIV", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kMls: {
      const auto &iargs = instr.mls;
      StrBuilderTernaryInstr<TMnemonicBuilderContext>::Build("MLS", mctx, bflags, iargs.flags,
                                                             iargs.d, iargs.n, iargs.m, iargs.a);
      break;
    }
    case InstrId::kMla: {
      const auto &iargs = instr.mla;
      StrBuilderTernaryInstr<TMnemonicBuilderContext>::Build("MLA", mctx, bflags, iargs.flags,
                                                             iargs.d, iargs.n, iargs.m, iargs.a);
      break;
    }
    case InstrId::kMul: {
      const auto &iargs = instr.mul;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("MUL", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kAddRegister: {
      const auto &iargs = instr.add_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ADD", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kAdcRegister: {
      const auto &iargs = instr.adc_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ADC", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kMvnRegister: {
      const auto &iargs = instr.mvn_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "MVN", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kLsrImmediate: {
      const auto &iargs = instr.lsr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      StrBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "LSR", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kAsrImmediate: {
      const auto &iargs = instr.asr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      StrBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ASR", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kLslImmediate: {
      const auto &iargs = instr.lsl_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kSupressShiftType);
      StrBuilderBinaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "LSL", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kLsrRegister: {
      const auto &iargs = instr.lsr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("LSR", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kAsrRegister: {
      const auto &iargs = instr.asr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("ASR", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kLslRegister: {
      const auto &iargs = instr.lsl_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstr<TMnemonicBuilderContext>::Build("LSL", mctx, bflags, iargs.flags,
                                                            iargs.d, iargs.n, iargs.m);
      break;
    }
    case InstrId::kStm: {
      const auto &iargs = instr.stm;
      StrBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "STM", mctx, bflags, iargs.flags, iargs.n, iargs.registers, false);
      break;
    }
    case InstrId::kPush: {
      const auto &iargs = instr.push;
      StrBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "PUSH", mctx, bflags, iargs.flags, static_cast<u32>(RegisterId::kSp), iargs.registers,
          true);
      break;
    }
    case InstrId::kLdm: {
      const auto &iargs = instr.ldm;
      StrBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "LDM", mctx, bflags, iargs.flags, iargs.n, iargs.registers, false);
      break;
    }
    case InstrId::kPop: {
      const auto &iargs = instr.pop;
      StrBuilderVariadicLoadStoreInstr<TMnemonicBuilderContext>::Build(
          "POP", mctx, bflags, iargs.flags, static_cast<u32>(RegisterId::kSp), iargs.registers,
          true);
      break;
    }
    case InstrId::kSxtb: {
      const auto &iargs = instr.sxtb;
      StrBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "SXTB", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.rotation);
      break;
    }
    case InstrId::kUxtb: {
      const auto &iargs = instr.uxtb;
      StrBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "UXTB", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.rotation);
      break;
    }
    case InstrId::kSxth: {
      const auto &iargs = instr.sxth;
      StrBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "SXTH", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.rotation);
      break;
    }
    case InstrId::kUxth: {
      const auto &iargs = instr.uxth;
      StrBuilderBinaryInstrWithRotation<TMnemonicBuilderContext>::Build(
          "UXTH", mctx, bflags, iargs.flags, iargs.d, iargs.m, iargs.rotation);
      break;
    }
    case InstrId::kUbfx: {
      const auto &iargs = instr.ubfx;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildUbfx(
          "UBFX", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.lsbit, iargs.widthminus1);
      break;
    }
    case InstrId::kBfi: {
      const auto &iargs = instr.bfi;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildBfi(
          "BFI", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.lsbit, iargs.msbit);
      break;
    }
    case InstrId::kSbcRegister: {
      const auto &iargs = instr.sbc_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "SBC", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kEorRegister: {
      const auto &iargs = instr.eor_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "EOR", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kOrrRegister: {
      const auto &iargs = instr.orr_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "ORR", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kAndRegister: {
      const auto &iargs = instr.and_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "AND", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kBicRegister: {
      const auto &iargs = instr.bic_register;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderTernaryInstrWithShift<TMnemonicBuilderContext>::Build(
          "BIC", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.m, iargs.shift_res);
      break;
    }
    case InstrId::kBicImmediate: {
      const auto &iargs = instr.bic_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "BIC", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kStrdImmediate: {
      const auto &iargs = instr.strd_immediate;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildLdrdStrd(
          "STRD", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.t2, iargs.imm32);
      break;
    }
    case InstrId::kCmpRegister: {
      const auto &iargs = instr.cmp_register;
      StrBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "CMP", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.shift_res, false);
      break;
    }
    case InstrId::kTeqImmediate: {
      const auto &iargs = instr.teq_immediate;
      StrBuilderBinaryNullInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "TEQ", mctx, bflags, iargs.flags, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kTstImmediate: {
      const auto &iargs = instr.tst_immediate;
      StrBuilderBinaryNullInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "TST", mctx, bflags, iargs.flags, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kTstRegister: {
      const auto &iargs = instr.tst_register;
      StrBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "TST", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.shift_res, false);
      break;
    }
    case InstrId::kTeqRegister: {
      const auto &iargs = instr.teq_register;
      StrBuilderTernaryNullInstrWithShift<TMnemonicBuilderContext>::Build(
          "TEQ", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.shift_res, false);
      break;
    }
    case InstrId::kEorImmediate: {
      const auto &iargs = instr.eor_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "EOR", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kOrrImmediate: {
      const auto &iargs = instr.orr_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "ORR", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kAndImmediate: {
      const auto &iargs = instr.and_immediate;
      bflags |= static_cast<MnemonicBuilderFlagsSet>(MnemonicBuilderFlags::kReduceRd);
      StrBuilderBinaryInstrWithImmCarry<TMnemonicBuilderContext>::Build(
          "AND", mctx, bflags, iargs.flags, iargs.d, iargs.n, iargs.imm32_carry);
      break;
    }
    case InstrId::kStrhImmediate: {
      const auto &iargs = instr.strh_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STRH", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kTbbH: {
      const auto &iargs = instr.tbb_h;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildTbbH("TBH", mctx, bflags, iargs.flags,
                                                                 iargs.n, iargs.m);
      break;
    }
    case InstrId::kStrbImmediate: {
      const auto &iargs = instr.strb_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STRB", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kStrImmediate: {
      const auto &iargs = instr.str_immediate;
      StrBuilderBinaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STR", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.imm32);
      break;
    }
    case InstrId::kStrex: {
      const auto &iargs = instr.strex;
      StrBuilderTernaryLoadStoreInstrWithImm<TMnemonicBuilderContext>::Build(
          "STREX", mctx, bflags, iargs.flags, iargs.n, iargs.t, iargs.d, iargs.imm32);
      break;
    }
    case InstrId::kLdrhRegister: {
      const auto &iargs = instr.ldrh_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDRH", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kLdrbRegister: {
      const auto &iargs = instr.ldrb_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDRB", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kLdrRegister: {
      const auto &iargs = instr.ldr_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "LDR", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kStrRegister: {
      const auto &iargs = instr.str_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STR", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kStrbRegister: {
      const auto &iargs = instr.strb_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STRB", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kStrhRegister: {
      const auto &iargs = instr.strh_register;
      StrBuilderTernaryLoadStoreInstrWithShift<TMnemonicBuilderContext>::Build(
          "STRH", mctx, bflags, iargs.flags, iargs.n, iargs.m, iargs.t, iargs.shift_res);
      break;
    }
    case InstrId::kCbNZ: {
      const auto &iargs = instr.cb_n_z;
      StrBuilderSpecialInstr<TMnemonicBuilderContext>::BuildCbNZ("CBNZ", mctx, bflags, iargs.flags,
                                                                 iargs.n, iargs.imm32);
      break;
    }
    case InstrId::kSvc: {
      const auto &iargs = instr.svc;
      StrBuilderUnaryNullInstrWithImm<TMnemonicBuilderContext>::Build("SVC", mctx, bflags,
                                                                      iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kBkpt: {
      const auto &iargs = instr.bkpt;
      StrBuilderUnaryNullInstrWithImm<TMnemonicBuilderContext>::Build("BKPT", mctx, bflags,
                                                                      iargs.flags, iargs.imm32);
      break;
    }
    case InstrId::kNop: {
      const auto &iargs = instr.nop;
      StrBuilderNullaryInstr<TMnemonicBuilderContext>::Build("NOP", mctx, bflags, iargs.flags);
      break;
    }
    case InstrId::kDmb: {
      const auto &iargs = instr.dmb;
      StrBuilderNullaryInstr<TMnemonicBuilderContext>::Build("DMB", mctx, bflags, iargs.flags);
      break;
    }
    default:
      ConstStringBuilder(buf, sizeof(buf)).AddString("invalid op").Terminate();
      break;
    };
  }

private:
  /// \brief Constructs a MnemonicBuilder object
  MnemonicBuilder() = delete;

  /// \brief Destructor
  ~MnemonicBuilder() = delete;

  /// \brief Copy constructor for MnemonicBuilder.
  /// \param r_src the object to be copied
  MnemonicBuilder(MnemonicBuilder &r_src) = delete;

  /// \brief Copy assignment operator for MnemonicBuilder.
  /// \param r_src the object to be copied
  MnemonicBuilder &operator=(const MnemonicBuilder &r_src) = delete;

  /// \brief Move constructor for MnemonicBuilder.
  /// \param r_src the object to be copied
  MnemonicBuilder(MnemonicBuilder &&r_src) = delete;

  /// \brief Move assignment operator for MnemonicBuilder.
  /// \param r_src the object to be copied
  MnemonicBuilder &operator=(MnemonicBuilder &&r_src) = delete;
};

} // namespace internal
} // namespace microemu