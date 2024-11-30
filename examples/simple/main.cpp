#include "libmicroemu/machine.h"
#include <iostream>
#include <vector>

// Path to the ELF file
constexpr static const char *kElfPath = "coremark/prebuilt/bin/coremark.elf";

constexpr static libmicroemu::me_adr_t kFlashSegVadr = 0x0U;
constexpr static libmicroemu::me_adr_t kRam1SegVadr = 0x20000000U;

int main() {
  libmicroemu::Machine machine;
  std::cout << "Using libmicroemu version: " << machine.GetVersion() << std::endl;

  // Allocate memory for FLASH and RAM1 segments
  auto flash_seg = std::vector<uint8_t>(0x20000u); // 128KB for FLASH
  auto ram1_seg = std::vector<uint8_t>(0x40000u);  // 256KB for RAM1

  // Set up memory segments
  machine.SetFlashSegment(flash_seg.data(), flash_seg.size(), kFlashSegVadr);
  machine.SetRam1Segment(ram1_seg.data(), ram1_seg.size(), kRam1SegVadr);

  // Load the ELF file and set the entry point
  if (machine.Load(kElfPath, true) != libmicroemu::StatusCode::kSuccess) {
    std::cerr << "Failed to load ELF file." << std::endl;
    return EXIT_FAILURE;
  }

  // Execute the ELF file
  auto res_exec = machine.Exec();
  if (res_exec.IsErr()) {
    std::cerr << "Failed to execute ELF file." << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}