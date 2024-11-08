#include "libmicroemu/microemu.h"
#include "libmicroemu/internal/elf/elf_reader.h"
#include "libmicroemu/internal/emulator.h"
#include "libmicroemu/internal/trace/intstr_to_mnemonic.h"

#include <ctype.h>
#include <fstream>
#include <iostream>
#include <utility>

namespace microemu {

using namespace internal;

Emulator<ProcessorStates> MicroEmu::BuildEmulator() {
  auto emu = Emulator<ProcessorStates>(pstates_);

  emu.SetFlashSegment(flash_, flash_size_, flash_vadr_);
  emu.SetRam1Segment(ram1_, ram1_size_, ram1_vadr_);
  emu.SetRam2Segment(ram2_, ram2_size_, ram2_vadr_);
  return emu;
}

MicroEmu::MicroEmu() noexcept {};
MicroEmu::~MicroEmu() noexcept {};

Result<void> MicroEmu::Load(const char *elf_file, bool set_entry_point) noexcept {

  std::fill(ram1_, ram1_ + ram1_size_, 0xFFu);
  std::fill(ram2_, ram2_ + ram2_size_, 0xFFu);

  u32 entry_point{0u};
  {
    auto file = std::ifstream(elf_file, std::ios::binary);
    if (!file.is_open()) {
      // Failed to open file
      return Err(StatusCode::kScOpenFileFailed);
    }
    auto res_reader = ElfReader::ReadElf(file);
    if (res_reader.IsErr()) {
      return Err<ElfReader, void>(res_reader);
    };
    auto reader = res_reader.content;

    for (auto it = reader.begin(); it != reader.end(); ++it) {
      const auto &phdr = *it;

      auto flags = phdr.p_flags;

      // CODE SEGMENT - READ-ONLY, EXECUTE
      if (((flags & PF_X) != 0u) && ((flags & PF_R) != 0u) && ((flags & PF_W) == 0u)) {
        if ((phdr.p_vaddr < flash_vadr_) ||
            (phdr.p_vaddr + phdr.p_filesz >= flash_vadr_ + flash_size_)) {
          // size of buffer is not big enough
          return Err(StatusCode::kScBufferTooSmall);
        }
        auto res = reader.GetSegmentData(phdr, flash_, phdr.p_filesz, 0x0, 0x0);
        if (res.IsErr()) {
          return Err<void, void>(res);
        }
      }

      // DATA SEGMENT - R/W, NO EXECUTE
      if (((flags & PF_X) == 0u) && ((flags & PF_R) != 0u) && ((flags & PF_W) != 0u)) {
        if ((phdr.p_vaddr < ram1_vadr_) ||
            (phdr.p_vaddr + phdr.p_filesz >= ram1_vadr_ + ram1_size_)) {
          // size of buffer is not big enough
          return Err(StatusCode::kScBufferTooSmall);
        }
        auto data_seg_vadr = static_cast<me_adr_t>(phdr.p_vaddr);

        auto res = reader.GetSegmentData(phdr, ram1_, phdr.p_filesz, ram1_vadr_, data_seg_vadr);
        if (res.IsErr()) {
          return Err<void, void>(res);
        }
      }
    }

    // Start from load address
    entry_point = reader.GetEntryPoint();
  }

  TRY(void, Reset());

  if (set_entry_point) {
    auto emu = BuildEmulator();
    emu.SetEntryPoint(entry_point);
  }
  return Ok();
}

void MicroEmu::SetFlashSegment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept {
  flash_ = seg_ptr;
  flash_size_ = seg_size;
  flash_vadr_ = seg_vadr;
}

void MicroEmu::SetRam1Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept {
  ram1_ = seg_ptr;
  ram1_size_ = seg_size;
  ram1_vadr_ = seg_vadr;
}

void MicroEmu::SetRam2Segment(u8 *seg_ptr, me_size_t seg_size, me_adr_t seg_vadr) noexcept {
  ram2_ = seg_ptr;
  ram2_size_ = seg_size;
  ram2_vadr_ = seg_vadr;
}

Result<void> MicroEmu::Reset() noexcept {
  auto emu = BuildEmulator();
  TRY(void, emu.Reset());
  return Ok();
}

Result<EmuResult> MicroEmu::Exec(i32 instr_limit, FPreExecStepCallback cb_pre_exec,
                                 FPostExecStepCallback cb_post_exec) noexcept {
  auto emu = BuildEmulator();
  auto res = emu.Exec(instr_limit, cb_pre_exec, cb_post_exec);
  return res;
}

void MicroEmu::EvaluateState(FStateCallback cb) noexcept {

  auto emu = BuildEmulator();
  using TProcessorStates = decltype(emu)::ProcessorStates;
  using Reg = decltype(emu)::RegOps;
  using SReg = decltype(emu)::SpecRegOps;
  auto reg_access = RegAccess<TProcessorStates, Reg, SReg>(pstates_);
  auto spec_reg_access = SpecialRegAccess<TProcessorStates, SReg>(pstates_);

  cb(reg_access, spec_reg_access);
}

void MicroEmu::RegisterLoggerCallback(void (*callback)(microemu::LogLevel level, const char *,
                                                       ...) noexcept) noexcept {
  StaticLogger::RegisterLoggerCallback(callback);
}
} // namespace microemu