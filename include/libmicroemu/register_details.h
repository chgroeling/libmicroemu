#pragma once

#include "libmicroemu/types.h"

namespace libmicroemu {

template <unsigned last_bit, unsigned first_bit> static constexpr u32 BitMask() {
  if constexpr (last_bit == sizeof(u32) * 8U - 1U) {
    return ~0U << first_bit;
  } else {
    return ((1U << (last_bit - first_bit + 1U)) - 1U) << first_bit;
  }
}

struct RegisterOffsets {
  static constexpr u8 kBusFaultOffset = 8U;
  static constexpr u8 kUsageFaultOffset = 16U;
};

struct CfsrUsageFault {
  // Bit positions for the UsageFault in the CFSR register
  static constexpr u8 kBit15Pos = 31U;
  static constexpr u8 kBit0Pos = 16U;
  static constexpr u32 kMsk = BitMask<kBit15Pos, kBit0Pos>();

  // UFSR flag for DIVBYZERO (bit 9 + offset)
  static constexpr u8 kDivByZeroBitPos = 9U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kDivByZeroMsk = 1U << kDivByZeroBitPos;

  // UFSR flag for UNALIGNED (bit 8 + offset)
  static constexpr u8 kUnalignedBitPos = 8U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kUnalignedMsk = 1U << kUnalignedBitPos;

  // UFSR flag for NOCP (bit 3 + offset)
  static constexpr u8 kNoCpBitPos = 3U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kNoCpMsk = 1U << kNoCpBitPos;

  // UFSR flag for INVPC (bit 2 + offset)
  static constexpr u8 kInvPcBitPos = 2U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kInvPcMsk = 1U << kInvPcBitPos;

  // UFSR flag for INVSTATE (bit 1 + offset)
  static constexpr u8 kInvStateBitPos = 1U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kInvStateMsk = 1U << kInvStateBitPos;

  // UFSR flag for UNDEFINSTR (bit 0 + offset)
  static constexpr u8 kUndefInstrBitPos = 0U + RegisterOffsets::kUsageFaultOffset;
  static constexpr u32 kUndefInstrMsk = 1U << kUndefInstrBitPos;
};

struct CfsrBusFault {
  // Bit positions for the BusFault in the CFSR register
  static constexpr u8 kBit7Pos = 15U;
  static constexpr u8 kBit0Pos = 8U;
  static constexpr u32 kMsk = BitMask<kBit7Pos, kBit0Pos>();

  // BFSR flag for BFARVALID (bit 7 + offset)
  static constexpr u8 kBfarValidBitPos = 7U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kBfarValidMsk = 1U << kBfarValidBitPos;

  // BFSR flag for LSPERR (bit 5 + offset)
  static constexpr u8 kLsperrBitPos = 5U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kLsperrMsk = 1U << kLsperrBitPos;

  // BFSR flag for STKERR (bit 4 + offset)
  static constexpr u8 kStkerrBitPos = 4U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kStkerrMsk = 1U << kStkerrBitPos;

  // BFSR flag for UNSTKERR (bit 3 + offset)
  static constexpr u8 kUnstkerrBitPos = 3U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kUnstkerrMsk = 1U << kUnstkerrBitPos;

  // BFSR flag for IMPRECISERR (bit 2 + offset)
  static constexpr u8 kImpreciseErrBitPos = 2U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kImpreciseErrMsk = 1U << kImpreciseErrBitPos;

  // BFSR flag for PRECISERR (bit 1 + offset)
  static constexpr u8 kPreciseErrBitPos = 1U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kPreciseErrMsk = 1U << kPreciseErrBitPos;

  // BFSR flag for IBUSERR (bit 0 + offset)
  static constexpr u8 kIbuErrBitPos = 0U + RegisterOffsets::kBusFaultOffset;
  static constexpr u32 kIbuErrMsk = 1U << kIbuErrBitPos;
};

struct CfsrMemManage {
  // Bit positions for the MemManage in the CFSR register
  static constexpr u8 kBit7Pos = 7U;
  static constexpr u8 kBit0Pos = 0U;
  static constexpr u32 kMsk = BitMask<kBit7Pos, kBit0Pos>();

  // MMFSR flag for MMARVALID (bit 7)
  static constexpr u8 kMmarValidBitPos = 7U;
  static constexpr u32 kMmarValidMsk = 1U << kMmarValidBitPos;

  // MMFSR flag for LSPERR (bit 5)
  static constexpr u8 kLsperrBitPos = 5U;
  static constexpr u32 kLsperrMsk = 1U << kLsperrBitPos;

  // MMFSR flag for STKERR (bit 4)
  static constexpr u8 kStkerrBitPos = 4U;
  static constexpr u32 kStkerrMsk = 1U << kStkerrBitPos;

  // MMFSR flag for UNSTKERR (bit 3)
  static constexpr u8 kUnstkerrBitPos = 3U;
  static constexpr u32 kUnstkerrMsk = 1U << kUnstkerrBitPos;

  // MMFSR flag for DACCVIOL (bit 1)
  static constexpr u8 kDaccViolBitPos = 1U;
  static constexpr u32 kDaccViolMsk = 1U << kDaccViolBitPos;

  // MMFSR flag for IACCVIOL (bit 0)
  static constexpr u8 kIaccViolBitPos = 0U;
  static constexpr u32 kIaccViolMsk = 1U << kIaccViolBitPos;
};

struct SysTickRegister {
  // SysTick Control and Status Register (CSR) flags
  static constexpr u8 kCsrEnablePos = 0U;
  static constexpr u32 kCsrEnableMsk = 1U << kCsrEnablePos;

  static constexpr u8 kCsrTickIntPos = 1U;
  static constexpr u32 kCsrTickIntMsk = 1U << kCsrTickIntPos;

  static constexpr u8 kCsrClockSourcePos = 2U;
  static constexpr u32 kCsrClockSourceMsk = 1U << kCsrClockSourcePos;

  static constexpr u8 kCsrCountFlagPos = 16U;
  static constexpr u32 kCsrCountFlagMsk = 1U << kCsrCountFlagPos;

  // SysTick Reload Value Register (RVR)
  static constexpr u8 kRvrReloadPos = 0U;
  static constexpr u32 kRvrReloadMsk = 0x00FFFFFFU << kRvrReloadPos; // 24-bit reload value

  // SysTick Current Value Register (CVR)
  static constexpr u8 kCvrCurrentPos = 0U;
  static constexpr u32 kCvrCurrentMsk = 0x00FFFFFFU << kCvrCurrentPos; // 24-bit current value

  // SysTick Calibration Value Register (CALIB)
  static constexpr u8 kCalibNoRefPos = 31U;
  static constexpr u32 kCalibNoRefMsk = 1U << kCalibNoRefPos;

  static constexpr u8 kCalibSkewPos = 30U;
  static constexpr u32 kCalibSkewMsk = 1U << kCalibSkewPos;

  static constexpr u8 kCalibTenMsPos = 0U;
  static constexpr u32 kCalibTenMsMsk = 0x00FFFFFFU << kCalibTenMsPos; // 24-bit tenms value
};

struct CcrRegister {
  // CCR - Configuration and Control Register flags
  static constexpr u8 kStkAlignPos = 9U;                  // Stack alignment bit
  static constexpr u32 kStkAlignMsk = 1U << kStkAlignPos; // Stack alignment bit mask

  static constexpr u8 kNonBaseThreadEnablePos = 0U; // Non-base thread enable bit
  static constexpr u32 kNonBaseThreadEnableMsk =
      1U << kNonBaseThreadEnablePos; // Non-base thread enable mask

  static constexpr u8 kUserSettablePendSvPos = 1U; // User-settable PendSV bit
  static constexpr u32 kUserSettablePendSvMsk =
      1U << kUserSettablePendSvPos; // User-settable PendSV mask

  static constexpr u8 kUnalignTrapEnablePos = 3U; // Unaligned access trap enable bit
  static constexpr u32 kUnalignTrapEnableMsk =
      1U << kUnalignTrapEnablePos; // Unaligned access trap enable mask

  static constexpr u8 kDivByZeroTrapEnablePos = 4U; // Divide by zero trap enable bit
  static constexpr u32 kDivByZeroTrapEnableMsk =
      1U << kDivByZeroTrapEnablePos; // Divide by zero trap enable mask

  static constexpr u8 kMainStackAlignPos = 9U;                        // Main stack alignment bit
  static constexpr u32 kMainStackAlignMsk = 1U << kMainStackAlignPos; // Main stack alignment mask

  static constexpr u8 kIgnoreLsbOnDataPos = 10U; // Ignore LSB in data address comparisons bit
  static constexpr u32 kIgnoreLsbOnDataMsk =
      1U << kIgnoreLsbOnDataPos; // Ignore LSB in data address mask

  static constexpr u8 kBranchPredictionEnablePos = 18U; // Branch prediction enable bit
  static constexpr u32 kBranchPredictionEnableMsk =
      1U << kBranchPredictionEnablePos; // Branch prediction enable mask

  static constexpr u8 kL1CacheEnablePos = 19U;                      // L1 cache enable bit
  static constexpr u32 kL1CacheEnableMsk = 1U << kL1CacheEnablePos; // L1 cache enable mask
};

struct ApsrRegister {
  // APSR - Application Program Status Register flags
  static constexpr u8 kNPos = 31U;   // Negative condition flag
  static constexpr u8 kZPos = 30U;   // Zero condition flag
  static constexpr u8 kCPos = 29U;   // Carry condition flag
  static constexpr u8 kVPos = 28U;   // Overflow condition flag
  static constexpr u8 kQPos = 27U;   // Saturation condition flag
  static constexpr u8 kGeBit0 = 16U; // Bit 0 of greater than or Equal flags
  static constexpr u8 kGeBit3 = 19U; // Bit 3 of greater than or Equal flags

  // APSR masks
  static constexpr u32 kNMsk = 1U << kNPos; // Negative condition flag mask
  static constexpr u32 kZMsk = 1U << kZPos; // Zero condition flag mask
  static constexpr u32 kCMsk = 1U << kCPos; // Carry condition flag mask
  static constexpr u32 kVMsk = 1U << kVPos; // Overflow condition flag mask
  static constexpr u32 kQMsk = 1U << kQPos; // Saturation condition flag mask

  // Greater than or Equal (GE) flags mask (bits 19 to 16)
  static constexpr u32 kGeMsk = BitMask<kGeBit3, kGeBit0>();
};

struct EpsrRegister {
  // EPSR bit positions
  static constexpr u8 kItBit1Pos = 26U;
  static constexpr u8 kItBit0Pos = 25U;
  static constexpr u8 kTPos = 24U; // Thumb mode bit
  static constexpr u8 kItBit7Pos = 15U;
  static constexpr u8 kItBit2Pos = 10U;

  // EPSR masks
  static constexpr u32 kTMsk = 1U << kTPos;                               // Thumb mode bit mask
  static constexpr u32 kItBit1to0Msk = BitMask<kItBit1Pos, kItBit0Pos>(); // IT bits 1-0
  static constexpr u32 kItBit7to2Msk = BitMask<kItBit7Pos, kItBit2Pos>(); // IT bits 7-2
  static constexpr u32 kItMsk = kItBit1to0Msk | kItBit7to2Msk;            // Full IT bits mask
};

struct IstateRegister {
  // Istate bit positions
  static constexpr u8 kItBit7Pos = 7U;
  static constexpr u8 kItBit4Pos = 4U;
  static constexpr u8 kItBit3Pos = 3U;
  static constexpr u8 kItBit2Pos = 2U;
  static constexpr u8 kItBit1Pos = 1U;
  static constexpr u8 kItBit0Pos = 0U;

  // Istate masks for specific bit ranges
  static constexpr u32 kItBit1to0Msk = BitMask<kItBit1Pos, kItBit0Pos>();
  static constexpr u32 kItBit2to0Msk = BitMask<kItBit2Pos, kItBit0Pos>();
  static constexpr u32 kItBit3to0Msk = BitMask<kItBit3Pos, kItBit0Pos>();
  static constexpr u32 kItBit3to1Msk = BitMask<kItBit3Pos, kItBit1Pos>();
  static constexpr u32 kItBit4to0Msk = BitMask<kItBit4Pos, kItBit0Pos>();
  static constexpr u32 kItBit7to2Msk = BitMask<kItBit7Pos, kItBit2Pos>();
  static constexpr u32 kItBit7to4Msk = BitMask<kItBit7Pos, kItBit4Pos>();

  // Full Istate mask (bits 7 to 0)
  static constexpr u32 kItMsk = BitMask<kItBit7Pos, kItBit0Pos>();
};

struct IpsrRegister {
  // IPSR bit positions
  static constexpr u8 kExceptionNumberBit0Pos = 0U;
  static constexpr u8 kExceptionNumberBit8Pos = 8U;

  // IPSR masks
  static constexpr u32 kExceptionNumberMsk =
      BitMask<kExceptionNumberBit8Pos, kExceptionNumberBit0Pos>(); // Exception number mask
};

struct SysCtrlRegister {
  // SYSCTRL bit positions
  static constexpr u8 kTPos = 0U;            // Thumb mode bit
  static constexpr u8 kExecModePos = 1U;     // Execution mode bit
  static constexpr u8 kControlNPrivPos = 2U; // Control register nPRIV bit
  static constexpr u8 kControlSpSelPos = 3U; // Control register SPSEL bit
  static constexpr u8 kControlFpcaPos = 4U;  // Processor includes FP extension bit

  // SYSCTRL masks
  static constexpr u32 kTMsk = 1U << kTPos;                   // Thumb mode bit mask
  static constexpr u32 kExecModeMsk = 1U << kExecModePos;     // Execution mode bit mask
  static constexpr u32 kExecModeThread = 0U << kExecModePos;  // Execution mode thread
  static constexpr u32 kExecModeHandler = 1U << kExecModePos; // Execution mode handler

  static constexpr u32 kControlSpSelMsk = 1U << kControlSpSelPos; // Control register SPSEL bit mask
  static constexpr u32 kControlNPrivMsk = 1U << kControlNPrivPos; // Control register nPRIV bit mask
  static constexpr u32 kControlFpcaMsk = 1U << kControlFpcaPos; // uC includes FP extension bit mask
};

struct ControlRegister {
  // Control bit positions
  static constexpr u8 kNPrivPos = 0U; // Privilige mode bit
  static constexpr u8 kSpselPos = 1U; // Spsel bit
  static constexpr u8 kFpcaPos = 2U;  // Processor includes FP extension bit

  // Control masks
  static constexpr u32 kNPrivMsk = 1U << kNPrivPos;
  static constexpr u32 kSpselMsk = 1U << kSpselPos;
  static constexpr u32 kFpcaMsk = 1U << kFpcaPos;

  // Control masks for specific bit ranges
  static constexpr u32 kControlBit1toBit0Msk = BitMask<kSpselPos, kNPrivPos>();
  static constexpr u32 kControlBit2toBit0Msk = BitMask<kFpcaPos, kNPrivPos>();
};

} // namespace libmicroemu
