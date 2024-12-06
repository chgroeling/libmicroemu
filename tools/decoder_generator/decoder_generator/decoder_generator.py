from jinja2 import Environment, PackageLoader, select_autoescape
import json
import importlib.resources as rimp
import pprint

NAME_PREFIX_STRUCT = "Instr"
NAME_SUFFIX_DECODER = "Decoder"


def create_file(template_name, path, template_env):
    env = Environment(
        loader=PackageLoader("decoder_generator"),
    )
    template_ = env.get_template(template_name)
    out_template = template_.render({"data": template_env})

    # to save the results
    with open(path, "w") as fh:
        fh.write(out_template)


def camel(inp: str):
    out = inp.split("_")
    out = [i.capitalize() for i in out]
    return "".join(out)


def unfold_bit_pattern(bit_pattern):
    unfolded_bit_patterns = []
    range_start = 0  # Start of the range
    range_end = 0  # End of the range

    for idx, bit in enumerate(bit_pattern):
        range_end = idx
        if bit == ".":
            # If the bit is a wildcard, we need to check if the range is
            # continuous
            if range_start == range_end:
                range_start = range_end + 1
                continue

            # If the range is not continuous, we need to add the range to the
            # list of ranges
            bit_range = (
                range_start,
                range_end - 1,
                bit_pattern[range_start:range_end],
            )
            unfolded_bit_patterns.append(bit_range)
            range_start = range_end + 1

    # If the range is not continuous, we need to add the range to the list of
    # ranges
    if range_start != range_end and range_start < len(bit_pattern):
        bit_range = (
            range_start,
            range_end,
            bit_pattern[range_start:range_end],
        )
        unfolded_bit_patterns.append(bit_range)
    return unfolded_bit_patterns


def unfold_patterns(pattern):
    ALLOWED_CHARS = {"0", "1", "."}
    unfold_bit_patterns = {}
    assert "lo" in pattern
    assert len(pattern["lo"]) == 16
    assert set(pattern["lo"]).issubset(ALLOWED_CHARS)
    unfold_bit_patterns["lo"] = unfold_bit_pattern(pattern["lo"])

    if "hi" in pattern:
        assert len(pattern["hi"]) == 16
        assert set(pattern["hi"]).issubset(ALLOWED_CHARS)
        unfold_bit_patterns["hi"] = unfold_bit_pattern(pattern["hi"])

    print(pattern)
    print(unfold_bit_patterns)
    print("---")
    return pattern


def decgen():
    print("Decoder generator")
    config = None
    with rimp.open_text("decoder_generator", "armv7-m.json") as fp:
        config = json.load(fp)

    template_env = {"opcodes": {}}

    # Splitters
    # ========================
    splitters = config["splitters"]

    # Instructions
    # ========================
    instructions = dict()
    for name, v in config["instructions"].items():
        item = dict()
        item["def"] = dict(v)
        cc_name = camel(name)
        item["name_struct"] = NAME_PREFIX_STRUCT + cc_name
        item["name_enum"] = "k" + cc_name

        instructions[name] = item

    # Decoders
    # ========================
    decoders = {}
    for name, v in config["decoders"].items():
        instruction = instructions[v["instruction"]]
        cc_name = camel(name)

        item = dict(v)

        if "pattern" in v:
            unfolded_pattern = unfold_patterns(v["pattern"])
        item["name_struct"] = instruction["name_struct"]
        item["name_callback"] = cc_name + NAME_SUFFIX_DECODER
        item["name_enum"] = instruction["name_enum"]
        decoders[name] = item

    # Opcodes
    # ========================
    json_opcodes = config["opcodes"]
    opcodes = {}
    for i in range(0, 32):
        opcode_bin = bin(i).replace("0b", "").zfill(5)
        if opcode_bin in json_opcodes:
            opcode = json_opcodes[opcode_bin]
            cb = json_opcodes[opcode_bin]["decoder"]
            if cb in decoders:
                opcode_decoder = decoders[cb]
                opcode["name_decoder"] = opcode_decoder["name_callback"]
                opcode["flags"] = opcode_decoder["flags"]
                opcode["bin"] = opcode_bin
            elif cb in splitters:
                opcode_splitter = splitters[cb]
                opcode["bin"] = opcode_bin
                opcode["flags"] = opcode_splitter["flags"]
                opcode["name_decoder"] = "Splitter" + cb
            else:
                raise Exception("Unknown callback")

            opcodes[f"{i}"] = opcode
        else:
            opcodes[f"{i}"] = {
                "bin": opcode_bin,
                "flags": [],
                "name_decoder": "InvalidInstrDecoder",
            }

    # Assign to template
    template_env["opcodes"] = opcodes
    template_env["decoders"] = decoders
    template_env["splitters"] = splitters
    template_env["instructions"] = instructions

    create_file("op_decoders.j2", "op_decoders.h", template_env)
    create_file("instr.j2", "instr.h", template_env)
