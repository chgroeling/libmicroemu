#pragma once

#include "libmicroemu/emu_context.h"
#include <cstddef>
#include <cstdint>

static constexpr uint8_t kRegCount = 18U;
static constexpr uint8_t kRegsPerRow = 5U;

struct SampledRegs {
  const char *names[kRegCount];
  uint32_t values[kRegCount];
};

class RegPrinter {
public:
  // Copyies the register values to the buffer
  static SampledRegs SampleRegs(const libmicroemu::IRegAccessor &reg_access,
                                const libmicroemu::ISpecialRegAccessor &spec_reg_access) {
    SampledRegs sampled_regs{};
    for (uint8_t reg_id = 0x0U; reg_id < kRegCount; reg_id += 1U) {
      if (reg_id < 16U) {
        auto value = reg_access.ReadRegister(static_cast<libmicroemu::RegisterId>(reg_id));
        sampled_regs.values[reg_id] = value;
        sampled_regs.names[reg_id] =
            reg_access.GetRegisterName(static_cast<libmicroemu::RegisterId>(reg_id)).data();
      } else {
        switch (reg_id) {
        case 16U: {
          auto spec_reg_id = libmicroemu::SpecialRegisterId::kApsr;
          auto value = spec_reg_access.ReadRegister(spec_reg_id);
          sampled_regs.values[reg_id] = value;
          sampled_regs.names[reg_id] = spec_reg_access.GetRegisterName(spec_reg_id).data();
          break;
        }
        case 17U: {
          auto spec_reg_id = libmicroemu::SpecialRegisterId::kIstate;
          auto value = spec_reg_access.ReadRegister(spec_reg_id);
          sampled_regs.values[reg_id] = value;
          sampled_regs.names[reg_id] = spec_reg_access.GetRegisterName(spec_reg_id).data();
          break;
        }
        default:
          break;
        };
      }
    }

    return sampled_regs;
  }

  static void PrintRegs(const SampledRegs &sampled_regs) {
    auto count = 0U;
    for (uint8_t reg_id = 0x0U; reg_id < kRegCount; ++reg_id) {
      count += 1U;

      if ((count % kRegsPerRow) != 1U) {
        printf(" | ");
      }

      const auto &name = sampled_regs.names[reg_id];
      const auto &value = sampled_regs.values[reg_id];
      printf("%6s = %08x", name, value);
      if ((count % kRegsPerRow) == 0) {
        printf("\n");
      }
    }
    // Print a newline if the last register was not printed
    if ((count % kRegsPerRow) != 0) {
      printf("\n");
    }
  }

  static void PrintRegDiffs(const SampledRegs &sampled_regs,
                            const SampledRegs &regs_from_last_step) {
    bool print[kRegCount];
    std::fill(print, print + kRegCount, true);

    // Print only changed registers
    for (uint8_t reg_id = 0x0U; reg_id < kRegCount; reg_id += 1U) {
      if (sampled_regs.values[reg_id] == regs_from_last_step.values[reg_id]) {
        print[reg_id] = false;
      }
    }

    auto count = 0U;
    for (uint8_t reg_id = 0x0U; reg_id < kRegCount; ++reg_id) {
      if (!print[reg_id]) {
        continue;
      }
      count += 1U;

      if ((count % kRegsPerRow) != 1U) {
        printf(" | ");
      }

      const auto &name = sampled_regs.names[reg_id];
      const auto &value = sampled_regs.values[reg_id];
      printf("%6s = %08x", name, value);
      if ((count % kRegsPerRow) == 0) {
        printf("\n");
      }
    }
    // Print a newline if the last register was not printed
    if ((count % kRegsPerRow) != 0) {
      printf("\n");
    }
  }

private:
  /**
   * @brief Constructs a RegPrinter object
   */
  RegPrinter() = delete;

  /**
   * @brief Destructor
   */
  ~RegPrinter() = delete;

  /**
   * @brief Copy constructor for RegPrinter.
   * @param r_src the object to be copied
   */
  RegPrinter(RegPrinter &r_src) = delete;

  /**
   * @brief Copy assignment operator for RegPrinter.
   * @param r_src the object to be copied
   */
  RegPrinter &operator=(const RegPrinter &r_src) = delete;

  /**
   * @brief Move constructor for RegPrinter.
   * @param r_src the object to be moved
   */
  RegPrinter(RegPrinter &&r_src) = delete;

  /**
   * @brief Move assignment operator for  RegPrinter.
   * @param r_src the object to be moved
   */
  RegPrinter &operator=(RegPrinter &&r_src) = delete;
};
