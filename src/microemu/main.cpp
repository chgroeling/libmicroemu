#include "libmicroemu/logger.h"
#include "libmicroemu/microemu.h"
#include "reg_printer.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <cxxopts.hpp>
#include <fmt/core.h>
#include <iostream>
#include <memory>
#include <set>
#include <spdlog/spdlog.h>
#include <stdarg.h>
#include <vector>

void LoggingCallback(microemu::LogLevel level, const char *format, ...) {
  // Initialize variadic argument list
  va_list args;
  va_start(args, format);

  // Estimate the required size of the formatted string
  const int size = std::vsnprintf(nullptr, 0u, format, args) + 1u; // +1 for null termination
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
  case microemu::LogLevel::kTrace:
    spdlog::trace(buf.data());
    break;
  case microemu::LogLevel::kDebug:
    spdlog::debug(buf.data());
    break;
  case microemu::LogLevel::kInfo:
    spdlog::info(buf.data());
    break;
  case microemu::LogLevel::kWarn:
    spdlog::warn(buf.data());
    break;
  case microemu::LogLevel::kError:
    spdlog::error(buf.data());
    break;
  case microemu::LogLevel::kCritical:
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
  cxxopts::Options options("microemu", "Armv7-m  emulator");
  // TODO: Fix weird error message when no positional argument is given
  options.positional_help("<elf_file>");
  // TODO: Add register dump per instruction trace option

  // clang-format off
  options.add_options()
    ("h,help", "Print usage information.")
    ("l,log", "Enable logging")
    ("log-level", "Set the log level (TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL)", 
             cxxopts::value<std::string>()->default_value("INFO"))
    ("log-file", "Specify log file path.", cxxopts::value<std::string>())
    ("t,trace", "Print instruction trace to stdout") 
    ("trace-regs", "Enable output of all register states during the trace.")
    ("trace-changed-regs", "Enable output of registers that have changed during the trace.")
    ("e,elf_ep", "Load and set entry point from ELF file.")
    ("i,instr_limit", "Set the maximum number of instructions to execute.", cxxopts::value<int>()) 
    ("p,profile", "Specify the emulation profile.", cxxopts::value<int>())
    ("elf_file", "Path to the executable to load.",cxxopts::value<std::string>())
  ;
  // clang-format on

  // parse positional arguments into vector filenames
  options.parse_positional({"elf_file"});

  cxxopts::ParseResult result;

  try {
    result = options.parse(argc, argv);
  } catch (const cxxopts::exceptions::exception &x) {
    std::cerr << "microemu: " << x.what() << '\n';
    std::cerr << "usage: microemu [options] <elf_file> \n";
    return EXIT_FAILURE;
  }

  // print out help if necessary
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    return EXIT_SUCCESS;
  }

  // Check if the elf_file argument is present
  if (!result.count("elf_file")) {
    std::cerr << "microemu: Missing required positional argument <elf_file>\n";
    std::cerr << "usage: microemu [options] <elf_file>\n";
    return EXIT_FAILURE;
  }

  const auto elf_file = result["elf_file"].as<std::string>();

  bool trace = result["trace"].as<bool>();
  bool trace_regs = result["trace-regs"].as<bool>();
  bool trace_changed_regs = result["trace-changed-regs"].as<bool>();

  if (trace_regs && trace_changed_regs) {
    std::cerr << "microemu: --trace-regs and --trace-changed-regs are mutually exclusive\n";
    return EXIT_FAILURE;
  }

  // CODE Segment
  std::vector<uint8_t> flash_seg;
  uint32_t flash_seg_size{0x0u};
  uint32_t flash_seg_vadr{0x0u};

  // RAM1 Segment
  std::vector<uint8_t> ram1_seg;
  uint32_t ram1_seg_size{0x0u};
  uint32_t ram1_seg_vadr{0x0u};

  // RAM2 Segment
  std::vector<uint8_t> ram2_seg;
  uint32_t ram2_seg_size{0x0u};
  uint32_t ram2_seg_vadr{0x0u};

  auto profile = 0;
  if (result.count("profile")) {
    profile = result["profile"].as<int>();
  }

  int32_t instr_limit = -1; // <0 means infinite

  if (result.count("instr_limit")) {
    instr_limit = result["instr_limit"].as<int>();
  }
  if (instr_limit < -1) {
    std::cerr << "microemu: instr_limit must be greater than or equal to -1\n";
    return EXIT_FAILURE;
  }
  // Check defined log levels
  std::set<std::string> valid_log_levels = {"TRACE",   "DEBUG", "INFO",
                                            "WARNING", "ERROR", "CRITICAL"};
  std::string log_level = result["log-level"].as<std::string>();

  if (valid_log_levels.find(log_level) == valid_log_levels.end()) {
    fmt::print(
        stderr,
        "Error: Invalid log level '{}'. Valid options are: TRACE, DEBUG, INFO, WARNING, ERROR, "
        "CRITICAL.\n",
        log_level);
    return 1; // Fehlercode zurückgeben
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

    microemu::MicroEmu::RegisterLoggerCallback(&LoggingCallback);
  }

  microemu::MicroEmu lib;

  // TODO: Profiles should be moved to json file
  switch (profile) {
  default: {
    flash_seg = std::vector<uint8_t>(0x10000u);
    flash_seg_size = static_cast<uint32_t>(flash_seg.size());
    flash_seg_vadr = 0x0;

    ram1_seg = std::vector<uint8_t>(0x20000u);
    ram1_seg_size = static_cast<uint32_t>(ram1_seg.size());
    ram1_seg_vadr = 0x10000u;

    ram2_seg = std::vector<uint8_t>(0x10000u);
    ram2_seg_size = static_cast<uint32_t>(ram2_seg.size());
    ram2_seg_vadr = 0x70000u;
    lib.SetFlashSegment(flash_seg.data(), flash_seg_size, flash_seg_vadr);
    lib.SetRam1Segment(ram1_seg.data(), ram1_seg_size, ram1_seg_vadr);
    lib.SetRam2Segment(ram2_seg.data(), ram2_seg_size, ram2_seg_vadr);
    break;
  }
  case 1u: {
    flash_seg = std::vector<uint8_t>(0x20000u);
    flash_seg_size = static_cast<uint32_t>(flash_seg.size());
    flash_seg_vadr = 0x0;

    ram1_seg = std::vector<uint8_t>(0x40000u);
    ram1_seg_size = static_cast<uint32_t>(ram1_seg.size());
    ram1_seg_vadr = 0x20000000u;

    lib.SetFlashSegment(flash_seg.data(), flash_seg_size, flash_seg_vadr);
    lib.SetRam1Segment(ram1_seg.data(), ram1_seg_size, ram1_seg_vadr);
    break;
  }
  }

  bool is_elf_entry_point = false;
  bool print_differences = true;
  bool first_trace = true;

  SampledRegs regs_from_last_step{};

  if (result.count("elf_ep")) {
    is_elf_entry_point = true;
  }

  auto res = lib.Load(elf_file.c_str(), is_elf_entry_point);
  if (res.IsErr()) {
    fmt::print(stderr, "ERROR: Emulator returned error: {}({})\n", res.ToString(),
               static_cast<uint32_t>(res.status_code));
    return EXIT_FAILURE;
  }

  microemu::FPreExecStepCallback pre_exec_instr_trace = [](microemu::EmuContext &ectx) {
    // TODO: Add heuristic: is_32bit_instr?
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

  microemu::FPostExecStepCallback post_exec_instr_trace_regs =
      [&regs_from_last_step](microemu::EmuContext &ectx) {
        // TODO: INCLUDE ABORT ON SPECIFIC ADDRESS COMMAND LINE OPTION
        // TODO: Count number of instructions executed and time taken
        const auto &pc = ectx.GetPc();
        const auto &reg_access = ectx.GetRegisterAccess();
        const auto &spec_reg_access = ectx.GetSpecialRegisterAccess();

        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegs(sampled_regs);
        regs_from_last_step = sampled_regs; // Update the last sampled registers
      };

  microemu::FPostExecStepCallback post_exec_instr_trace_changed_regs =
      [&regs_from_last_step](microemu::EmuContext &ectx) {
        // TODO: INCLUDE ABORT ON SPECIFIC ADDRESS COMMAND LINE OPTION
        // TODO: Count number of instructions executed and time taken
        const auto &pc = ectx.GetPc();
        const auto &reg_access = ectx.GetRegisterAccess();
        const auto &spec_reg_access = ectx.GetSpecialRegisterAccess();

        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegDiffs(sampled_regs, regs_from_last_step);
        regs_from_last_step = sampled_regs; // Update the last sampled registers
      };

  microemu::FStateCallback initial_state_cb =
      [&regs_from_last_step](microemu::IRegisterAccess &reg_access,
                             microemu::ISpecialRegAccess &spec_reg_access) {
        // Print the initial state
        fmt::print(stdout, "Initial register states:\n");
        auto sampled_regs = RegPrinter::SampleRegs(reg_access, spec_reg_access);
        RegPrinter::PrintRegs(sampled_regs);
        regs_from_last_step = sampled_regs;
      };

  microemu::FPreExecStepCallback pre_instr{nullptr};
  microemu::FPostExecStepCallback post_instr{nullptr};
  if (trace == true) {
    pre_instr = pre_exec_instr_trace;

    if (trace_regs) {
      post_instr = post_exec_instr_trace_regs;
    }

    if (trace_changed_regs) {
      post_instr = post_exec_instr_trace_changed_regs;
    }

    // Evaluate the initial state
    if (trace_regs || trace_changed_regs) {
      lib.EvaluateState(initial_state_cb);
    }
  }

  auto res_exec = lib.Exec(instr_limit, pre_instr, post_instr);

  if (res_exec.IsErr()) {
    // TODO: Move error handler to application
    fmt::print(stderr, "ERROR: Emulator returned error: {}({})\n", res_exec.ToString(),
               static_cast<uint32_t>(res_exec.status_code));
    return EXIT_FAILURE;
  }
  auto &emu_flags = res_exec.content.flags;
  // TODO: Getter for flags
  if ((emu_flags &
       static_cast<microemu::EmuFlagsSet>(microemu::EmuFlags::kEmuMaxInstructionsReached)) != 0) {
    fmt::print(stdout, "INFO: Max instructions reached\n");
    return 0;
  }
  if ((emu_flags & static_cast<microemu::EmuFlagsSet>(microemu::EmuFlags::kEmuTerminated)) != 0) {
    return res_exec.content.status_code;
  }
  return EXIT_FAILURE;
}
