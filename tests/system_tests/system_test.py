import argparse
import trace_diff
from pathlib import Path
import subprocess

COMAND_LINE_ARGS = ["-e", "-t", "--trace-changed-regs"]

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("emu_path", type=str)
    parser.add_argument("config", type=str)
    parser.add_argument("source_dir", type=str)
    parser.add_argument("output_dir", type=str)
    parser.add_argument("test_name", type=str)
    args = parser.parse_args()

    emu_path = args.emu_path
    config = args.config
    source_dir = args.source_dir
    output_dir = args.output_dir
    test_name = args.test_name

    print("Emu path:   ", emu_path)
    print("Config:     ", config)
    print("Source dir: ", source_dir)
    print("Output dir: ", output_dir)
    print("Test name:  ", test_name)

    # Create output directory if it does not exist
    Path(output_dir).mkdir(exist_ok=True)

    test_pos = (
        source_dir
        + "/"
        + test_name
        + "/"
        + "prebuilt"
        + "/"
        + "bin"
        + "/"
        + test_name
        + ".elf"
    )

    emu_output = subprocess.check_output(
        [emu_path] + COMAND_LINE_ARGS + config.split(" ") + [test_pos]
    )

    trace_input = source_dir + "/" + test_name + "/" + test_name + ".trace"
    trace_output = output_dir + "/" + test_name + ".trace"
    emu_output = emu_output.decode("utf-8")

    # in case of windows, replace \r with ""
    emu_output = emu_output.replace("\r", "")

    # write output to file
    with open(trace_output, "w", encoding="utf-8") as f:
        f.write(emu_output)

    # Diff the output
    exit_code = trace_diff.diff_files(trace_input, trace_output)
    exit(exit_code)
