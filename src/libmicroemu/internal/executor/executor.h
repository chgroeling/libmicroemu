#pragma once

#include "libmicroemu/internal/decoder/decoder.h"
#include "libmicroemu/internal/executor/exec_results.h"
#include "libmicroemu/internal/executor/instr/binary_instr.h"
#include "libmicroemu/internal/executor/instr/binary_instr_with_imm.h"
#include "libmicroemu/internal/executor/instr/binary_instr_with_imm_carry.h"
#include "libmicroemu/internal/executor/instr/binary_instr_with_rotation.h"
#include "libmicroemu/internal/executor/instr/binary_instr_with_shift.h"
#include "libmicroemu/internal/executor/instr/binary_load_instr_with_imm.h"
#include "libmicroemu/internal/executor/instr/binary_null_instr_with_imm.h"
#include "libmicroemu/internal/executor/instr/binary_null_instr_with_imm_carry.h"
#include "libmicroemu/internal/executor/instr/binary_store_instr_with_imm.h"
#include "libmicroemu/internal/executor/instr/load_store/load_mem.h"
#include "libmicroemu/internal/executor/instr/load_store/store_mem.h"
#include "libmicroemu/internal/executor/instr/nullary_instr.h"
#include "libmicroemu/internal/executor/instr/special_instr.h"
#include "libmicroemu/internal/executor/instr/ternary_instr.h"
#include "libmicroemu/internal/executor/instr/ternary_instr_with_shift.h"
#include "libmicroemu/internal/executor/instr/ternary_load_instr_with_shift.h"
#include "libmicroemu/internal/executor/instr/ternary_null_instr_with_shift.h"
#include "libmicroemu/internal/executor/instr/ternary_store_instr_with_imm.h"
#include "libmicroemu/internal/executor/instr/ternary_store_instr_with_shift.h"
#include "libmicroemu/internal/executor/instr/unary_branch_instr.h"
#include "libmicroemu/internal/executor/instr/unary_branch_instr_imm.h"
#include "libmicroemu/internal/executor/instr/unary_instr.h"
#include "libmicroemu/internal/executor/instr/unary_instr_imm.h"
#include "libmicroemu/internal/executor/instr/unary_instr_imm_carry.h"
#include "libmicroemu/internal/executor/instr/unary_load_instr_imm.h"
#include "libmicroemu/internal/executor/instr/variadic_load_instr.h"
#include "libmicroemu/internal/executor/instr/variadic_store_instr.h"
#include "libmicroemu/internal/executor/instr/variadic_store_instr_db.h"
#include "libmicroemu/internal/logic/alu.h"
#include "libmicroemu/internal/logic/reg_ops.h"
#include "libmicroemu/internal/semihosting/semihosting.h"
#include "libmicroemu/internal/trace/memory_viewer.h"
#include "libmicroemu/internal/utils/bit_manip.h"
#include "libmicroemu/internal/utils/rarg.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>

namespace microemu {
namespace internal {

// TODO: Use Template ProcessorOPs
template <typename TProcessorStates, typename TBus, typename TRegOps, typename TSpecRegOps,
          typename TPcOps, typename TItOps, typename TExceptionTrigger,
          typename TLogger = NullLogger>
class Executor {
public:
  template <typename TDelegates>
  static Result<ExecResult> Execute(TProcessorStates &pstates, TBus &bus, const Instr &instr,
                                    TDelegates &delegates) {
    u8 flags = 0x0U;
    using TInstrCtx = InstrContext<TProcessorStates, TBus, TRegOps, TSpecRegOps, TPcOps, TItOps,
                                   TExceptionTrigger>;
    auto ictx = TInstrCtx{bus, pstates};

    switch (instr.id) {
    case InstrId::kLdrLiteral: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.246
      const auto &iargs = instr.ldr_immediate;
      using TOp = LoadMemU32<TInstrCtx>;
      using TInstr = UnaryLoadInstrImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kClz: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.220
      const auto &iargs = instr.clz;
      using TOp = Clz1Op<TInstrCtx>;
      using TInstr = UnaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrsbImmediate: {
      const auto &iargs = instr.ldrsb_immediate;
      using TOp = LoadMemI8<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrbImmediate: {
      const auto &iargs = instr.ldrb_immediate;
      using TOp = LoadMemU8<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrImmediate: {
      const auto &iargs = instr.ldr_immediate;
      using TOp = LoadMemU32<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }

    case InstrId::kLdrex: {
      const auto &iargs = instr.ldrex;
      using TOp = LoadMemExU32<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUmull: {
      const auto &iargs = instr.umull;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Umull(ictx, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi),
                                                RArg(iargs.n), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMsr: {
      const auto &iargs = instr.msr;
      TRY_ASSIGN(out_flags, ExecResult,
                 (SpecialInstr<TInstrCtx, TLogger>::Msr(ictx, iargs.flags, RArg(iargs.n),
                                                        iargs.mask, iargs.SYSm)));
      flags |= out_flags.flags;
      break;
    }

    case InstrId::kMrs: {
      const auto &iargs = instr.mrs;

      TRY_ASSIGN(out_flags, ExecResult,
                 (SpecialInstr<TInstrCtx, TLogger>::Mrs(ictx, iargs.flags, RArg(iargs.d),
                                                        iargs.mask, iargs.SYSm)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUmlal: {
      const auto &iargs = instr.umlal;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Umlal(ictx, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi),
                                                RArg(iargs.n), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSmull: {
      const auto &iargs = instr.smull;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Smull(ictx, iargs.flags, RArg(iargs.dLo), RArg(iargs.dHi),
                                                RArg(iargs.n), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrdImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.257
      const auto &iargs = instr.ldrd_immediate;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Ldrd(ictx, iargs.flags, RArg(iargs.t), RArg(iargs.t2),
                                               RArg(iargs.n), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrhImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.264
      const auto &iargs = instr.ldrh_immediate;
      using TOp = LoadMemU16<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrshImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.275
      const auto &iargs = instr.ldrsh_immediate;
      using TOp = LoadMemI16<TInstrCtx>;
      using TInstr = BinaryLoadInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32, RArg(iargs.t)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAsrImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.203
      const auto &iargs = instr.asr_immediate;
      using TOp = Asr1ShiftOp<TInstrCtx>;
      using TInstr = BinaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLslImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.282
      const auto &iargs = instr.lsl_immediate;
      using TOp = Lsl1ShiftOp<TInstrCtx>;
      using TInstr = BinaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLsrRegister: {
      const auto &iargs = instr.lsr_register;
      using TOp = Lsr2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAsrRegister: {
      const auto &iargs = instr.asr_register;
      using TOp = Asr2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLslRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.283
      const auto &iargs = instr.lsr_register;
      using TOp = Lsl2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kCmpRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.224
      const auto &iargs = instr.cmp_register;
      using TOp = Cmp2ShiftOp<TInstrCtx>;
      using TInstr = TernaryNullInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(stm_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.n), iargs.shift_res));
      flags |= stm_flags.flags;
      break;
    }
    case InstrId::kCmpImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.224
      const auto &iargs = instr.cmp_immediate;
      using TOp = Cmp1ImmOp<TInstrCtx>;
      using TInstr = BinaryNullInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(stm_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32));
      flags |= stm_flags.flags;
      break;
    }
    case InstrId::kCmnImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.221
      const auto &iargs = instr.cmp_immediate;
      using TOp = Cmn1ImmOp<TInstrCtx>;
      using TInstr = BinaryNullInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(stm_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32));
      flags |= stm_flags.flags;
      break;
    }
    case InstrId::kIt: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.236
      const auto &iargs = instr.it;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::ItInstr(ictx, iargs.flags, iargs.firstcond, iargs.mask));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMovImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.291
      const auto &iargs = instr.mov_immediate;
      using TOp = MovImmCarryOp<TInstrCtx>;
      using TInstr = UnaryInstrImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMvnImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.291
      const auto &iargs = instr.mvn_immediate;
      using TOp = MvnImmCarryOp<TInstrCtx>;
      using TInstr = UnaryInstrImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMovRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.293
      const auto &iargs = instr.mov_register;
      using TOp = Mov1Op<TInstrCtx>;
      using TInstr = UnaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }

    case InstrId::kRrx: {
      const auto &iargs = instr.rrx;
      using TOp = Rrx1Op<TInstrCtx>;
      using TInstr = UnaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUxtb: {
      const auto &iargs = instr.uxtb;
      using TOp = Uxtb1Rotation<TInstrCtx>;
      using TInstr = BinaryInstrWithRotation<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.rotation));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSxtb: {
      const auto &iargs = instr.sxtb;
      using TOp = Sxtb1Rotation<TInstrCtx>;
      using TInstr = BinaryInstrWithRotation<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.rotation));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSxth: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.414
      const auto &iargs = instr.sxth;
      using TOp = Sxth1Rotation<TInstrCtx>;
      using TInstr = BinaryInstrWithRotation<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.rotation));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUxth: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.454
      const auto &iargs = instr.uxth;
      using TOp = Uxth1Rotation<TInstrCtx>;
      using TInstr = BinaryInstrWithRotation<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.rotation));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBCond: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.205
      const auto &iargs = instr.b_cond;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::BCond(ictx, iargs.flags, iargs.imm32, iargs.cond));
      flags |= out_flags.flags;

      break;
    }
    case InstrId::kTbbH: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.416
      const auto &iargs = instr.tbb_h;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Tbhh(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.n)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kB: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.205
      const auto &iargs = instr.b;
      using TOp = B1ImmOp<TInstrCtx>;
      using TInstr = UnaryBranchInstrImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags, iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBl: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.213
      const auto &iargs = instr.b;
      using TOp = Bl1ImmOp<TInstrCtx>;
      using TInstr = UnaryBranchInstrImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags, iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kCbNZ: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.216
      const auto &iargs = instr.cb_n_z;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::CbNZ(ictx, iargs.flags, RArg(iargs.n), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBx: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.215
      const auto &iargs = instr.bx;
      using TOp = Bx1Op<TInstrCtx>;
      using TInstr = UnaryBranchInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags, RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBlx: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.214
      const auto &iargs = instr.blx;
      using TOp = Blx1Op<TInstrCtx>;
      using TInstr = UnaryBranchInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags, RArg(iargs.m)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSubImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.402
      const auto &iargs = instr.sub_immediate;
      using TOp = Sub1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSbcImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.346
      const auto &iargs = instr.sbc_immediate;
      using TOp = Sbc1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kRsbImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.341
      const auto &iargs = instr.rsb_immediate;
      using TOp = Rsb1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSubSpMinusImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.406
      const auto &iargs = instr.sub_sp_minus_immediate;
      using TOp = Sub1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArgConst<RegisterId::kSp>(), RArg(iargs.d),
                              iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAddImmediate: {
      const auto &iargs = instr.add_immediate;
      using TOp = Add1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAdcImmediate: {
      const auto &iargs = instr.adc_immediate;
      using TOp = Adc1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAddPcPlusImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.198
      const auto &iargs = instr.add_pc_plus_immediate;
      using TOp = AddToPcImmOp<TInstrCtx>;
      using TInstr = UnaryInstrImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.d), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAddSpPlusImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.190
      const auto &iargs = instr.add_sp_plus_immediate;
      using TOp = Add1ImmOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArgConst<RegisterId::kSp>(), RArg(iargs.d),
                              iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kTeqImmediate: {
      const auto &iargs = instr.teq_immediate;
      using TOp = Teq1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryNullInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kTstImmediate: {
      const auto &iargs = instr.tst_immediate;
      using TOp = Tst1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryNullInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kTstRegister: {
      const auto &iargs = instr.tst_register;
      using TOp = Tst2ShiftOp<TInstrCtx>;
      using TInstr = TernaryNullInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.n), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kTeqRegister: {
      const auto &iargs = instr.teq_register;
      using TOp = Teq2ShiftOp<TInstrCtx>;
      using TInstr = TernaryNullInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.n), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kEorImmediate: {
      const auto &iargs = instr.eor_immediate;
      using TOp = Eor1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kOrrImmediate: {
      const auto &iargs = instr.orr_immediate;
      using TOp = Orr1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAndImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.200
      const auto &iargs = instr.and_immediate;
      using TOp = And1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUbfx: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.424
      const auto &iargs = instr.ubfx;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Ubfx(ictx, iargs.flags, RArg(iargs.d), RArg(iargs.n),
                                               iargs.lsbit, iargs.widthminus1));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBfi: {
      const auto &iargs = instr.bfi;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Bfi(ictx, iargs.flags, RArg(iargs.d), RArg(iargs.n),
                                              iargs.lsbit, iargs.msbit));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kUdiv: {
      const auto &iargs = instr.udiv;
      using TOp = UDiv2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSdiv: {
      const auto &iargs = instr.sdiv;
      using TOp = SDiv2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMul: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.302
      const auto &iargs = instr.mul;
      using TOp = Mul2Op<TInstrCtx>;
      using TInstr = BinaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMls: {
      const auto &iargs = instr.mls;
      using TOp = Mls3Op<TInstrCtx>;
      using TInstr = TernaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              RArg(iargs.a)));
      flags |= out_flags.flags;
      break;
    }

    case InstrId::kMla: {
      const auto &iargs = instr.mla;
      using TOp = Mla3Op<TInstrCtx>;
      using TInstr = TernaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              RArg(iargs.a)));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSubRegister: {
      const auto &iargs = instr.sub_register;
      using TOp = Sub2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kRsbRegister: {
      const auto &iargs = instr.rsb_register;
      using TOp = Rsb2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAdcRegister: {
      const auto &iargs = instr.adc_register;
      using TOp = Adc2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAddRegister: {
      const auto &iargs = instr.add_register;
      using TOp = Add2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kPush: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.322
      const auto &iargs = instr.push;
      using TInstr = VariadicStoreInstrDb<TInstrCtx>;
      InstrFlagsSet iflags{0x0U};

      // rewrite flags to ldm instruction
      iflags |= static_cast<InstrFlagsSet>(InstrFlags::kWBack);
      iflags |= iargs.flags & InstrFlags::k32Bit; // copy 32bit flag

      TRY_ASSIGN(stm_flags, ExecResult,
                 TInstr::Call(ictx, iflags, RArgConst<RegisterId::kSp>(), iargs.registers));

      flags |= stm_flags.flags;
      break;
    }
    case InstrId::kStm: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.383
      const auto &iargs = instr.ldm;
      using TInstr = VariadicStoreInstr<TInstrCtx>;

      TRY_ASSIGN(stm_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.registers));
      flags |= stm_flags.flags;
      break;
    }
    case InstrId::kLdm: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.242
      const auto &iargs = instr.ldm;
      using TInstr = VariadicLoadInstr<TInstrCtx>;
      TRY_ASSIGN(ldm_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), iargs.registers));
      flags |= ldm_flags.flags;
      break;
    }
    case InstrId::kPop: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.319
      const auto &iargs = instr.pop;
      using TInstr = VariadicLoadInstr<TInstrCtx>;
      InstrFlagsSet iflags{0x0U};

      // rewrite flags to ldm instruction
      iflags |= static_cast<InstrFlagsSet>(InstrFlags::kWBack);
      iflags |= iargs.flags & InstrFlags::k32Bit; // copy 32bit flag

      TRY_ASSIGN(ldm_flags, ExecResult,
                 TInstr::Call(ictx, iflags, RArgConst<RegisterId::kSp>(), iargs.registers));

      flags |= ldm_flags.flags;
      break;
    }
    case InstrId::kOrrRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.309
      const auto &iargs = instr.orr_register;
      using TOp = Orr2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSbcRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.347
      const auto &iargs = instr.sbc_register;
      using TOp = Sbc2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kEorRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.233
      const auto &iargs = instr.eor_register;
      using TOp = Eor2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kAndRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.201
      const auto &iargs = instr.and_register;
      using TOp = And2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBicRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.309
      const auto &iargs = instr.bic_register;
      using TOp = Bic2ShiftOp<TInstrCtx>;
      using TInstr = TernaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.d),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBicImmediate: {
      const auto &iargs = instr.bic_immediate;
      using TOp = Bic1ImmCarryOp<TInstrCtx>;
      using TInstr = BinaryInstrWithImmCarry<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.d), iargs.imm32_carry));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kMvnRegister: {
      const auto &iargs = instr.lsr_immediate;
      using TOp = Mvn1ShiftOp<TInstrCtx>;
      using TInstr = BinaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLsrImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.309
      const auto &iargs = instr.lsr_immediate;
      using TOp = Lsr1ShiftOp<TInstrCtx>;
      using TInstr = BinaryInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.m), RArg(iargs.d), iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrdImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.393
      const auto &iargs = instr.strd_immediate;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Strd(ictx, iargs.flags, RArg(iargs.t), RArg(iargs.t2),
                                               RArg(iargs.n), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.388
      const auto &iargs = instr.str_register;
      using TOp = StoreMemU32<TInstrCtx>;
      using TInstr = TernaryStoreInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrbRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.391
      const auto &iargs = instr.strh_register;
      using TOp = StoreMemU8<TInstrCtx>;
      using TInstr = TernaryStoreInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrhRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.399
      const auto &iargs = instr.strh_register;
      using TOp = StoreMemU16<TInstrCtx>;
      using TInstr = TernaryStoreInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrhRegister: {
      const auto &iargs = instr.ldrh_register;
      using TOp = LoadMemU16<TInstrCtx>;
      using TInstr = TernaryLoadInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrbRegister: {
      const auto &iargs = instr.ldrb_register;
      using TOp = LoadMemU8<TInstrCtx>;
      using TInstr = TernaryLoadInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kLdrRegister: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.386
      const auto &iargs = instr.ldr_register;
      using TOp = LoadMemU32<TInstrCtx>;
      using TInstr = TernaryLoadInstrWithShift<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.m), RArg(iargs.t),
                              iargs.shift_res));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.386
      const auto &iargs = instr.str_immediate;
      using TOp = StoreMemU32<TInstrCtx>;
      using TInstr = BinaryStoreInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.t), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }

    case InstrId::kStrex: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.386
      const auto &iargs = instr.strex;
      using TOp = StoreMemExU32<TInstrCtx>;
      using TInstr = TernaryStoreInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.t), RArg(iargs.d),
                              iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrhImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.389
      const auto &iargs = instr.strh_immediate;
      using TOp = StoreMemU16<TInstrCtx>;
      using TInstr = BinaryStoreInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.t), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kStrbImmediate: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.389
      const auto &iargs = instr.strb_immediate;
      using TOp = StoreMemU8<TInstrCtx>;
      using TInstr = BinaryStoreInstrWithImm<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult,
                 TInstr::Call(ictx, iargs.flags, RArg(iargs.n), RArg(iargs.t), iargs.imm32));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kSvc: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.213
      const auto &iargs = instr.svc;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Svc(ictx, iargs.flags, iargs.imm32, delegates));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kBkpt: {
      // see Armv7-M Architecture Reference Manual Issue E.e p.212
      const auto &iargs = instr.bkpt;
      TRY_ASSIGN(out_flags, ExecResult,
                 SpecialInstr<TInstrCtx>::Bkpt(ictx, iargs.flags, iargs.imm32, delegates));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kNop: {
      const auto &iargs = instr.nop;
      using TOp = Nop0Op<TInstrCtx>;
      using TInstr = NullaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags));
      flags |= out_flags.flags;
      break;
    }
    case InstrId::kDmb: {
      const auto &iargs = instr.dmb;
      using TOp = Dmb0Op<TInstrCtx>;
      using TInstr = NullaryInstr<TOp, TInstrCtx>;
      TRY_ASSIGN(out_flags, ExecResult, TInstr::Call(ictx, iargs.flags));
      flags |= out_flags.flags;
      break;
    }
    default: {
      break;
    }
    }

    return Ok(ExecResult{flags});
  };

private:
  /// \brief Constructs a Executor object
  Executor() = delete;

  /// \brief Destructor
  ~Executor() = delete;

  /// \brief Copy assignment operator for Executor.
  /// \param r_src the object to be copied
  Executor &operator=(const Executor &r_src) = delete;

  /// \brief Move constructor for Executor.
  /// \param r_src the object to be copied
  Executor(Executor &&r_src) = delete;

  /// \brief Move assignment operator for Executor.
  /// \param r_src the object to be copied
  Executor &operator=(Executor &&r_src) = delete;
};

} // namespace internal
} // namespace microemu
