python -m decoder_generator
clang-format -i op_decoders.h
clang-format -i instr.h
git diff op_decoders.h
