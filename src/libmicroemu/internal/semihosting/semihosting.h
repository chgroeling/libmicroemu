#pragma once

#include "libmicroemu/internal/i_breakpoint.h"
#include "libmicroemu/internal/utils/const_string_builder.h"
#include "libmicroemu/internal/utils/memory_helpers.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/result.h"
#include "libmicroemu/types.h"

#include <string.h>
#include <time.h>
#include <tuple>

#undef _DISABLE_EXT_EXIT

namespace microemu {
namespace internal {

enum {
  // File operations
  kSysOpen = 0x01u,   //- Open a file or stream on the host system.
  kSysIsTTY = 0x09u,  //- Check whether a file handle is associated with a file or
                      // a stream/terminal such as stdout.
  kSysWrite = 0x05u,  //- Write to a file or stream.
  kSysRead = 0x06u,   //- Read from a file at the current cursor position.
  kSysClose = 0x02u,  //- Closes a file on the host which has been opened by SysOPEN.
  kSysFLen = 0x0Cu,   //- Get the length of a file.
  kSysSeek = 0x0Au,   //- Set the file cursor to a given position in a file.
  kSysTmpNam = 0x0Du, //- Get a temporary absolute file path to create a temporary file.
  kSysRemove = 0x0Eu, //- Remove a file on the host system. Possibly insecure!
  kSysRename = 0x0Fu, //- Rename a file on the host system. Possibly insecure!
  // Terminal I/O operations
  kSysWriteC = 0x03u, //- Write one character to the debug terminal.
  kSysWrite0 = 0x04u, //- Write a 0-terminated string to the debug terminal.
  kSysReadC = 0x07u,  //- Read one character from the debug terminal.
  // Time operations
  kSysClock = 0x10u,
  kSysElapsed = 0x30u,
  kSysTickFreq = 0x31u,
  kSysTime = 0x11u,
  // System/Misc. operations
  kSysErrNo = 0x13u,      //- Returns the value of the C library errno variable that
                          // is associated with the semihosting implementation.
  kSysGetCmdLine = 0x15u, //- Get commandline parameters for the application to
                          // run with=argc and argv for main())
  kSysHeapInfo = 0x16u,
  kSysExit = 0x18u, // An application calls this operation to report an
                    // exception to the debugger directly.
  kSysIsError = 0x08u,
  kSysSystem = 0x12u,

  // Extended feature - must be enabled
  kSysExitExtended = 0x20u,
};

enum class ReasonCodes : uint32_t {
  // Hardware reason codes
  kADPStoppedBranchThroughZero = 0x20000u,
  kADPStoppedUndefinedInstr = 0x20001u,
  kADPStoppedSoftwareInterrupt = 0x20002u,
  kADPStoppedPrefetchAbort = 0x20003u,
  kADPStoppedDataAbort = 0x20004u,
  kADPStoppedAddressException = 0x20005u,
  kADPStoppedIRQ = 0x20006u,
  kADPStoppedFIQ = 0x20007u,

  // Software reason codes
  kADPStoppedBreakPoint = 0x20020u,
  kADPStoppedWatchPoint = 0x20021u,
  kADPStoppedStepComplete = 0x20022u,
  kADPStoppedRunTimeErrorUnknown = 0x20023u,
  kADPStoppedInternalError = 0x20024u,
  kADPStoppedUserInterruption = 0x20025u,
  kADPStoppedApplicationExit = 0x20026u,
  kADPStoppedStackOverflow = 0x20027u,
  kADPStoppedDivisionByZero = 0x20028u,
  kADPStoppedOSSpecific = 0x20029u
};

constexpr u32 kHandleStdin = 1u;
constexpr u32 kHandleStdout = 2u;
constexpr u32 kHandleStderr = 3u;
constexpr u32 kHandleSemihostFeatures = 4u;

static char kFeatureData[] = {
    0x53u, // Magic Byte 0
    0x48u, // Magic Byte 1
    0x46u, // Magic Byte 2
    0x42u, // Magic Byte 3
    0x3u   // feature byte 0 : SH_EXT_EXIT_EXTENDED + SH_EXT_STDOUT_STDERR
};

template <typename TProcessorStates, typename TBus, typename TRegOps, typename TSpecRegOps,
          typename TLogger = NullLogger>
class Semihosting : public IBreakpoint {
public:
  using Reg = TRegOps;
  using SReg = TSpecRegOps;

  /// \brief Constructs a Semihosting object
  Semihosting(TProcessorStates &pstates, TBus bus) : bus_(bus), pstates_(pstates) {}

  template <u32 N> Result<std::array<u32, N>> ReadR1Words();

  template <> Result<std::array<u32, 3>> ReadR1Words<3u>() {
    auto r1 = Reg::template ReadRegister<1u>(pstates_);
    auto mem = MemoryHelpers::ReadMemory(pstates_, bus_, r1, r1 + 0x4, r1 + 0x8);
    return mem;
  }

  template <> Result<std::array<u32, 2>> ReadR1Words<2u>() {
    auto r1 = Reg::template ReadRegister<1u>(pstates_);
    auto mem = MemoryHelpers::ReadMemory(pstates_, bus_, r1, r1 + 0x4);
    return mem;
  }

  template <> Result<std::array<u32, 1>> ReadR1Words<1u>() {
    auto r1 = Reg::template ReadRegister<1u>(pstates_);
    auto mem = MemoryHelpers::ReadMemory(pstates_, bus_, r1);
    return mem;
  }

  struct SemihostResult {
    i32 ret_r0;
    BkptFlagsSet bkpt_flags;
  };

  Result<SemihostResult> CallSemihost(const uint32_t &r0) {
    i32 sh_ret = -1u;
    BkptFlagsSet bkpt_flags = 0u;
    bkpt_flags |= static_cast<BkptFlagsSet>(BkptFlags::kOmitException);
    switch (r0) {

    case kSysHeapInfo: {
      LOG_DEBUG(TLogger, "kSysHeapInfo(0x%0x)", r0);
      sh_ret = r0; // On exit, R1 contains the address of the pointer to the
                   // structure.
      break;
    }

    case kSysOpen: {
      TRY_ASSIGN(mem3, SemihostResult, ReadR1Words<3>());
      const auto &ptr = mem3[0];
      const auto &mode = mem3[1];
      const auto &w_len = mem3[2];

      char buf[kBufferLen];
      // keep one char reserve for null-termination
      MemoryHelpers::CpyFromEmuMem(pstates_, bus_, buf, sizeof(buf) - 1, ptr, w_len);
      buf[w_len] = '\0';
      LOG_DEBUG(TLogger, "kSysOpen(0x%0x) - 0x%0x 0x%0x 0x%0x - '%s'", r0, ptr, mode, w_len, buf);
      u32 result = -1u;
      if (strcmp(buf, ":tt") == 0) {
        if ((mode >= 0u) && (mode <= 3u)) {
          result = kHandleStdin;
        } else if ((mode >= 4u) && (mode <= 7u)) {
          result = kHandleStdout;
        } else if ((mode >= 8u) && (mode <= 11u)) {
          result = kHandleStderr;
        } else {
          return Err<SemihostResult>(StatusCode::kScOutOfRange);
        }
      } else if (strcmp(buf, ":semihosting-features") == 0) {
        result = kHandleSemihostFeatures;
      } else {
        return Err<SemihostResult>(StatusCode::kScOpenFileFailed);
      }
      // return handle
      sh_ret = result;
      break;
    }
    case kSysWrite: {
      TRY_ASSIGN(mem3, SemihostResult, ReadR1Words<3>());
      const auto &fhandle = mem3[0];
      const auto &ptr = mem3[1];
      const auto &w_len = mem3[2];

      if ((fhandle != kHandleStdout) && (fhandle != kHandleStderr)) {
        // currently only support Write to stdout and stderr
        return Err<SemihostResult>(StatusCode::kScUnsuporrted);
      }
      char buf[kBufferLen];
      // keep one char reserve for null-termination
      TRY(SemihostResult,
          MemoryHelpers::CpyFromEmuMem(pstates_, bus_, buf, sizeof(buf) - 1, ptr, w_len));
      buf[w_len] = '\0';

      LOG_TRACE(TLogger, "kSysWrite(0x%0x)- 0x%0x 0x%0x 0x%0x", r0, fhandle, ptr, w_len, buf);
      LOG_INFO(TLogger, "stdout << '%s'", buf);
      printf("%s", buf);

      // 0 indicates everything is ok
      sh_ret = 0u;
      break;
    }
    case kSysRead: {
      TRY_ASSIGN(mem3, SemihostResult, ReadR1Words<3>());
      const auto &fhandle = mem3[0];
      const auto &ptr = mem3[1];
      const auto &r_len = mem3[2];

      LOG_DEBUG(TLogger, "kSysRead(0x%0x)- 0x%0x 0x%0x 0x%0x", r0, fhandle, ptr, r_len);

      if (fhandle == kHandleSemihostFeatures) {

        auto *feature_data = &kFeatureData[semihost_features_position_];
        auto features_data_size = sizeof(kFeatureData) - semihost_features_position_;

        TRY_ASSIGN(read_bytes, SemihostResult,
                   MemoryHelpers::CpyToEmuMem(pstates_, bus_, ptr, r_len, feature_data,
                                              features_data_size));

        semihost_features_position_ += read_bytes;
        // 0 indicates everything is ok
        sh_ret = 0u;
      } else {
        return Err<SemihostResult>(StatusCode::kScUnsuporrted);
      }
      break;
    }
    case kSysIsTTY: {
      TRY_ASSIGN(mem1, SemihostResult, ReadR1Words<1>());
      const auto &fhandle = mem1[0];

      LOG_DEBUG(TLogger, "kSysIsTTY(0x%0x) - 0x%0x", r0, fhandle);

      if (fhandle != kHandleStdout) {
        // currently only support Write to stdout
        return Err<SemihostResult>(StatusCode::kScUnsuporrted);
      }

      // 1 if the handle identifies an interactive device
      sh_ret = 1u;
      break;
    }
    case kSysFLen: {
      TRY_ASSIGN(mem1, SemihostResult, ReadR1Words<1>());
      const auto &fhandle = mem1[0];
      LOG_DEBUG(TLogger, "kSysFLen(0x%0x) - 0x%0x", r0, fhandle);

      if ((fhandle == kHandleStdin) || (fhandle == kHandleStdout) || (fhandle == kHandleStderr)) {
        sh_ret = 0u;
        break;
      }
      if (fhandle == kHandleSemihostFeatures) {
#ifdef _DISABLE_EXT_EXIT
        sh_ret = 0u; // no extended features
#else
        sh_ret = sizeof(kFeatureData); // x bytes needed to store the feature sequence
#endif
        break;
      }
      return Err<SemihostResult>(StatusCode::kScUnexpected);
      break;
    }
    case kSysSeek: {
      TRY_ASSIGN(mem2, SemihostResult, ReadR1Words<2>());
      const auto &fhandle = mem2[0];
      const auto &fpos = mem2[1];
      LOG_DEBUG(TLogger, "kSysSeek(0x%0x) - 0x%0x - %u", r0, fhandle, fpos);

      if (fhandle == kHandleSemihostFeatures) {
        if (fpos < sizeof(kFeatureData)) {
          semihost_features_position_ = fpos;
          sh_ret = 0u;
        } else {
          return Err<SemihostResult>(StatusCode::kScOutOfRange);
        }

      } else {
        return Err<SemihostResult>(StatusCode::kScUnexpected);
      }

      break;
    }
    case kSysErrNo: {
      LOG_ERROR(TLogger, "kSysErrNo(0x%0x)", r0);

      break;
    }
    case kSysClock: {
      auto reason_code = Reg::template ReadRegister<1u>(pstates_);
      if (reason_code != 0x0) {
        return Err<SemihostResult>(StatusCode::kScUnexpected);
      }
      u32 ticks = clock();

      LOG_DEBUG(TLogger, "kSysClock(0x%0x) - host_clock:%u, CLOCKS_PER_SEC:%u", r0,
                static_cast<unsigned int>(ticks), static_cast<unsigned int>(CLOCKS_PER_SEC));

      uint64_t ticks_64 = static_cast<u64>(ticks);
      // caculate the time in centiseconds. This is expected by libgloss
      uint64_t centiseconds = (ticks_64 * 100) / CLOCKS_PER_SEC;

      sh_ret = static_cast<i32>(centiseconds);
      break;
    }

    case kSysExit: {
      LOG_INFO(TLogger, "kSysExit(0x%0x)", r0);

      auto reason_code = Reg::template ReadRegister<1u>(pstates_);
      status_code_ = 0u; // no status code available
      if (reason_code == static_cast<u32>(ReasonCodes::kADPStoppedApplicationExit)) {
        bkpt_flags |= static_cast<BkptFlagsSet>(BkptFlags::kRequestExit);
      } else {
        bkpt_flags |= static_cast<BkptFlagsSet>(BkptFlags::kRequestErrorExit);
      }
      break;
    }

    case kSysExitExtended: {
      TRY_ASSIGN(mem2, SemihostResult, ReadR1Words<2>());
      const auto &reason_code = mem2[0];
      const auto &reason_subcode = mem2[1];
      LOG_INFO(TLogger, "kSysExitExtended(0x%0x) - reason_code: 0x%x - reason_subcode: 0x%x", r0,
               reason_code, reason_subcode);
      status_code_ = reason_subcode;
      if (reason_code == static_cast<u32>(ReasonCodes::kADPStoppedApplicationExit)) {
        bkpt_flags |= static_cast<BkptFlagsSet>(BkptFlags::kRequestExit);
      } else {
        bkpt_flags |= static_cast<BkptFlagsSet>(BkptFlags::kRequestErrorExit);
      }
      break;
    }

    case kSysGetCmdLine: {

      TRY_ASSIGN(mem2, SemihostResult, ReadR1Words<2>());
      const auto &w1 = mem2[0];
      const auto &w2 = mem2[1];
      static_cast<void>(w1);
      static_cast<void>(w2);
      LOG_DEBUG(TLogger, "kSysGetCmdLine(0x%0x) - 0x%0x 0x%0x", r0, w1, w2);

      sh_ret = -1u; // accepted but not supported
      break;
    }
    case kSysClose: {
      TRY_ASSIGN(mem1, SemihostResult, ReadR1Words<1>());
      const auto &fhandle = mem1[0];

      if ((fhandle != kHandleStdin) && (fhandle != kHandleStdout) && (fhandle != kHandleStderr) &&
          (fhandle != kHandleSemihostFeatures)) {
        return Err<SemihostResult>(StatusCode::kScUnexpected);
      }
      LOG_DEBUG(TLogger, "kSysClose(0x%0x) - 0x%0x", r0, fhandle);
      sh_ret = 0u;
      break;
    }

    default: {
      LOG_ERROR(TLogger, "Unknown(0x%0x)", r0);
      return Err<SemihostResult>(StatusCode::kScUnsuporrted);
      sh_ret = -1u; // signals error
      break;
    }
    }

    return Ok<SemihostResult>(SemihostResult{sh_ret, bkpt_flags});
  }

  virtual Result<BkptFlagsSet> Call(const uint32_t &imm32) override {
    if (imm32 != 0xabu) {
      return Ok<u8>(0u); // no semihosting call
    }
    auto r0 = Reg::template ReadRegister<0u>(pstates_);
    TRY_ASSIGN(sh_res, u8, CallSemihost(r0));
    Reg::WriteRegister(pstates_, RegisterId::kR0, sh_res.ret_r0);

    return Ok(sh_res.bkpt_flags);
  }

  virtual uint32_t GetExitStatusCode() const override { return status_code_; }

private:
  static constexpr u32 kBufferLen = 128u;

  TBus bus_;
  TProcessorStates &pstates_;
  u32 file_id_{0xa};
  i32 status_code_{0u};
  u32 semihost_features_position_{0u};
};

} // namespace internal
} // namespace microemu