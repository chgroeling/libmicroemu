from jinja2 import Environment, PackageLoader, select_autoescape
import json
import importlib.resources as rimp
import pprint

def generate_pre_block(pattern):
    def process_bit_string(bit_string, instr_name):
        pre_block = []
        length = len(bit_string)
        i = 0
        while i < length:
            if bit_string[i] in "01":  # Fixed bit detected
                # Check if this bit is isolated (enclosed by 'x') or at the start/end
                if (i == 0 or bit_string[i - 1] == "x") and (
                    i == length - 1 or bit_string[i + 1] == "x"
                ):
                    bit_pos = length - 1 - i
                    pre_block.append(
                        f"assert((Bm16::IsolateBit<{bit_pos}u>({instr_name})) == 0b{bit_string[i]}u);"
                    )
                    i += 1
                else:
                    # Group consecutive '0' or '1' bits for Slice1R
                    slice_start = i
                    while i < length and bit_string[i] in "01":
                        i += 1
                    slice_end = i - 1
                    fixed_bits = bit_string[slice_start:i]

                    # Calculate bit positions for Slice1R
                    bit_pos_start = length - 1 - slice_start
                    bit_pos_end = length - 1 - slice_end
                    pre_block.append(
                        f"assert((Bm16::Slice1R<{bit_pos_start}u, {bit_pos_end}u>({instr_name})) == 0b{fixed_bits}u);"
                    )
            elif bit_string[i] == "x":  # Skip 'x' (don't care) bits
                i += 1
        return pre_block

    # Process the "lo" and "hi" parts of the pattern
    pre_block = []
    if "lo" in pattern:
        pre_block += process_bit_string(pattern["lo"], "rinstr.low")
    if "hi" in pattern:
        pre_block += process_bit_string(pattern["hi"], "rinstr.high")

    return pre_block


def create(template_name, filepos, data):
    env = Environment(
        loader=PackageLoader("decoder_generator"),
    )
    template_ = env.get_template(template_name)
    out_template = template_.render({"data": data})

    # to save the results
    with open(filepos, "w") as fh:
        fh.write(out_template)


def camel(inp: str):
    out = inp.split("_")
    out = [i.capitalize() for i in out]
    return "".join(out)


def decgen():
    print("Decoder generator")
    config = None
    with rimp.open_text("decoder_generator", "armv7-m.json") as fp:
        config = json.load(fp)

    name_prefix_struct = "Instr"
    name_suffix_decoder = "Decoder"
    opcodes = config["opcodes"]

    splitters = config["splitters"]

    data = {"opcodes": {}}

    instructions = dict()
    for name, v in config["instructions"].items():
        item = dict()
        item["def"] = dict(v)
        cc_name = camel(name)
        item["name_struct"] = name_prefix_struct + cc_name
        item["name_enum"] = "k" + cc_name

        instructions[name] = item

    decoders = {}
    for name, v in config["decoders"].items():
        cc_encoding = camel(v["encoding"])
        instruction = instructions[v["instruction"]]
        cc_name = camel(name)

        item = dict(v)

        if "pattern" in item:
            pass
            # Experimental: Generate assertions for the pattern
            # print("\n".join(generate_pre_block(item["pattern"])))
            # print("---")

        item["name_struct"] = instruction["name_struct"]
        item["name_callback"] = cc_name + name_suffix_decoder
        item["name_enum"] = instruction["name_enum"]
        decoders[name] = item

    for i in range(0, 32):
        opcode_bin = bin(i).replace("0b", "").zfill(5)
        if opcode_bin in opcodes:
            opcode = opcodes[opcode_bin]
            cb = opcodes[opcode_bin]["decoder"]
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
            data["opcodes"][f"{i}"] = opcode
        else:
            data["opcodes"][f"{i}"] = {
                "bin": opcode_bin,
                "flags": [],
                "name_decoder": "InvalidInstrDecoder",
            }

    data["decoders"] = decoders
    data["splitters"] = splitters
    data["instructions"] = instructions
    create("op_decoders.j2", "op_decoders.h", data)
    create("instr.j2", "instr.h", data)
