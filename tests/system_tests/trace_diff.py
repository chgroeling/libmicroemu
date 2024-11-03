import argparse
from difflib import unified_diff, ndiff


SHOW_DIFFS = 2


def limited_unified_diff(a, b, max_headers=10):
    diff = unified_diff(a, b)
    result = []
    header_count = 0

    for line in diff:
        # Count headers
        if line.startswith("@@"):
            header_count += 1
        if header_count > max_headers:
            break
        result.append(line.rstrip())

    return result


def diff_files(file1, file2):
    print("Comparing files:")

    print(f" 0. {file1}")
    print(f" 1. {file2}")

    file1 = open(file1, "r", encoding="utf-8")
    file2 = open(file2, "r", encoding="utf-8")

    data1 = [i.rstrip() for i in file1.readlines()]
    data2 = [i.rstrip() for i in file2.readlines()]
    diff_report = limited_unified_diff(data1, data2, SHOW_DIFFS)
    if len(diff_report) > 0:
        print("Files are different")
        print(f"Diff report (max {SHOW_DIFFS} diffs are shown):")
        for line in diff_report:
            print(line)
        return 1

    print("Ok")
    return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", nargs=2, type=str)
    args = parser.parse_args()

    ret = diff_files(args.file[0], args.file[1])
    exit(ret)
