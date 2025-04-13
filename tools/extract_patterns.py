"""
Simple script which loads armv7-m.json and outputs a transformed format which
should be used by bit-gen project
"""

import json


def main():
    config = None
    with open("./decoder_generator/decoder_generator/armv7-m.json") as fp:
        config = json.load(fp)

    decoders = config["decoders"]

    names = []
    patterns = []
    for dec, i in decoders.items():
        if 'hi' not in i['pattern']:
            hi = "xxxxxxxxxxxxxxxx"
        else:
            hi = i['pattern']['hi']
        names.append(f"{dec}")
        patterns.append(f"{i['pattern']['lo']}{hi}")


    patterns, names = zip(*sorted(zip(patterns,names)))
    lines = []
    for name, pattern in zip(names,patterns):
        lines.append(f'{{"pattern": "{pattern}", "name": "{name}"}}')

    json_out="[\n"
    json_out+=",\n".join(lines)
    json_out+="\n]"
    # validate - throws exception if not valid
    json.loads(json_out)

    print(json_out)
if __name__ == '__main__':
    main()
