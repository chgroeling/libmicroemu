#include "libmicroemu/logger.h"
#include "libmicroemu/machine.h"
#include "reg_printer.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <algorithm>
#include <cxxopts.hpp>
#include <fmt/core.h>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdarg.h>
#include <vector>

static const std::vector<std::string> kValidLogLevels = {"TRACE",   "DEBUG", "INFO",
                                                         "WARNING", "ERROR", "CRITICAL"};

static const std::vector<std::string> kValidMemoryConfigs = {"NONE", "STDLIB", "MINIMAL"};

std::string CreateCommaSeparatedString(const std::vector<std::string> &input_set) {
  std::ostringstream oss;
  for (auto it = input_set.begin(); it != input_set.end(); ++it) {
    if (it != input_set.begin()) {
      oss << ", ";
    }
    oss << *it;
  }
  return oss.str();
}

void LoggingCallback(libmicroemu::LogLevel level, const char *format, ...) noexcept {
  // Initialize variadic argument list
  va_list args;
  va_start(args, format);

  // Estimate the required size of the formatted string
  const int size = std::vsnprintf(nullptr, 0U, format, args) + 1U; // +1 for null termination
  if (size <= 0) {
    spdlog::critical("Error during formatting");
    // end variadic argument list
    va_end(args);
    return;
  }

  // Create a buffer to store the formatted message
  std::vector<char> buf(size);
  std::vsnprintf(buf.data(), size, format, args);

  switch (level) {
  case libmicroemu::LogLevel::kTrace:
    spdlog::trace(buf.data());
    break;
  case libmicroemu::LogLevel::kDebug:
    spdlog::debug(buf.data());
    break;
  case libmicroemu::LogLevel::kInfo:
    spdlog::info(buf.data());
    break;
  case libmicroemu::LogLevel::kWarn:
    spdlog::warn(buf.data());
    break;
  case libmicroemu::LogLevel::kError:
    spdlog::error(buf.data());
    break;
  case libmicroemu::LogLevel::kCritical:
    spdlog::critical(buf.data());
    break;
  default:
    spdlog::critical(buf.data());
    break;
  };

  // end variadic argument list
  va_end(args);
}

int main(int argc, const char *argv[]) {
  // Parse command line options
  cxxopts::Options options("libmicroemu", "Armv7-m  emulator");
  options.positional_help("<elf_file>");

  const std::string kLogLevelOption =
      fmt::format("Set the log level ({})", CreateCommaSeparatedString(kValidLogLevels));

  const std::string kMemoryConfigOption =
      fmt::format("Set the emulation memory configuration ({})",
                  CreateCommaSeparatedString(kValidMemoryConfigs));

  // clang-format off
  options.add_options()
    ("h,help", "Print usage information.")
    ("version", "Print version information.")
    ("l,log", "Enable logging")
    ("log-level" , kLogLevelOption, 
        cxxopts::value<std::string>()->default_value("INFO"))
    ("log-file", "Specify log file path.", 
        cxxopts::value<std::string>())
    ("t,trace", "Print instruction trace to stdout") 
    ("trace-regs", "Enable output of all register states during the trace.")
    ("trace-changed-regs", "Enable output of registers that have changed during the trace.")
    ("e,elf_ep", "Load and set entry point from ELF file.")
    ("i,instr_limit", "Set the maximum number of instructions to execute.", 
        cxxopts::value<int64_t>()) 
    ("m,memory-config", kMemoryConfigOption, 
        cxxopts::value<std::string>()->default_value("NONE"))
    ("elf_file", "Path to the executable to load.",
        cxxopts::value<std::string>())
    ("flash-size", "Override the flash segment size (in bytes).", 
        cxxopts::value<uint32_t>())
    ("flash-vaddr", "Override the flash segment virtual address.", 
        cxxopts::value<uint32_t>())
    ("ram1-size", "Override the RAM1 segment size (in bytes).", 
        cxxopts::value<uint32_t>())
    ("ram1-vaddr", "Override the RAM1 segment virtual address.", 
        cxxopts::value<uint32_t>())
    ("ram2-size", "Override the RAM2 segment size (in bytes).", 
        cxxopts::value<uint32_t>())
    ("ram2-vaddr", "Override the RAM2 segment virtual address.", 
        cxxopts::value<uint32_t>());
  ;
  // clang-format on

  // parse positional arguments into vector filenames
  options.parse_positional({"elf_file"});

  cxxopts::ParseResult result;

  try {
    result = options.parse(argc, argv);
  } catch (const cxxopts::exceptions::exception &x) {
    fmt::print(stderr, "libmicroemu: {}\n", x.what());
    fmt::print(stderr, "usage: libmicroemu [options] <elf_file> \n");
    return EXIT_FAILURE;
  }
  libmicroemu::Machine machine;

  // print out help if necessary
  if (result.count("help")) {
    fmt::print(stdout, "{}", options.help());
    return EXIT_SUCCESS;
  }

  // Print version information if necessary
  if (result.count("version")) {

    // Both versions are the same for now
    fmt::print(stdout, "libmicroemu version: {}\n", machine.GetVersion());
    return EXIT_SUCCESS;
  }

  // =====================================
  // Checking command line options
  // =====================================

  // Check if the elf_file argument is present
  if (!result.count("elf_file")) {
    fmt::print(stderr, "libmicroemu: Missing required positional argument <elf_file>\n");
    fmt::print(stderr, "usage: libmicroemu [options] <elf_file>\n");
    return EXIT_FAILURE;
  }

  const auto elf_file = result["elf_file"].as<std::string>();

  bool is_trace = result["trace"].as<bool>();
  bool is_trace_regs = result["trace-regs"].as<bool>();
  bool is_trace_changed_regs = result["trace-changed-regs"].as<bool>();

  if (is_trace_regs && is_trace_changed_regs) {
    fmt::print(stderr,
               "libmicroemu: --trace-regs and --trace-changed-regs are mutually exclusive\n");
    return EXIT_FAILURE;
  }

  // Check defined log levels
  std::string log_level = result["log-level"].as<std::string>();
  if (std::find(kValidLogLevels.begin(), kValidLogLevels.end(), log_level) ==
      kValidLogLevels.end()) {
    fmt::print(stderr, "Error: Invalid log level '{}'. Valid log levels are: {}\n", log_level,
               CreateCommaSeparatedString(kValidLogLevels));
    return EXIT_FAILURE;
  }

  // Check defined memory-configs
  std::string memory_config = result["memory-config"].as<std::string>();
  if (std::find(kValidMemoryConfigs.begin(), kValidMemoryConfigs.end(), memory_config) ==
      kValidMemoryConfigs.end()) {
    fmt::print(stderr, "Error: Invalid memory-config '{}'. Valid memory-configs are: {}\n",
               memory_config, CreateCommaSeparatedString(kValidMemoryConfigs));
    return EXIT_FAILURE;
  }

  if (result.count("instr_limit")) {
    auto instr_limit = result["instr_limit"].as<int>();

    if (instr_limit < -1) {
      fmt::print(stderr, "libmicroemu: instr_limit must be greater than or equal to -1\n");
      return EXIT_FAILURE;
    }
  }

  // =====================================
  // Emulator configuration
  // =====================================
  SampledRegs regs_from_last_step{};

  // Flash Segment
  std::vector<uint8_t> flash_seg;
  uint32_t flash_seg_size{0x0U};
  uint32_t flash_seg_vadr{0x0U};

  // RAM1 Segment
  std::vector<uint8_t> ram1_seg;
  uint32_t ram1_seg_size{0x0U};
  uint32_t ram1_seg_vadr{0x0U};

  // RAM2 Segment
  std::vector<uint8_t> ram2_seg;
  uint32_t ram2_seg_size{0x0U};
  uint32_t ram2_seg_vadr{0x0U};

  int64_t instr_limit = -1; // <0 means infinite

  if (result.count("instr_limit")) {
    auto instr_limit = result["instr_limit"].as<int>();
  }

  if (result.count("log")) {
    if (result.count("log-file")) {
      std::string log_file = result["log-file"].as<std::string>();
      // Create a file sink for the global logger and point it to a specific file
      auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);

      // Set the default logger to use the file sink
      spdlog::set_default_logger(std::make_shared<spdlog::logger>("global_logger", file_sink));
    }

    spdlog::set_pattern("[%H:%M:%S,%f] [%^%l%$] %v");

    if (log_level == "TRACE") {
      spdlog::set_level(spdlog::level::trace);
    } else if (log_level == "DEBUG") {
      spdlog::set_level(spdlog::level::debug);
    } else if (log_level == "INFO") {
      spdlog::set_level(spdlog::level::info);
    } else if (log_level == "ERROR") {
      spdlog::set_level(spdlog::level::err);
    } else if (log_level == "WARNING") {
      spdlog::set_level(spdlog::level::warn);
    } else if (log_level == "CRITICAL") {
      spdlog::set_level(spdlog::level::critical);
    }

    libmicroemu::Machine::RegisterLoggerCallback(&LoggingCallback);
  }

  // Memory configuration
  if (memory_config == "STDLIB") {
    flash_seg_vadr = 0x0U;
    flash_seg_size = 0x10000U;

    ram1_seg_vadr = 0x10000U;
    ram1_seg_size = 0x20000U;

    ram2_seg_vadr = 0x70000U;
    ram2_seg_size = 0x10000U;

  } else if (memory_config == "MINIMAL") {
    flash_seg_vadr = 0x0U;
    flash_seg_size = 0x20000U;

    ram1_seg_vadr = 0x20000000U;
    ram1_seg_size = 0x40000U;

    // No RAM2 segment
    ram2_seg_vadr = 0x0U;
    ram2_seg_size = 0x0U;
  } else {
    // default or NONE
    // Do nothing
  }

  // Override memory configuration if command line options are present
  if (result.count("flash-size")) {
    flash_seg_size = result["flash-size"].as<uint32_t>();
  }
  if (result.count("flash-vaddr")) {
    flash_seg_vadr = result["flash-vaddr"].as<uint32_t>();
  }
  if (result.count("ram1-size")) {
    ram1_seg_size = result["ram1-size"].as<uint32_t>();
  }
  if (result.count("ram1-vaddr")) {
    ram1_seg_vadr = result["ram1-vaddr"].as<uint32_t>();
  }
  if (result.count("ram2-size")) {
    ram2_seg_size = result["ram2-size"].as<uint32_t>();
  }
  if (result.count("ram2-vaddr")) {
    ram2_seg_vadr = result["ram2-vaddr"].as<uint32_t>();
  }

  // Allocate memory for the segments
  flash_seg = std::vector<uint8_t>(flash_seg_size);
  ram1_seg = std::vector<uint8_t>(ram1_seg_size);
  ram2_seg = std::vector<uint8_t>(ram2_seg_size);

  // Set the memory segments
  machine.SetFlashSegment(flash_seg.data(), flash_seg_size, flash_seg_vadr);
  machine.SetRam1Segment(ram1_seg.data(), ram1_seg_size, ram1_seg_vadr);
  machine.SetRam2Segment(ram2_seg.data(), ram2_seg_size, ram2_seg_vadr);

  // Check if the entry point should be set from the ELF file
  // If not set, the entry point is set through the vector tabledoc:
  bool is_elf_entry_point = false;
  if (result.count("elf_ep")) {
    is_elf_entry_point = true;
  }

  // =====================================
  // Emulator - Load elf file
  // =====================================

  libmicroemu::FStateCallback initial_state_cb =
      [&regs_from_last_step](libmicroemu::IRegAccessor &reg_access,
                             libmicroemu::ISpecialRegAccessor &spec_reg_access) {
        // Print the initial state
        fmt::print(stdout, "Initial register states:\n");
        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegs(sampled_regs);
        regs_from_last_step = sampled_regs;
      };

  // Load the ELF file
  const auto sc = machine.Load(elf_file.c_str(), is_elf_entry_point);
  if (sc != libmicroemu::StatusCode::kSuccess) {
    const auto sc_str = libmicroemu::StatusCodeToString(sc);
    fmt::print(stderr, "ERROR: Emulator returned error: {}({})\n", sc_str,
               static_cast<uint32_t>(sc));
    return EXIT_FAILURE;
  }

  // Evaluate the initial state in case of trace
  if (is_trace == true) {
    // Evaluate the initial state
    if (is_trace_regs || is_trace_changed_regs) {
      machine.EvaluateState(initial_state_cb);
    }
  }

  // =====================================
  // Emulator execution
  // =====================================

  libmicroemu::FPreExecStepCallback pre_exec_instr_trace = [](libmicroemu::EmuContext &ectx) {
    const auto &pc = ectx.GetPc();
    const auto &raw_instr = ectx.GetOpCode();

    if (raw_instr.is_32bit) {
      fmt::print(stdout, "{:x}: {:0>4x} {:0>4x}  ", pc, raw_instr.low, raw_instr.high);
    } else {
      fmt::print(stdout, "{:x}: {:0>4x}       ", pc, raw_instr.low);
    }

    char buf[128];
    ectx.BuildMnemonic(buf, sizeof(buf));
    printf("%s\n", buf);
  };

  libmicroemu::FPostExecStepCallback post_exec_instr_trace_regs =
      [&regs_from_last_step](libmicroemu::EmuContext &ectx) {
        const auto &reg_access = ectx.GetRegisterAccessor();
        const auto &spec_reg_access = ectx.GetSpecialRegisterAccessor();

        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegs(sampled_regs);

        // Update the last sampled registers. Not needed here. Just in case.
        regs_from_last_step = sampled_regs;
      };

  libmicroemu::FPostExecStepCallback post_exec_instr_trace_changed_regs =
      [&regs_from_last_step](libmicroemu::EmuContext &ectx) {
        const auto &reg_access = ectx.GetRegisterAccessor();
        const auto &spec_reg_access = ectx.GetSpecialRegisterAccessor();

        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegDiffs(sampled_regs, regs_from_last_step);

        // Update the last sampled registers
        regs_from_last_step = sampled_regs;
      };

  libmicroemu::FPreExecStepCallback pre_instr{nullptr};
  libmicroemu::FPostExecStepCallback post_instr{nullptr};

  if (is_trace == true) {
    pre_instr = pre_exec_instr_trace;

    if (is_trace_regs) {
      post_instr = post_exec_instr_trace_regs;
    } else if (is_trace_changed_regs) {
      post_instr = post_exec_instr_trace_changed_regs;
    } else {
      post_instr = nullptr;
    }
  }

  // Execute the arm code
  const auto exec_result = machine.Exec(instr_limit, pre_instr, post_instr);
  if (exec_result.IsErr()) {
    // Is Max instructions reached error?
    if (exec_result.IsMaxInstructionsReached()) {
      fmt::print(stdout, "INFO: Max instructions reached\n");
    } else {
      fmt::print(stderr, "ERROR: Emulator returned error: {}({})\n", exec_result.ToString(),
                 static_cast<uint32_t>(exec_result.GetStatusCode()));
    }
  }
  return exec_result.GetProgramExitCode();
}
